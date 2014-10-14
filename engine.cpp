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
#include <iostream>
#include <ctime>
#include "engine.h"
Engine::Engine(Goban *goban):tree(DEF_TREESIZE, goban), amaf(goban->get_size())
{
	main_goban = goban;
	max_playouts = DEF_PLAYOUTS;
	max_time = 10*CLOCKS_PER_SEC;
	max_time = INFINITE;
	tree_size = DEF_TREESIZE;
	rand_movs = 0;
}

void Engine::reset()
{
	rand_movs = 0;
	tree.clear();
	amaf.set_up(main_goban->get_side(), main_goban->get_size());
}

void Engine::set_playouts(int playouts)
{
	max_playouts = playouts;
	max_time = INFINITE;
}

void Engine::set_times(int main_time, int byo_time, int stones)
{
	max_playouts = INFINITE;
	if(stones) max_time = (main_time/10 + byo_time/stones)*CLOCKS_PER_SEC;
	else max_time = main_time/60;
}

void Engine::set_times(int time_left, int stones)
{
	max_playouts = INFINITE;
	if(stones) max_time = (time_left/stones)*CLOCKS_PER_SEC;
	std::cerr << max_time << "\n";
}

int Engine::play_random_game()
{
	int pass = 0, move, m;
	//main_goban->shuffle_empty();
	while (pass < 2){
		//move = main_goban->play_random();
		move = main_goban->play_heavy();
		amaf.play(move, ++simul_len);
		int pass_now = !move;
		if(pass) pass = 2*pass_now;
		else pass = pass_now;
		#ifdef DEBUG_INFO
			main_goban->print_goban();
			getchar();
		#endif
		rand_movs++;
		if((m = main_goban->mercy()) != -1){
			return 1-m;
		}
		/*if(simul_len > 3*main_goban->get_size2()) {
			std::cerr << "WARNING: Simulation exceeded max length.\n";
			discarded++;
			return -1;
		}*/
	}
	return (main_goban->chinese_count() > 0)? 1:0;
}

float Engine::score()
{
	int score = 0;
	int score_table[MAXSIZE2+1] = {0};
	for(int i = 0; i < 10000; i++){
		play_random_game();
		main_goban->score_area(score_table);
		main_goban->restore();
	}
	for(int i = 1; i <= main_goban->get_size2(); i++){
		if(score_table[i] > 5000) score_table[i] = 1;
		else if(score_table[i] < -5000) score_table[i] = -1;
		else score_table[i] = 0;
	}
	for(int i = 1; i <= main_goban->get_size2(); i++){
		score += score_table[i];
	}
	return score - main_goban->get_komi();
}

void Engine::list_dead(std::vector<int> &dead)
{
	int score_table[MAXSIZE2+1] = {0};
	for(int i = 0; i < 10000; i++){
		play_random_game();
		main_goban->score_area(score_table);
		main_goban->restore();
	}
	for(int i = 1; i <= main_goban->get_size2(); i++){
		if(score_table[i] > 5000) score_table[i] = 1;
		else if(score_table[i] < -5000) score_table[i] = -1;
		else score_table[i] = 0;
	}
	for(int i = 1; i <= main_goban->get_size2(); i++){
		if(main_goban->get_color(i) && score_table[i] !=  main_goban->get_color(i)){
			dead.insert(dead.end(), i);
		}
	}
}

int Engine::generate_move()
{
	const int EXPAND = 2;
	int result = 0, move = 1, nlegal = 0, nnode_hist = 0;
	int legal_moves[MAXSIZE2+1];
	Node *node_history[3*MAXSIZE2];
	bool side = main_goban->get_side();

	nplayouts = 0, rand_movs = 0, discarded = 0;
	fin_clock = clock();

	while(nplayouts < max_playouts && clock() - fin_clock < max_time){
		nnode_hist = 0, simul_len = 0;
		amaf.set_up(main_goban->get_side(), main_goban->get_size());
		Node *node = tree.get_root();
		
		while(node->get_visits() > EXPAND){			
			if(!node->has_childs()){
				Prior priors[MAXSIZE2+1] = {0};
				nlegal = main_goban->legal_moves(legal_moves);
				main_goban->init_priors(priors);
				tree.expand(node, legal_moves, nlegal, priors);
			}
			if(node->has_childs()){
				node_history[nnode_hist++] = node;
				//node = node->select_UCT_child();
				node = node->select_RAVE_child();
				move = node->get_move();
				main_goban->play_move(move);
				amaf.play(move, ++simul_len);
			}
			else break;
		}
		node_history[nnode_hist++] = node;
		
		result = play_random_game();
		main_goban->restore();
		nplayouts++;
		if(result == -1) continue;
		if(side) result = 1 - result;
		
		back_up_amaf(result, node_history, nnode_hist, side);
	}
	Node *best = tree.get_best();
	if(best == 0) return 0;
	print_PV();
	
	if(best->get_value() < RESIGN_THRESHOLD) return -1;
	return best->get_move();
}

void Engine::back_up_amaf(int result, Node *node_history[], int nnodes, bool side)
{
	for(int i = 0; i < nnodes; i++){
		result = 1-result;
		side = !side;
		node_history[i]->set_results(result);
		node_history[i]->set_amaf(1-result, amaf, !side, i);
	}
}

void Engine::print_PV() const
{
	Node *node = tree.get_root();
	Node *best = tree.get_best();
	node = node->get_best_child();
	std::string move;
	fin_clock = clock() - fin_clock;

	std::cerr << "#Expected: " << best->get_value() << ", visits " << best->get_visits();
	std::cerr << " / PV: ";
	while(node && node->has_childs()){
		coord_to_char(node->get_move(), move, main_goban->get_size());
		std::cerr << move << ", ";
		if(node->get_move() == 0) break;
		node = node->get_best_child();
		move.clear();
	}
	std::cerr << tree.get_size() <<" nodes.\n";
	std::cerr << "#Playouts: "<< nplayouts <<  ", average length: " << rand_movs/nplayouts;
	std::cerr << ", discarded: " << discarded << ", playouts/sec: "<< (float)nplayouts/fin_clock*CLOCKS_PER_SEC <<"\n";
	tree.print(4);
}

void Engine::perft(int max)
{
	for (int i = 0; i < max; i++){
		simul_len = 0;
		play_random_game();
		std::cerr << "restoring\n";
		main_goban->restore();
	}
}
