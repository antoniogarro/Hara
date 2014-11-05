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
#include "group.h"

Group::Group()
{
  color = 0;
  stones[0] = 0;
  liberties[0] = 0;
  nsts = 0;
  nlibs = 0;
}

void Group::set_up(int point, bool new_color, const PList &new_liberties)
{
  color = new_color;
  nsts = 0;
  stones[nsts++] = point;
  stones[nsts] = 0;
  nlibs = 0;
  for (int i = 0; i < new_liberties.length(); i++) {
    liberties[nlibs++] = new_liberties[i];
    liberties[nlibs] = 0;
  }
}

int Group::add_liberties(int i)
{
  for (int j = 0; j < nlibs; j++) {
    if (liberties[j] == i) return 0;
  }
  liberties[nlibs++] = i;
  liberties[nlibs] = 0;
  return nlibs;
}

int Group::erase_liberties(int lib)
{
  for (int j = 0; j < nlibs; j++) {
    if (liberties[j] == lib) {
      liberties[j] = liberties[--nlibs];
      liberties[nlibs]  = 0;
      return nlibs;
    }
  }
  return 0;
}

void Group::clear()
{
  nsts = 0;
  stones[nsts] = 0;
  nlibs = 0;
  liberties[nlibs] = 0;
}

void Group::attach_group(Group *attached)
{  
  for (int i = 0; i < attached->nsts; i++) {
    this->stones[nsts++] = attached->stones[i];
    this->stones[nsts] = 0;
  }
  for (int i = 0; i < attached->nlibs; i++) {
    this->add_liberties(attached->liberties[i]);
  }
}
void Group::print_group() const
{
  std::cerr << "Color: " << color;
  for (int i = 0; i < nsts; i++) std::cerr << " " << stones[i];
  std::cerr << "\n";
}
