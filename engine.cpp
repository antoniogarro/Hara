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
//#define DEBUG_INFO
#include <iostream>
#include <ctime>
#include "engine.h"

#define DEF_PLAYOUTS 3000
#define DEF_TREESIZE 5000000

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
  this->byo_time = byo_time;
  if (stones) {
    max_time = (main_time/10 + byo_time/stones)*CLOCKS_PER_SEC;
  } else {
    max_time = (0.9*byo_time + main_time/60)*CLOCKS_PER_SEC;
  }
}

void Engine::set_times(int time_left, int stones)
{
  max_playouts = INFINITE;
  if (stones) max_time = (0.9*time_left/stones)*CLOCKS_PER_SEC;
  else max_time = (0.9*time_left + byo_time)*CLOCKS_PER_SEC;
  std::cerr << max_time << "\n";
}

int Engine::play_random_game(bool heavy)
{
  int pass = 0;
  main_goban->shuffle_empty();
  while (pass < 2) {
    int move = heavy ? main_goban->play_heavy() : main_goban->play_random();
    amaf.play(move, ++simul_len);
    rand_movs++;
    if (move == Goban::PASS) pass++;
    else pass = 0;
#ifdef DEBUG_INFO
      main_goban->print_goban();
#endif
    int mercy = main_goban->mercy();    
    if (mercy != -1) {
      return 1-mercy;
    }
//#ifdef DEBUG_INFO
    if (simul_len > 2*main_goban->get_size2()) {
      std::cerr << "WARNING: Simulation exceeded max length.\n";
      discarded++;
      return -1;
    }
//#endif
  }
    return (main_goban->chinese_count() > 0) ? 1:0;
}

int Engine::generate_move(bool early_pass)
{
  const int EXPAND = 8;
  const double RESIGN_THRESHOLD = 0.10, PASS_THRESHOLD = 0.90;
  
  bool side = main_goban->get_side();
  Node *root = tree.get_root();
  rand_movs = 0, discarded = 0;
  fin_clock = clock();

  while (root->get_visits() < max_playouts && clock() - fin_clock < max_time) {
    Node *node_history[3*MAXSIZE2];
    int nnode_hist = 0, pass = 0;
    simul_len = 0;
    amaf.set_up(main_goban->get_side(), main_goban->get_size());
    Node *node = root;
    while (node->has_childs() && pass < 2) {
      node_history[nnode_hist++] = node;
      node = node->select_child();
      int move = node->get_move();
      if(move == Goban::PASS) pass++;
      else pass = 0;
      main_goban->play_move(move);
      amaf.play(move, ++simul_len);
    }
    if (node->get_visits() >= EXPAND || node == root) {
      Prior priors[MAXSIZE2+1] = {0};
      int legal_moves[MAXSIZE2+1];
      int nlegal = main_goban->legal_moves(legal_moves);
      //main_goban->init_priors(priors);
      tree.expand(node, legal_moves, nlegal, priors); //TODO: break if expand fails.
    }
    node_history[nnode_hist++] = node;
    int result = play_random_game(HEAVY); //Black wins.
#ifdef DEBUG_INFO
      main_goban->print_goban();
      std::cerr << result << "\n";
#endif
    main_goban->restore();
    if (result == -1) continue;
    if (side) result = 1-result;    
    back_up_results(result, node_history, nnode_hist, side);
#ifdef DEBUG_INFO
      tree.print();
#endif
  }
  Node *best = tree.get_best();
  print_PV();
  if (best->get_move() == Goban::PASS) return Goban::PASS;
  if (best->get_value(1) < RESIGN_THRESHOLD) return -1;
  if (early_pass && best->get_value(1) >= PASS_THRESHOLD && !root->get_move()) return Goban::PASS;
  return best->get_move();
}

void Engine::back_up_results(int result, Node *node_history[], int nnodes, bool side)
{
  for (int i = 0; i < nnodes; i++) {
    node_history[i]->set_results(1-result); //Because root node belongs to opponent.
    node_history[i]->set_amaf(result, amaf, side, i+1);
    side = !side;
    result = 1-result;
  }
}

void Engine::print_PV() const
{
  fin_clock = clock() - fin_clock;
  tree.print();
  int nplayouts = tree.get_root()->get_visits();
  std::cerr << "#Playouts: " << nplayouts
            << ", average length: " << rand_movs/nplayouts
            << ", discarded: " << discarded << ", playouts/sec: "
            << (float)nplayouts/fin_clock*CLOCKS_PER_SEC << "\n";
}

float Engine::score(std::vector<int> *dead)
{
  const int PLAYOUTS = 5000;
  int score = 0;
  int score_table[MAXSIZE2+1] = {0};
  for (int i = 0; i < PLAYOUTS; i++) {
    simul_len = 0;
    play_random_game(LIGHT);
    main_goban->score_area(score_table);
    main_goban->restore();
  }
  for (int i = 1; i <= main_goban->get_size2(); i++) {
    if (score_table[i] > PLAYOUTS/2) score_table[i] = 1;
    else if (score_table[i] < -PLAYOUTS/2) score_table[i] = -1;
    else score_table[i] = 0;
    
    if (dead && main_goban->get_value(i)
        && score_table[i] !=  main_goban->get_value(i)) {
      dead->insert(dead->end(), i);
    }
  }

  for (int i = 1; i <= main_goban->get_size2(); i++) {
    score += score_table[i];
  }
  return score - main_goban->get_komi();
}

void Engine::perft(int max)
{
  for (int i = 0; i < max; i++) {
    simul_len = 0;
    play_random_game(LIGHT);
    std::cerr << "restoring\n";
    main_goban->restore();
  }
}
