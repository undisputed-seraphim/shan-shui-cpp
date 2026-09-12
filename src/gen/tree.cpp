#include "gen/tree.h"
#include "core/noise.h"
#include "draw/shapes.h"
#include <cmath>

namespace ss {

namespace {

// leaf col parse: "rgba(r,g,b,a)" -> {r,g,b,a}; else {100,100,100,a}
struct LeafCol {
	std::string r = "100", g = "100", b = "100";
	double a = 0.5;
};

LeafCol parseLeafCol(const std::string& col) {
	LeafCol lc;
	if (col.find("rgba(") != std::string::npos) {
		std::string inner = col;
		size_t p0 = inner.find("rgba(");
		inner = inner.substr(p0 + 5);
		size_t p1 = inner.find(')');
		inner = inner.substr(0, p1);
		std::vector<std::string> parts;
		size_t start = 0;
		while (true) {
			size_t c = inner.find(',', start);
			if (c == std::string::npos) {
				parts.push_back(inner.substr(start));
				break;
			}
			parts.push_back(inner.substr(start, c - start));
			start = c + 1;
		}
		if (parts.size() >= 4) {
			lc.r = parts[0];
			lc.g = parts[1];
			lc.b = parts[2];
			lc.a = std::stod(parts[3]);
		}
	} else {
		lc.a = 0.5;
	}
	return lc;
}

struct BranchArg {
	double hei = 300, wid = 6, ang = 0;
	double det = 10;
	double ben = PI * 0.2;
};

// returns [trlist1, trlist2]
std::pair<Pts, Pts> branch(const BranchArg& a) {
	Pts tlist;
	double nx = 0, ny = 0;
	tlist.push_back({nx, ny});
	double a0 = 0;
	const int g = 3;
	for (int i = 0; i < g; i++) {
		a0 += (a.ben / 2 + (rnd() * a.ben) / 2) * randChoice<double>({-1.0, 1.0});
		nx += (std::cos(a0) * a.hei) / g;
		ny -= (std::sin(a0) * a.hei) / g;
		tlist.push_back({nx, ny});
	}
	double ta = std::atan2(tlist[tlist.size() - 1][1], tlist[tlist.size() - 1][0]);
	for (size_t i = 0; i < tlist.size(); i++) {
		double ang2 = std::atan2(tlist[i][1], tlist[i][0]);
		double d = std::hypot(tlist[i][0], tlist[i][1]);
		tlist[i][0] = d * std::cos(ang2 - ta + a.ang);
		tlist[i][1] = d * std::sin(ang2 - ta + a.ang);
	}
	Pts trlist1, trlist2;
	double span = a.det;
	double tl = ((double)tlist.size() - 1) * span; // JS: float, loop runs while i < tl
	double lx = 0, ly = 0;
	for (int i = 0; i < tl; i++) {
		const Pt& lastp = tlist[(size_t)std::floor(i / span)];
		const Pt& nextp = tlist[(size_t)std::ceil(i / span)];
		double p = std::fmod((double)i, span) / span;
		double nxx = lastp[0] * (1 - p) + nextp[0] * p;
		double nyy = lastp[1] * (1 - p) + nextp[1] * p;
		double ang2 = std::atan2(nyy - ly, nxx - lx);
		double woff = ((nse(i * 0.3) - 0.5) * a.wid * a.hei) / 80;
		double b = 0;
		if (p == 0)
			b = rnd() * a.wid;
		double nw = a.wid * (((tl - i) / tl) * 0.5 + 0.5);
		trlist1.push_back(
			{nxx + std::cos(ang2 + PI / 2) * (nw + woff + b), nyy + std::sin(ang2 + PI / 2) * (nw + woff + b)});
		trlist2.push_back(
			{nxx + std::cos(ang2 - PI / 2) * (nw - woff + b), nyy + std::sin(ang2 - PI / 2) * (nw - woff + b)});
		lx = nxx;
		ly = nyy;
	}
	return {trlist1, trlist2};
}

struct TwigArg {
	double dir = 1, sca = 1, wid = 1, ang = 0;
	bool hasLea = true;
	double lea = 12;
};

void twig(Painter& p, double tx, double ty, int dep, const TwigArg& a) {
	Pts twlist;
	const int tl = 10;
	double hs = rnd() * 0.5 + 0.5;
	rnd(); // JS: randChoice([fun2]) consumes a draw even with a single element
	// tfun = fun2 (always chosen from [fun2])
	auto tfun = [](double x, int i) { return -1 / std::pow((double)i / tl + 1, 5) + 1; };
	double a0 = ((rnd() * PI) / 6) * a.dir + a.ang;
	for (int i = 0; i < tl; i++) {
		double mx = a.dir * tfun((double)i / tl, i) * 50 * a.sca * hs;
		double my = -i * 5 * a.sca;
		double ang2 = std::atan2(my, mx);
		double d = std::pow(mx * mx + my * my, 0.5);
		double nx = std::cos(ang2 + a0) * d;
		double ny = std::sin(ang2 + a0) * d;
		twlist.push_back({nx + tx, ny + ty});
		if ((i == ((tl / 3) | 0) || i == (((tl * 2) / 3) | 0)) && dep > 0) {
			twig(
				p,
				nx + tx,
				ny + ty,
				dep - 1,
				TwigArg{
					.dir = a.dir * randChoice<double>({-1.0, 1.0}),
					.sca = a.sca * 0.8,
					.wid = a.wid,
					.ang = a.ang,
					.hasLea = a.hasLea,
					.lea = a.lea});
		}
		if (i == tl - 1 && a.hasLea) {
			for (int j = 0; j < 5; j++) {
				double dj = (j - 2.5) * 5;
				BArg ba;
				ba.wid = (6 + 3 * rnd()) * a.wid;
				ba.len = (15 + 12 * rnd()) * a.wid;
				ba.ang = a.ang / 2 + PI / 2 + PI * 0.2 * (rnd() - 0.5);
				ba.col = "rgba(100,100,100," + toFixed(0.5 + dep * 0.2, 3) + ")";
				ba.fun = [](double x) {
					return x <= 1 ? std::pow(std::sin(x * PI) * x, 0.5)
								  : -std::pow(std::sin((x - 2) * PI * (x - 2)), 0.5);
				};
				blob(
					p,
					nx + tx + std::cos(a.ang) * dj * a.wid,
					ny + ty + (std::sin(a.ang) * dj - a.lea / (dep + 1)) * a.wid,
					ba);
			}
		}
	}
	SArg sa;
	sa.wid = 1;
	sa.fun = [](double x) { return std::cos((x * PI) / 2); };
	sa.col = "rgba(100,100,100,0.5)";
	stroke(p, twlist, sa);
}

// bark texture stroke
void bark(Painter& p, double x, double y, double wid, double ang) {
	double len = 10 + 10 * rnd();
	auto fun = [](double x) {
		return x <= 1 ? std::pow(std::sin(x * PI), 0.5) : -std::pow(std::sin((x + 1) * PI), 0.5);
	};
	const double reso = 20.0;
	std::vector<std::array<double, 2>> lalist;
	for (int i = 0; i < (int)reso + 1; i++) {
		double pp = ((double)i / reso) * 2;
		double xo = len / 2 - std::fabs(pp - 1) * len;
		double yo = (fun(pp) * wid) / 2;
		double a2 = std::atan2(yo, xo);
		double l = std::sqrt(xo * xo + yo * yo);
		lalist.push_back({l, a2});
	}
	std::vector<double> nslist;
	double n0 = rnd() * 10;
	for (int i = 0; i < (int)reso + 1; i++)
		nslist.push_back(nse(i * 0.05, n0));
	loopNoise(nslist);
	Pts brklist;
	for (size_t i = 0; i < lalist.size(); i++) {
		double ns = nslist[i] * 0.5 + (1 - 0.5);
		double nx = x + std::cos(lalist[i][1] + ang) * lalist[i][0] * ns;
		double ny = y + std::sin(lalist[i][1] + ang) * lalist[i][0] * ns;
		brklist.push_back({nx, ny});
	}
	double fr = rnd();
	SArg sa;
	sa.wid = 0.8;
	sa.noi = 0;
	sa.col = "rgba(100,100,100,0.4)";
	sa.out = 0;
	sa.fun = [fr](double x) { return std::sin((x + fr) * PI * 3); };
	stroke(p, brklist, sa);
}

void barkify(Painter& p, double x, double y, std::pair<Pts, Pts>& tr) {
	const Pts& tr0 = tr.first;
	const Pts& tr1 = tr.second;
	for (size_t i = 2; i + 1 < tr0.size(); i++) {
		double a0 = std::atan2(tr0[i][1] - tr0[i - 1][1], tr0[i][0] - tr0[i - 1][0]);
		double a1 = std::atan2(tr1[i][1] - tr1[i - 1][1], tr1[i][0] - tr1[i - 1][0]);
		double pp = rnd();
		double nx = tr0[i][0] * (1 - pp) + tr1[i][0] * pp;
		double ny = tr0[i][1] * (1 - pp) + tr1[i][1] * pp;
		if (rnd() < 0.2) {
			BArg ba;
			ba.noi = 1;
			ba.len = 15;
			ba.wid = 6 - std::fabs(pp - 0.5) * 10;
			ba.ang = (a0 + a1) / 2;
			ba.col = "rgba(100,100,100,0.6)";
			blob(p, nx + x, ny + y, ba);
		} else {
			bark(p, nx + x, ny + y, 5 - std::fabs(pp - 0.5) * 10, (a0 + a1) / 2);
		}
		if (rnd() < 0.05) {
			double jl = rnd() * 2 + 2;
			double xya[3];
			if (rnd() < 0.5) {
				xya[0] = tr0[i][0];
				xya[1] = tr0[i][1];
				xya[2] = a0;
			} else {
				xya[0] = tr1[i][0];
				xya[1] = tr1[i][1];
				xya[2] = a1;
			}
			for (double j = 0; j < jl; j++) {
				BArg ba;
				ba.wid = 4;
				ba.len = 4 + 6 * rnd();
				ba.ang = a0 + PI / 2;
				ba.col = "rgba(100,100,100,0.6)";
				blob(
					p,
					xya[0] + x + std::cos(xya[2]) * (j - jl / 2) * 4,
					xya[1] + y + std::sin(xya[2]) * (j - jl / 2) * 4,
					ba);
			}
		}
	}

	Pts trflist = tr0;
	trflist.insert(trflist.end(), tr1.rbegin(), tr1.rend());

	// JS quirk: div() returns the last point by reference, and the jitter loop
	// below mutates it in place - which permanently alters the caller's points.
	std::vector<Pts> rglist;
	std::vector<int> lastSrc;
	rglist.push_back({});
	lastSrc.push_back(-1);
	for (size_t i = 0; i < trflist.size(); i++) {
		if (rnd() < 0.5) {
			rglist.push_back({});
			lastSrc.push_back(-1);
		} else {
			rglist.back().push_back(trflist[i]);
			lastSrc.back() = (int)i;
		}
	}
	for (size_t i = 0; i < rglist.size(); i++) {
		Pts divRow = div(rglist[i], 4);
		for (size_t j = 0; j < divRow.size(); j++) {
			divRow[j][0] += (nse((double)i, j * 0.1, 1) - 0.5) * (15 + 5 * randGaussian());
			divRow[j][1] += (nse((double)i, j * 0.1, 2) - 0.5) * (15 + 5 * randGaussian());
		}
		// write back the shared last point into the caller's tr0/tr1
		if (lastSrc[i] >= 0 && !divRow.empty()) {
			int k = lastSrc[i];
			if (k < (int)tr0.size()) {
				tr.first[k] = divRow.back();
			} else {
				tr.second[tr0.size() + tr1.size() - 1 - k] = divRow.back();
			}
		}
		SArg sa;
		sa.wid = 1.5;
		sa.col = "rgba(100,100,100,0.7)";
		sa.out = 0;
		Pts shifted;
		for (const auto& v : divRow)
			shifted.push_back({v[0] + x, v[1] + y});
		stroke(p, shifted, sa);
	}
}

} // namespace

void Tree::tree01(Painter& p, double x, double y, double hei, double wid, const std::string& col, double noi) {
	const int reso = 10;
	std::vector<std::array<double, 2>> nslist;
	for (int i = 0; i < reso; i++) {
		nslist.push_back({nse(i * 0.5), nse(i * 0.5, 0.5)});
	}
	LeafCol leafcol = parseLeafCol(col);

	Pts line1, line2;
	for (int i = 0; i < reso; i++) {
		double nx = x;
		double ny = y - ((double)i * hei) / reso;
		if (i >= reso / 4.0) {
			for (double j = 0; j < (reso - i) / 5.0; j++) {
				double bx = nx + (rnd() - 0.5) * wid * 1.2 * (reso - i);
				double by = ny + (rnd() - 0.5) * wid;
				BArg ba;
				ba.len = rnd() * 20 * (reso - i) * 0.2 + 10;
				ba.wid = rnd() * 6 + 3;
				ba.ang = ((rnd() - 0.5) * PI) / 6;
				ba.col = "rgba(" + leafcol.r + "," + leafcol.g + "," + leafcol.b + "," +
						 toFixed(rnd() * 0.2 + leafcol.a, 1) + ")";
				blob(p, bx, by, ba);
			}
		}
		line1.push_back({nx + (nslist[i][0] - 0.5) * wid - wid / 2, ny});
		line2.push_back({nx + (nslist[i][1] - 0.5) * wid + wid / 2, ny});
	}
	p.poly(line1, PArg{.fil = "none", .str = col, .wid = 1.5});
	p.poly(line2, PArg{.fil = "none", .str = col, .wid = 1.5});
}

void Tree::tree02(Painter& p, double x, double y, double hei, double wid, int clu, const std::string& col, double noi) {
	for (int i = 0; i < clu; i++) {
		// JS evaluates call args left to right: x+gauss, y+gauss, then args obj
		double bx = x + randGaussian() * clu * 4;
		double by = y + randGaussian() * clu * 4;
		BArg ba;
		ba.ang = PI / 2;
		ba.fun = [](double xx) {
			return xx <= 1 ? std::pow(std::sin(xx * PI) * xx, 0.5) : -std::pow(std::sin((xx - 2) * PI * (xx - 2)), 0.5);
		};
		ba.wid = rnd() * wid * 0.75 + wid * 0.5;
		ba.len = rnd() * hei * 0.75 + hei * 0.5;
		ba.col = col;
		blob(p, bx, by, ba);
	}
}

void Tree::tree03(
	Painter& p,
	double x,
	double y,
	double hei,
	double wid,
	const std::function<double(double)>& ben,
	const std::string& col,
	double noi) {
	auto benf = ben ? ben : [](double) { return 0.0; };
	const int reso = 10;
	std::vector<std::array<double, 2>> nslist;
	for (int i = 0; i < reso; i++) {
		nslist.push_back({nse(i * 0.5), nse(i * 0.5, 0.5)});
	}
	LeafCol leafcol = parseLeafCol(col);

	Pts line1, line2;
	Painter blobs;
	for (int i = 0; i < reso; i++) {
		double nx = x + benf((double)i / reso) * 100;
		double ny = y - ((double)i * hei) / reso;
		if (i >= reso / 5.0) {
			for (int j = 0; j < (reso - i) * 2; j++) {
				auto shape = [](double xx) { return std::log(50 * xx + 1) / 3.95; };
				double ox = rnd() * wid * 2 * shape((double)(reso - i) / reso);
				double bx = nx + ox * randChoice<double>({-1.0, 1.0});
				double by = ny + (rnd() - 0.5) * wid * 2;
				BArg ba;
				ba.len = ox * 2;
				ba.wid = rnd() * 6 + 3;
				ba.ang = ((rnd() - 0.5) * PI) / 6;
				ba.col = "rgba(" + leafcol.r + "," + leafcol.g + "," + leafcol.b + "," +
						 toFixed(rnd() * 0.2 + leafcol.a, 3) + ")";
				blob(blobs, bx, by, ba);
			}
		}
		line1.push_back({nx + ((nslist[i][0] - 0.5) * wid - wid / 2) * (reso - i) / reso, ny});
		line2.push_back({nx + ((nslist[i][1] - 0.5) * wid + wid / 2) * (reso - i) / reso, ny});
	}
	Pts lc = line1;
	lc.insert(lc.end(), line2.rbegin(), line2.rend());
	p.poly(lc, PArg{.fil = "white", .str = col, .wid = 1.5});
	p.absorb(std::move(blobs));
}

void Tree::tree04(Painter& p, double x, double y, double hei, double wid, const std::string& col, double noi) {
	Painter tx, tw;
	auto tr = branch(BranchArg{.hei = hei, .wid = wid, .ang = -PI / 2});
	barkify(tx, x, y, tr);
	Pts trlist = tr.first;
	trlist.insert(trlist.end(), tr.second.rbegin(), tr.second.rend());

	Pts trmlist;
	for (size_t i = 0; i < trlist.size(); i++) {
		if ((i >= trlist.size() * 0.3 && i <= trlist.size() * 0.7 && rnd() < 0.1) || i == trlist.size() / 2 - 1) {
			double ba = PI * 0.2 - PI * 1.4 * (i > trlist.size() / 2);
			auto br = branch(BranchArg{.hei = hei * (rnd() + 1) * 0.3, .wid = wid * 0.5, .ang = ba});
			br.first.erase(br.first.begin());
			br.second.erase(br.second.begin());
			// JS keeps brlist raw; barkify gets an offset COPY (.map(foff))
			auto brCopy = br;
			for (auto& v : brCopy.first) {
				v[0] += trlist[i][0];
				v[1] += trlist[i][1];
			}
			for (auto& v : brCopy.second) {
				v[0] += trlist[i][0];
				v[1] += trlist[i][1];
			}
			barkify(tx, x, y, brCopy);

			for (size_t j = 0; j < br.first.size(); j++) {
				if (rnd() < 0.2 || j == br.first.size() - 1) {
					TwigArg ta;
					ta.wid = hei / 300;
					ta.ang = ba > -PI / 2 ? ba : ba + PI;
					ta.sca = (0.5 * hei) / 300;
					ta.dir = ba > -PI / 2 ? 1 : -1;
					twig(tw, br.first[j][0] + trlist[i][0] + x, br.first[j][1] + trlist[i][1] + y, 1, ta);
				}
			}
			Pts brall = br.first;
			brall.insert(brall.end(), br.second.rbegin(), br.second.rend());
			for (const auto& v : brall)
				trmlist.push_back({v[0] + trlist[i][0], v[1] + trlist[i][1]});
		} else {
			trmlist.push_back(trlist[i]);
		}
	}
	p.poly(trmlist, PArg{.xof = x, .yof = y, .fil = "white", .str = col, .wid = 0});

	trmlist.erase(trmlist.begin());
	trmlist.erase(trmlist.end() - 1);
	SArg sa;
	sa.col = "rgba(100,100,100," + toFixed(0.4 + rnd() * 0.1, 3) + ")";
	sa.wid = 2.5;
	sa.fun = [](double) { return std::sin(1); };
	sa.noi = 0.9;
	sa.out = 0;
	Pts shifted;
	for (const auto& v : trmlist)
		shifted.push_back({v[0] + x, v[1] + y});
	stroke(p, shifted, sa);

	p.absorb(std::move(tx));
	p.absorb(std::move(tw));
}

void Tree::tree05(Painter& p, double x, double y, double hei, double wid, const std::string& col, double noi) {
	Painter tx, tw;
	auto tr = branch(BranchArg{.hei = hei, .wid = wid, .ang = -PI / 2, .ben = 0});
	barkify(tx, x, y, tr);
	Pts trlist = tr.first;
	trlist.insert(trlist.end(), tr.second.rbegin(), tr.second.rend());

	Pts trmlist;
	for (size_t i = 0; i < trlist.size(); i++) {
		double pp = std::fabs((double)i - trlist.size() * 0.5) / (trlist.size() * 0.5);
		if ((i >= trlist.size() * 0.2 && i <= trlist.size() * 0.8 && i % 3 == 0 && rnd() > pp) ||
			i == trlist.size() / 2 - 1) {
			double bar = rnd() * 0.2;
			double ba = -bar * PI - (1 - bar * 2) * PI * (i > trlist.size() / 2);
			auto br =
				branch(BranchArg{.hei = hei * (0.3 * pp - rnd() * 0.05), .wid = wid * 0.5, .ang = ba, .ben = 0.5});
			br.first.erase(br.first.begin());
			br.second.erase(br.second.begin());
			// JS: //txcanv += barkify(...) - side-branch barkify is COMMENTED OUT

			for (size_t j = 0; j < br.first.size(); j++) {
				if (j % 20 == 0 || j == br.first.size() - 1) {
					TwigArg ta;
					ta.wid = hei / 300;
					ta.ang = ba > -PI / 2 ? ba : ba + PI;
					ta.sca = (0.2 * hei) / 300;
					ta.dir = ba > -PI / 2 ? 1 : -1;
					ta.hasLea = true;
					ta.lea = 5;
					twig(tw, br.first[j][0] + trlist[i][0] + x, br.first[j][1] + trlist[i][1] + y, 0, ta);
				}
			}
			Pts brall = br.first;
			brall.insert(brall.end(), br.second.rbegin(), br.second.rend());
			for (const auto& v : brall)
				trmlist.push_back({v[0] + trlist[i][0], v[1] + trlist[i][1]});
		} else {
			trmlist.push_back(trlist[i]);
		}
	}
	p.poly(trmlist, PArg{.xof = x, .yof = y, .fil = "white", .str = col, .wid = 0});

	trmlist.erase(trmlist.begin());
	trmlist.erase(trmlist.end() - 1);
	SArg sa;
	sa.col = "rgba(100,100,100," + toFixed(0.4 + rnd() * 0.1, 3) + ")";
	sa.wid = 2.5;
	sa.fun = [](double) { return std::sin(1); };
	sa.noi = 0.9;
	sa.out = 0;
	Pts shifted;
	for (const auto& v : trmlist)
		shifted.push_back({v[0] + x, v[1] + y});
	stroke(p, shifted, sa);

	p.absorb(std::move(tx));
	p.absorb(std::move(tw));
}

namespace {

// tree06's fracTree
Pts fracTree6(
	Painter& tx,
	Painter& tw,
	double xoff,
	double yoff,
	int dep,
	double hei,
	double wid,
	double ang,
	double ben) {
	auto tr = branch(BranchArg{.hei = hei, .wid = wid, .ang = ang, .det = hei / 20, .ben = ben});
	barkify(tx, xoff, yoff, tr);
	Pts trlist = tr.first;
	trlist.insert(trlist.end(), tr.second.rbegin(), tr.second.rend());
	Pts trmlist;
	for (size_t i = 0; i < trlist.size(); i++) {
		double pp = std::fabs((double)i - trlist.size() * 0.5) / (trlist.size() * 0.5);
		if (((rnd() < 0.025 && i >= trlist.size() * 0.2 && i <= trlist.size() * 0.8) ||
			 i == ((trlist.size() / 2) | 0) - 1 || i == ((trlist.size() / 2) | 0) + 1) &&
			dep > 0) {
			double bar = 0.02 + rnd() * 0.08;
			double ba = bar * PI - bar * 2 * PI * (i > trlist.size() / 2);

			Pts brlist = fracTree6(
				tx,
				tw,
				trlist[i][0] + xoff,
				trlist[i][1] + yoff,
				dep - 1,
				hei * (0.7 + rnd() * 0.2),
				wid * 0.6,
				ang + ba,
				0.55);

			for (size_t j = 0; j < brlist.size(); j++) {
				if (rnd() < 0.03) {
					TwigArg ta;
					ta.ang = ba * (rnd() * 0.5 + 0.75);
					ta.sca = 0.3;
					ta.dir = ba > 0 ? 1 : -1;
					ta.hasLea = false;
					ta.lea = 0;
					twig(tw, brlist[j][0] + trlist[i][0] + xoff, brlist[j][1] + trlist[i][1] + yoff, 2, ta);
				}
			}
			for (const auto& v : brlist)
				trmlist.push_back({v[0] + trlist[i][0], v[1] + trlist[i][1]});
		} else {
			trmlist.push_back(trlist[i]);
		}
	}
	return trmlist;
}

// tree08's fracTree
void fracTree8(Painter& p, double xoff, double yoff, int dep, double ang, double len, double ben) {
	auto fun = dep == 0 ? [](double x) { return std::cos(0.5 * PI * x); } : [](double) { return 1.0; };
	Pt spt = {xoff, yoff};
	Pt ept = {xoff + std::cos(ang) * len, yoff + std::sin(ang) * len};

	Pts trmlist = {{xoff, yoff}, {xoff + len, yoff}};
	auto bfun = randChoice<std::function<double(double)>>(
		{[](double x) { return std::sin(x * PI); }, [](double x) { return -std::sin(x * PI); }});
	trmlist = div(trmlist, 10);
	for (size_t i = 0; i < trmlist.size(); i++) {
		trmlist[i][1] += bfun((double)i / trmlist.size()) * 2;
	}
	for (size_t i = 0; i < trmlist.size(); i++) {
		double d = distance(trmlist[i], spt);
		double a2 = std::atan2(trmlist[i][1] - spt[1], trmlist[i][0] - spt[0]);
		trmlist[i][0] = spt[0] + d * std::cos(a2 + ang);
		trmlist[i][1] = spt[1] + d * std::sin(a2 + ang);
	}
	SArg sa;
	sa.fun = fun;
	sa.wid = 0.8;
	sa.col = "rgba(100,100,100,0.5)";
	stroke(p, trmlist, sa);

	if (dep != 0) {
		double nben = ben + randChoice<double>({-1.0, 1.0}) * PI * 0.001 * dep * dep;
		if (rnd() < 0.5) {
			// JS evaluates args left to right: ang first, then len
			double angA = ang + ben + PI * randChoice<double>({normRand(-1, 0.5), normRand(0.5, 1)}) * 0.2;
			double lenA = len * normRand(0.8, 0.9);
			fracTree8(p, ept[0], ept[1], dep - 1, angA, lenA, nben);
			double angB = ang + ben + PI * randChoice<double>({normRand(-1, -0.5), normRand(0.5, 1)}) * 0.2;
			double lenB = len * normRand(0.8, 0.9);
			fracTree8(p, ept[0], ept[1], dep - 1, angB, lenB, nben);
		} else {
			fracTree8(p, ept[0], ept[1], dep - 1, ang + ben, len * normRand(0.8, 0.9), nben);
		}
	}
}

} // namespace

void Tree::tree06(Painter& p, double x, double y, double hei, double wid, const std::string& col, double noi) {
	Painter tx, tw;
	Pts trmlist = fracTree6(tx, tw, x, y, 3, hei, wid, -PI / 2, 0);
	p.poly(trmlist, PArg{.xof = x, .yof = y, .fil = "white", .str = col, .wid = 0});
	trmlist.erase(trmlist.begin());
	trmlist.erase(trmlist.end() - 1);
	SArg sa;
	sa.col = "rgba(100,100,100," + toFixed(0.4 + rnd() * 0.1, 3) + ")";
	sa.wid = 2.5;
	sa.fun = [](double) { return std::sin(1); };
	sa.noi = 0.9;
	sa.out = 0;
	Pts shifted;
	for (const auto& v : trmlist)
		shifted.push_back({v[0] + x, v[1] + y});
	stroke(p, shifted, sa);
	p.absorb(std::move(tx));
	p.absorb(std::move(tw));
}

void Tree::tree07(
	Painter& p,
	double x,
	double y,
	double hei,
	double wid,
	const std::function<double(double)>& ben,
	const std::string& col,
	double noi) {
	auto benf = ben ? ben : [](double xx) { return std::sqrt(xx) * 0.2; };
	const int reso = 10;
	std::vector<std::array<double, 2>> nslist;
	for (int i = 0; i < reso; i++) {
		nslist.push_back({nse(i * 0.5), nse(i * 0.5, 0.5)});
	}
	LeafCol leafcol = parseLeafCol(col);

	Pts line1, line2;
	std::vector<Pts> T;
	for (int i = 0; i < reso; i++) {
		double nx = x + benf((double)i / reso) * 100;
		double ny = y - ((double)i * hei) / reso;
		if (i >= reso / 4.0) {
			for (int j = 0; j < 1; j++) {
				double bx = nx + (rnd() - 0.5) * wid * 1.2 * (reso - i) * 0.5;
				double by = ny + (rnd() - 0.5) * wid * 0.5;
				BArg ba;
				ba.len = rnd() * 50 + 20;
				ba.wid = rnd() * 12 + 12;
				ba.ang = (-rnd() * PI) / 6;
				ba.col = "rgba(" + leafcol.r + "," + leafcol.g + "," + leafcol.b + "," + toFixed(leafcol.a, 3) + ")";
				ba.fun = [](double xx) {
					return xx <= 1 ? 2.75 * xx * std::pow(1 - xx, 1 / 1.8)
								   : 2.75 * (xx - 2) * std::pow(xx - 1, 1 / 1.8);
				};
				ba.ret = 1;
				Pts bpl = blobPts(bx, by, ba);
				auto tris = PolyTools::triangulate(bpl, TriArgs{.area = 50, .convex = true, .optimize = false});
				T.insert(T.end(), tris.begin(), tris.end());
			}
		}
		line1.push_back({nx + (nslist[i][0] - 0.5) * wid - wid / 2, ny});
		line2.push_back({nx + (nslist[i][1] - 0.5) * wid + wid / 2, ny});
	}
	Pts lc = line1;
	lc.insert(lc.end(), line2.rbegin(), line2.rend());
	auto tris = PolyTools::triangulate(lc, TriArgs{.area = 50, .convex = true, .optimize = true});
	tris.insert(tris.end(), T.begin(), T.end());

	for (const auto& tri : tris) {
		Pt m = PolyTools::midPt(tri);
		int c = (int)(nse(m[0] * 0.02, m[1] * 0.02) * 200 + 50);
		std::string co = "rgba(" + std::to_string(c) + "," + std::to_string(c) + "," + std::to_string(c) + ",0.8)";
		p.poly(tri, PArg{.fil = co, .str = co, .wid = 0});
	}
}

void Tree::tree08(Painter& p, double x, double y, double hei, double wid, const std::string& col, double noi) {
	double ang = normRand(-1, 1) * PI * 0.2;
	auto tr = branch(BranchArg{.hei = hei, .wid = wid, .ang = -PI / 2 + ang, .det = hei / 20, .ben = PI * 0.2});
	Pts trlist = tr.first;
	trlist.insert(trlist.end(), tr.second.rbegin(), tr.second.rend());

	Painter tw;
	for (size_t i = 0; i < trlist.size(); i++) {
		if (rnd() < 0.2) {
			// JS evaluates args left to right: dep first, then ang
			int dep8 = (int)std::floor(4 * rnd());
			double ang8 = -PI / 2 - ang * rnd();
			fracTree8(tw, x + trlist[i][0], y + trlist[i][1], dep8, ang8, 15, 0);
		} else if (i == (size_t)std::floor(trlist.size() / 2)) {
			fracTree8(tw, x + trlist[i][0], y + trlist[i][1], 3, -PI / 2 + ang, 15, 0);
		}
	}
	p.poly(trlist, PArg{.xof = x, .yof = y, .fil = "white", .str = col, .wid = 0});
	SArg sa;
	sa.col = "rgba(100,100,100," + toFixed(0.6 + rnd() * 0.1, 3) + ")";
	sa.wid = 2.5;
	sa.fun = [](double) { return std::sin(1); };
	sa.noi = 0.9;
	sa.out = 0;
	Pts shifted;
	for (const auto& v : trlist)
		shifted.push_back({v[0] + x, v[1] + y});
	stroke(p, shifted, sa);
	p.absorb(std::move(tw));
}

} // namespace ss
