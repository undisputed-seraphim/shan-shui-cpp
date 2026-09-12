#include "gen/mount.h"
#include "core/noise.h"
#include "core/rng.h"
#include "draw/shapes.h"
#include "gen/arch.h"
#include "gen/tree.h"
#include <algorithm>
#include <cmath>
#include <format>

namespace ss {

namespace {

// Mount.foot - folds at the base of a mountain
void foot(Painter& p, const ScratchPtsList& ptlist, double xof, double yof) {
	ScratchPtsList ftlist;
	const double span = 10;
	int ni = 0;
	for (size_t i = 0; i + 2 < ptlist.size(); i++) {
		if ((int)i == ni) {
			ni = std::min(ni + (int)randChoice<double>({1.0, 2.0}), (int)ptlist.size() - 1);
			ftlist.push_back({});
			ftlist.push_back({});
			// JS: for (j = 0; j < min(ptlist[i].length/8, 10); j++) - float bound!
			double jmax = std::min(ptlist[i].size() / 8.0, 10.0);
			for (double j = 0; j < jmax; j++) {
				ftlist[ftlist.size() - 2].push_back({ptlist[i][j][0] + nse(j * 0.1, (double)i) * 10, ptlist[i][j][1]});
				ftlist[ftlist.size() - 1].push_back(
					{ptlist[i][ptlist[i].size() - 1 - j][0] - nse(j * 0.1, (double)i) * 10,
					 ptlist[i][ptlist[i].size() - 1 - j][1]});
			}
			std::reverse(ftlist[ftlist.size() - 2].begin(), ftlist[ftlist.size() - 2].end());
			std::reverse(ftlist[ftlist.size() - 1].begin(), ftlist[ftlist.size() - 1].end());
			for (int j = 0; j < (int)span; j++) {
				double pp = j / span;
				double x1 = std::lerp(ptlist[i][0][0], ptlist[ni][0][0], pp);
				double y1 = std::lerp(ptlist[i][0][1], ptlist[ni][0][1], pp);
				double x2 = std::lerp(ptlist[i][ptlist[i].size() - 1][0], ptlist[ni][ptlist[i].size() - 1][0], pp);
				double y2 = std::lerp(ptlist[i][ptlist[i].size() - 1][1], ptlist[ni][ptlist[i].size() - 1][1], pp);
				double vib = -1.7 * (pp - 1) * std::pow(pp, 1.0 / 5);
				y1 += vib * 5 + nse(xof * 0.05, (double)i) * 5;
				y2 += vib * 5 + nse(xof * 0.05, (double)i) * 5;
				ftlist[ftlist.size() - 2].push_back({x1, y1});
				ftlist[ftlist.size() - 1].push_back({x2, y2});
			}
		}
	}
	for (const auto& row : ftlist) {
		p.poly(row, PArg{.xof = xof, .yof = yof, .fil = "white", .str = "none"});
	}
	for (const auto& row : ftlist) {
		SArg sa;
		sa.col = Color{100, 100, 100, 0.1 + rnd() * 0.1}.rgba();
		sa.wid = 1;
		sa.xof = xof;
		sa.yof = yof;
		stroke(p, row, sa);
	}
}

} // namespace

void Mount::mountain(Painter& p, double xoff, double yoff, double seed, const MountainArg& a) {
	double hei = a.hei >= 0 ? a.hei : 100 + rnd() * 400;
	double wid = a.wid >= 0 ? a.wid : 400 + rnd() * 200;
	int tex = a.tex;
	bool veg = a.veg;
	int ret = a.ret;

	ScratchPtsList ptlist;
	double h = hei;
	double w = wid;
	const int reso0 = 10, reso1 = 50;
	ptlist.reserve(reso0);

	double hoff = 0;
	for (int j = 0; j < reso0; j++) {
		hoff += (rnd() * yoff) / 100;
		ScratchPts row;
		row.reserve(reso1);
		for (int i = 0; i < reso1; i++) {
			double x = ((double)i / reso1 - 0.5) * pi;
			double y = std::cos(x);
			y *= nse(x + 10, j * 0.15, seed);
			double pp = 1 - (double)j / reso0;
			row.push_back({(x / pi) * w * pp, -y * h * pp + hoff});
		}
		ptlist.push_back(std::move(row));
	}

	auto vegetate = [&](const auto& treeFunc, const auto& growthRule, const auto& proofRule) {
		Pts veglist;
		veglist.reserve(ptlist.size() * ptlist[0].size());
		for (size_t i = 0; i < ptlist.size(); i++) {
			for (size_t j = 0; j < ptlist[i].size(); j++) {
				if (growthRule((int)i, (int)j)) {
					veglist.push_back({ptlist[i][j][0], ptlist[i][j][1]});
				}
			}
		}
		for (size_t i = 0; i < veglist.size(); i++) {
			if (proofRule(veglist, (int)i)) {
				treeFunc(veglist[i][0], veglist[i][1]);
			}
		}
	};

	// RIM
	vegetate(
		[&](double x, double y) {
			Color col{100, 100, 100, nse(0.01 * x, 0.01 * y) * 0.5 * 0.3 + 0.5};
			Tree::tree02(p, x + xoff, y + yoff - 5, 16, 8, 2, col, 0.5);
		},
		[&](int i, int j) {
			double ns = nse(j * 0.1, seed);
			return i == 0 && ns * ns * ns < 0.1 && std::fabs(ptlist[i][j][1]) / h > 0.2;
		},
		[](const Pts&, int) { return true; });

	// WHITE BG
	ScratchPts bg = ptlist[0];
	bg.push_back({0, reso0 * 4});
	p.poly(bg, PArg{.xof = xoff, .yof = yoff, .fil = "white", .str = "none"});

	// OUTLINE
	{
		SArg sa;
		sa.col = "rgba(100,100,100,0.3)";
		sa.noi = 1;
		sa.wid = 3;
		sa.xof = xoff;
		sa.yof = yoff;
		stroke(p, ptlist[0], sa);
	}

	foot(p, ptlist, xoff, yoff);

	{
		TArg2 ta;
		ta.xof = xoff;
		ta.yof = yoff;
		ta.tex = tex;
		ta.sha = (double)randChoice<int>({0, 0, 0, 0, 5});
		texture(p, ptlist, ta);
	}

	// TOP
	vegetate(
		[&](double x, double y) {
			Color col{100, 100, 100, nse(0.01 * x, 0.01 * y) * 0.5 * 0.3 + 0.5};
			Tree::tree02(p, x + xoff, y + yoff, 16, 8, 5, col, 0.5);
		},
		[&](int i, int j) {
			double ns = nse(i * 0.1, j * 0.1, seed + 2);
			return ns * ns * ns < 0.1 && std::fabs(ptlist[i][j][1]) / h > 0.5;
		},
		[](const Pts&, int) { return true; });

	if (veg) {
		// MIDDLE
		vegetate(
			[&](double x, double y) {
				double ht = ((h + y) / h) * 70;
				ht = ht * 0.3 + rnd() * ht * 0.7;
				Color col{100, 100, 100, nse(0.01 * x, 0.01 * y) * 0.5 * 0.3 + 0.3};
				Tree::tree01(p, x + xoff, y + yoff, ht, rnd() * 3 + 1, col, 0.5);
			},
			[&](int i, int j) {
				double ns = nse(i * 0.2, j * 0.05, seed);
				return j % 2 && ns * ns * ns * ns < 0.012 && std::fabs(ptlist[i][j][1]) / h < 0.3;
			},
			[](const Pts& veglist, int i) {
				int counter = 0;
				for (size_t j = 0; j < veglist.size(); j++) {
					if ((int)j != i &&
						std::pow(veglist[i][0] - veglist[j][0], 2) + std::pow(veglist[i][1] - veglist[j][1], 2) <
							30 * 30) {
						counter++;
					}
					if (counter > 2)
						return true;
				}
				return false;
			});

		// BOTTOM
		vegetate(
			[&](double x, double y) {
				double ht = ((h + y) / h) * 120;
				ht = ht * 0.5 + rnd() * ht * 0.5;
				double bc = rnd() * 0.1;
				double bp = 1;
				Color col{100, 100, 100, nse(0.01 * x, 0.01 * y) * 0.5 * 0.3 + 0.3};
				Tree::tree03(
					p, x + xoff, y + yoff, ht, 5, [bc, bp](double xx) { return std::pow(xx * bc, bp); }, col, 0.5);
			},
			[&](int i, int j) {
				double ns = nse(i * 0.2, j * 0.05, seed);
				return (j == 0 || j == (int)ptlist[i].size() - 1) && ns * ns * ns * ns < 0.012;
			},
			[](const Pts&, int) { return true; });
	}

	// BOTT ARCH
	vegetate(
		[&](double x, double y) {
			int tt = (int)randChoice<double>({0, 0, 1, 1, 1, 2});
			if (tt == 1) {
				// JS: {wid: normRand(40,70), sto: randChoice([1,2,2,3]),
				//      rot: Math.random(), sty: randChoice([1,2,3])}
				double w = normRand(40, 70);
				int sto = (int)randChoice<double>({1, 2, 2, 3});
				double rot = rnd();
				int sty = (int)randChoice<double>({1, 2, 3});
				Arch::arch02(p, x + xoff, y + yoff, seed, 10, w, rot, 5, sto, sty, false);
			} else if (tt == 2) {
				Arch::arch04(p, x + xoff, y + yoff, seed, 15, 30, 0.7, (double)randChoice<double>({1, 1, 1, 2, 2}), 1);
			}
		},
		[&](int i, int j) {
			double ns = nse(i * 0.2, j * 0.05, seed + 10);
			return i != 0 && (j == 1 || j == (int)ptlist[i].size() - 2) && ns * ns * ns * ns < 0.008;
		},
		[](const Pts&, int) { return true; });

	// TOP ARCH
	vegetate(
		[&](double x, double y) {
			// JS: {sto: randChoice([5, 7]), wid: 40 + Math.random() * 20}
			int sto = (int)randChoice<double>({5, 7});
			double w = 40 + rnd() * 20;
			Arch::arch03(p, x + xoff, y + yoff, seed, 10, w, 0.7, 5, sto);
		},
		[&](int i, int j) { return i == 1 && std::fabs((double)j - ptlist[i].size() / 2) < 1 && rnd() < 0.02; },
		[](const Pts&, int) { return true; });

	// TRANSM
	vegetate(
		[&](double x, double y) { Arch::transmissionTower01(p, x + xoff, y + yoff, seed); },
		[&](int i, int j) {
			double ns = nse(i * 0.2, j * 0.05, seed + 20 * pi);
			return i % 2 == 0 && (j == 1 || j == (int)ptlist[i].size() - 2) && ns * ns * ns * ns < 0.002;
		},
		[](const Pts&, int) { return true; });

	// BOTT ROCK
	vegetate(
		[&](double x, double y) {
			double rw = 20 + rnd() * 20;
			double rh = 20 + rnd() * 20;
			Mount::rock(p, x + xoff, y + yoff, seed, rh, rw, 40, 0, 2);
		},
		[&](int i, int j) { return (j == 0 || j == (int)ptlist[i].size() - 1) && rnd() < 0.1; },
		[](const Pts&, int) { return true; });

	(void)ret;
}

namespace {

// flatDec decorations for flatMount
void flatDec(Painter& p, double xoff, double yoff, double xmin, double xmax, double ymin, double ymax) {
	int tt = (int)randChoice<double>({0, 0, 1, 2, 3, 4});

	for (double j = 0; j < rnd() * 5; j++) {
		{
			double rx = xoff + normRand(xmin, xmax);
			double ry = yoff + (ymin + ymax) / 2 + normRand(-10, 10) + 10;
			double rs = rnd() * 100;
			double rw = 10 + rnd() * 20;
			double rh = 10 + rnd() * 20;
			Mount::rock(p, rx, ry, rs, rh, rw, 40, 0, 2);
		}
	}
	for (double j = 0; j < (double)randChoice<int>({0, 0, 1, 2}); j++) {
		double xr = xoff + normRand(xmin, xmax);
		double yr = yoff + (ymin + ymax) / 2 + normRand(-5, 5) + 20;
		for (double k = 0; k < 2 + rnd() * 3; k++) {
			double tx = xr + std::clamp(normRand(-30, 30), xmin, xmax);
			double th = 60 + rnd() * 40;
			Tree::tree08(p, tx, yr, th, 1, {}, 0.5);
		}
	}

	if (tt == 0) {
		for (double j = 0; j < rnd() * 3; j++) {
			{
				double rx = xoff + normRand(xmin, xmax);
				double ry = yoff + (ymin + ymax) / 2 + normRand(-5, 5) + 20;
				double rs = rnd() * 100;
				double rw = 50 + rnd() * 20;
				double rh = 40 + rnd() * 20;
				Mount::rock(p, rx, ry, rs, rh, rw, 40, 0, 5);
			}
		}
	}
	if (tt == 1) {
		double pmin = rnd() * 0.5;
		double pmax = rnd() * 0.5 + 0.5;
		double x0 = std::lerp(xmin, xmax, pmin);
		double x1 = std::lerp(xmin, xmax, pmax);
		for (double i = x0; i < x1; i += 30) {
			{
				double tx = xoff + i + 20 * normRand(-1, 1);
				double th = 100 + rnd() * 200;
				Tree::tree05(p, tx, yoff + (ymin + ymax) / 2 + 20, th, 5, {}, 0.5);
			}
		}
		for (double j = 0; j < rnd() * 4; j++) {
			{
				double rx = xoff + normRand(xmin, xmax);
				double ry = yoff + (ymin + ymax) / 2 + normRand(-5, 5) + 20;
				double rs = rnd() * 100;
				double rw = 50 + rnd() * 20;
				double rh = 40 + rnd() * 20;
				Mount::rock(p, rx, ry, rs, rh, rw, 40, 0, 5);
			}
		}
	} else if (tt == 2) {
		for (int i = 0; i < (int)randChoice<double>({1, 1, 1, 1, 2, 2, 3}); i++) {
			double xr = normRand(xmin, xmax);
			double yr = (ymin + ymax) / 2;
			Tree::tree04(p, xoff + xr, yoff + yr + 20, 300, 6, {}, 0.5);
			for (double j = 0; j < rnd() * 2; j++) {
				{
					double rx = xoff + std::clamp(xr + normRand(-50, 50), xmin, xmax);
					double ry = yoff + yr + normRand(-5, 5) + 20;
					double rs = j * i * rnd() * 100;
					double rw = 50 + rnd() * 20;
					double rh = 40 + rnd() * 20;
					Mount::rock(p, rx, ry, rs, rh, rw, 40, 0, 5);
				}
			}
		}
	} else if (tt == 3) {
		for (int i = 0; i < (int)randChoice<double>({1, 1, 1, 1, 2, 2, 3}); i++) {
			{
				double tx = xoff + normRand(xmin, xmax);
				double th = 60 + rnd() * 60;
				Tree::tree06(p, tx, yoff + (ymin + ymax) / 2, th, 6, {}, 0.5);
			}
		}
	} else if (tt == 4) {
		double pmin = rnd() * 0.5;
		double pmax = rnd() * 0.5 + 0.5;
		double x0 = std::lerp(xmin, xmax, pmin);
		double x1 = std::lerp(xmin, xmax, pmax);
		for (double i = x0; i < x1; i += 20) {
			{
				double tx = xoff + i + 20 * normRand(-1, 1);
				double ty = yoff + (ymin + ymax) / 2 + normRand(-1, 1) + 0;
				double th = normRand(40, 80);
				Tree::tree07(p, tx, ty, th, 4, {}, {100, 100, 100, 1.0}, 0.5);
			}
		}
	}

	for (double i = 0; i < 50 * rnd(); i++) {
		{
			double tx = xoff + normRand(xmin, xmax);
			double ty = yoff + normRand(ymin, ymax);
			Tree::tree02(p, tx, ty, 16, 8, 5, {}, 0.5);
		}
	}

	int ts = (int)randChoice<double>({0, 0, 0, 0, 1});
	if (ts == 1 && tt != 4) {
		// JS: x normRand, seed rnd, then args {wid: normRand, hei: normRand, per: rnd}
		double ax = xoff + normRand(xmin, xmax);
		double aseed = rnd();
		double aw = normRand(160, 200);
		double ah = normRand(80, 100);
		double ap = rnd();
		Arch::arch01(p, ax, yoff + (ymin + ymax) / 2 + 20, aseed, ah, aw, 0.7, ap);
	}
}

} // namespace

void Mount::flatMount(Painter& p, double xoff, double yoff, double seed, const FlatMountArg& a) {
	double hei = a.hei >= 0 ? a.hei : 40 + rnd() * 400;
	double wid = a.wid >= 0 ? a.wid : 400 + rnd() * 200;
	int tex = a.tex;
	double cho = a.cho;

	ScratchPtsList ptlist;
	const int reso0 = 5, reso1 = 50;
	ptlist.reserve(reso0);
	double hoff = 0;
	ScratchPtsList flat;
	flat.reserve(reso0);
	for (int j = 0; j < reso0; j++) {
		hoff += (rnd() * yoff) / 100;
		ScratchPts row;
		row.reserve(reso1);
		ScratchPts frow;
		for (int i = 0; i < reso1; i++) {
			double x = ((double)i / reso1 - 0.5) * pi;
			double y = std::cos(x * 2) + 1;
			y *= nse(x + 10, j * 0.1, seed);
			double pp = 1 - ((double)j / reso0) * 0.6;
			double nx = (x / pi) * wid * pp;
			double ny = -y * hei * pp + hoff;
			double h = 100;
			if (ny < -h * cho + hoff) {
				ny = -h * cho + hoff;
				if (frow.size() % 2 == 0) {
					frow.push_back({nx, ny});
				}
			} else {
				if (frow.size() % 2 == 1) {
					frow.push_back(row[row.size() - 1]);
				}
			}
			row.push_back({nx, ny});
		}
		ptlist.push_back(std::move(row));
		flat.push_back(std::move(frow));
	}

	ScratchPts bg = ptlist[0];
	bg.push_back({0, reso0 * 4});
	p.poly(bg, PArg{.xof = xoff, .yof = yoff, .fil = "white", .str = "none"});

	{
		SArg sa;
		sa.col = "rgba(100,100,100,0.3)";
		sa.noi = 1;
		sa.wid = 3;
		sa.xof = xoff;
		sa.yof = yoff;
		stroke(p, ptlist[0], sa);
	}

	{
		TArg2 ta;
		ta.xof = xoff;
		ta.yof = yoff;
		ta.tex = tex;
		ta.wid = 2;
		texture(p, ptlist, ta, {}, {}, []() {
			if (rnd() > 0.5)
				return 0.1 + 0.4 * rnd();
			return 0.9 - 0.4 * rnd();
		});
	}

	ScratchPts grlist1, grlist2;
	for (size_t i = 0; i < flat.size(); i += 2) {
		if (flat[i].size() >= 2) {
			grlist1.push_back(flat[i][0]);
			grlist2.push_back(flat[i][flat[i].size() - 1]);
		}
	}
	if (grlist1.empty())
		return;

	double wb[2] = {grlist1[0][0], grlist2[0][0]};
	for (int i = 0; i < 3; i++) {
		double pp = 0.8 - i * 0.2;
		grlist1.insert(grlist1.begin(), {wb[0] * pp, grlist1[0][1] - 5});
		grlist2.insert(grlist2.begin(), {wb[1] * pp, grlist2[0][1] - 5});
	}
	wb[0] = grlist1[grlist1.size() - 1][0];
	wb[1] = grlist2[grlist2.size() - 1][0];
	for (int i = 0; i < 3; i++) {
		double pp = 0.6 - i * i * 0.1;
		grlist1.push_back({wb[0] * pp, grlist1[grlist1.size() - 1][1] + 1});
		grlist2.push_back({wb[1] * pp, grlist2[grlist2.size() - 1][1] + 1});
	}

	const double d = 5;
	grlist1 = subdivide(grlist1, d);
	grlist2 = subdivide(grlist2, d);

	ScratchPts grlist = grlist1;
	std::reverse(grlist.begin(), grlist.end());
	grlist.insert(grlist.end(), grlist2.begin(), grlist2.end());
	grlist.push_back(grlist.front()); // JS: [grlist1[0]] aliases the first point
	for (size_t i = 0; i < grlist.size(); i++) {
		double v = (1 - std::fabs((double)(i % (int)d) - d / 2) / (d / 2)) * 0.12;
		grlist[i][0] *= 1 - v + nse(grlist[i][1] * 0.5) * v;
	}
	grlist[0] = grlist.back(); // JS array aliasing: jitter at last index hits first too

	p.poly(grlist, PArg{.xof = xoff, .yof = yoff, .fil = "white", .str = "none", .wid = 2});
	{
		SArg sa;
		sa.wid = 3;
		sa.col = "rgba(100,100,100,0.2)";
		sa.xof = xoff;
		sa.yof = yoff;
		stroke(p, grlist, sa);
	}

	auto [xminmax, yminmax] = std::pair{
		std::ranges::minmax(grlist, {}, [](const Pt& p) { return p[0]; }),
		std::ranges::minmax(grlist, {}, [](const Pt& p) { return p[1]; })};
	double xmin = xminmax.min[0], xmax = xminmax.max[0];
	double ymin = yminmax.min[1], ymax = yminmax.max[1];

	flatDec(p, xoff, yoff, xmin, xmax, ymin, ymax);
}

void Mount::distMount(Painter& p, double xoff, double yoff, double seed, const DistMountArg& a) {
	double hei = a.hei;
	double len = a.len;
	int seg = a.seg;

	const double span = 10;
	std::vector<Pts> ptlist;
	ptlist.reserve((int)(len / span / seg));
	for (int i = 0; i < (int)(len / span / seg); i++) {
		Pts row;
		row.reserve(seg + 1 + seg / 2 + 1);
		for (int j = 0; j < seg + 1; j++) {
			double k = i * seg + j;
			row.push_back(
				{xoff + k * span, yoff - hei * nse(k * 0.05, seed) * std::pow(std::sin((pi * k) / (len / span)), 0.5)});
		}
		for (int j = 0; j < seg / 2.0 + 1; j++) {
			double k = i * seg + j * 2;
			row.insert(
				row.begin(),
				{xoff + k * span, yoff + 24 * nse(k * 0.05, 2, seed) * std::pow(std::sin((pi * k) / (len / span)), 1)});
		}
		ptlist.push_back(std::move(row));
	}
	for (const auto& row : ptlist) {
		auto getCol = [&](double x, double y) {
			int c = (int)(nse(x * 0.02, y * 0.02, yoff) * 55 + 200);
			return std::format("rgb({},{},{})", c, c, c);
		};
		std::string col = getCol(row[row.size() - 1][0], row[row.size() - 1][1]);
		p.poly(row, PArg{.fil = col, .str = "none", .wid = 1});

		auto T = PolyTools::triangulate(row, TriArgs{.area = 100, .convex = true, .optimize = false});
		for (const auto& tri : T) {
			Pt m = PolyTools::centroid(tri);
			std::string co = getCol(m[0], m[1]);
			p.poly(tri, PArg{.fil = co, .str = co, .wid = 1});
		}
	}
}

void Mount::rock(
	Painter& p,
	double xoff,
	double yoff,
	double seed,
	double hei,
	double wid,
	int tex,
	int ret,
	double sha) {
	const int reso0 = 10, reso1 = 50;
	ScratchPtsList ptlist;
	ptlist.reserve(reso0);
	for (int i = 0; i < reso0; i++) {
		ScratchPts row;
		row.reserve(reso1);
		std::vector<double> nslist;
		nslist.reserve(reso1);
		for (int j = 0; j < reso1; j++) {
			nslist.push_back(nse(i, j * 0.2, seed));
		}
		seamless_noise(nslist);
		for (int j = 0; j < reso1; j++) {
			double a = ((double)j / reso1) * pi * 2 - pi / 2;
			double l = (wid * hei) / std::sqrt(std::pow(hei * std::cos(a), 2) + std::pow(wid * std::sin(a), 2));
			l *= 0.7 + 0.3 * nslist[j];
			double pp = 1 - (double)i / reso0;
			double nx = std::cos(a) * l * pp;
			double ny = -std::sin(a) * l * pp;
			if (pi < a || a < 0) {
				ny *= 0.2;
			}
			ny += hei * ((double)i / reso0) * 0.2;
			row.push_back({nx, ny});
		}
		ptlist.push_back(std::move(row));
	}

	ScratchPts bg = ptlist[0];
	bg.push_back({0, 0});
	p.poly(bg, PArg{.xof = xoff, .yof = yoff, .fil = "white", .str = "none"});

	{
		SArg sa;
		sa.col = "rgba(100,100,100,0.3)";
		sa.noi = 1;
		sa.wid = 3;
		sa.xof = xoff;
		sa.yof = yoff;
		stroke(p, ptlist[0], sa);
	}

	{
		TArg2 ta;
		ta.xof = xoff;
		ta.yof = yoff;
		ta.tex = tex;
		ta.wid = 3;
		ta.sha = sha;
		texture(p, ptlist, ta, {}, [](double) { return Color{180, 180, 180, 0.3 + rnd() * 0.3}.rgba(); }, []() {
			if (rnd() > 0.5)
				return 0.15 + 0.15 * rnd();
			return 0.85 - 0.15 * rnd();
		});
	}
	(void)ret;
}

} // namespace ss
