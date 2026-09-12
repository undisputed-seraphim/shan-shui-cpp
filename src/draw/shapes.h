#pragma once
#include "core/noise.h"
#include "core/poly.h"
#include "core/scratch.h"
#include "draw/painter.h"
#include <algorithm>

namespace ss {

inline constexpr auto kDefaultStrokeFun = [](double x) { return std::sin(x * pi); };
inline constexpr auto kDefaultBlobFun = [](double x) {
	return x <= 1 ? std::pow(std::sin(x * pi), 0.5) : -std::pow(std::sin((x + 1) * pi), 0.5);
};
inline constexpr auto kDefaultTexNoi = [](double x) { return 30.0 / x; };
inline constexpr auto kDefaultTexCol = [](double) { return Color{100, 100, 100, rnd() * 0.3}.rgba(); };
inline constexpr auto kDefaultTexDis = []() {
	if (rnd() > 0.5)
		return (1.0 / 3) * rnd();
	return (1.0 * 2) / 3 + (1.0 / 3) * rnd();
};

struct SArg {
	double xof = 0, yof = 0;
	double wid = 2;
	std::string col = "rgba(200,200,200,0.9)";
	double noi = 0.5;
	double out = 1;
};

struct BArg {
	double len = 20, wid = 5, ang = 0;
	std::string col = "rgba(200,200,200,0.9)";
	double noi = 0.5;
	int ret = 0;
};

struct TArg2 { // texture args
	double xof = 0, yof = 0;
	int tex = 400;
	double wid = 1.5;
	double len = 0.2;
	double sha = 0;
	int ret = 0;
};

template <typename PtsT, typename Fun = decltype(kDefaultStrokeFun)>
void stroke(Painter& p, const PtsT& ptlist, const SArg& a = {}, const Fun& fun = kDefaultStrokeFun) {
	if (ptlist.empty())
		return;

	ScratchPts vtxlist0, vtxlist1;
	vtxlist0.reserve(ptlist.size());
	vtxlist1.reserve(ptlist.size());
	double n0 = rnd() * 10;
	for (size_t i = 1; i + 1 < ptlist.size(); i++) {
		double w = a.wid * fun((double)i / ptlist.size());
		w = std::lerp(w, w * nse(i * 0.5, n0), a.noi);
		double a1 = std::atan2(ptlist[i][1] - ptlist[i - 1][1], ptlist[i][0] - ptlist[i - 1][0]);
		double a2 = std::atan2(ptlist[i][1] - ptlist[i + 1][1], ptlist[i][0] - ptlist[i + 1][0]);
		double ang = (a1 + a2) / 2;
		if (ang < a2)
			ang += pi;
		vtxlist0.push_back({ptlist[i][0] + w * std::cos(ang), ptlist[i][1] + w * std::sin(ang)});
		vtxlist1.push_back({ptlist[i][0] - w * std::cos(ang), ptlist[i][1] - w * std::sin(ang)});
	}

	ScratchPts vtxlist;
	vtxlist.reserve(ptlist.size() * 2 + 2);
	vtxlist.push_back(ptlist[0]);
	vtxlist.insert(vtxlist.end(), vtxlist0.begin(), vtxlist0.end());
	// vtxlist1 + [last] reversed
	vtxlist.push_back(ptlist[ptlist.size() - 1]);
	vtxlist.insert(vtxlist.end(), vtxlist1.rbegin(), vtxlist1.rend());
	vtxlist.push_back(ptlist[0]);

	p.poly(vtxlist, PArg{.xof = a.xof, .yof = a.yof, .fil = a.col, .str = a.col, .wid = a.out});
}

template <typename Fun = decltype(kDefaultBlobFun)>
Pts blob_points(double x, double y, const BArg& a, const Fun& fun = kDefaultBlobFun) {
	const double reso = 20.0;
	std::vector<std::array<double, 2>> lalist;
	lalist.reserve((size_t)reso + 1);
	for (int i = 0; i < (int)reso + 1; i++) {
		double pp = ((double)i / reso) * 2;
		double xo = a.len / 2 - std::fabs(pp - 1) * a.len;
		double yo = (fun(pp) * a.wid) / 2;
		double ang = std::atan2(yo, xo);
		double l = std::sqrt(xo * xo + yo * yo);
		lalist.push_back({l, ang});
	}
	std::vector<double> nslist;
	nslist.reserve((size_t)reso + 1);
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

template <typename Fun = decltype(kDefaultBlobFun)>
void blob(Painter& p, double x, double y, const BArg& a, const Fun& fun = kDefaultBlobFun) {
	Pts plist = blob_points(x, y, a, fun);
	if (a.ret == 0) {
		p.poly(plist, PArg{.fil = a.col, .str = a.col, .wid = 0});
	}
}

template <typename PtsT = Pts>
inline ScratchPts subdivide(const PtsT& plist, double reso) {
	double tl = ((double)plist.size() - 1) * reso; // float bound (negative if empty)
	double lx = 0, ly = 0;
	ScratchPts rlist;
	rlist.reserve((size_t)std::max((int)tl, 0) + 1);
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

template <
	typename PtsList,
	typename Noi = decltype(kDefaultTexNoi),
	typename Col = decltype(kDefaultTexCol),
	typename Dis = decltype(kDefaultTexDis)>
void texture(
	Painter& p,
	const PtsList& ptlist,
	const TArg2& a = {},
	const Noi& noi = kDefaultTexNoi,
	const Col& col = kDefaultTexCol,
	const Dis& dis = kDefaultTexDis) {
	int reso1 = (int)ptlist.size();
	int reso2 = (int)ptlist[0].size();
	ScratchPtsList texlist;
	texlist.reserve(a.tex);
	for (int i = 0; i < a.tex; i++) {
		int mid = (int)(dis() * reso2);
		int hlen = (int)std::floor(rnd() * (reso2 * a.len));
		int start = mid - hlen;
		int end = mid + hlen;
		start = std::clamp(start, 0, reso2);
		end = std::clamp(end, 0, reso2);
		double layer = ((double)i / a.tex) * (reso1 - 1);
		ScratchPts row;
		row.reserve(end - start);
		for (int j = start; j < end; j++) {
			double pp = layer - std::floor(layer);
			double x = std::lerp(ptlist[(size_t)std::ceil(layer)][j][0], ptlist[(size_t)std::floor(layer)][j][0], pp);
			double y = std::lerp(ptlist[(size_t)std::ceil(layer)][j][1], ptlist[(size_t)std::floor(layer)][j][1], pp);
			double ns0 = noi(layer + 1) * (nse(x, j * 0.5) - 0.5);
			double ns1 = noi(layer + 1) * (nse(y, j * 0.5) - 0.5);
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
		sa.col = col((double)j / texlist.size());
		sa.wid = a.wid;
		sa.xof = a.xof;
		sa.yof = a.yof;
		stroke(p, texlist[j], sa);
	}
}

} // namespace ss
