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
#ifndef GTPH
#define GTPH

#include <string>
#include <vector>
#include <fstream>
#include <time.h>
#include "goban.h"
#include "engine.h"

#define PROTOCOLVERSION "2"
#define PROGVERSION "0.9"
#define PROGNAME "Hara"
//#define LOG

class GTP{

private:
  bool loop;
  int cmd;
  std::string cmd_id;
  std::string cmd_name;
  std::vector<std::string> cmd_args;
  std::vector<float> cmd_int_args;
  std::string command_string;
  std::string response;
#ifdef LOG
  ofstream engine_log;
#endif
  Goban main_goban;
  Engine go_engine;
  void protocol_version();
  void name();
  void version();
  void known_command();
  void list_commands();
  void quit();
  void boardsize();
  void clear_board();
  void komi();
  void play();
  void genmove();
  void unknown_command();
  void showboard();
  void fixed_handicap();
  void level();
  void time_settings();
  void kgs_time_settings();
  void time_left();
  void final_score();
  void final_status_list();
  
  
  enum {PROTOCOL_VERSION, NAME, VERSION, KNOWN_COMMAND, LIST_COMMANDS,
        QUIT, BOARDSIZE, CLEAR_BOARD, KOMI, PLAY, GENMOVE, SHOWBOARD,
        FIXED_HANDICAP, LEVEL,TIME_SETTINGS, TIME_LEFT, FINAL_SCORE,
        FINAL_STATUS_LIST, KGS_TIME_SETTINGS, NCOMMANDS};

  const std::string COMMANDS[NCOMMANDS]=
        {"protocol_version","name","version","known_command","list_commands",
         "quit","boardsize","clear_board","komi","play","genmove","showboard",
         "fixed_handicap","level","time_settings","time_left","final_score",
         "final_status_list","kgs-time_settings"};
      
  int parse(const std::string&);
  int string_to_cmd(const std::string&);
  int char_to_color(std::string&);
  int char_to_coordinate(std::string&);
  void print_coordinate(int);
  
public:
  GTP();
  int GTP_loop();
  int exec();
  void perft(int);
};

#endif
