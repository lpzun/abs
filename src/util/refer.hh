/**
 * @name refs.hh
 *
 * @date: Jun 21, 2015
 * @author: Peizun Liu
 */

#ifndef REFS_HH_
#define REFS_HH_

#include "state.hh"
namespace bws {
using vertex = unsigned int;
/// adjacency list
using adj_list = map<thread_state, deque<thread_state>>;

class refer {
public:
    refer();
    ~refer();

    static bool OPT_PRINT_ALL;
    static bool OPT_PRINT_CMD;
    static bool OPT_PRINT_ADJ;

    static bool OPT_INPUT_TTS;

    /// global variable for elapsed time
    static clock_t ELAPSED_TIME;

};
} /* namespace SURA */

#endif /* REFS_HH_ */
