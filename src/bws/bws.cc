/**
 * @brief bws.cc
 *
 * @date  : Jan 25, 2016
 * @author: lpzun
 */

#include "bws.hh"

namespace sura {

BWS::BWS() :
		initl_TS(), final_TS(), reverse_TTS(), respawn_TTS() {
}

BWS::~BWS() {

}

bool BWS::reachability_analysis_via_bws(const string& filename, const string& initl, const string& final,
		const bool& is_self_loop) {
	this->initl_TS = Util::create_thread_state_from_str(initl);
	this->final_TS = Util::create_thread_state_from_str(final);

	cout << this->initl_TS << endl; // delete-----------------
	cout << this->final_TS << endl; // delete-----------------

	if (filename == "X") {
		throw bws_runtime_error("no input file");
	} else {
		string file = filename;
		if (!Refs::OPT_INPUT_TTS) {
			cout << "I am here...\n";
			file = file.substr(0, file.find_last_of("."));
			file += ".tts";
		}
		ifstream org_in(filename.c_str());
		if (!org_in.good())
			throw bws_runtime_error("Input file does not find!");
		Parser::remove_comments(org_in, "/tmp/tmp.ttd.no_comment", "#");
		org_in.close();

		/// new input file after removing comments
		ifstream new_in("/tmp/tmp.ttd.no_comment");

		new_in >> Thread_State::S >> Thread_State::L;

		cout << Thread_State::S << "" << Thread_State::L << endl;

		Shared_State s1, s2;              /// shared states
		Local_State l1, l2;               /// local  states
		string sep;                       /// separator
		while (new_in >> s1 >> l1 >> sep >> s2 >> l2) {
			DBG_STD(
					cout << s1 << " " << l1 << " -> " << s2 << " " << l2 << " "
					<< transition_ID << "\n")
			if (!is_self_loop && s1 == s2 && l1 == l2) /// remove self loops
				continue;

			if (sep == "->" || sep == "+>") {
				const Thread_State src_TS(s1, l1);
				const Thread_State dst_TS(s2, l2);
				if (sep == "+>") {
					respawn_TTS[dst_TS].emplace_back(src_TS);
				}
				reverse_TTS[dst_TS].emplace_back(src_TS);
			} else {
				throw bws_runtime_error("illegal transition");
			}
		}
		new_in.close();

		if (Refs::OPT_PRINT_ADJ) {
			for (auto idst = reverse_TTS.begin(); idst != reverse_TTS.end(); ++idst) {
				for (auto isrc = idst->second.begin(); isrc != idst->second.end(); ++isrc) {
					if (this->is_spawn_transition(idst->first, *isrc))
						cout << *isrc << "+>" << idst->first << "\n";
					else
						cout << *isrc << "->" << idst->first << "\n";
				}
			}
			cout << endl;
		}

	}
	return this->standard_BWS();
}

/**
 * @brief the standard backward search
 * @return
 */
bool BWS::standard_BWS() {
	queue<Global_State, deque<Global_State>> worklist;
	deque<Global_State> explored;
	if (this->final_TS == this->initl_TS)
		return false;
	cout << "I ma here......1...\n";
	Global_State start(this->final_TS);
	cout<<"start..."<<start<<endl;
	worklist.emplace(start);
	if (!worklist.empty()) {
		const auto& _tau = worklist.front();
		cout << "I ma here......3..."<<_tau<<endl;;
		if (this->is_minimal(_tau, explored)) {

			cout << "I ma here......4...\n";
			const auto& images = this->pre_image(_tau);
			cout << "I ma here......5...\n";
			/// step 1: insert all pre-images to worklist
			for (auto ip = images.cbegin(); ip != images.cend(); ++ip) {
				const auto& tau = *ip;
				if (this->is_reached(tau)) { /// tau covered by upward(init)
					return true;
				}
				cout << tau << endl;
				worklist.emplace(tau);
			}

			cout << "I ma here......2...\n";
			/// step 2: insert _tau to explored states
			this->minimize(_tau, explored); /// minimize the explored states
			explored.emplace_back(_tau); /// insert tau to explored

			worklist.pop();
		} /// discard _tau if it is non-minimal
	}
	return false;
}

/**
 * @brief to determine if s is a minimal state w.r.t. R
 * @param s
 * @param R
 * @return
 */
bool BWS::is_minimal(const Global_State& s, const deque<Global_State>& R) {
	for (auto im = R.cbegin(); im != R.cend(); ++im) {
		if (is_covered(*im, s)) {
			return false;
		}
	}
	return true;
}

/**
 * @brief minimize a reachable set
 * @param s
 * @param R
 * @return
 */
void BWS::minimize(const Global_State& s, deque<Global_State>& R) {
	for (auto im = R.begin(); im != R.end(); ++im) {
		if (is_covered(s, *im)) {
			R.erase(im);
		}
	}
}

deque<Global_State> BWS::pre_image(const Global_State& _tau) {
	deque<Global_State> images;
	for (auto local = 0; local < Thread_State::L; ++local) {
		Thread_State curr(_tau.get_share(), local);
		auto ifind = this->reverse_TTS.find(curr);
		if (ifind != this->reverse_TTS.end()) {
			const auto& predecessors = ifind->second;
			for (auto iprev = predecessors.begin(); iprev != predecessors.end(); ++iprev) {
				const auto& prev = *iprev;
				cout << "I ma here......6..." << prev << endl;
				if (this->is_spawn_transition(curr, prev)) {
					const auto& p = this->update_counter(_tau.get_locals(), local);
					if (p.second)
						images.emplace_back(_tau.get_share(), p.first);
				} else {
					const auto& Z = this->update_counter(_tau.get_locals(), local, prev.get_local());
					images.emplace_back(_tau.get_share(), Z);
				}
			}
		}
	}
	return images;
}

Locals BWS::update_counter(const Locals &Z, const Local_State &dec, const Local_State &inc) {
	cout << "I ma here......7...\n";
	auto _Z = Z;
	if (dec == inc)
		return Z;

	/// decrease counter: this is executed only when there is a local
	/// state dec in current local part
	auto idec = _Z.find(dec);
	if (idec != _Z.end()) {
		idec->second--;
		if (idec->second == 0)
			_Z.erase(idec);
	}

	auto iinc = _Z.find(inc);
	if (iinc != _Z.end()) {
		iinc->second++;
	} else {
		_Z.emplace(inc, 1);
	}

	return _Z;
}

pair<Locals, bool> BWS::update_counter(const Locals &Z, const Local_State &dec) {
	auto _Z = Z;
	auto idec = _Z.find(dec);
	if (idec != _Z.end()) {
		idec->second--;
		if (idec->second == 0)
			_Z.erase(idec);
		return std::make_pair(_Z, true);
	} else {
		return std::make_pair(_Z, false);
	}
}

/**
 * @brief to determine if s belongs to upward-closure of initial states
 * @param s
 * @return bool
 */
bool BWS::is_reached(const Global_State& s) {
	if (s.get_share() == this->initl_TS.get_share()) {
		if (s.get_locals().size() == 1) {
			if (s.get_locals().begin()->first == this->initl_TS.get_local())
				return true;
		}
	}
	return false;
}

/**
 *
 * @param s1
 * @param s2
 * @return
 */
bool BWS::is_covered(const Global_State& s1, const Global_State& s2) {
	if (s1.get_share() == s2.get_share() && s1.get_locals().size() <= s2.get_locals().size()) { /// compare shared state
		for (auto is1 = s1.get_locals().cbegin(); is1 != s1.get_locals().cend(); ++is1) { /// compare local parts
			auto ifind = s2.get_locals().find(is1->first);
			if (ifind == s2.get_locals().end()) { /// no same local state
				return false;
			} else { /// there is a same local states
				if (is1->second > ifind->second) /// the counter is bigger
					return false;
			}
		}
		return true;
	}
	return false;
}

/**
 * @brief determine if (src, dst) corresponds to a spawn transition
 * @param src
 * @param dst
 * @param spawn_trans
 * @return bool
 *          true : src +> dst
 *          false: otherwise
 */
bool BWS::is_spawn_transition(const Thread_State& src, const Thread_State& dst) {
	auto ifind = this->respawn_TTS.find(src);
	if (ifind == this->respawn_TTS.end()) {
		return false;
	} else {
		auto ifnd = std::find(ifind->second.begin(), ifind->second.end(), dst);
		if (ifnd == ifind->second.end())
			return false;
		else
			return true;
	}
}

} /* namespace sura */
