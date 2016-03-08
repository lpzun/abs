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

	const Thread_State& getFinalTs() const {
		return final_TS;
	}

	const Thread_State& getInitlTs() const {
		return initl_TS;
	}

private:
	Thread_State initl_TS;
	Thread_State final_TS;
	adj_list reverse_TTS;
	adj_list respawn_TTS;

	bool standard_BWS();
	deque<Global_State> step(const Global_State& _tau);
	bool is_spawn_transition(const Thread_State& src, const Thread_State& dst);

	bool is_reached(const Global_State& s);
	bool is_covered(const Global_State& s1, const Global_State& s2);
	bool is_minimal(const Global_State& s, const deque<Global_State>& R);
	void minimize(const Global_State& s, deque<Global_State>& R);

	Locals update_counter(const Locals &Z, const Local_State &dec,
			const Local_State &inc);
	Locals update_counter(const Locals &Z, const Local_State &dec,
			const Local_State &inc, bool& is_spawn);
	string parse_BP(const string& filename);

	bool is_connected();

	Thread_State set_up_TS(const string& s_ts);
};

} /* namespace sura */

#endif /* BWS_BWS_HH_ */
