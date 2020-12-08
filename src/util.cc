#include "util.hh"
#include "attack.hh"

namespace chess {
	castling rights_mask   [64];
	bitboard attack_pawn   [ 2][64];
	bitboard attack_knight [64];
	bitboard attack_king   [64];
	bitboard attack_line   [64][64];
	bitboard magic_table   [107648];
	magic    magic_rook    [64];
	magic    magic_bishop  [64];
	key      zobrist_piece [12][64];
	key      zobrist_rights[16];
	key      zobrist_en_pas[65];
	key      zobrist_side;

	void init_magic(square sq, magic& m, bitboard(*atk)(bitboard, bitboard)) {
		static size_t magic_index = 0;
		bitboard h = (to_bb(file_a) | to_bb(file_h)) & ~to_bb(file_of(sq));
		bitboard v = (to_bb(rank_1) | to_bb(rank_8)) & ~to_bb(rank_of(sq));
		
		uint64_t seed = 1070372;
		bitboard bb   = 0ULL;
		bitboard occ[4096];
		bitboard ref[4096];
		size_t   age[4096] = {};
		size_t   size = 0;

		m.mask   = atk(to_bb(sq), ~0ULL) & ~(h | v);
		m.shift  = 64 - cnt(m.mask);
		m.attack = magic_table + magic_index;
		do {
			occ[size  ] = bb;
			ref[size++] = atk(to_bb(sq), ~bb);
			bb = (bb - m.mask) & m.mask;
			magic_index += 1;
		} while (bb);

		for (size_t i = 0, j = 1; i < size; ++j) {
			m.magic = random(seed) & random(seed) & random(seed);
			for (i = 0; i < size; ++i) {
				size_t k = m.index(occ[i]);
				if (age[k] < j) {
					age[k] = j;
					m.attack[k] = ref[i];
				} else if (m.attack[k] != ref[i]) {
					break;
				}
			}
		}
	}

	void init() {
		for (square sq = a1; sq < no_square; ++sq) {
			attack_pawn[white][sq] = pawn_attacks<white>(to_bb(sq));
			attack_pawn[black][sq] = pawn_attacks<black>(to_bb(sq));
			attack_knight     [sq] = knight_attacks     (to_bb(sq));
			attack_king       [sq] = king_attacks       (to_bb(sq));
			init_magic(sq, magic_bishop[sq], bishop_attacks);
			init_magic(sq, magic_rook  [sq], rook_attacks  );

		}

		for (square s1 = a1; s1 < no_square; ++s1) {
			for (square s2 = a1; s2 < no_square; ++s2) {
				bitboard bb1 = to_bb(s1), bb2 = to_bb(s2);
				bitboard mb1 = magic_bishop[s1].get();
				bitboard mb2 = magic_bishop[s2].get();
				bitboard mr1 = magic_rook  [s1].get();
				bitboard mr2 = magic_rook  [s2].get();
				if (mb1 & bb2) attack_line[s1][s2] = (mb1 & mb2) | bb1 | bb2;
				if (mr1 & bb2) attack_line[s1][s2] = (mr1 & mr2) | bb1 | bb2;
			}
		}

		for (size_t i = 0; i < 64; i++)
			rights_mask[i] = ~0;
		rights_mask[a1] = ~white_ooo;
		rights_mask[e1] = ~white_any;
		rights_mask[h1] = ~white_oo;
		rights_mask[a8] = ~black_ooo;
		rights_mask[e8] = ~black_any;
		rights_mask[h8] = ~black_oo;

		uint64_t seed = 1070372;
		for (size_t i = 0; i < 12; i++)
			for (size_t j = 0; j < 64; j++)
				zobrist_piece[i][j] = random(seed);
		for (size_t i = 0; i < 16; i++)
			zobrist_rights[i] = random(seed);
		for (size_t i = 0; i < 8; i++)
			for (size_t j = 0; j < 8; j++)
				zobrist_en_pas[i + j * 8] = random(seed);
		zobrist_en_pas[no_square] = 0;
		zobrist_side = random(seed);
	}

	std::ostream& write_bitboard(std::ostream& os, bitboard bb) {
		for (rank r = rank_1; r < no_rank; ++r) {
			for (file f = file_a; f < no_file; ++f) {
				square sq = to_sq(f, ~r);
				if (f != file_a)    os << " ";
				if (bb & to_bb(sq)) os << "X";
				else                os << "-";
			}
			os << std::endl;
		}
		return os;
	}

	std::ostream& write_move (std::ostream& os, move mv) {
		os << char('a' + file_of(from_of(mv)));
		os << char('1' + rank_of(from_of(mv)));
		os << char('a' + file_of(dest_of(mv)));
		os << char('1' + rank_of(dest_of(mv)));
		if (promo_of(mv) != no_type) {
			os << "pnbrqk"[promo_of(mv)];
		}
		return os;
	}

	static const int init_ = (init(), 0);
}