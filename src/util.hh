#ifndef CHESS_UTIL_HH
#define CHESS_UTIL_HH

#include "def.hh"

#include <cstddef>
#include <ostream>

#define INTRINSICS 1

#if defined(__BMI2__) && INTRINSICS
# include <immintrin.h>
#endif

namespace chess {
	struct magic {
		bitboard* attack;
		bitboard mask;
		bitboard magic;
		unsigned shift;

		inline size_t index(bitboard occ) const {
#if defined(__BMI2__) && INTRINSICS
			return _pext_u64(occ, mask);
#else
			return (occ & mask) * magic >> shift;
#endif
		}

		inline bitboard get(bitboard occ = 0) const {
			return attack[index(occ)];
		}
	};

	constexpr bitboard k1_ = 0x5555555555555555ULL;
	constexpr bitboard k2_ = 0x3333333333333333ULL;
	constexpr bitboard k4_ = 0x0F0F0F0F0F0F0F0FULL;
	constexpr bitboard kf_ = 0x0101010101010101ULL;

	constexpr uint8_t lsb_[64] = {
		 0,  1, 48,  2, 57, 49, 28,  3,
		61, 58, 50, 42, 38, 29, 17,  4,
		62, 55, 59, 36, 53, 51, 43, 22,
		45, 39, 33, 30, 24, 18, 12,  5,
		63, 47, 56, 27, 60, 41, 37, 16,
		54, 35, 52, 21, 44, 32, 23, 11,
		46, 26, 40, 15, 34, 20, 31, 10,
		25, 14, 19,  9, 13,  8,  7,  6,
	};

	extern castling rights_mask   [64];
	extern bitboard attack_pawn   [ 2][64];
	extern bitboard attack_knight [64];
	extern bitboard attack_king   [64];
	extern bitboard attack_line   [64][64];
	extern bitboard magic_table   [107648];
	extern magic    magic_rook    [64];
	extern magic    magic_bishop  [64];
	extern key      zobrist_piece [12][64];
	extern key      zobrist_rights[16];
	extern key      zobrist_en_pas[65];
	extern key      zobrist_side;

	constexpr bitboard to_bb(square sq) { return 1ULL    << sq   ; }
	constexpr bitboard to_bb(file   f ) { return kf_     << f    ; }
	constexpr bitboard to_bb(rank   r ) { return 0xFFULL << r * 8; }

	template<direction D> constexpr bitboard rotate(bitboard bb)
		{ return D > 0 ? bb << D : bb >> -D; }
	template<direction D> constexpr bitboard mask()
		{ return kf_ * (rotate<(D + 68) % 8 - 4>(0xFF) & 0xFF); }
	template<direction D> constexpr bitboard shift(bitboard bb)
		{ return rotate<D>(bb) & mask<D>(); }

	template<direction D>
	inline bitboard ray(bitboard gen, bitboard pro) {
		pro &= mask<D>();
		gen |= pro & rotate<D * 1>(gen);
		pro &=       rotate<D * 1>(pro);
		gen |= pro & rotate<D * 2>(gen);
		pro &=       rotate<D * 2>(pro);
		gen |= pro & rotate<D * 4>(gen);
		return       shift <D * 1>(gen);
	}

	inline uint64_t random(uint64_t& s) {
		s ^= s >> 12, s ^= s << 25, s ^= s >> 27;
		return s * 2685821657736338717ULL;
	}

	inline int cnt(bitboard bb) {
#if defined(__GNUC__) && INTRINSICS
		return __builtin_popcountll(bb);
#elif (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && INTRINSICS
		return _mm_popcnt_u64(bb);
#else
		bb =   (bb      ) - (bb >> 1 & k1_);
		bb =   (bb & k2_) + (bb >> 2 & k2_);
		bb =   (bb      ) + (bb >> 4      );
		return (bb & k4_) * kf_ >> 56;
#endif
	}

	inline square lsb(bitboard bb) {
#if defined(__GNUC__) && INTRINSICS
		return square(__builtin_ctzll(bb));
#elif (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && INTRINSICS
		unsigned long idx;
		_BitScanForward64(&idx, bb);
		return square(idx);
#else
		constexpr uint64_t db = 0x03f79d71b4cb0a89ULL;
		return square(lsb_[(bb & -bb) * db >> 58]);
#endif
	}

	inline square pop(bitboard& bb) {
		square result = lsb(bb);
		bb &= bb - 1;
		return result;
	}

	std::ostream& write_bitboard(std::ostream& os, bitboard bb);
	std::ostream& write_move    (std::ostream& os, move mv);
}

#endif