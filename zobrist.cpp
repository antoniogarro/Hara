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
#include "zobrist.h"

Zobrist::Zobrist()
{
  zob_key = 0;
  zob_side = genrand64_int64();
  for(int i = 0; i < MAXSIZE2; i++){
    zob_points [0][i] = genrand64_int64();
    zob_points [1][i] = genrand64_int64();
    zob_ko[i] = genrand64_int64();
  }
  clear_history();
}

unsigned long long Zobrist::get_key() const
{
  return zob_key;  
}

void Zobrist::set_key(unsigned long long key)
{
  zob_key = key;
}

void Zobrist::reset()
{
  zob_key = 0;
  clear_history();
}

void Zobrist::toggle_side()
{
#ifdef SITUATIONAL_SUPERKO
  zob_key ^= zob_side;
#endif
}

void Zobrist::toggle_ko(int ko_point)
{
  //if(ko_point) zob_key ^= zob_ko[ko_point]; Of course not.
}

void Zobrist::update(int point, bool color)
{
  if(point) zob_key ^= zob_points[color][point-1];
}

void Zobrist::update(const int *moves, const int moves_len)
{
  bool side = 0;
  reset();
  for(int i = 0; i < moves_len; i++){
    if(moves[i]) zob_key ^= zob_points[side][moves[i]-1];
    side = !side;
  }
  if(side) zob_key ^= zob_side;
}

void Zobrist::record_key()
{
  zob_history[active] = zob_key;
  active = (active+1)%6;
}

void Zobrist::clear_history()
{
  for(int i = 0; i < 6; i++){
    zob_history[i] = 0;
  }
  active = 0;
}

bool Zobrist::check_history(unsigned long long key) const
{
  for(int i = 0; i < 6; i++){
    if(zob_history[i] == key) return true;
  }
  return false;
}
