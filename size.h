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
#ifndef SIZEH
#define SIZEH
#include <string>
#include <sstream>
//#define DEBUG_INFO

const int MAXSIZE = 19;
const int MAXSIZE2 = MAXSIZE*MAXSIZE;

const char COORDINATES[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T'};

const int handicap19[][9]= {
	{61, 301, 289, 73,  175, 187, 67, 295, 181},
	{61, 301, 289, 73,  175, 187, 67, 295, 0},
	{61, 301, 289, 73,  175, 187, 181, 0, 0},
	{61, 301, 289, 73,  175, 187, 0, 0, 0},
	{61, 301, 289, 73,  181, 0, 0, 0, 0},
	{61, 301, 289, 73,  0, 0, 0, 0, 0},
	{61, 301, 289, 0, 0, 0, 0, 0, 0},
	{61, 301, 0, 0, 0, 0, 0, 0, 0}
};

const int handicap13[][9]= {
	{43, 127, 121, 49,  82, 88, 46, 124, 85},
	{43, 127, 121, 49,  82, 88, 46, 124, 0},
	{43, 127, 121, 49,  82, 88, 85, 0, 0},
	{43, 127, 121, 49,  82, 88, 0, 0, 0},
	{43, 127, 121, 49,  85, 0, 0, 0, 0},
	{43, 127, 121, 49,  0, 0, 0, 0, 0},
	{43, 127, 121, 0, 0, 0, 0, 0, 0},
	{43, 127, 0, 0, 0, 0, 0, 0, 0}
};

const int handicap9[][9] = {
	{21, 61, 57, 25,  39, 43, 23, 59, 41},
	{21, 61, 57, 25,  39, 43, 23, 59, 0},
	{21, 61, 57, 25,  39, 43, 41, 0, 0},
	{21, 61, 57, 25,  39, 43, 0, 0, 0},
	{21, 61, 57, 25,  41, 0, 0, 0, 0},
	{21, 61, 57, 25,  0, 0, 0, 0, 0},
	{21, 61, 57, 0, 0, 0, 0, 0, 0},
	{21, 61, 0, 0, 0, 0, 0, 0, 0}
};

static void coord_to_char(int coord, std::string &response, int size){
	if(coord == 0){
		response.append("pass");
	}
	else{
		int y = (coord - 1)/size + 1;
		int x = (coord - 1) % size;
		std::string auxstring;
		std::stringstream auxstream;
		auxstream << COORDINATES[x];
		auxstream >> auxstring;
		response.append(auxstring);

		auxstream.clear();
		auxstream << y;
		auxstream >> auxstring;

		response.append(auxstring);
	}
}
#endif
