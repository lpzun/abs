//============================================================================
// Name        : Abdulla's backward search
// Author      : Peizun Liu
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>

#include "util/cmd.hh"

using namespace std;

int main(const int argc, const char * const * const argv) {
    try {
        cmd_line cmd;
        try {
            cmd.get_command_line(argc, argv);
        } catch (cmd_line::Help) {
            return 0;
        }
    } catch (const sura_exception & e) {
        e.what();
    } catch (const std::exception& e) {
        std::cerr << e.what() << endl;
    } catch (...) {
        std::cerr << sura_exception("main: unknown exception occurred").what()
                << endl;
    }
}
