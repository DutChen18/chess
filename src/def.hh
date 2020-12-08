#ifndef CHESS_DEF_HH
#define CHESS_DEF_HH

#include <cstdint>

namespace chess {
	typedef uint64_t key;
	typedef uint64_t bitboard;
	typedef int8_t direction;
	typedef uint8_t castling;
	typedef uint16_t move;
	typedef int16_t score;

	constexpr direction north =  8;
	constexpr direction east  =  1;
	constexpr direction south = -8;
	constexpr direction west  = -1;
	constexpr castling white_oo  = 1;
	constexpr castling white_ooo = 2;
	constexpr castling black_oo  = 4;
	constexpr castling black_ooo = 8;
	constexpr castling white_any = white_oo  | white_ooo;
	constexpr castling black_any = black_oo  | black_ooo;
	constexpr castling any_oo    = white_oo  | black_oo ;
	constexpr castling any_ooo   = white_ooo | black_ooo;
	constexpr score max_score = INT16_MAX;
	constexpr score min_score = INT16_MIN;

	enum file : uint8_t {
		file_a, file_b, file_c, file_d,
		file_e, file_f, file_g, file_h,
		no_file,
	};

	enum rank : uint8_t {
		rank_1, rank_2, rank_3, rank_4,
		rank_5, rank_6, rank_7, rank_8,
		no_rank,
	};

	enum square : uint8_t {
		a1, b1, c1, d1, e1, f1, g1, h1,
		a2, b2, c2, d2, e2, f2, g2, h2,
		a3, b3, c3, d3, e3, f3, g3, h3,
		a4, b4, c4, d4, e4, f4, g4, h4,
		a5, b5, c5, d5, e5, f5, g5, h5,
		a6, b6, c6, d6, e6, f6, g6, h6,
		a7, b7, c7, d7, e7, f7, g7, h7,
		a8, b8, c8, d8, e8, f8, g8, h8,
		no_square,
	};

	enum color : uint8_t {
		white, black,
	};

	enum type : uint8_t {
		pawn, knight, bishop, rook, queen, king,
		no_type,
	};

	enum piece : uint8_t {
		white_pawn,   black_pawn,
		white_knight, black_knight,
		white_bishop, black_bishop,
		white_rook,   black_rook,
		white_queen,  black_queen,
		white_king,   black_king,
		no_piece,
	};

	constexpr square to_sq   (file  f, rank  r) { return square(f + r * 8); }
	constexpr file   file_of (square         v) { return file  (v     % 8); }
	constexpr rank   rank_of (square         v) { return rank  (v     / 8); }
	constexpr piece  to_pc   (color c, type  t) { return piece (c + t * 2); }
	constexpr color  color_of(piece          v) { return color (v     % 2); }
	constexpr type   type_of (piece          v) { return type  (v     / 2); }

	constexpr square    to_sq  (color c, square sq)
		{ return c == white ? sq : square(sq ^ 56); }
	constexpr rank      to_rank(color c, rank   r )
		{ return c == white ? r  : rank  (r  ^ 7 ); }
	constexpr direction to_dir (color c           )
		{ return c == white ? north : south;        }

	constexpr square from_of (move   mv) { return square(mv >>  0 & 0x3F); }
	constexpr square dest_of (move   mv) { return square(mv >>  6 & 0x3F); }
	constexpr type   promo_of(move   mv) { return type  (mv >> 12 & 0x07); }
	constexpr move   mv_from (square sq) { return move  (sq <<  0       ); }
	constexpr move   mv_dest (square sq) { return move  (sq <<  6       ); }
	constexpr move   mv_promo(type    p) { return move  (p  << 12       ); }

	constexpr move to_mv(square fr, square to, type p = no_type)
		{ return mv_from(fr) | mv_dest(to) | mv_promo(p); }

	inline square& operator++(square& v) { return v = square(v + 1); }
	inline file  & operator++(file  & v) { return v = file  (v + 1); }
	inline rank  & operator++(rank  & v) { return v = rank  (v + 1); }
	constexpr square operator~(square v) { return square(v ^ 56); }
	constexpr rank   operator~(rank   v) { return rank  (v ^  7); }
	constexpr color  operator~(color  v) { return color (v ^  1); }

	constexpr square operator+(square l, direction r)
		{ return square(direction(l) + r); }
	constexpr square operator-(square l, direction r)
		{ return square(direction(l) - r); }
}

#endif