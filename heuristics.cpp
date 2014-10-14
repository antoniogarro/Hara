/***************************************************************************************
* Copyright (c) 2014, Antonio Garro.                                                   *
* All rights reserved                                                                  *
*                                                                                      *
* Redistribution and use in source and binary forms, with or without modification, are *
* permitted provided that the following conditions are met:                            *
*                                                                                      *
* 1. Redistributions of source code must retain the above copyright notice, this list  *
* of conditions and the following disclaimer.                                          *
*                                                                                      *
* 2. Redistributions in binary form must reproduce the above copyright notice, this    *
* list of conditions and the following disclaimer in the documentation and/or other    *
* materials provided with the distribution.                                            *
*                                                                                      *
* 3. Neither the name of the copyright holder nor the names of its contributors may be *
* used to endorse or promote products derived from this software without specific      *
* prior written permission.                                                            *
*                                                                                      * 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"          *
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE            *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE           *
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE            *
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL    *
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR           *
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER           *
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR     *
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF        *
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                    *
***************************************************************************************/
#include "goban.h"

int Goban::random_choose(const PList &list, bool(Goban::*Policy)(int, bool) const) const
{
	if(list.length() == 0) return 0;
	int first_choice = rand() % list.length();
	for (int i = first_choice; i < list.length(); i++){
		int point = list[i];
		if ((this->*Policy)(point, side)){
			return point;
		}
	}
	for (int i = 0; i < first_choice; i++){
		int point = list[i];
		if ((this->*Policy)(point, side)){
			return point;
		}
	}
	return 0;
}

bool Goban::random_policy(int point, bool side) const
{	
	if(is_eye(point, side)) return false;
	if(!is_legal(point, side)) return false;
	if(bad_self_atari(point, side)) return false;
	return true;
}

bool Goban::heavy_policy(int point, bool side) const
{	
	if(is_eye(point, side)) return false;
	if(!is_legal(point, side)) return false;
	if(is_self_atari(point, side)) return false;
	if(fast_ladder(point,side)) return false;
	return true;
}

int Goban::play_random()
{
	return play_move(random_choose(empty_points, &Goban::random_policy));
}

int Goban::play_heavy()
{
	if(int move = last_atari_heuristic()){
		return play_move(move);
	}

	if(last_point){
		PointSet<MAXSIZE2> list;
		save_heuristic(last_point, list);
		if(int move = random_choose(list, &Goban::heavy_policy)){
			return play_move(move);
		}
		list.clear();
	
		pattern_heuristic(last_point, list);
		if(int move = random_choose(list, &Goban::heavy_policy)){
			return play_move(move);
		}
		list.clear();
	
		capture_heuristic(last_point, list);
		if(int move = random_choose(list, &Goban::heavy_policy)){
			return play_move(move);
		}
		list.clear();
	}
	return play_random();
}

int Goban::last_atari_heuristic() const
{
	if(last_atari[!side]){
		int move = last_atari[!side]->get_liberty(0);
		if(is_legal(move, side) && !is_self_atari(move, side)){
			return move;
		}
	}
	
	PointList<MAXSIZE2> save;
	if(last_atari[side]){
		atari_escapes(last_atari[side], save);
		if(int move = random_choose(save, &Goban::heavy_policy)){
			return move;
		}
	}
	return 0;
}

void Goban::nakade_heuristic(int point, PList &list) const
{
	for(int i = 0; i < 8; i++){
		if(int v = vicinity[point][i]){
			if(points[v] == 0 && (creates_eyes(v, 0) > 1 || creates_eyes(v, 1) > 1)){
				list.add(v);
			}
		}
	}
}

void Goban::capture_heuristic(int point, PList &list) const
{
	for(int i = 0; i < 8; i++){
		if(int v = vicinity[point][i]){
			if(points[v] != 0 && points[v]->get_color() != side && points[v]->get_nliberties() == 1){
				atari_escapes(points[v], list);
			}
		}
	}
}

void Goban::save_heuristic(int point, PList &list) const
{
	if(points[point] && points[point]->get_nliberties() == 2){
		for(int i = 0; i < 2; i++){
			int lib = points[point]->get_liberty(i);
			if(gains_liberties(lib, points[point])){
				list.add(lib);
			}
		}
	}
	GroupSet<4> neigh;
	int nneigh = neighbour_groups(point, neigh);
	for(int j = 0; j < nneigh; j++){
		if(neigh[j]->get_color() == side && neigh[j]->get_nliberties() == 2){ // <2?
			atari_escapes(neigh[j], list);
		}
	}
/*TODO!!!
	for(int i = 0; i < 8; i++){
		if(int v = vicinity[point][i]){
			if(points[v] != 0 && points[v]->get_color() == side && points[v]->get_nliberties() == 1){
				atari_escapes(points[v], list);
			}
		}
	}
*/
}

void Goban::pattern_heuristic(int point, PList &list) const
{
	for(int i = 0; i < 8; i++){
		if(int v = vicinity[point][i]){
			if(points[v] == 0 && match_mogo_pattern(v, side)){
				list.add(v);
			}
		}
	}
}

bool Goban::stones_around(int point, int distance) const
{
	for(int i = 0; i < distance; i++){
		for(int j = 0; j < 4*(i+1); j++){
			int v =  within_manhattan[point][i][j];
			if(is_occupied(v)) return true;
		}
	}
	return false;
}

int Goban::total_liberties(int point, bool color, PList &liberties, int enough = 0, const Group *exclude=0) const
{
	PointSet<MAXSIZE2> libs;
	point_liberties(point, liberties);
	point_liberties(point, libs);
	if(enough && libs.length() > enough) return libs.length();
	
	GroupSet<4> neighbours;
	int nneigh = neighbour_groups(point, neighbours);
	for(int i = 0; i < nneigh; i++){
		const Group *curr_neigh = neighbours[i];
		if(curr_neigh != exclude){
			if(curr_neigh->get_color() == color){
				for(Group::LibertyIterator lib(curr_neigh); lib; ++lib){
					if(*lib != point) {
						liberties.add(*lib);
						libs.add(*lib);
						if(enough && libs.length() > enough) return libs.length();
					}
				}
			}
			else if(curr_neigh->get_nliberties() == 1){
				for(Group::StoneIterator st(curr_neigh); st; ++st){
					for(int j = 0; adjacent[*st][j]; j++){
						if(adjacent[*st][j] == point){
							libs.add(*st);
							if(enough && libs.length() > enough) return libs.length();
						}
						else if(points[adjacent[*st][j]] && points[adjacent[*st][j]]->get_color() == color){
							for(int k = 0; k < nneigh; k++){
								if(points[adjacent[*st][j]] == neighbours[k]){
									libs.add(*st);
									if(enough && libs.length() > enough) return libs.length();
								}
							}
						}
					}
				}
			}
		}
	}
	return libs.length();
}

bool Goban::gains_liberties(int point, const Group *group) const
{
	int curr_liberties = 1;
	if(group) curr_liberties = group->get_nliberties();
	
	PointSet<MAXSIZE2> liberties;
	int nlibs = total_liberties(point, group->get_color(), liberties,curr_liberties, group);
	return nlibs > curr_liberties;
}

bool Goban::is_self_atari(int point, bool color) const
{
	PointSet<MAXSIZE2> liberties;
	return (total_liberties(point, color, liberties, 1) == 1);
}

int Goban::atari_last_liberty(int point, bool color) const
{
	PointSet<MAXSIZE2> liberties;
	if(total_liberties(point, color, liberties, 1) == 1) return liberties[0]; //Maybe 0!
	return -1;
}

int Goban::atari_escapes(const Group *group, PList &escapes) const
{
	for(Group::LibertyIterator lib(group); lib; ++lib){
		if(gains_liberties(*lib, group)){
			escapes.add(*lib);
		}
	}
	GroupSet<MAXSIZE2/3> neighbours;
	int nneigh = neighbour_groups(group, !group->get_color(), group->get_nliberties(), neighbours);
	for(int i = 0; i < nneigh; i++){
		for(Group::LibertyIterator lib(neighbours[i]); lib; ++lib){
			if(gains_liberties(*lib, group)){
				escapes.add(*lib);
			}
		}
	}
	return escapes.length();
}

bool Goban::fast_ladder(int point, bool color) const
{
	PointSet<MAXSIZE2> tliberties;
	if(total_liberties(point, color, tliberties) != 2) return false;
	if(neighbour_groups(point, !color, 2, 0)) return false;
	PointList<5> liberties;
	point_liberties(point, liberties);
	for(int i = 0; i < liberties.length(); i++){
		PointList<5> secondary_libs;
		int delta[2] = {0};
		if(point_liberties(liberties[i], secondary_libs) == 4) continue;
	
		delta[0] = liberties[i] - point;
		for(int j = 0; j < secondary_libs.length(); j++){
			if(secondary_libs[j] != point && secondary_libs[j] != liberties[i] + delta[0]){
				delta[1] = secondary_libs[j] - liberties[i];
				break;
			}
		}
		if(delta[1] == 0) return true;
		int p = point, act = 0;
		while(distance_to_edge[p] > 1){
			p = p + delta[act];
			
			//look for ladder breakers. TODO: complete.
			if(points[p]){
				if(points[p]->get_color() == color) break;
				else return true;
			}
			if(points[p + delta[act]]){
				if(points[p + delta[act]]->get_color() == color) break;
				else return true;
			}
			act = 1-act;
		}
		if(distance_to_edge[p] < 2) return true;
	}
	return false;
}

int Goban::creates_eyes(int point, bool color) const
{
	int neyes = 0;
	for(int i = 0; adjacent[point][i]; i++){
		if(is_eye(adjacent[point][i], color, point)){
			neyes++;
		}
	}
#ifdef DEBUG_INFO
	if(neyes){
		std::cerr << point << " creates " << neyes << " eyes\n";
	}
#endif
	return neyes;
}

bool Goban::bad_self_atari(int point, bool color) const
{
	int last_lib = atari_last_liberty(point, color);
	if(last_lib == -1) return false;
#ifdef DEBUG_INFO
	std::cerr << "self_atari at " << point << "\n";
#endif

	//Only handle throw-ins up to 2 stones:?
	//if(neighbours_size(point, color) < 2){
		PointSet<MAXSIZE2> liberties;
		if(total_liberties(last_lib, !color, liberties) < 2){
	#ifdef DEBUG_INFO
			std::cerr << "snapback\n";
	#endif	
			return false;
		}
		if(creates_eyes(point, !color) && !creates_eyes(last_lib, !color)){
		#ifdef DEBUG_INFO
				std::cerr << "throw-in\n";
		#endif
			return false;
		}
	//}
	if(nakade_shape(point, color)){
	#ifdef DEBUG_INFO
		std::cerr << "nakade\n";
	#endif
		return false;
	}
	return true;
}

int Goban::bulkiness(const Group *group, int point) const
{
	int max_bulkiness = 0;
	for(Group::StoneIterator st(group); st; ++st){
		int bulk = 0;
		for(int i = 0; adjacent[*st][i]; i++){
			if(points[adjacent[*st][i]] == group || adjacent[*st][i] == point){
				bulk++;
			}
		}
		if(bulk > max_bulkiness) max_bulkiness = bulk;
	}
	return max_bulkiness;
}

int Goban::neighbour_bulkiness(int point, bool color) const
{
	GroupSet<4> neighbours;
	int nneigh = neighbour_groups(point, neighbours);
	int max_bulkiness = nneigh;
	for(int i = 0; i < nneigh; i++){
		Group *neigh = neighbours[i];
		if(neigh->get_color() == color){
			int bulk = bulkiness(neigh, point);
			if(bulk > max_bulkiness) max_bulkiness = bulk;
		}
	}
	return max_bulkiness;
}

bool Goban::nakade_shape(int point, bool color) const
{
	int bulk = neighbour_bulkiness(point, color);
			
	switch(neighbours_size(point, color)){
		case 2:
			return true;
			break;
		case 3:
			if(bulk == 3) return true;
			break;
		case 4:
			if(bulk == 4 || bulk == 3) return true;		//TODO.	
			break;
		case 5:
			if(bulk == 4) return true;
			break;
		default:
			return false;
	}
	return false;
}

//Hard-coded MoGo patterns, a bit nasty:
bool Goban::match_mogo_pattern(int point, bool side) const
{		
	if(point == 1 || point == size || point == size2 || point == size*(size-1)+1) return false;	//filter corners.
	
	if(point > size && point % size && point <= size*(size-1) && point % size != 1){
		for(int i = 1; i < 8; i+=2){
			const int *vic = vicinity[point];
			if(points[vic[i]]){
				bool adj_color = points[vic[i]]->get_color();
				if(points[vic[i-1]] && points[vic[i-1]]->get_color() != adj_color){
					if(points[vic[i+2]] == 0 && points[vic[i+6]] == 0){
						if(points[vic[i+1]] && points[vic[i+1]]->get_color() != adj_color){
							return true;	//Hane1
						}
						if(points[vic[i+1]] == 0 && points[vic[i+4]] == 0){
							return true;	//Hane2
						}
					}
					if(points[vic[i+2]] == 0 && points[vic[i+4]] == 0
						&& points[vic[i+6]] && points[vic[i+6]]->get_color() != adj_color){
							return true;	//Hane3
					}
					if(points[vic[i+6]] && points[vic[i+6]]->get_color() == adj_color){
						if((points[vic[i+2]] || points[vic[i+4]] == 0 || points[vic[i+4]]->get_color() == adj_color) 
							&& (points[vic[i+4]] || points[vic[i+2]] == 0 || points[vic[i+2]]->get_color() == adj_color)){
							return true;	//Cut1
						}
					}
				}
				if(points[vic[i+1]] && points[vic[i+1]]->get_color() != adj_color){
					if(points[vic[i+2]] == 0 && points[vic[i+6]] == 0){
						if(points[vic[i-1]] == 0 && points[vic[i+4]] == 0){
							return true;	//Mirror Hane2
						}
					}
					if(points[vic[i+4]] == 0 && points[vic[i+6]] == 0
						&& points[vic[i+2]] && points[vic[i+2]]->get_color() != adj_color){
							return true;	//Mirror Hane3
					}
				}
				if(points[vic[i+2]] && points[vic[i+6]] && points[vic[i+2]]->get_color() != adj_color
					&& points[vic[i+6]]->get_color() != adj_color
					&&(points[vic[i+4]] == 0 || points[vic[i+4]]->get_color() == adj_color)
					&&(points[vic[i+3]] == 0 || points[vic[i+3]]->get_color() == adj_color)
					&&(points[vic[i+5]] == 0 || points[vic[i+5]]->get_color() == adj_color)){
					return true;	//Cut2
				}
				if(adj_color != side && points[vic[i-1]] && points[vic[i-1]]->get_color() == side
					&& points[vic[i+2]] == 0 && points[vic[i+4]] == 0
					&& points[vic[i+6]] == 0 && points[vic[i+1]] && points[vic[i+1]]->get_color() == adj_color){
						return true;	//Hane4
				}
				if(adj_color != side && points[vic[i+1]] && points[vic[i+1]]->get_color() == side
					&& points[vic[i+2]] == 0 && points[vic[i+4]] == 0
					&& points[vic[i+6]] == 0 && points[vic[i-1]] && points[vic[i-1]]->get_color() == adj_color){
					return true;	//Mirror Hane4
				}
			}
		}
	}
	else{
		for(int i = 1; i < 8; i+=2){
			const int *vic = vicinity[point];
			if(vic[i]){
				if(points[vic[i]]){
					if(vic[i+2] && points[vic[i+2]] && points[vic[i+2]]->get_color() != points[vic[i]]->get_color()
						&& (vic[i+6]==0 || points[vic[i+6]] == 0 || points[vic[i+6]]->get_color() != points[vic[i]]->get_color())){
						return true;	//Side2
					}
					if(vic[i+6] && points[vic[i+6]] && points[vic[i+6]]->get_color() != points[vic[i]]->get_color()
						&& (vic[i+2]== 0 || points[vic[i+2]] == 0 || points[vic[i+2]]->get_color() != points[vic[i]]->get_color())){
						return true;	//Mirror Side2
					}
					if(points[vic[i]]->get_color() == side){
						if((vic[i+1] && points[vic[i+1]] && points[vic[i+1]]->get_color() != side)){
							return true;	//Side3
						}
						if((vic[i-1] && points[vic[i-1]] && points[vic[i-1]]->get_color() != side)){
							return true;	//Mirror Side3
						}
					}
					if(points[vic[i]]->get_color() != side){
						if(vic[i+1] && points[vic[i+1]] && points[vic[i+1]]->get_color() == side){
							if(vic[i+2] == 0 || points[vic[i+2]] == 0 || points[vic[i+2]]->get_color() == side){
								return true;	//Side4
							}
							if(vic[i+2] && points[vic[i+2]] && points[vic[i+2]]->get_color() != side
								&& vic[i+6] && points[vic[i+6]] && points[vic[i+6]]->get_color() == side){
								return true; //Side5
							}
						}
						if(vic[i-1] && points[vic[i-1]] && points[vic[i-1]]->get_color() == side){
							if(vic[i+6] == 0 || points[vic[i+6]] == 0 || points[vic[i+6]]->get_color() == side){
								return true;	//Mirror Side4
							}
							if(vic[i+6] && points[vic[i+6]] && points[vic[i+6]]->get_color() != side
								&& vic[i+2] && points[vic[i+2]] && points[vic[i+2]]->get_color() == side){
								return true; //Mirror Side5
							}
						}
					}
				}
				else if ((vic[i+6] && points[vic[i+6]] && vic[i-1] && points[vic[i-1]]
						&& points[vic[i+6]]->get_color() != points[vic[i-1]]->get_color())
					|| (vic[i+2] && points[vic[i+2]] && vic[i+1] && points[vic[i+1]]
						&& points[vic[i+2]]->get_color() != points[vic[i+1]]->get_color())){
					return true;	//Side1 and mirror.
				}
			}
		}
	}
	return false;
}
