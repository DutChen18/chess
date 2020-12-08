#include "search.hh"

#include <vector>
#include <cinttypes>
#include <cstring>
#include <ctime>

static const std::pair<const char*, std::vector<uint64_t>> tests[] = {
{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
	{ 20, 400, 8902, 197281, 4865609, 119060324, 3195901860 } },
{ "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
	{ 48, 2039, 97862, 4085603, 193690690, 8031647685 } },
{ "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
	{ 14, 191, 2812, 43238, 674624, 11030083, 178633661, 3009794393 } },
{ "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
	{ 6, 264, 9467, 422333, 15833292, 706045033 } },
{ "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",
	{ 6, 264, 9467, 422333, 15833292, 706045033 } },
{ "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
	{ 44, 1486, 62379, 2103487, 89941194 } },
{ "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
	{ 46, 2079, 89890, 3894594, 164075551, 6923051137 } },
};

namespace chess {
	uint64_t perft(position& pos, size_t depth) {
		uint64_t count = 0;
		move list[218];
		move* end = generate<1>(pos, list);
		for (chess::move* i = list; i < end; ++i) {
			pos_info info;
			pos.do_move<0>(*i, &info);
			if (depth == 1) {
				count += 1;
			} else if (depth == 2) {
				count += generate<0>(pos, list) - list;
			} else {
				count += perft(pos, depth - 1);
			}
			pos.un_move(*i);
		}
		return count;
	}

	void test(size_t max) {
		uint64_t total = 0, count;
		double time = 0, delta;

		for (auto& test : tests) {
			pos_info info;
			position pos;
			pos.load(test.first, &info);
			size_t depth = std::min(max, test.second.size());

			clock_t start = clock();
			total += count = perft(pos, depth);
			time += delta = double(clock() - start) / CLOCKS_PER_SEC;
			
			if (count == test.second[depth - 1]) {
				double mnps = count / delta / 1000000;
				printf("ok %5.2lfs %4.0lfm", delta, mnps);
			} else {
				printf("err %11" PRIu64, count);
			}
			printf(" %2zdd %s\n", depth, test.first);
		}

		double mnps = total / time / 1000000;
		printf("%8.2lfs %4.0lfm\n", time, mnps);
	}
}