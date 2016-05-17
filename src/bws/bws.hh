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

namespace bws {

class BWS {
public:
    BWS();
    ~BWS();
    bool reachability_analysis_via_bws(const string& filename,
            const string& s_initl, const string& s_final,
            const bool& is_self_loop = false);

    const deque<thread_state>& get_final_TS() const {
        return final_TS;
    }

    const deque<thread_state>& get_initl_TS() const {
        return initl_TS;
    }

private:
    deque<thread_state> initl_TS;
    deque<thread_state> final_TS;
    adj_list reverse_TTS;
    adj_list respawn_TTS;
    map<shared_state, set<local_state>> candidate_L;

    bool standard_BWS();
    deque<global_state> step(const global_state& _tau);
    bool is_spawn_transition(const thread_state& src, const thread_state& dst);

    bool is_reached(const global_state& s);
    bool is_covered(const global_state& s1, const global_state& s2);
    bool is_minimal(const global_state& s, const deque<global_state>& R);
    void minimize(const global_state& s, deque<global_state>& R);

    ca_locals update_counter(const ca_locals &Z, const local_state &dec,
            const local_state &inc);
    ca_locals update_counter(const ca_locals &Z, const local_state &dec,
            const local_state &inc, bool& is_spawn);
    string parse_BP(const string& filename);

    bool is_connected();

    thread_state set_up_TS(const string& s_ts);
};

} /* namespace sura */

#endif /* BWS_BWS_HH_ */
