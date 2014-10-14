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
#ifndef ENGINEH
#define ENGINEH
#include <vector>
#include "zobrist.h"
#include "goban.h"
#include "amaf.h"
#include "tree.h"

#define PASS_THRESHOLD 0.99
#define RESIGN_THRESHOLD 0.10
#define DEF_PLAYOUTS 3000
#define DEF_TREESIZE 10000000
#define INFINITE -1u/2

class Engine{
private:
	Goban *main_goban;
	int max_playouts, nplayouts, tree_size, rand_movs, simul_len, discarded;
	Tree tree;
	AmafBoard amaf;
	mutable clock_t fin_clock, max_time;

	int get_best_move() const;
	int play_random_game();
	void back_up_amaf(int, Node*[], int, bool);
	void print_PV() const;

public:
	Engine(Goban*);
	void reset();
	void set_playouts(int);
	void set_times(int, int, int);
	void set_times(int, int);
	float score();
	void list_dead(std::vector<int>&);
	int generate_move();
	void perft(int max);
	void report_move(int move){ tree.promote(move);}
};
#endif
