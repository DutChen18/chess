#include "search.hh"

#include <sstream>

namespace chess {
	score eval(position& pos) {
		bitboard wb = pos.get(white), bb = pos.get(black);
		score ws = 0;
		ws += 9 * (cnt(wb & pos.get(queen )) - cnt(bb & pos.get(queen )));
		ws += 5 * (cnt(wb & pos.get(rook  )) - cnt(bb & pos.get(rook  )));
		ws += 3 * (cnt(wb & pos.get(bishop)) - cnt(bb & pos.get(bishop)));
		ws += 3 * (cnt(wb & pos.get(knight)) - cnt(bb & pos.get(knight)));
		ws += 1 * (cnt(wb & pos.get(pawn  )) - cnt(bb & pos.get(pawn  )));
		return pos.side() == white ? ws : -ws;
	}

	score engine::search(position& pos, int depth) {
		if (depth == 0) return eval(pos);
		score max = min_score;
		move list[218];
		move* end = generate<1>(pos, list);
		if (list == end) {
			return max + pos.ply;
		}
		for (move* i = list; i != end; ++i) {
			pos_info info;
			pos.do_move<1>(*i, &info);
			score value = -search(pos, depth - 1);
			if (value > max) max = value;
			pos.un_move(*i);
		}
		return max;
	}

	void engine::search() {
		position pos;
		this->pos.copy(pos);

		score max = min_score;
		move list[218], best = 0;
		move* end = generate<1>(pos, list);
		for (move* i = list; i != end; ++i) {
			pos_info info;
			pos.do_move<1>(*i, &info);
			score value = -search(pos, 4);
			if (value > max) {
				max = value;
				best = *i;
			}
			pos.un_move(*i);
		}

		move_callback(best);
	}

	void engine::go() {
		if (thr.joinable()) {
			thr.join();
		}
		thr = std::thread([](engine* eng) {
			eng->search();
		}, this);
	}

	void engine::load(const char* fen) {
		info.resize(1);
		pos.load(fen, &info.front());
	}

	void engine::do_move(const char* mv) {
		move list[218];
		move* end = generate<1>(pos, list);
		for (move* i = list; i != end; ++i) {
			std::stringstream ss;
			write_move(ss, *i);
			if (ss.str() == mv) {
				info.emplace_front();
				pos.do_move<1>(*i, &info.front());
			}
		}
	}
}