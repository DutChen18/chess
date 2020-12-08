#include "pos.hh"

namespace chess {
	template<bool H>
	inline void mv_pc(position& pos, square s1, square s2, piece pc) {
		pos.board[s1] = no_piece;
		pos.board[s2] = pc;
		pos.type_bb [type_of (pc)] ^= to_bb(s1) | to_bb(s2);
		pos.color_bb[color_of(pc)] ^= to_bb(s1) | to_bb(s2);
		if (H) pos.info->hash ^= zobrist_piece[pc][s1];
		if (H) pos.info->hash ^= zobrist_piece[pc][s2];
	}

	template<color C, bool H>
	void position::do_move(move mv, pos_info* info) {
		square   sq_fr = from_of(mv   ), sq_to = dest_of(mv   );
		piece    pc_fr = get    (sq_fr), pc_to = get    (sq_to);
		type     ty_fr = type_of(pc_fr), ty_to = type_of(pc_to);

		pos_info* prev = this->info;
		info->prev = prev;
		this->info = info;
		ply += 1;

		info->en_pas = no_square;
		info->captured = pc_to;
		info->rights = prev->rights;
		info->rights &= rights_mask[sq_fr];
		info->rights &= rights_mask[sq_to];
		if (H) info->hash = prev->hash ^ zobrist_side;
		if (H) info->hash ^= zobrist_en_pas[prev->en_pas];
		if (H) info->hash ^= zobrist_rights[prev->rights];
		if (H) info->hash ^= zobrist_rights[info->rights];
		if (H) info->clock = prev->clock + 1;

		mv_pc<H>(*this, sq_fr, sq_to, pc_fr);

		if (pc_to != no_piece) {
			type_bb [ty_to] ^= to_bb(sq_to);
			color_bb[~C   ] ^= to_bb(sq_to);
			if (H) info->hash ^= zobrist_piece[pc_to][sq_to];
		}

		if (ty_fr == pawn) {
			type  ty_pr = promo_of(mv);
			piece pc_pr = to_pc(C, ty_pr);
			if (ty_pr != no_type) {
				board[sq_to] = pc_pr;
				type_bb[pawn ] ^= to_bb(sq_to);
				type_bb[ty_pr] ^= to_bb(sq_to);
				if (H) info->hash ^= zobrist_piece[pc_fr][sq_to];
				if (H) info->hash ^= zobrist_piece[pc_pr][sq_to];
			} else if (sq_to == prev->en_pas) {
				sq_fr = prev->en_pas - to_dir(C);
				board[sq_fr] = no_piece;
				type_bb [pawn] ^= to_bb(sq_fr);
				color_bb[  ~C] ^= to_bb(sq_fr);
				if (H) info->hash ^= zobrist_piece[to_pc(~C, pawn)][sq_fr];
			} else if (int(sq_to) - sq_fr == to_dir(C) * 2) {
				info->en_pas = square((sq_to + direction(sq_fr)) / 2);
				if (H) info->hash ^= zobrist_en_pas[info->en_pas];
			}
		} else if (ty_fr == king) {
			if (sq_to == sq_fr + east + east)
				mv_pc<H>(*this, to_sq(C, h1), to_sq(C, f1), to_pc(C, rook));
			if (sq_to == sq_fr + west + west)
				mv_pc<H>(*this, to_sq(C, a1), to_sq(C, d1), to_pc(C, rook));
		}
	}

	template<color C>
	void position::un_move(move mv) {
		square   sq_fr = from_of(mv   ), sq_to = dest_of(mv   );
		piece    pc_fr = get    (sq_to), pc_to = info->captured;
		type     ty_fr = type_of(pc_fr), ty_to = type_of(pc_to);

		pos_info* info = this->info;
		pos_info* prev = info->prev;
		this->info = prev;
		ply -= 1;

		board[sq_to] = pc_to;
		type_bb [ty_fr] ^= to_bb(sq_to);
		if (promo_of(mv) != no_type) {
			ty_fr = pawn;
			pc_fr = to_pc(C, pawn);
		}
		board[sq_fr] = pc_fr;
		type_bb [ty_fr] ^= to_bb(sq_fr);
		color_bb[C    ] ^= to_bb(sq_fr) | to_bb(sq_to);

		if (pc_to != no_piece) {
			type_bb [ty_to] ^= to_bb(sq_to);
			color_bb[~C   ] ^= to_bb(sq_to);
		}

		if (ty_fr == pawn) {
			if (sq_to == prev->en_pas) {
				sq_fr = prev->en_pas - to_dir(C);
				board[sq_fr] = to_pc(~C, pawn);
				type_bb [pawn] ^= to_bb(sq_fr);
				color_bb[  ~C] ^= to_bb(sq_fr);
			}
		} else if (ty_fr == king) {
			if (sq_to == sq_fr + east + east)
				mv_pc<0>(*this, to_sq(C, f1), to_sq(C, h1), to_pc(C, rook));
			if (sq_to == sq_fr + west + west)
				mv_pc<0>(*this, to_sq(C, d1), to_sq(C, a1), to_pc(C, rook));
		}
	}

	template void position::do_move<white, false>(move, pos_info*);
	template void position::do_move<black, false>(move, pos_info*);
	template void position::do_move<white, true >(move, pos_info*);
	template void position::do_move<black, true >(move, pos_info*);
	template void position::un_move<white>(move);
	template void position::un_move<black>(move);
}