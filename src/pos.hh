#ifndef CHESS_POS_HH
#define CHESS_POS_HH

#include "def.hh"
#include "util.hh"

namespace chess {
	struct pos_info {
		pos_info* prev;
		key       hash;
		square    en_pas;
		castling  rights;
		uint8_t   clock;
		piece     captured;
	};

	struct position {
		pos_info* info;
		piece     board   [64];
		bitboard  color_bb[2];
		bitboard  type_bb [6];
		uint16_t  ply = 0;
		
		position() = default;
		position(const position& other) = delete;
		position& operator=(const position& other) = delete;

		void copy(position& pos) const;
		size_t load(const char* fen, pos_info* info);
		size_t dump(char* fen) const;
		std::ostream& write(std::ostream& os) const;

		template<color C, bool H>
		void do_move(move mv, pos_info* info);
		template<color C>
		void un_move(move mv);

		template<bool H>
		inline void do_move(move mv, pos_info* info) {
			if (side() == white) {
				do_move<white, H>(mv, info);
			} else {
				do_move<black, H>(mv, info);
			}
		}

		inline void un_move(move mv) {
			if (side() == white) {
				un_move<black>(mv);
			} else {
				un_move<white>(mv);
			}
		}

		inline piece    get(square sq) const { return board   [sq]; }
		inline bitboard get(color  c ) const { return color_bb[c ]; }
		inline bitboard get(type   t ) const { return type_bb [t ]; }

		inline castling rights(color c) const
			{ return info->rights & (c == white ? white_any : black_any); }
		inline square en_pas() const
			{ return info->en_pas; }
		inline color side() const
			{ return color(ply & 1); }
	};
}

#endif