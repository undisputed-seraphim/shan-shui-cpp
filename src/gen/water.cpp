#include "gen/water.h"
#include "core/noise.h"
#include "draw/shapes.h"

namespace ss {

void water(Painter& p, double xoff, double yoff, double seed, double hei, double len, int clu) {
	std::vector<Pts> ptlist;
	double yk = 0;
	for (int i = 0; i < clu; i++) {
		Pts row;
		double xk = (rnd() - 0.5) * (len / 8);
		yk += rnd() * 5;
		double lk = len / 4 + rnd() * (len / 4);
		const double reso = 5;
		for (double j = -lk; j < lk; j += reso) {
			row.push_back({j + xk, std::sin(j * 0.2) * hei * nse(j * 0.1) - 20 + yk});
		}
		ptlist.push_back(std::move(row));
	}
	for (size_t j = 1; j < ptlist.size(); j++) {
		SArg sa;
		sa.col = "rgba(100,100,100," + toFixed(0.3 + rnd() * 0.3, 3) + ")";
		sa.wid = 1;
		Pts shifted;
		for (const auto& v : ptlist[j])
			shifted.push_back({v[0] + xoff, v[1] + yoff});
		stroke(p, shifted, sa);
	}
}

} // namespace ss
