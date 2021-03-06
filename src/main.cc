//============================================================================
// Name        : Abdulla's backward search
// Author      : Peizun Liu
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>

#include "util/cmd.hh"
#include "bws/bws.hh"
#include "util/refer.hh"

using namespace bws;
using namespace std;

int main(const int argc, const char * const * const argv) {
    try {
        cmd_line cmd;
        try {
            cmd.get_command_line(argc, argv);
        } catch (cmd_line::Help) {
            return 0;
        }

        refer::OPT_PRINT_ADJ = cmd.arg_bool(cmd_line::prob_inst_opts(),
                "--adj-list");
        refer::OPT_INPUT_TTS = cmd.arg_bool(cmd_line::prob_inst_opts(),
                "--input-tts");

        refer::OPT_PRINT_CMD = cmd.arg_bool(cmd_line::other_opts(),
                "--cmd-line");
        refer::OPT_PRINT_ALL = cmd.arg_bool(cmd_line::other_opts(), "--all");

        const string& filename = cmd.arg_value(cmd_line::prob_inst_opts(),
                "--input-file");
        const string& initl_ts = cmd.arg_value(cmd_line::prob_inst_opts(),
                "--initial");
        const string& final_ts = cmd.arg_value(cmd_line::prob_inst_opts(),
                "--target");

        BWS bws;
        bool is_reachable = bws.reachability_analysis_via_bws(filename,
                initl_ts, final_ts);
        cout << "======================================================\n";
        cout << "Target";
        if (is_reachable)
            cout << " is reachable: verification failed!\n";
        else
            cout << " is unreachable: verification successful!\n";
        cout << "======================================================"
                << endl;

    } catch (const bws_exception & e) {
        e.what();
    } catch (const std::exception& e) {
        std::cerr << e.what() << endl;
    } catch (...) {
        std::cerr << bws_exception("main: unknown exception occurred").what()
                << endl;
    }
}
