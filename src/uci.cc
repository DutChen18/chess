#include "search.hh"

#include <string>
#include <iostream>
#include <sstream>

namespace chess {
	static engine eng;
	static std::string tok, cmd;

	static const char* start_fen =
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	void uci_position(std::istream& is) {
		is >> tok;

		if (tok == "startpos") {
			eng.load(start_fen);
			is >> tok;
		} else if (tok == "fen") {
			std::string fen;
			while (is >> tok && tok != "moves")
				fen += tok + " ";
			eng.load(fen.c_str());
		} else {
			return;
		}

		while (is >> tok) {
			eng.do_move(tok.c_str());
		}
	}

	void uci_go(std::istream& is) {
		eng.ponder = false;
		eng.stopped = false;

		while (is >> tok) {
			if (tok == "ponder") {
				eng.ponder = true;
			}
		}

		eng.go();
	}

	void uci_bestmove(move mv) {
		write_move(std::cout << "bestmove ", mv) << std::endl;
	}

	void uci_main() {
		eng.move_callback = uci_bestmove;
		do {
			std::getline(std::cin, cmd);
			std::istringstream is(cmd);
			tok.clear();
			is >> std::skipws >> tok;

			if (tok == "uci") {
				std::cout << "id name chess" << std::endl;
				std::cout << "id author DutChen18" << std::endl;
				std::cout << "uciok" << std::endl;
			} else if (tok == "isready") {
				std::cout << "readyok" << std::endl;
			} else if (tok == "position") {
				uci_position(is);
			} else if (tok == "go") {
				uci_go(is);
			} else if (tok == "ponderhit") {
				eng.ponder = false;
			} else if (tok == "stop") {
				eng.ponder = false;
				eng.stopped = true;
			}
		} while (tok != "quit");
	}
}