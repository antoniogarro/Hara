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
#ifndef AMAFH
#define AMAFH
#include "size.h"
#include <iostream>
#include <iomanip>

class AmafBoard{
 private:
  int board[MAXSIZE2+1];
  int size;
  bool side;

 public:
  AmafBoard(int sz)
  {
    size = sz <= MAXSIZE ? sz : MAXSIZE; //Overkill.
    side = 0;
    clear();
  }
  
  void clear()
  {
    for (int i = 0; i <= size*size; i++) {
      board[i] = 0;
    }
  }

  void set_up(bool sd, int sz)
  {
    size = sz;
    side = sd;
    clear();
  }

  int play(int coord, int depth)  //only for coord <= size2
  {
    if (board[coord] == 0) {
      board[coord] = side ? -depth : depth;
      side = !side;
      return 0;
    } else {
      side = !side; //Careful!
      return -1;
    }
  }

  double value(int coord, int depth, bool side, double discount) const
  {
    if (coord == 0) return 0.0;
    int val = side ? -board[coord] : board[coord];
    if (val >= depth) {
      return 1.0 - discount*val;
    } else {
      return 0;
    }
  }

  void print() const{
    for (int y = size - 1; y > -1; y--) {
      std::cerr << "  |";
      for (int x = 1; x < size+1; x++) {
        
        if (board[size*y + x] < 0) {
          std::cerr << std::setw(3) << board[size*y + x] << "|";
        } else if(board[size*y + x] > 0) {
          std::cerr << std::setw(3) << board[size*y + x] << "|";
        } else {
          std::cerr << "   |";
        }
      }
      std::cerr << "|" << "\n";
    }
    std::cerr << "pass: " << board[0] << "\n";
  }
};
#endif
