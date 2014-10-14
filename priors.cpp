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


void Goban::init_priors(Prior priors[]) const
{
	const double EQUIV = size/2;
	priors[PASS].prior = 0.1*EQUIV, priors[PASS].equiv = EQUIV;
	
	for(int i = 0; i < empty_points.length(); ++i){		
		int p = empty_points[i];
		if(is_self_atari(p, side)){
			priors[p].prior = 0.2*EQUIV, priors[p].equiv = 2*EQUIV;
			continue;
		}	
		if(is_false_eye(p, side)){
			priors[p].prior = 0.3*EQUIV, priors[p].equiv = EQUIV;
			continue;
		}
		priors[p].prior = 0.5*EQUIV, priors[p].equiv = EQUIV;
		if(size > 11){
			if(distance_to_edge[p] == 0 && !stones_around(p, 4)){
				priors[p].prior = 0.1*EQUIV, priors[p].equiv = EQUIV;
			}
			else if(distance_to_edge[p] == 3 && !stones_around(p, 4)){
				priors[p].prior = 0.9*EQUIV, priors[p].equiv = EQUIV;
			}
		}
		
		GroupSet<4> neigh;
		int nneigh = neighbour_groups(p, neigh);
		for(int j = 0; j < nneigh; j++){
			if(neigh[j]->has_one_liberty()){
				if(neigh[j]->get_color() != side){
					priors[p].prior = 1.4*EQUIV, priors[p].equiv = 2*EQUIV;
					goto endloop;
				}
				else{
					priors[p].prior = 0.6*EQUIV, priors[p].equiv = EQUIV;
					goto endloop;
				}
			}
		}
		if(match_mogo_pattern(p, side)){
			priors[p].prior = 0.9*EQUIV, priors[p].equiv = EQUIV;
			continue;
		}
	endloop:;
	}
	
	if(last_point == 0) return;
	
	PointSet<MAXSIZE2> list;
	capture_heuristic(last_point, list);
	for(int i = 0; i < list.length(); i++){
		priors[list[i]].prior += 3*EQUIV, priors[list[i]].equiv += 3*EQUIV;
	}
	list.clear();

	save_heuristic(last_point, list);
	for(int i = 0; i < list.length(); i++){
		priors[list[i]].prior += 2*EQUIV, priors[list[i]].equiv += 2*EQUIV;
	}
	list.clear();

	pattern_heuristic(last_point, list);
	for(int i = 0; i < list.length(); i++){
		priors[list[i]].prior += 2*EQUIV, priors[list[i]].equiv += 2*EQUIV;
	}
	list.clear();

	
	for(int i = 0; last_point && i < 4; i++){
		for(int j = 0; j < 4*(i+1); j++){
			int v =  within_manhattan[last_point][i][j];
			if(v) {
				priors[v].prior += (1.0-0.1*i)*EQUIV, priors[v].equiv += EQUIV;
			}
		}
	}
	
}
