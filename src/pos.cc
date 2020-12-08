#include "pos.hh"

namespace chess {
	const std::string chars = "PpNnBbRrQqKk-";

	void position::copy(position& pos) const {
		pos.info = info;
		pos.ply = ply;
		std::copy(&board[0], &board[64], pos.board);
		std::copy(&color_bb[0], &color_bb[2], pos.color_bb);
		std::copy(&type_bb [0], &type_bb [6], pos.type_bb );
	}

	size_t position::load(const char* fen, pos_info* info) {
		const char* start = fen;
		this->info = info;
		info->prev = nullptr;
		info->hash = 0;
		info->rights = 0;
		std::fill(&board   [0], &board   [64], no_piece);
		std::fill(&color_bb[0], &color_bb[ 2], 0);
		std::fill(&type_bb [0], &type_bb [ 6], 0);

		for (square sq = a1; *fen != ' '; ++fen) {
			if (isdigit(*fen)) {
				sq = sq + direction(*fen - '0');
			} else if (*fen != '/') {
				piece pc = piece(chars.find(*fen));
				board[~sq] = pc;
				color_bb[color_of(pc)] ^= to_bb(~sq);
				type_bb [type_of (pc)] ^= to_bb(~sq);
				info->hash ^= zobrist_piece[pc][~sq];
				++sq;
			}
		}

		ply = *((fen += 3) - 2) == 'b';
		if (*fen   == '-') info->rights  = 0        , ++fen;
		if (*fen   == 'K') info->rights |= white_oo , ++fen;
		if (*fen   == 'Q') info->rights |= white_ooo, ++fen;
		if (*fen   == 'k') info->rights |= black_oo , ++fen;
		if (*fen++ == 'q') info->rights |= black_ooo, ++fen;
		info->hash ^= zobrist_rights[info->rights];

		info->en_pas = no_square;
		if (*fen != '-') {
			file f = file(*fen++ - 'a');
			rank r = rank(*fen   - '1');
			info->en_pas = to_sq(f, r);
		}
		info->hash ^= zobrist_en_pas[info->en_pas];

		long long size, half, full;
		sscanf(fen + 1, " %lld %lld%lln", &half, &full, &size);
		info->clock = half;
		ply += 2 * full - 2;
		if (ply & 1) info->hash ^= zobrist_side;
		return fen + 1 + size - start;
	}

	size_t position::dump(char* fen) const {
		const char* start = fen;

		for (rank r = rank_1; r < no_rank; ++r) {
			for (file f = file_a; f < no_file; ++f) {
				int empty = 0;
				piece pc = no_piece;
				for (; f < no_file; ++empty, ++f) {
					pc = board[to_sq(f, ~r)];
					if (pc != no_piece) break;
				}
				if (empty != 0)  *fen++ = '0' + empty;
				if (f < no_file) *fen++ = chars[pc];
			}
			*fen++ = r == rank_8 ? ' ' : '/';
		}

		*fen++ = ply & 1 ? 'b' : 'w';
		*fen++ = ' ';
		if (info->rights & white_oo ) *fen++ = 'K';
		if (info->rights & white_ooo) *fen++ = 'Q';
		if (info->rights & black_oo ) *fen++ = 'k';
		if (info->rights & black_ooo) *fen++ = 'q';
		if (info->rights == 0)        *fen++ = '-';

		*fen++ = ' ';
		*fen   = '-';
		if (info->en_pas != no_square) {
			*fen++ = 'a' + file_of(info->en_pas);
			*fen   = '1' + rank_of(info->en_pas);
		}

		long long size, half, full;
		half = info->clock;
		full = ply / 2 + 1;
		sprintf(fen + 1, " %lld %lld%lln%c", half, full, &size, 0);
		return fen + 1 + size - start;
	}

	std::ostream& position::write(std::ostream& os) const {
		for (rank r = rank_1; r < no_rank; ++r) {
			for (file f = file_a; f < no_file; ++f) {
				square sq = to_sq(f, ~r);
				if (f != file_a) os << " ";
				os << chars[get(sq)];
			}
			os << std::endl;
		}
		return os;
	}
}