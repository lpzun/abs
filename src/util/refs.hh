/**
 * @name refs.hh
 *
 * @date: Jun 21, 2015
 * @author: Peizun Liu
 */

#ifndef REFS_HH_
#define REFS_HH_

#include "state.hh"
namespace sura {

using vertex = unsigned int;
/// adjacency list
using adj_list = map<Thread_State, deque<Thread_State>>;

class Refs {
public:
    Refs();
    ~Refs();

    static bool OPT_PRINT_ALL;
    static bool OPT_PRINT_CMD;
    static bool OPT_PRINT_ADJ;

    static bool OPT_INPUT_TTS;

    static Thread_State INITL_TS;
    static Thread_State FINAL_TS;

    static adj_list reverse_TTD;
    static adj_list spawntr_TTD;

    /// global variable for elapsed time
    static clock_t ELAPSED_TIME;
};
} /* namespace SURA */

#endif /* REFS_HH_ */
