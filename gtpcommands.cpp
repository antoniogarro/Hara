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
#include "gtp.h"

void GTP::protocol_version()
{
  response.append(PROTOCOLVERSION);
}

void GTP::name()
{
  response.append(PROGNAME);
}

void GTP::version()
{
  response.append(PROGVERSION);
}

void GTP::known_command()
{
  for(int i = 0; i < NCOMMANDS; i++){
    if(!cmd_args[0].compare(COMMANDS[i])){
      response.append("true");
    }
  }
  response.append("false");
}

void GTP::list_commands()
{
  for (int i = 0; i < NCOMMANDS; i++){
    response.append(COMMANDS[i]);
    response.append("\n");
  }
}

void GTP::quit()
{
  loop = false;
}

void GTP::boardsize()
{
  if(cmd_args.size() > 0){
    if(cmd_int_args[0] != main_goban.set_size(cmd_int_args[0])){
      response[0] = '?';
      response.append("unacceptable size");
    } else {
      go_engine.reset();
    }
  } else {
    response[0] = '?';
    response.append("syntax error");
  }
}

void GTP::clear_board()
{
  main_goban.clear();
  go_engine.reset();
}

void GTP::komi()
{
  if(cmd_args.size() > 0){
    main_goban.set_komi(cmd_int_args[0]);
  } else {
    response[0] = '?';
    response.append("syntax error");
  }
}

void GTP::play()
{
  int color, coord;
  if(cmd_args.size() > 1){
    color = char_to_color(cmd_args[0]);
    coord = char_to_coordinate(cmd_args[1]);
    
    if(color > -1 && coord > -1){
      //Check legality.
      if(main_goban.play_move(coord, color) == -1){
        response[0] = '?';
        response.append("illegal move");
      } else {
        go_engine.report_move(coord);
      }
    } else {
      response[0] = '?';
      response.append("invalid color or coordinate");
    }
  } else {
    response[0] = '?';
    response.append("syntax error");
  }
}

void GTP::kgs_genmove_cleanup()
{
  early_pass = false;
  genmove();
  early_pass = true;
}

void GTP::genmove()
{
  if(cmd_args.size() > 0){
    bool color = char_to_color(cmd_args[0]);
    if(color != main_goban.get_side()){
      main_goban.play_move(0, !color);
      go_engine.report_move(0);
    }
    int move = go_engine.generate_move(early_pass);
    main_goban.play_move(move, color);
    go_engine.report_move(move);
    print_coordinate(move);
  } else {
    response[0] = '?';
    response.append("syntax error");
  }
    
}


void GTP::unknown_command()
{
  response[0] = '?';
  response.append("unknown_command");
}

void GTP::showboard()
{
  main_goban.print_goban();
}

void GTP::fixed_handicap()
{
  
  if(cmd_int_args.size() > 0 && cmd_int_args[0] > 1 && cmd_int_args[0] < 10){
    if(main_goban.set_fixed_handicap(cmd_int_args[0]) != cmd_int_args[0]){
    }
  } else {
    response[0] = '?';
    response.append("syntax error");
  }
}

void GTP::level()
{
  if(cmd_int_args.size() > 0 && cmd_int_args[0] > 0){
    go_engine.set_playouts(10000*cmd_int_args[0]);
  } else {
    response[0] = '?';
    response.append("syntax error");
  }
}

void GTP::time_settings()
{
  if(cmd_int_args.size() > 2){
    go_engine.set_times(cmd_int_args[0], cmd_int_args[1], cmd_int_args[2]);
  } else {
    response[0] = '?';
    response.append("syntax error");
  }
}

void GTP::kgs_time_settings()
{
  if (cmd_int_args.size() > 3
      && (!cmd_args[0].compare("byoyomi")
          || !cmd_args[0].compare("canadian"))) {
    go_engine.set_times(cmd_int_args[1], cmd_int_args[2], cmd_int_args[3]);
  } else if (cmd_int_args.size() > 1 && !cmd_args[0].compare("absolute")) {
    go_engine.set_times(cmd_int_args[1], 0, 0);
  } else if (cmd_int_args.size() > 0 && !cmd_args[0].compare("none")) {
    go_engine.set_times(30, 0, 0); //To be adjusted.
  } else {
    response[0] = '?';
    response.append("syntax error");
  }
}

void GTP::time_left()
{
  if (cmd_args.size() > 2 && cmd_int_args.size() > 1) {
    go_engine.set_times(cmd_int_args[1], cmd_int_args[2]);
  } else {
    response[0] = '?';
    response.append("syntax error");
  }
}

void GTP::final_score()
{
  std::stringstream auxstream;
  float score = go_engine.score(0);
  
  if (score > 0) {
    auxstream << score;
    response.append("B+");
    response.append(auxstream.str());
  } else {
    auxstream << -score;
    response.append("W+");
    response.append(auxstream.str());
  }

}

void GTP::final_status_list()
{
  if (cmd_args.size() > 0 && !cmd_args[0].compare("dead")) {
    std::vector<int> list;
    go_engine.score(&list);
    //TODO: support 'alive' status.
    for(std::vector<int>::iterator it = list.begin(); it != list.end(); ++it){
      coord_to_char(*it, response, main_goban.get_size());
      response.append("\n");
    }
  } else {
    response[0] = '?';
    response.append("syntax error");
  }

}
