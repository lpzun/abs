/**
 * @brief bws.cc
 *
 * @date  : Jan 25, 2016
 * @author: lpzun
 */

#include "bws.hh"

namespace bws {

BWS::BWS() :
        initl_TS(), final_TS(), reverse_TTS(), respawn_TTS() {
}

BWS::~BWS() {

}

bool BWS::reachability_analysis_via_bws(const string& filename,
        const string& s_initl, const string& s_final,
        const bool& is_self_loop) {
    if (filename == "X") {
        throw bws_runtime_error("no input file");
    } else {
        ifstream org_in;
        if (!Refs::OPT_INPUT_TTS) {
            cout << "Boolean program analysis...\n";
            org_in.open((this->parse_BP(filename) + ".tts").c_str());
            string line;
            std::getline(org_in, line);
        } else {
            org_in.open(filename.c_str());
        }
        if (!org_in.good())
            throw bws_runtime_error("Input file does not find!");
        iparser::remove_comments(org_in, "/tmp/tmp.ttd.no_comment", "#");
        org_in.close();

        /// new input file after removing comments
        ifstream new_in("/tmp/tmp.ttd.no_comment");

        new_in >> Thread_State::S >> Thread_State::L;

        if (!Refs::OPT_INPUT_TTS) {
            final_TS = this->set_up_TS(this->parse_BP(filename) + ".prop");
            cout << final_TS << endl;
        } else {
            initl_TS = this->set_up_TS(s_initl);
            final_TS = this->set_up_TS(s_final);
        }

        Shared_State s1, s2;              /// shared states
        Local_State l1, l2;               /// local  states
        string sep;                       /// separator
        while (new_in >> s1 >> l1 >> sep >> s2 >> l2) {
            DBG_STD(
                    cout << s1 << " " << l1 << " -> " << s2 << " " << l2 << " "<< "\n")
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

        if (Refs::OPT_PRINT_ADJ && Refs::OPT_INPUT_TTS) {
            for (auto idst = reverse_TTS.begin(); idst != reverse_TTS.end();
                    ++idst) {
                for (auto isrc = idst->second.begin();
                        isrc != idst->second.end(); ++isrc) {
                    if (this->is_spawn_transition(idst->first, *isrc))
                        cout << *isrc << "+>" << idst->first << "\n";
                    else
                        cout << *isrc << "->" << idst->first << "\n";
                }
            }
            cout << endl;
        }
    }
    if (this->is_connected())
        return true;
    return this->standard_BWS();
}

string BWS::parse_BP(const string& filename) {
    string file = filename;
    file = file.substr(0, file.find_last_of("."));
    return file;
}

Thread_State BWS::set_up_TS(const string& s_ts) {
    /// setup the initial thread state
    if (s_ts.find('|') != std::string::npos) {
        return utils::create_thread_state_from_str(s_ts);
    } else {
        ifstream in(s_ts.c_str());
        if (in.good()) {
            string s_io;
            std::getline(in, s_io);
            in.close();
            return utils::create_thread_state_from_str(s_io);
        } else {
            //throw bws_runtime_error("initial state file does not find!");
            return Thread_State(Thread_State::S - 1, Thread_State::L - 1);
        }
    }
}

bool BWS::is_connected() {
    queue<Thread_State, deque<Thread_State>> worklist;
    worklist.push(this->final_TS);

    vector<vector<bool>> visited(Thread_State::S,
            vector<bool>(Thread_State::L, false));
    visited[this->final_TS.get_share()][this->final_TS.get_local()] = true;
    while (!worklist.empty()) {
        const auto u = worklist.front();
        worklist.pop();
        auto ifind = this->reverse_TTS.find(u);
        if (ifind != this->reverse_TTS.end()) {
            const auto& predecessors = ifind->second;
            for (auto iprev = predecessors.begin(); iprev != predecessors.end();
                    ++iprev) {
                const auto& prev = *iprev;
                if (prev == this->initl_TS)
                    return true;
                if (!visited[prev.get_share()][prev.get_local()]) {
                    visited[prev.get_share()][prev.get_local()] = true;
                    worklist.push(prev);
                }
            }
        }
    }
    return false;
}

/**
 * @brief the standard backward search
 * @return
 */
bool BWS::standard_BWS() {
    cout << "begin backward search..." << endl;
    /// the set of backward discovered global states
    queue<Global_State, deque<Global_State>> worklist;
    /// the set of explored global states
    deque<Global_State> explored;
    if (this->final_TS == this->initl_TS)
        return false;
    Global_State start(this->final_TS);
    worklist.emplace(start);
    while (!worklist.empty()) {
        const auto _tau = worklist.front();
        worklist.pop();

        if (this->is_minimal(_tau, explored)) {
            const auto images = this->step(_tau);
            /// step 1: insert all pre-images to worklist
            for (auto ip = images.cbegin(); ip != images.cend(); ++ip) {
                const auto& tau = *ip;
                if (this->is_reached(tau)) { /// tau covered by upward(init)
                    return true;
                }
                //cout << tau << endl;
                worklist.emplace(tau);
            }
            // DBG_LOC(2);
            /// step 2: insert _tau to explored states
            this->minimize(_tau, explored); /// minimize the explored states
            explored.emplace_back(_tau); /// insert tau to explored
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
    for (auto im = R.begin(); im != R.end();) {
        if (is_covered(s, *im)) {
            im = R.erase(im);
        } else
            ++im;
    }
}

/**
 * @brief preimage computation
 * @param _tau
 * @return
 */
deque<Global_State> BWS::step(const Global_State& _tau) {
    deque<Global_State> images;
    for (auto local = 0; local < Thread_State::L; ++local) {
        Thread_State curr(_tau.get_share(), local);
        auto ifind = this->reverse_TTS.find(curr);
        if (ifind != this->reverse_TTS.end()) {
            const auto& predecessors = ifind->second;
            for (auto iprev = predecessors.begin(); iprev != predecessors.end();
                    ++iprev) {
                const auto& prev = *iprev;
                if (this->is_spawn_transition(curr, prev)) {
                    bool is_updated = true;
                    const auto& Z = this->update_counter(_tau.get_locals(),
                            local, prev.get_local(), is_updated);
                    if (is_updated)
                        images.emplace_back(prev.get_share(), Z);
                } else {
                    const auto& Z = this->update_counter(_tau.get_locals(),
                            local, prev.get_local());
                    images.emplace_back(prev.get_share(), Z); ///update shared state
                }
            }
        }
    }
    return images;
}

/**
 * @brief update counters
 * @param Z
 * @param dec
 * @param inc
 * @return
 */
Locals BWS::update_counter(const Locals &Z, const Local_State &dec,
        const Local_State &inc) {
    if (dec == inc)
        return Z;

    auto _Z = Z;

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

/**
 * @brief this is used to update counter for spawn transitions
 * @param Z
 * @param dec
 * @param inc
 * @param is_updated
 * @return
 */
Locals BWS::update_counter(const Locals &Z, const Local_State &dec,
        const Local_State &inc, bool& is_updated) {
    auto _Z = Z;
    auto iinc = _Z.find(inc);
    if (iinc != _Z.end()) {
        /// decrease counter: this is executed only when there is a local
        /// state dec in current local part
        auto idec = _Z.find(dec);
        if (idec != _Z.end()) {
            idec->second--;
            if (idec->second == 0)
                _Z.erase(idec);
        }
        is_updated = true;
    } else {
        is_updated = false;
    }
    return _Z;
}

/**
 * @brief to determine if s belongs to upward-closure of initial states
 * @param s
 * @return true : if s is in the upward-closure of initial state
 *         false: otherwise
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
 * @brief to determine whether s1 is covered by s2.
 *        NOTE: this function assumes that the local parts of s1 and s2
 *        are ordered.
 * @param s1
 * @param s2
 * @return true : if s1 <= s2
 *         false: otherwise
 */
bool BWS::is_covered(const Global_State& s1, const Global_State& s2) {
    if (s1.get_share() == s2.get_share()
            && s1.get_locals().size() <= s2.get_locals().size()) {
        auto is1 = s1.get_locals().cbegin();
        auto is2 = s2.get_locals().cbegin();
        while (is1 != s1.get_locals().cend()) {
            /// check if is2 reaches to the end
            if (is2 == s2.get_locals().cend())
                return false;
            /// compare the map's contents
            if (is1->first == is2->first) {
                if (is1->second <= is2->second)
                    ++is1, ++is2;
                else
                    return false;
            } else if (is1->first > is2->first) {
                ++is2;
            } else if (is1->first < is2->first) {
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
bool BWS::is_spawn_transition(const Thread_State& src,
        const Thread_State& dst) {
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
