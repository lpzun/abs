/**
 * @brief bws.hh
 *
 * @date  : Jan 25, 2016
 * @author: lpzun
 */

#ifndef BWS_BWS_HH_
#define BWS_BWS_HH_

#include "../util/state.hh"
#include "../util/algs.hh"
#include "../util/utilities.hh"

namespace sura {

class BWS {
public:
    BWS();
    ~BWS();
    bool reachability_analysis_via_bws(const string& filename,
            const string& initl, const string& final, const bool& is_self_loop =
                    false);
private:
    Thread_State initl_TS;
    Thread_State final_TS;
    adj_list reverse_TTS;
    adj_list respawn_TTS;

    bool standard_BWS();

    bool is_spawn_transition(const Thread_State& src, const Thread_State& dst);

    bool is_reached(const Global_State& s);
    bool is_covered(const Global_State& s1, const Global_State& s2);
    bool is_minimal(const Global_State& s, const deque<Global_State>& R);
    void minimize(const Global_State& s, deque<Global_State>& R);

    deque<Global_State> pre_image(const Global_State& _tau);

    Locals update_counter(const Locals &Z, const Local_State &dec,
            const Local_State &inc);
    Locals update_counter(const Locals &Z, const Local_State &dec,
            const Local_State &inc, bool& is_spawn);
    string parse_BP(const string& filename);

    bool is_connected();
};

} /* namespace sura */

#endif /* BWS_BWS_HH_ */
