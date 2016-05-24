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

/**
 * @brief the interface of backward reachability analysis
 * @param filename
 * @param s_initl
 * @param s_final
 * @param is_self_loop
 * @return true : if the final state is reachable;
 *         false: otherwise.
 */
bool BWS::reachability_analysis_via_bws(const string& filename,
        const string& s_initl, const string& s_final,
        const bool& is_self_loop) {
    if (filename == "X") {
        throw bws_runtime_error("no input file");
    } else {
        ifstream org_in;
        if (!refer::OPT_INPUT_TTS) {
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

        new_in >> thread_state::S >> thread_state::L;

        if (!refer::OPT_INPUT_TTS) {
            final_TS.emplace_back(
                    this->set_up_TS(this->parse_BP(filename) + ".prop"));
            for (const auto& final : this->final_TS)
                cout << final << endl;
        } else {
            initl_TS.emplace_back(this->set_up_TS(s_initl));
            final_TS.emplace_back(this->set_up_TS(s_final));
            for (const auto& initl : this->initl_TS) {
                for (const auto& final : this->final_TS)
                    if (initl == final)
                        return true;
            }
        }

        shared_state s1, s2;              /// shared states
        local_state l1, l2;               /// local  states
        string sep;                       /// separator
        while (new_in >> s1 >> l1 >> sep >> s2 >> l2) {
            DBG_STD(
                    cout << s1 << " " << l1 << " -> " ///
                    << s2 << " " << l2 << " "<< "\n")
            if (!is_self_loop && s1 == s2 && l1 == l2) /// remove self loops
                continue;

            if (sep == "->" || sep == "+>") {
                const thread_state src_TS(s1, l1);
                const thread_state dst_TS(s2, l2);
                if (sep == "+>") {
                    respawn_TTS[dst_TS].emplace_back(src_TS);
                }
                reverse_TTS[dst_TS].emplace_back(src_TS);
                candidate_L[s2].insert(l2); /// store expansion local states
            } else {
                throw bws_runtime_error("illegal transition");
            }
        }
        new_in.close();

        if (refer::OPT_PRINT_ADJ && refer::OPT_INPUT_TTS) {
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
            for (const auto& p : candidate_L) {
                cout << "s" << p.first << ": ";
                for (const auto& l : p.second)
                    cout << l << " ";
                cout << "\n";
            }
            cout << endl;
        }
    }
    if (this->is_connected())
        return true;
    return this->standard_BWS();
}

/**
 * @brief parse Boolean program
 * @param filename
 * @return
 */
string BWS::parse_BP(const string& filename) {
    string file = filename;
    file = file.substr(0, file.find_last_of("."));
    return file;
}

/**
 * @brief setup initial or final thread state
 * @param s_ts
 * @return
 */
thread_state BWS::set_up_TS(const string& s_ts) {
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
            // throw bws_runtime_error("initial state file does not find!");
            return thread_state(thread_state::S - 1, thread_state::L - 1);
        }
    }
}

/**
 * @brief an optimization
 * @return
 */
bool BWS::is_connected() {
    queue<thread_state, deque<thread_state>> worklist;
    vector<vector<bool>> visited(thread_state::S,
            vector<bool>(thread_state::L, false));

    for (const auto& final : this->final_TS) {
        if (final.get_share() > thread_state::S
                || final.get_local() > thread_state::L)
            continue;
        worklist.emplace(final);
        visited[final.get_share()][final.get_local()] = true;
    }

    while (!worklist.empty()) {
        const auto u = worklist.front();
        worklist.pop();
        auto ifind = this->reverse_TTS.find(u);
        if (ifind != this->reverse_TTS.end()) {
            const auto& predecessors = ifind->second;
            for (auto iprev = predecessors.begin(); iprev != predecessors.end();
                    ++iprev) {
                const auto& prev = *iprev;

                for (const auto& initl : this->initl_TS) {
                    if (prev == initl)
                        return true;
                }

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
    queue<global_state, deque<global_state>> worklist;
    /// the set of explored global states
    deque<global_state> explored;
    for (const auto& final : final_TS)
        worklist.emplace(final); /// insert all final states

    while (!worklist.empty()) {
        const auto _tau = worklist.front();
        worklist.pop();

        /// step 1: if exists t \in <explored> such that
        ///         t <= _tau, then discard _tau
        if (this->is_minimal(_tau, explored)) {
            const auto& images = this->step(_tau);
            /// step 1: insert all pre-images to worklist
            for (const auto& tau : images) {
                /// return true if tau \in upward(init)
                if (this->is_reached(tau))
                    return true;

                worklist.emplace(tau);
            }
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
bool BWS::is_minimal(const global_state& s, const deque<global_state>& R) {
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
void BWS::minimize(const global_state& s, deque<global_state>& R) {
    for (auto im = R.begin(); im != R.end();) {
        if (is_covered(s, *im)) {
            im = R.erase(im);
        } else {
            ++im;
        }
    }
}

/**
 * @brief preimage computation
 * @param _tau
 * @return deque
 */
deque<global_state> BWS::step(const global_state& _tau) {
    deque<global_state> images;
//    cout << _tau << "\n"; /// delete this ---------------
    auto iexps = this->candidate_L.find(_tau.get_share());
    if (iexps != this->candidate_L.end())
        for (const auto& local : iexps->second) {
            thread_state curr(_tau.get_share(), local);
            auto ifind = this->reverse_TTS.find(curr);
            if (ifind != this->reverse_TTS.end()) {
                const auto& predecessors = ifind->second;
                for (const auto& prev : predecessors) {
                    if (this->is_spawn_transition(curr, prev)) {
                        bool is_updated = true;
                        const auto& Z = this->update_counter(_tau.get_locals(),
                                local, prev.get_local(), is_updated);
                        if (is_updated)
                            images.emplace_back(prev.get_share(), Z);
                    } else {
                        const auto& Z = this->update_counter(_tau.get_locals(),
                                local, prev.get_local());
                        /// update shared state ...
                        images.emplace_back(prev.get_share(), Z);
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
ca_locals BWS::update_counter(const ca_locals &Z, const local_state &dec,
        const local_state &inc) {
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
ca_locals BWS::update_counter(const ca_locals &Z, const local_state &dec,
        const local_state &inc, bool& is_updated) {
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
    }
    return _Z;
}

/**
 * @brief to determine if s belongs to upward-closure of initial states
 * @param s
 * @return true : if s is in the upward-closure of initial state
 *         false: otherwise
 */
bool BWS::is_reached(const global_state& s) {
    for (const auto& initl : this->initl_TS)
        if (s.get_share() == initl.get_share()) {
            if (s.get_locals().size() == 1
                    && s.get_locals().begin()->first == initl.get_local())
                return true;

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
bool BWS::is_covered(const global_state& s1, const global_state& s2) {
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
                if (is1->second > is2->second)
                    return false;
                ++is1, ++is2;
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
bool BWS::is_spawn_transition(const thread_state& src,
        const thread_state& dst) {
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
