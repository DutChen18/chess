#ifndef CHESS_GEN_HH
#define CHESS_GEN_HH

#include "pos.hh"

#include <thread>
#include <vector>
#include <forward_list>
#include <functional>
#include <condition_variable>

namespace chess {
	template<bool G, color C>
	move* generate(const position& pos, move* list);

	template<bool G>
	inline move* generate(const position& pos, move* list) {
		if (pos.side() == white) {
			return generate<G, white>(pos, list);
		} else {
			return generate<G, black>(pos, list);
		}
	}

	uint64_t perft(position& pos, size_t depth);
	void test(size_t max = ~0ULL);

	struct engine {
		std::function<void(move)> move_callback;
		std::forward_list<pos_info> info;
		position pos;

		std::thread thr;
		bool stopped;
		bool ponder;

		score search(position& pos, int depth);
		void search();
		
		void go();
		void load(const char* fen);
		void do_move(const char* mv);
	};
}

#endif