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
Goban::Goban(int newsize)
{
  size = (newsize <= MAXSIZE) ? newsize : size;
  size2 = size*size;
  komi = 0.5;
  clear();
  init_adjacent(newsize);
  init_diagonals(newsize);
  init_vicinity(newsize);
  init_distance(newsize);
  init_manhattan(newsize);
}

void Goban::clear()
{
  reset();
  handicap = 0;
  game_history.clear();
}

void Goban::reset()
{
  side = BLACK;
  ko_point = 0;
#ifdef ZOBRIST
  zobrist.reset();
#endif
  stones_on_board[BLACK] = 0, stones_on_board[WHITE] = 0;
  last_atari[BLACK] = 0, last_atari[WHITE] = 0;
  last_point = 0, last_point2 = 0;
  
  for (int i = 0; i <= size2; i++) {
    if (Group *point = points[i]) {
      point->clear();
    }
    points[i] = 0;
  }
  empty_points.clear();
  for (int j = 0; j < size2; j++) {
    empty_points.add(j+1);
  }
}

void Goban::restore()
{
  reset();
  set_fixed_handicap(handicap);
  for (int i = 0; i < game_history.length(); i++) {
    if (game_history[i]) {
      drop_stone(game_history[i], side);
      side = !side;
    } else {
      side = !side;
    }
  }
}

void Goban::init_adjacent(int newsize)
{
  for (int k = 1; k <= size2; k++) {
    int nadj = 0;
    if (k <= size*(size-1)) {  //N
      adjacent[k][nadj++] =  k + size;
    }
    if (k % size) {  //E
      adjacent[k][nadj++] =  k + 1;
    }
    if (k > size) {  //S
      adjacent[k][nadj++] = k - size;
    }
    if (k % size != 1) {  //W
      adjacent[k][nadj++] = k -1;
    }
    adjacent[k][nadj] = 0;
  }
}

void Goban::init_diagonals(int newsize)
{

  for (int k = 1; k <= size2; k++) {
    int ndiag = 0;
    if (k <= size*(size-1) && k % size != 1) {  //NW
      diagonals[k][ndiag++] =  k + size - 1;
    }  
    if (k <= size*(size-1) && k % size) {  //NE
      diagonals[k][ndiag++] =  k + size + 1;
    }
    if (k > size && k % size) {  //SE
      diagonals[k][ndiag++] = k - size +1;
    }
    if (k > size && k % size != 1) {  //SW
      diagonals[k][ndiag++] = k - size -1;
    }
    diagonals[k][ndiag] = 0;
  }
}

//Precomputed 8 neighbours, to be used in pattern matching. Order matters.
void Goban::init_vicinity(int newsize)
{  
  for (int i = 0; i <= size2; i++) {
    for (int j = 0; j < 16; j++) {
      vicinity[i][j] = 0;
    }
  }
  //Duplicate to calculate rotations,avoiding run-time modulo division:
  for (int i = 0; i < 2; i++) {
    for (int k = 1; k <= size2; k++) {
      if (k <= size*(size-1)) {
        if (k % size != 1) {  //NW
          vicinity[k][0+8*i] =  k + size - 1;
        }
        vicinity[k][1+8*i] =  k + size;  //N
      }
      if (k % size ) {
        if (k <= size*(size-1)) {  //NE
          vicinity[k][2+8*i] =  k + size + 1;
        }
        vicinity[k][3+8*i] =  k + 1;  //E
      }
      if (k > size) {
        if (k % size) {  //SE
          vicinity[k][4+8*i] = k - size +1;
        }
        vicinity[k][5+8*i] = k - size;  //S
      }
      if (k % size != 1) {
        if (k > size) {  //SW
          vicinity[k][6+8*i] = k - size -1;
        }
        vicinity[k][7+8*i] = k - 1;  //W
      }
    }
  }
}

void Goban::init_distance(int newsize)
{
  for (int p = 0; p <= size2; p++) {
    int d = (p-1)%size < (p-1)/size ? (p-1)%size : (p-1)/size;
    if (size - (p-1)%size - 1 < d) {
      d = size - (p-1)%size - 1;
    }
    if (size - (p-1)/size - 1 < d) {
      d = size - (p-1)/size - 1;
    }
    distance_to_edge[p] = d;
  }
}

void Goban::init_manhattan(int newsize)
{
  for (int p = 1; p <= size2; p++) {
    for (int dis = 1; dis <= 4; dis++) {
      int len = 0;
      for (int d = 0; d <= dis; d++) {
        if ((p-1)%size >= d && size-(p-1)/size-1 >= (dis-d) ) {
          within_manhattan[p][dis-1][len++] = p - d + (dis-d)*size;
        } else {
          within_manhattan[p][dis-1][len++] = 0;
        }
        if (d != dis) {
          if ((p-1)%size >= d && (p-1)/size >= (dis-d)) {
            within_manhattan[p][dis-1][len++] = p - d - (dis-d)*size;
          } else {
            within_manhattan[p][dis-1][len++] = 0;
          }
        }
        if (d != 0) {
          if (size-(p-1)%size-1 >= d && size-(p-1)/size-1 >= (dis-d)) {
            within_manhattan[p][dis-1][len++] = p + d + (dis-d)*size;
          } else {
            within_manhattan[p][dis-1][len++] = 0;
          }
          if (d != dis) {
            if (size-(p-1)%size-1 >= d && (p-1)/size >= (dis-d)) {
              within_manhattan[p][dis-1][len++] = p + d - (dis-d)*size;
            } else {
              within_manhattan[p][dis-1][len++] = 0;
            }
          }
        }
      }
    }
  }
}

int Goban::set_fixed_handicap(int newhandicap)
{  //Expects newhandicap belonging to [2, 9].
  handicap = newhandicap;
  if (handicap) {
    switch(size) {
      case 19:
        return set_handicap(handicap19[9 - handicap]);
      case 13:
        return set_handicap(handicap13[9 - handicap]);
      case 9:
        return set_handicap(handicap9[9 - handicap]);
      default:
        return -1;
    }
  }
  return -1;
}

int Goban::set_handicap(const int handicap[])
{  
  for (int i = 0; i < 9 && handicap[i]; i++) {
    drop_stone(handicap[i], BLACK);
#ifdef ZOBRIST
    zobrist.update(handicap[i], 0);
#endif
    }
  side = 1;
#ifdef ZOBRIST
  zobrist.toggle_side();
#endif
  return 0;
}

int Goban::set_size(int newsize)
{
  size = (newsize <= MAXSIZE) ? newsize : size;
  size2 = size*size;
  init_adjacent(newsize);
  init_diagonals(newsize);
  init_vicinity(newsize);
  init_distance(newsize);
  init_manhattan(newsize);
  return size;
}

void Goban::remove_empty(int point)
{
  empty_points.remove(point);
}


int Goban::drop_stone(int point, bool color)
{
  if (points[point] != 0) print_goban();
  
  PointSet<5> liberties;
  point_liberties(point, liberties);
  
  groups[point].set_up(point, color, liberties);
  points[point] = &(groups[point]);
  remove_empty(point);
  stones_on_board[color]++;
#ifdef ZOBRIST
  zobrist.update(point, color);
  zobrist.toggle_ko(ko_point);
#endif
  ko_point = handle_neighbours(point);
#ifdef ZOBRIST
  zobrist.toggle_ko(ko_point);
#endif
  if (points[point]->get_nliberties() == 0) {    //suicide.
      erase_neighbour(points[point]);
  } else if (points[point]->has_one_liberty()) { //self-atari.
    last_atari[color] = points[point] ;
  }
  last_point2 = last_point;
  last_point = point;
  return point;
}

int Goban::handle_neighbours(int point)
{
  int captured_lone = 0, ncapt_lone = 0;
  GroupSet<4> neighbours;
  int nneigh = neighbour_groups(point, neighbours);
  
  for (int i = 0; i < nneigh; i++) {
    Group *current_neigh = neighbours[i];
    if (current_neigh->get_color() == points[point]->get_color()) {
      merge_neighbour(point, current_neigh);
    } else {
      if (current_neigh->has_one_liberty()) {
        if (current_neigh->get_nstones() == 1) {  
          ncapt_lone++;
          captured_lone = current_neigh->get_stone(0);
        }
        erase_neighbour(current_neigh);
      } else {
        current_neigh->erase_liberties(point);
        if (current_neigh->has_one_liberty()) {
          last_atari[current_neigh->get_color()] = current_neigh;
        }
      }
    }
  }
  if (ncapt_lone == 1 && points[point]->get_nstones()== 1) {
    return captured_lone;  //new ko point.
  } else {
    return 0;
  }
}

void Goban::merge_neighbour(int point, Group *neigh)
{
  Group *group = points[point];
  neigh->erase_liberties(point);
  group->attach_group(neigh);
  for (Group::StoneIterator st(neigh);  st; ++st) {
    points[*st] = group;
  }
  neigh->clear();
  if (neigh == last_atari[neigh->get_color()]) {
    last_atari[neigh->get_color()] = 0;
    //merged group may still be in atari, but this case is handled in 'drop_stone()'.
  }    
}

void Goban::erase_neighbour(Group *neigh)
{
  for (Group::StoneIterator st(neigh);  st; ++st) {
    points[*st] = 0;
    stones_on_board[neigh->get_color()]--;
#ifdef ZOBRIST
    zobrist.update(*st, neigh->get_color());
#endif    
    empty_points.add(*st);

    GroupSet<4> meta_neigh;
    int nmeta = neighbour_groups(*st, meta_neigh);
    for (int k = 0; k < nmeta; k++) {
      meta_neigh[k]->add_liberties(*st);
      if (meta_neigh[k] == last_atari[meta_neigh[k]->get_color()]) {
        last_atari[meta_neigh[k]->get_color()] = 0;
      }
    }
  }
  
  if (neigh == last_atari[neigh->get_color()]) {
    last_atari[neigh->get_color()] = 0;
  }
  neigh->clear();
}

int Goban::play_move(int point)  //this move isn't stored in game_history.
{
  if (point) {
    drop_stone(point, side);
  }
  side = !side;
#ifdef ZOBRIST
  zobrist.toggle_side();
  zobrist.record_key();
#endif
  return point;
}

int Goban::play_move(int point, bool color)
{
  if (side != color) {      //two consecutive moves of the same color are 
    game_history.add(PASS); //represented by a pass inbetween.
  }
#ifdef ZOBRIST
  else zobrist.toggle_side();
#endif
  if (point > 0) {
    if (!is_occupied(point) && is_legal(point, color)) {
      drop_stone(point, color);
    } else {
      return  -1;
    }
  } else {
    ko_point = 0;
  }
  side = !color;
  game_history.add(point);
#ifdef ZOBRIST
  zobrist.record_key();
#endif
  //print_goban();
  return point;
}

bool Goban::set_position(const PList &moves)
{
  for (int i = 0; i < moves.length(); i++) {
    if (moves[i]) {
      if (!points[moves[i]] && is_legal(moves[i], side)) {
        drop_stone(moves[i], side);
        game_history.add(moves[i]);
        side = !side;
      } else {
        return false;
      }
    } else {
      side = !side;
    }
  #ifdef ZOBRIST
    zobrist.record_key();
  #endif
  }
  return true;
}

bool Goban::set_position(const Goban *original)
{
  return set_position(original->game_history);
}
