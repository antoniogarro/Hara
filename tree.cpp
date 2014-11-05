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
#include "tree.h"
#include <cmath>

void Node::reset()
{
  child = 0;
  sibling = 0;
  visits = 0;
  results = 0;
  rave_visits = 0;
  rave_results = 0;
  move = 0;
}

void Node::copy_values(const Node *orig)
 {
  child = 0;
  sibling = 0;
  visits = orig->visits;
  results = orig->results;
  rave_visits = orig->rave_visits;
  rave_results = orig->rave_results;
  prior_visits = orig->prior_visits;
  prior_results = orig->prior_results;
  move = orig->move;
 }

void Node::add_child(Node *child)
{
  if (!this->child)  this->child = child;
  else {
    Node *next;
    for (next = this->child; next->sibling; next = next->sibling) {
    }
    next->sibling = child;
  }
}

void Node::set_move(int mv, const Prior &prior)
{
  move = mv;
  rave_results = prior.prior;
  rave_visits = prior.equiv;
  prior_results = prior.prior;
  prior_visits = prior.equiv;
}

void Node::set_results(int result)
{
  visits++;
  results += result;
}

void Node::set_amaf(int result, const AmafBoard &amaf, bool side, int depth)
{
  const double discount = 0.0; //0.0001;
  for (Node *next = child; next; next = next->sibling) {
    next->rave_results += result * amaf.value(next->move, depth, side, discount);
    next->rave_visits += amaf.value(next->move, depth, side, discount);
  }
}

double Node::get_value(double parent_visits) const
{
  const double BIAS = 1.0/3000, UCTK = 0;
  if (visits) {
    if (rave_visits) {
      double beta = rave_visits/(rave_visits + visits + rave_visits*visits*BIAS);
      return (1.0-beta)*results/visits + beta*rave_results/rave_visits;
    } else {
      return results/visits + UCTK*sqrt(log(parent_visits)/visits);
    }
  } else if (rave_visits) {
      return rave_results/rave_visits;
  }
  return 1.0;   //first player urgency.
}

Node *Node::select_child() const
{
  double best_value = -1;
  Node *RAVE_child = 0;
  for (Node *next = child; next; next = next->sibling) {
    if (next->get_value(visits) > best_value) {
      best_value = next->get_value(visits);
      RAVE_child = next;
    }
  }
  return RAVE_child;
}

Node *Node::get_best_child() const
{
  //return select_child();
  Node *best_child = 0;
  double best_visits = 0;
  for (Node *next = child; next; next = next->sibling) {
    double curr_visits = next->visits + next->rave_visits;
    if (curr_visits > best_visits) {
      best_visits = curr_visits;
      best_child= next;
    }
  }
  return best_child;
}

void Node::print(int boardsize) const
{
  std::string mv;
  coord_to_char(move, mv, boardsize);
    std::cerr.precision(4);
    std::cerr << std::right << std::setw(4) << mv
     << ": " << std::right << std::setw(6) << results/visits
     << "/" << std::left << std::setw(5) << visits
     << "| RAVE: " << std::right<< std::setw(6) << rave_results/rave_visits
     << "/" << std::left << std::setw(5) << rave_visits
     << " Prior: " << std::right << std::setw(6) << prior_results/prior_visits
     << "/" << std::left << std::setw(5) << prior_visits
     << "Value: " << std::setw(6) << get_value(1) << "\n";
}

Tree::Tree(int maxsize, Goban *goban)
{
  active = 0;
  this->maxsize = maxsize;
  this->goban = goban;
  root[0] = new Node[maxsize];
  root[1] = new Node[maxsize];
  root[0]->reset();
  root[1]->reset();
  size[0] = 1;
  size[1] = 1;
}

Tree::~Tree()
{
  delete[] root[0];
  delete[] root[1];
}

void Tree::clear()
{
  active = 0;
  root[0]->reset();
  root[1]->reset();
  size[0] = 1;
  size[1] = 1;
}

void Tree::clear_active()
{
  size[active] = 1;
  root[active]->reset();
}

Node *Tree::insert(Node *parent, int move, const Prior &prior)
{
  if (size[active] < maxsize) {
    Node *child = root[active] + size[active]++;
    child->reset();
    parent->add_child(child);
    child->set_move(move, prior);
    return child;
  } else {
    std::cerr << "WARNING: Out of memory!\n";
    return 0;
  }
}

Node *Tree::insert(Node *parent, const Node *orig)
{
  if (size[1-active] < maxsize) {
    Node *child = root[1-active] + size[1-active]++;
    child->copy_values(orig);
    parent->add_child(child);
    return child;
  } else {
    std::cerr << "WARNING: Out of memory!\n";
    return 0;
  }
}

void Tree::copy_recursive(Node *parent, const Node *orig)
{
  for (Node *node = orig->get_child(); node; node = node->get_sibling()) {
    Node *child = insert(parent, node);
    if (child) copy_recursive(child, node);
  }
}

int Tree::promote(int new_root)
{
  for (Node *n = root[active]->get_child(); n; n = n->get_sibling()) {
    if (n->get_move() == new_root) {
      root[1-active]->copy_values(n);
      copy_recursive(root[1-active], n);
      clear_active();
      active = 1-active;
      return active;
    }
  }
  clear();
  return active;
}

int Tree::expand(Node *parent, const int *moves, int nmovs, const Prior priors[])
{
  //if (parent->has_childs()) return 0; //No need to expand, but we check before calling.
  for (int i = 0; i < nmovs; i++) {
    if (insert(parent, moves[i], priors[moves[i]]) == 0) {  //Out of memory.
      return -1;
    }
  }
  return 0;
}

void Tree::print(Node *node, int threshold, int depth) const
{
  for (int i = 0; i < depth; i++) std::cerr << "  ";
  std::cerr << "->";
  node->print(goban->get_size());
  
  Node *childs[MAXSIZE2] = {0};
  int len = 0;
  for (Node *child = node->get_child(); child; child=child->get_sibling()) {
    if (child->get_visits() >= threshold) childs[len++] = child;
  }

  while (true) {
    int best = -1, visits = 0;
    for (int i = 0; i < len; i++) {
      if (childs[i] && childs[i]->get_visits() > visits) {
        best = i;
        visits = childs[best]->get_visits();
      }
    }
    if (best < 0) break;
    if (childs[best]->get_move()) print(childs[best], threshold, depth+1);
    childs[best] = 0;
  }
}

void Tree::print() const
{
  int threshold = root[active]->get_visits()/30;
  std::string sp = "  ";
  std::cerr << "Active tree: " << active << " " << get_size() <<" nodes.\n";
  print(root[active], threshold, 0);

  Node *best = get_best();
  std::cerr << "#Expected: " << best->get_value(1) << ", visits "
            << best->get_visits() << " PV:\n";
  for (Node *n = root[active]->get_best_child(); n; n = n->get_best_child()) {
      std::cerr << "*";
      n->print(goban->get_size());
  }
  std::cerr << "\n";
}
