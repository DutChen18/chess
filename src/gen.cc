#include "search.hh"
#include "attack.hh"

namespace chess {
	struct pos_meta {
		bitboard pin = 0;
		bitboard chk = 0;
		bitboard atk = 0;
		bitboard tgt = 0;
	};

	template<bool G, bool P>
	inline move* add(move* list, square s1, square s2) {
		if (!G) return list + (P ? 4 : 1);
		if (P) {
			*list++ = to_mv(s1, s2, queen );
			*list++ = to_mv(s1, s2, knight);
			*list++ = to_mv(s1, s2, rook  );
			*list++ = to_mv(s1, s2, bishop);
		} else {
			*list++ = to_mv(s1, s2);
		}
		return list;
	}

	template<bool G, bool P>
	inline move* add(move* list, bitboard bb, square s1) {
		if (!G) return list + cnt(bb) * (P ? 4 : 1);
		while (bb) {
			square s2 = pop(bb);
			list = add<1, P>(list, s1, s2);
		}
		return list;
	}

	template<bool G, direction D, bool P>
	inline move* add(move* list, bitboard bb) {
		if (!G) return list + cnt(bb) * (P ? 4 : 1);
		while (bb) {
			square s2 = pop(bb);
			list = add<1, P>(list, s2 - D, s2);
		}
		return list;
	}

	template<bool G>
	inline move* add_pawn(move* list, bitboard bb, square s1) {
		list = add<G, 0>(list, bb & ~(to_bb(rank_1) | to_bb(rank_8)), s1);
		return add<G, 1>(list, bb &  (to_bb(rank_1) | to_bb(rank_8)), s1);
	}

	template<bool G, direction D>
	inline move* add_pawn(move* list, bitboard bb) {
		list = add<G, D, 0>(list, bb & ~(to_bb(rank_1) | to_bb(rank_8)));
		return add<G, D, 1>(list, bb &  (to_bb(rank_1) | to_bb(rank_8)));
	}

	template<color C>
	inline pos_meta gen_meta(const position& pos) {
		pos_meta meta;
		bitboard oc = (pos.get(C    ) | pos.get(~C    ));
		bitboard bq = (pos.get(queen) | pos.get(bishop)) & pos.get(~C);
		bitboard rq = (pos.get(queen) | pos.get(rook  )) & pos.get(~C);
		square ki = lsb(pos.get(C) & pos.get(king));

		bitboard bb, pin = 0;
		meta.chk |= (bb = magic_bishop[ki].get(oc)) & bq;
		pin |= (magic_bishop[ki].get(oc ^ (pos.get(C) & bb)) ^ bb) & bq;
		meta.chk |= (bb = magic_rook  [ki].get(oc)) & rq;
		pin |= (magic_rook  [ki].get(oc ^ (pos.get(C) & bb)) ^ bb) & rq;
		while (pin) meta.pin |= between(ki, pop(pin)) & pos.get(C);

		meta.tgt = ~pos.get(C);
		meta.chk |= attack_pawn[C][ki] & pos.get(pawn  );
		meta.chk |= attack_knight [ki] & pos.get(knight);
		if (meta.chk &= pos.get(~C)) {
			meta.tgt &= meta.chk;
			meta.tgt |= between(ki, lsb(meta.chk));
		}

		meta.atk |= pawn_attacks<~C>(pos.get(~C) & pos.get(pawn  ));
		meta.atk |= knight_attacks  (pos.get(~C) & pos.get(knight));
		meta.atk |= king_attacks    (pos.get(~C) & pos.get(king  ));
		while (bq) meta.atk |= magic_bishop[pop(bq)].get(oc & ~to_bb(ki));
		while (rq) meta.atk |= magic_rook  [pop(rq)].get(oc & ~to_bb(ki));

		return meta;
	}

	template<bool G, color C>
	move* generate(const position& pos, move* list) {
		pos_meta meta = gen_meta<C>(pos);
		bitboard oc = pos.get(C    ) | pos.get(~C    );
		bitboard bq = pos.get(queen) | pos.get(bishop);
		bitboard rq = pos.get(queen) | pos.get(rook  );
		square ki = lsb(pos.get(C) & pos.get(king));

		constexpr direction up = to_dir(C);
		constexpr bitboard r3 = to_bb(to_rank(C, rank_3));
		bitboard bb, atk, tmp;
		square sq, s1;

		if (!meta.chk) {
			constexpr bitboard kc = C == white ? 0x60ULL : 0x60ULL << 56;
			constexpr bitboard qc = C == white ? 0x0EULL : 0x0EULL << 56;
			constexpr bitboard ka = C == white ? 0x70ULL : 0x70ULL << 56;
			constexpr bitboard qa = C == white ? 0x1CULL : 0x1CULL << 56;
			if ((pos.rights(C) & any_oo ) && !(kc & oc) && !(ka & meta.atk))
				list = add<G, 0>(list, to_sq(C, e1), to_sq(C, g1));
			if ((pos.rights(C) & any_ooo) && !(qc & oc) && !(qa & meta.atk))
				list = add<G, 0>(list, to_sq(C, e1), to_sq(C, c1));
			
			bb = pos.get(pawn) & pos.get(C) & meta.pin;
			while (bb) {
				atk  = shift<up>(bb & -bb) & ~oc;
				atk |= shift<up>(r3 & atk) & ~oc;
				sq = pop(bb);
				atk |= attack_pawn[C] [sq] & pos.get(~C);
				atk &= attack_line[ki][sq] & meta.tgt;
				list = add_pawn<G>(list, atk, sq);
			}

			bb = bq & pos.get(C) & meta.pin;
			while (bb) {
				sq = pop(bb);
				atk = magic_bishop[sq].get(oc) & attack_line[sq][ki];
				list = add<G, 0>(list, atk & meta.tgt, sq);
			}
			bb = rq & pos.get(C) & meta.pin;
			while (bb) {
				sq = pop(bb);
				atk = magic_rook  [sq].get(oc) & attack_line[sq][ki];
				list = add<G, 0>(list, atk & meta.tgt, sq);
			}
		}

		atk = attack_king[ki] & ~meta.atk & ~pos.get(C);
		list = add<G, 0>(list, atk, ki);
		if (meta.chk & (meta.chk - 1)) return list;
		bb = pos.get(knight) & pos.get(C) & ~meta.pin;
		while (bb) {
			sq = pop(bb);
			atk = attack_knight[sq] & meta.tgt;
			list = add<G, 0>(list, atk, sq);
		}

		bb = pos.get(pawn) & pos.get(C) & ~meta.pin;
		atk = shift<up       >(bb      ) & ~oc;
		list = add_pawn<G, up        >(list, atk & meta.tgt);
		atk = shift<up       >(r3 & atk) & ~oc;
		list = add     <G, up + up, 0>(list, atk & meta.tgt);
		atk = shift<up + east>(bb      ) & pos.get(~C);
		list = add_pawn<G, up + east >(list, atk & meta.tgt);
		atk = shift<up + west>(bb      ) & pos.get(~C);
		list = add_pawn<G, up + west >(list, atk & meta.tgt);

		sq = pos.en_pas();
		if (sq != no_square && (!meta.chk || lsb(meta.chk) == sq - up)) {
			bb = pos.get(pawn) & pos.get(C) & attack_pawn[~C][sq];
			while (bb) {
				s1 = pop(bb);
				tmp = oc ^ to_bb(sq) ^ to_bb(s1) ^ to_bb(sq - up);
				atk  = magic_bishop[ki].get(tmp) & bq & pos.get(~C);
				atk |= magic_rook  [ki].get(tmp) & rq & pos.get(~C);
				if (!atk) list = add<G, 0>(list, s1, sq);
			}
		}

		bb = bq & pos.get(C) & ~meta.pin;
		while (bb) {
			sq = pop(bb);
			atk = magic_bishop[sq].get(oc);
			list = add<G, 0>(list, atk & meta.tgt, sq);
		}
		bb = rq & pos.get(C) & ~meta.pin;
		while (bb) {
			sq = pop(bb);
			atk = magic_rook  [sq].get(oc);
			list = add<G, 0>(list, atk & meta.tgt, sq);
		}

		return list;
	}

	template move* generate<0, white>(const position&, move*);
	template move* generate<0, black>(const position&, move*);
	template move* generate<1, white>(const position&, move*);
	template move* generate<1, black>(const position&, move*);
}