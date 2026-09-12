#include "draw/shapes.h"
#include "core/noise.h"
#include <algorithm>

namespace ss {

void stroke(Painter& p, const Pts& ptlist, const SArg& a) {
	if (ptlist.empty())
		return;

	Pts vtxlist0, vtxlist1;
	double n0 = rnd() * 10;
	for (size_t i = 1; i + 1 < ptlist.size(); i++) {
		double w = a.wid * a.fun((double)i / ptlist.size());
		w = std::lerp(w, w * nse(i * 0.5, n0), a.noi);
		double a1 = std::atan2(ptlist[i][1] - ptlist[i - 1][1], ptlist[i][0] - ptlist[i - 1][0]);
		double a2 = std::atan2(ptlist[i][1] - ptlist[i + 1][1], ptlist[i][0] - ptlist[i + 1][0]);
		double ang = (a1 + a2) / 2;
		if (ang < a2)
			ang += pi;
		vtxlist0.push_back({ptlist[i][0] + w * std::cos(ang), ptlist[i][1] + w * std::sin(ang)});
		vtxlist1.push_back({ptlist[i][0] - w * std::cos(ang), ptlist[i][1] - w * std::sin(ang)});
	}

	Pts vtxlist;
	vtxlist.push_back(ptlist[0]);
	vtxlist.insert(vtxlist.end(), vtxlist0.begin(), vtxlist0.end());
	// vtxlist1 + [last] reversed
	vtxlist.push_back(ptlist[ptlist.size() - 1]);
	vtxlist.insert(vtxlist.end(), vtxlist1.rbegin(), vtxlist1.rend());
	vtxlist.push_back(ptlist[0]);

	p.poly(offset(vtxlist, a.xof, a.yof), PArg{.fil = a.col, .str = a.col, .wid = a.out});
}

Pts blob_points(double x, double y, const BArg& a) {
	const double reso = 20.0;
	std::vector<std::array<double, 2>> lalist;
	for (int i = 0; i < (int)reso + 1; i++) {
		double pp = ((double)i / reso) * 2;
		double xo = a.len / 2 - std::fabs(pp - 1) * a.len;
		double yo = (a.fun(pp) * a.wid) / 2;
		double ang = std::atan2(yo, xo);
		double l = std::sqrt(xo * xo + yo * yo);
		lalist.push_back({l, ang});
	}
	std::vector<double> nslist;
	double n0 = rnd() * 10;
	for (int i = 0; i < (int)reso + 1; i++) {
		nslist.push_back(nse(i * 0.05, n0));
	}
	seamless_noise(nslist);
	Pts plist;
	plist.reserve(lalist.size());
	for (size_t i = 0; i < lalist.size(); i++) {
		double ns = std::lerp(1.0, nslist[i], a.noi);
		double nx = x + std::cos(lalist[i][1] + a.ang) * lalist[i][0] * ns;
		double ny = y + std::sin(lalist[i][1] + a.ang) * lalist[i][0] * ns;
		plist.push_back({nx, ny});
	}
	return plist;
}

void blob(Painter& p, double x, double y, const BArg& a) {
	Pts plist = blob_points(x, y, a);
	if (a.ret == 0) {
		p.poly(plist, PArg{.fil = a.col, .str = a.col, .wid = 0});
	}
}

Pts subdivide(const Pts& plist, double reso) {
	double tl = ((double)plist.size() - 1) * reso; // JS: float bound (negative if empty)
	double lx = 0, ly = 0;
	Pts rlist;
	for (int i = 0; i < tl; i++) {
		const Pt& lastp = plist[(size_t)std::floor(i / reso)];
		const Pt& nextp = plist[(size_t)std::ceil(i / reso)];
		double pp = std::fmod((double)i, reso) / reso;
		double nx = std::lerp(lastp[0], nextp[0], pp);
		double ny = std::lerp(lastp[1], nextp[1], pp);
		rlist.push_back({nx, ny});
		lx = nx;
		ly = ny;
	}
	if (!plist.empty())
		rlist.push_back(plist[plist.size() - 1]);
	return rlist;
}

void texture(Painter& p, const std::vector<Pts>& ptlist, const TArg2& a) {
	int reso1 = (int)ptlist.size();
	int reso2 = (int)ptlist[0].size();
	std::vector<Pts> texlist;
	for (int i = 0; i < a.tex; i++) {
		int mid = (int)(a.dis() * reso2);
		int hlen = (int)std::floor(rnd() * (reso2 * a.len));
		int start = mid - hlen;
		int end = mid + hlen;
		start = std::clamp(start, 0, reso2);
		end = std::clamp(end, 0, reso2);
		double layer = ((double)i / a.tex) * (reso1 - 1);
		Pts row;
		for (int j = start; j < end; j++) {
			double pp = layer - std::floor(layer);
			double x = std::lerp(ptlist[(size_t)std::ceil(layer)][j][0], ptlist[(size_t)std::floor(layer)][j][0], pp);
			double y = std::lerp(ptlist[(size_t)std::ceil(layer)][j][1], ptlist[(size_t)std::floor(layer)][j][1], pp);
			double ns0 = a.noi(layer + 1) * (nse(x, j * 0.5) - 0.5);
			double ns1 = a.noi(layer + 1) * (nse(y, j * 0.5) - 0.5);
			row.push_back({x + ns0, y + ns1});
		}
		texlist.push_back(std::move(row));
	}

	// SHADE
	if (a.sha) {
		for (size_t j = 0; j < texlist.size(); j += 1 + (a.sha != 0)) {
			SArg sa;
			sa.col = "rgba(100,100,100,0.1)";
			sa.wid = a.sha;
			sa.xof = a.xof;
			sa.yof = a.yof;
			stroke(p, texlist[j], sa);
		}
	}
	// TEXTURE
	for (size_t j = 0 + (size_t)a.sha; j < texlist.size(); j += 1 + (size_t)a.sha) {
		SArg sa;
		sa.col = a.col((double)j / texlist.size());
		sa.wid = a.wid;
		sa.xof = a.xof;
		sa.yof = a.yof;
		stroke(p, texlist[j], sa);
	}
}

} // namespace ss
