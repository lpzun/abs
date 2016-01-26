/**
 * @name refs.cc
 *
 * @date: Jun 21, 2015
 * @author: Peizun Liu
 */

#include "refs.hh"

namespace sura {

Refs::Refs() {

}

Refs::~Refs() {

}
bool Refs::OPT_INPUT_TTS = false;

bool Refs::OPT_PRINT_ADJ = false;
bool Refs::OPT_PRINT_CMD = false;
bool Refs::OPT_PRINT_ALL = false;

//string Refs::FILE_NAME_PREFIX = "";

Thread_State Refs::INITL_TS;
Thread_State Refs::FINAL_TS;

adj_list Refs::reverse_TTD;
adj_list Refs::spawntr_TTD;

clock_t Refs::ELAPSED_TIME = clock();
} /* namespace SURA */
