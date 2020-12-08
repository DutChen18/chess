#ifndef CHESS_ATTACK_HH
#define CHESS_ATTACK_HH

#include "util.hh"

namespace chess {
	template<color C>
	inline bitboard pawn_attacks(bitboard bb) {
		constexpr direction dir = C == white ? north : south;
		return shift<dir + east>(bb) | shift<dir + west>(bb);
	}

	inline bitboard knight_attacks(bitboard bb) {
		bitboard h1 = shift<east  * 1>(bb) | shift<west  * 1>(bb);
		bitboard h2 = shift<east  * 2>(bb) | shift<west  * 2>(bb);
		bitboard v2 = shift<north * 2>(h1) | shift<south * 2>(h1);
		bitboard v1 = shift<north * 1>(h2) | shift<south * 1>(h2);
		return   v1 | v2;
	}

	inline bitboard king_attacks(bitboard bb) {
		bitboard attacks = shift<east >(bb) | shift<west >(bb);
		bb |=    attacks;
		return   attacks | shift<north>(bb) | shift<south>(bb);
	}

	inline bitboard bishop_attacks(bitboard bb, bitboard empty) {
		bitboard ne = ray<north + east>(bb, empty);
		bitboard nw = ray<north + west>(bb, empty);
		bitboard se = ray<south + east>(bb, empty);
		bitboard sw = ray<south + west>(bb, empty);
		return   ne | nw | se | sw;
	}

	inline bitboard rook_attacks(bitboard bb, bitboard empty) {
		bitboard ne = ray<north>(bb, empty) | ray<east>(bb, empty);
		bitboard sw = ray<south>(bb, empty) | ray<west>(bb, empty);
		return   ne | sw;
	}

	inline bitboard between(square s1, square s2) {
		bitboard mask = ~0ULL << s1 ^ ~0ULL << s2;
		bitboard bb = attack_line[s1][s2] & mask;
		return bb & (bb - 1);
	}
}

#endif