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
#include "goban.h"

#ifdef ZOBRIST
unsigned long long Goban::get_zobrist() const
{
  return zobrist.get_key();
}

unsigned long long Goban::get_zobrist(int move)const
{
  unsigned long long zob_backup = get_zobrist();
  unsigned long long new_zobrist = set_zobrist(move);
  zobrist.set_key(zob_backup);
  return new_zobrist;
}

unsigned long long Goban::set_zobrist(int move) const
{
  if (move) {
    zobrist.update(move, side);
    
    GroupSet<4> neighbours;
    int nneigh = neighbour_groups(move, neighbours);
    
    for (int i = 0; i < nneigh; i++) {
      Group *current_neigh = neighbours[i];
      if (current_neigh->get_color() != side) {
        if (current_neigh->has_one_liberty()) {
          for (Group::StoneIterator st(current_neigh); st; ++st) {
            zobrist.update(*st, current_neigh->get_color());
          }
        }
      }
    }
  }
  zobrist.toggle_side();
  return get_zobrist();
}
#endif
int Goban::point_liberties(int point, PList &liberties) const
{  // liberties must be of size > 4.
  for (int i = 0; int adj=adjacent[point][i]; i++) {
    if (points[adj] == 0) {
      liberties.add(adj);
    }
  }
  return liberties.length();
}

int Goban::point_liberties(int point) const
{
  int nlibs = 0;
  for (int i = 0; adjacent[point][i]; i++) {
    if (points[adjacent[point][i]] == 0) nlibs++;
  }
  return nlibs;
}

int Goban::neighbour_groups(int point, GroupSet<4> &neighbours) const
{
  for (int i = 0; adjacent[point][i]; i++) {
    neighbours.add(points[adjacent[point][i]]);
  }
  return neighbours.length();  
}

int Goban::neighbour_groups(int point, bool color, int max_liberties,
                            GroupSet<MAXSIZE2/3> *neighbours) const
{
  int nneigh = 0;
  for (int i = 0; adjacent[point][i]; i++) {
    Group *current_group = points[adjacent[point][i]];
    if (current_group && current_group->get_color() == color
                     && current_group->get_nliberties() <= max_liberties) {
      if (neighbours) neighbours->add(current_group);
      nneigh++;
    }
  }
  return nneigh;  
}

int Goban::neighbour_groups(const Group *group, bool color, int max_liberties,
                            GroupSet<MAXSIZE2/3> &neighbours) const
{
  for (Group::StoneIterator st(group); st; ++st) {
    neighbour_groups(*st, color, max_liberties, &neighbours);
  }
  return neighbours.length();
}

int Goban::neighbours_size(int point, bool color) const
{
  int nstones = 0;
  GroupSet<4> neighbours;
  int nneigh = neighbour_groups(point, neighbours);
  for (int i = 0; i < nneigh; i++) {
    const Group *curr_neigh = neighbours[i];
    if (curr_neigh->get_color() == color) {
      nstones += curr_neigh->get_nstones();
    }
  }
  return nstones;
}

int Goban::neighbours_in_atari(int point, bool color, const GroupSet<4> &neighbours) const
{
  int natari = 0;
  for (int i = 0; i < neighbours.length(); i++) {
    if (neighbours[i]->get_color() !=color && neighbours[i]->has_one_liberty()) {
      natari++;
    }
  }
  return natari;
}

bool Goban::is_surrounded(int point, bool color, int consider_occupied) const
{
  if (points[point] != 0) return false;
  for (int i = 0; int adj=adjacent[point][i]; i++) {
    if (adj == consider_occupied) continue;
    if (points[adj] == 0 || points[adj]->get_color() != color) {
      return false;
    }
  }
  return true;
}

bool Goban::is_true_eye(int point, bool color, int consider_occupied) const
{
  int i, ncontrolled = 0;
  if (!is_surrounded(point, color, consider_occupied)) return false;
  for (i = 0; int diag=diagonals[point][i]; i++) {
    if (points[diag]) {
      if (points[diag]->get_color() == color) {
          ncontrolled++;
      }
    } else {
      if (is_surrounded(diag, color, consider_occupied)) {
        ncontrolled++;
      }
    }
  }
  if (i == 4) {
    if (ncontrolled > 2) return true;
  } else if (ncontrolled == i) {
    return true;
  }
  return false;
}
#define FALSE_EYES
#ifdef FALSE_EYES
bool Goban::is_virtual_eye(int point, bool color) const
{
  if (!is_surrounded(point, color)) return false;
  int nopponent = 0;
  for (int i = 0; i < 4; i++) {
    if (int diag = diagonals[point][i]) {
      if (points[diag] && points[diag]->get_color() != color) {
        nopponent++;                                          
      } 
    } else {
      nopponent++;
      break;
    }
  }
  return nopponent < 2;
}
#else
bool Goban::is_virtual_eye(int point, bool color) const
{
  if (!is_surrounded(point, color)) return false;
  for (int i = 0; int adj=adjacent[point][i]; i++) {
    if (points[adj]->has_one_liberty()) return false;
  }
  return true;   
}
#endif

bool Goban::is_legal(int point, bool color) const
{  //asumes an empty point in the range [1, size2]
  if (point == ko_point) return false;
#ifdef ZOBRIST
  if (zobrist.check_history(get_zobrist(point))) return false;
#endif
  if (point_liberties(point) > 0) return true;

  GroupSet<4> neighbours;
  int nneigh = neighbour_groups(point, neighbours);
  for (int i = 0; i < nneigh; i++) {
    if (neighbours[i]->get_color() == color && !neighbours[i]->has_one_liberty()) {
      return true;
    }
  }
  return neighbours_in_atari(point, color, neighbours) > 0;
}

int Goban::legal_moves(int moves[]) const
{
  int nlegal = 0;
  for (int i = 0; i < empty_points.length(); i++) {
    int point = empty_points[i];
    if (is_legal(point, side)) {
      moves[nlegal++] = point;
    }
  }
  moves[nlegal++] = PASS;
  return nlegal;
}

int Goban::get_value(int point) const   //Value 1 for Black, -1 for White, 0 for empty.
{
  if (points[point]) {
    return points[point]->get_color() ? -1:1;
  }
  return 0;
}

float Goban::chinese_count() const
{
  int black_score = 0, white_score = 0, eyes_result = 0;
  for (int i = 1; i <= size2; i++) {
    if (points[i]) {
      if (points[i]->get_color()) white_score++;
      else black_score++;
    } else {
      if (is_surrounded(i, BLACK)) eyes_result++;
      else if (is_surrounded(i, WHITE)) eyes_result--;
    }
  }
  return eyes_result + black_score - white_score - komi;
}

 void Goban::score_area(int point_list[]) const
{
  for (int i = 1; i <= size2; i++) {
    if (points[i]) {
      if (points[i]->get_color()) point_list[i]--;
      else point_list[i]++;
    } else {
      if (is_surrounded(i, WHITE)) point_list[i]--;
      else if (is_surrounded(i, BLACK)) point_list[i]++;
    }
  }
}

int Goban::mercy() const
{
  for (int s = 0; s < 2; s++) {
    if (stones_on_board[s] - stones_on_board[1-s] > size2/3) {
      return s; 
    }
  }
  return -1;
}

void Goban::print_goban() const
{
  std::cerr << "   ";
  for (int i = 0; i<size; i++) std::cerr << "--";
  std::cerr << "\n";
  for (int y = size - 1; y > -1; y--) {
    std::cerr << "  |";
    for (int x = 1; x < size+1; x++) {
      if (points[size*y + x]) {
        if (points[size*y + x]->get_color()) std::cerr << "#";
        else std::cerr << "o";
        if (size*y + x == last_point) std::cerr << "!";
        else std::cerr << " ";
      } else {
          std::cerr << ". ";
      }
    }
    std::cerr << "|" << y+1 << "\n";
  }
  std::cerr << "   ";
  for (int i = 0; i<size; i++) std::cerr << "--";
  std::cerr << "\n  ";
  for (int i = 0; i<size; i++) std::cerr << " " << COORDINATES[i];
  std::cerr << "\nMoves: " << game_history.length() << " Side: "
      << side << " Komi: " << komi << " empty: " << empty_points.length();
#ifdef ZOBRIST
  std::cerr << "\nZobrist: " << zobrist.get_key();
#endif
  int lat1 = (last_atari[BLACK] ? last_atari[BLACK]->get_stone(0): 0);
  int lat2 = (last_atari[WHITE] ? last_atari[WHITE]->get_stone(0) : 0);
  std::cerr << " last atari: black " << lat1<< " white " << lat2 << "\n";
}
