#include "gen/tree.h"
#include "core/noise.h"
#include "draw/shapes.h"
#include <cmath>

namespace ss {

namespace {

struct BranchArg {
	double hei = 300, wid = 6, ang = 0;
	double det = 10;
	double ben = pi * 0.2;
};

// returns [trlist1, trlist2]
std::pair<Pts, Pts> branch(const BranchArg& a) {
	Pts tlist;
	double nx = 0, ny = 0;
	tlist.push_back({nx, ny});
	double a0 = 0;
	const int g = 3;
	tlist.reserve(g + 1);
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
	double tl = ((double)tlist.size() - 1) * span;
	trlist1.reserve((int)tl + 1);
	trlist2.reserve((int)tl + 1);
	double lx = 0, ly = 0;
	for (int i = 0; i < tl; i++) {
		const Pt& lastp = tlist[(size_t)std::floor(i / span)];
		const Pt& nextp = tlist[(size_t)std::ceil(i / span)];
		double p = std::fmod((double)i, span) / span;
		double nxx = std::lerp(lastp[0], nextp[0], p);
		double nyy = std::lerp(lastp[1], nextp[1], p);
		double ang2 = std::atan2(nyy - ly, nxx - lx);
		double woff = ((nse(i * 0.3) - 0.5) * a.wid * a.hei) / 80;
		double b = 0;
		if (p == 0)
			b = rnd() * a.wid;
		double nw = a.wid * (((tl - i) / tl) * 0.5 + 0.5);
		trlist1.push_back(
			{nxx + std::cos(ang2 + pi / 2) * (nw + woff + b), nyy + std::sin(ang2 + pi / 2) * (nw + woff + b)});
		trlist2.push_back(
			{nxx + std::cos(ang2 - pi / 2) * (nw - woff + b), nyy + std::sin(ang2 - pi / 2) * (nw - woff + b)});
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
	twlist.reserve(tl + 1);
	double hs = rnd() * 0.5 + 0.5;
	rnd(); // the pick among a single candidate still consumes a draw
	auto tfun = [](double x, int i) { return -1 / std::pow((double)i / tl + 1, 5) + 1; };
	double a0 = ((rnd() * pi) / 6) * a.dir + a.ang;
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
				ba.ang = a.ang / 2 + pi / 2 + pi * 0.2 * (rnd() - 0.5);
				ba.col = Color{100, 100, 100, 0.5 + dep * 0.2}.rgba();
				blob(
					p,
					nx + tx + std::cos(a.ang) * dj * a.wid,
					ny + ty + (std::sin(a.ang) * dj - a.lea / (dep + 1)) * a.wid,
					ba,
					[](double x) {
						return x <= 1 ? std::pow(std::sin(x * pi) * x, 0.5)
									  : -std::pow(std::sin((x - 2) * pi * (x - 2)), 0.5);
					});
			}
		}
	}
	SArg sa;
	sa.wid = 1;
	sa.col = "rgba(100,100,100,0.5)";
	stroke(p, twlist, sa, [](double x) { return std::cos((x * pi) / 2); });
}

// bark texture stroke
void bark(Painter& p, double x, double y, double wid, double ang) {
	double len = 10 + 10 * rnd();
	auto fun = [](double x) {
		return x <= 1 ? std::pow(std::sin(x * pi), 0.5) : -std::pow(std::sin((x + 1) * pi), 0.5);
	};
	const double reso = 20.0;
	std::vector<std::array<double, 2>> lalist;
	lalist.reserve((size_t)reso + 1);
	for (int i = 0; i < (int)reso + 1; i++) {
		double pp = ((double)i / reso) * 2;
		double xo = len / 2 - std::fabs(pp - 1) * len;
		double yo = (fun(pp) * wid) / 2;
		double a2 = std::atan2(yo, xo);
		double l = std::sqrt(xo * xo + yo * yo);
		lalist.push_back({l, a2});
	}
	std::vector<double> nslist;
	nslist.reserve((size_t)reso + 1);
	double n0 = rnd() * 10;
	for (int i = 0; i < (int)reso + 1; i++)
		nslist.push_back(nse(i * 0.05, n0));
	seamless_noise(nslist);
	Pts brklist;
	brklist.reserve((size_t)reso + 1);
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
	stroke(p, brklist, sa, [fr](double x) { return std::sin((x + fr) * pi * 3); });
}

// Bark texture along a branch ribbon. `xof`/`yof` shift the branch points
// before they are consumed (side branches bake in their attachment point
// here instead of handing in a pre-shifted copy). `write_back` mirrors the
// original JS behavior where the jittered row-final point leaks back into
// the caller's outline points - kept only for the main trunk.
void barkify(
	Painter& p,
	double x,
	double y,
	std::pair<Pts, Pts>& tr,
	double xof = 0,
	double yof = 0,
	bool write_back = true) {
	const Pts& tr0 = tr.first;
	const Pts& tr1 = tr.second;
	auto ptx = [&](const Pt& v) { return v[0] + xof; };
	auto pty = [&](const Pt& v) { return v[1] + yof; };
	for (size_t i = 2; i + 1 < tr0.size(); i++) {
		double a0 = std::atan2(pty(tr0[i]) - pty(tr0[i - 1]), ptx(tr0[i]) - ptx(tr0[i - 1]));
		double a1 = std::atan2(pty(tr1[i]) - pty(tr1[i - 1]), ptx(tr1[i]) - ptx(tr1[i - 1]));
		double pp = rnd();
		double nx = std::lerp(ptx(tr0[i]), ptx(tr1[i]), pp);
		double ny = std::lerp(pty(tr0[i]), pty(tr1[i]), pp);
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
				xya[0] = ptx(tr0[i]);
				xya[1] = pty(tr0[i]);
				xya[2] = a0;
			} else {
				xya[0] = ptx(tr1[i]);
				xya[1] = pty(tr1[i]);
				xya[2] = a1;
			}
			for (double j = 0; j < jl; j++) {
				BArg ba;
				ba.wid = 4;
				ba.len = 4 + 6 * rnd();
				ba.ang = a0 + pi / 2;
				ba.col = "rgba(100,100,100,0.6)";
				blob(
					p,
					xya[0] + x + std::cos(xya[2]) * (j - jl / 2) * 4,
					xya[1] + y + std::sin(xya[2]) * (j - jl / 2) * 4,
					ba);
			}
		}
	}

	Pts trflist;
	trflist.reserve(tr0.size() + tr1.size());
	for (const auto& v : tr0)
		trflist.push_back({ptx(v), pty(v)});
	for (auto it = tr1.rbegin(); it != tr1.rend(); ++it)
		trflist.push_back({ptx(*it), pty(*it)});

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
		ScratchPts divRow = subdivide(rglist[i], 4);
		for (size_t j = 0; j < divRow.size(); j++) {
			divRow[j][0] += (nse((double)i, j * 0.1, 1) - 0.5) * (15 + 5 * randGaussian());
			divRow[j][1] += (nse((double)i, j * 0.1, 2) - 0.5) * (15 + 5 * randGaussian());
		}
		// the jittered row-final point leaks back into the caller's points
		if (write_back && lastSrc[i] >= 0 && !divRow.empty()) {
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
		sa.xof = x;
		sa.yof = y;
		stroke(p, divRow, sa);
	}
}

} // namespace

void Tree::tree01(Painter& p, double x, double y, double hei, double wid, const Color& col, double noi) {
	const int reso = 10;
	std::vector<std::array<double, 2>> nslist;
	for (int i = 0; i < reso; i++) {
		nslist.push_back({nse(i * 0.5), nse(i * 0.5, 0.5)});
	}

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
				ba.ang = ((rnd() - 0.5) * pi) / 6;
				ba.col = Color{col.r, col.g, col.b, rnd() * 0.2 + col.a}.rgba();
				blob(p, bx, by, ba);
			}
		}
		line1.push_back({nx + (nslist[i][0] - 0.5) * wid - wid / 2, ny});
		line2.push_back({nx + (nslist[i][1] - 0.5) * wid + wid / 2, ny});
	}
	p.poly(line1, PArg{.fil = "none", .str = col.rgba(), .wid = 1.5});
	p.poly(line2, PArg{.fil = "none", .str = col.rgba(), .wid = 1.5});
}

void Tree::tree02(Painter& p, double x, double y, double hei, double wid, int clu, const Color& col, double noi) {
	for (int i = 0; i < clu; i++) {
		// JS evaluates call args left to right: x+gauss, y+gauss, then args obj
		double bx = x + randGaussian() * clu * 4;
		double by = y + randGaussian() * clu * 4;
		BArg ba;
		ba.ang = pi / 2;
		ba.wid = rnd() * wid * 0.75 + wid * 0.5;
		ba.len = rnd() * hei * 0.75 + hei * 0.5;
		ba.col = col.rgba();
		blob(
			p,
			bx,
			by,
			ba,
			[](double xx) {
				return xx <= 1 ? std::pow(std::sin(xx * pi) * xx, 0.5)
							   : -std::pow(std::sin((xx - 2) * pi * (xx - 2)), 0.5);
			});
	}
}

void Tree::tree04(Painter& p, double x, double y, double hei, double wid, const Color& col, double noi) {
	Painter tx, tw;
	auto tr = branch(BranchArg{.hei = hei, .wid = wid, .ang = -pi / 2});
	barkify(tx, x, y, tr);
	Pts trlist = tr.first;
	trlist.insert(trlist.end(), tr.second.rbegin(), tr.second.rend());

	Pts trmlist;
	for (size_t i = 0; i < trlist.size(); i++) {
		if ((i >= trlist.size() * 0.3 && i <= trlist.size() * 0.7 && rnd() < 0.1) || i == trlist.size() / 2 - 1) {
			double ba = pi * 0.2 - pi * 1.4 * (i > trlist.size() / 2);
			auto br = branch(BranchArg{.hei = hei * (rnd() + 1) * 0.3, .wid = wid * 0.5, .ang = ba});
			br.first.erase(br.first.begin());
			br.second.erase(br.second.begin());
			// side branch: bake the attachment point into the barkify call;
			// the outline leak (write_back) only hits the main trunk
			barkify(tx, x, y, br, trlist[i][0], trlist[i][1], false);

			for (size_t j = 0; j < br.first.size(); j++) {
				if (rnd() < 0.2 || j == br.first.size() - 1) {
					TwigArg ta;
					ta.wid = hei / 300;
					ta.ang = ba > -pi / 2 ? ba : ba + pi;
					ta.sca = (0.5 * hei) / 300;
					ta.dir = ba > -pi / 2 ? 1 : -1;
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
	p.poly(trmlist, PArg{.xof = x, .yof = y, .fil = "white", .str = col.rgba(), .wid = 0});

	trmlist.erase(trmlist.begin());
	trmlist.erase(trmlist.end() - 1);
	SArg sa;
	sa.col = Color{100, 100, 100, 0.4 + rnd() * 0.1}.rgba();
	sa.wid = 2.5;
	sa.noi = 0.9;
	sa.out = 0;
	sa.xof = x;
	sa.yof = y;
	stroke(p, trmlist, sa, [](double) { return std::sin(1); });

	p.absorb(std::move(tx));
	p.absorb(std::move(tw));
}

void Tree::tree05(Painter& p, double x, double y, double hei, double wid, const Color& col, double noi) {
	Painter tx, tw;
	auto tr = branch(BranchArg{.hei = hei, .wid = wid, .ang = -pi / 2, .ben = 0});
	barkify(tx, x, y, tr);
	Pts trlist = tr.first;
	trlist.insert(trlist.end(), tr.second.rbegin(), tr.second.rend());

	Pts trmlist;
	for (size_t i = 0; i < trlist.size(); i++) {
		double pp = std::fabs((double)i - trlist.size() * 0.5) / (trlist.size() * 0.5);
		if ((i >= trlist.size() * 0.2 && i <= trlist.size() * 0.8 && i % 3 == 0 && rnd() > pp) ||
			i == trlist.size() / 2 - 1) {
			double bar = rnd() * 0.2;
			double ba = -bar * pi - (1 - bar * 2) * pi * (i > trlist.size() / 2);
			auto br =
				branch(BranchArg{.hei = hei * (0.3 * pp - rnd() * 0.05), .wid = wid * 0.5, .ang = ba, .ben = 0.5});
			br.first.erase(br.first.begin());
			br.second.erase(br.second.begin());
			// (side-branch barkify intentionally omitted, as in the original)

			for (size_t j = 0; j < br.first.size(); j++) {
				if (j % 20 == 0 || j == br.first.size() - 1) {
					TwigArg ta;
					ta.wid = hei / 300;
					ta.ang = ba > -pi / 2 ? ba : ba + pi;
					ta.sca = (0.2 * hei) / 300;
					ta.dir = ba > -pi / 2 ? 1 : -1;
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
	p.poly(trmlist, PArg{.xof = x, .yof = y, .fil = "white", .str = col.rgba(), .wid = 0});

	trmlist.erase(trmlist.begin());
	trmlist.erase(trmlist.end() - 1);
	SArg sa;
	sa.col = Color{100, 100, 100, 0.4 + rnd() * 0.1}.rgba();
	sa.wid = 2.5;
	sa.noi = 0.9;
	sa.out = 0;
	sa.xof = x;
	sa.yof = y;
	stroke(p, trmlist, sa, [](double) { return std::sin(1); });

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
			double ba = bar * pi - bar * 2 * pi * (i > trlist.size() / 2);

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
	Pt spt = {xoff, yoff};
	Pt ept = {xoff + std::cos(ang) * len, yoff + std::sin(ang) * len};

	ScratchPts trmlist = {{xoff, yoff}, {xoff + len, yoff}};
	// randChoice over {sin(x*pi), -sin(x*pi)}: one rnd() draw
	double bflip = rnd();
	trmlist = subdivide(trmlist, 10);
	for (size_t i = 0; i < trmlist.size(); i++) {
		double bv = std::sin(((double)i / trmlist.size()) * pi);
		if (bflip >= 0.5)
			bv = -bv;
		trmlist[i][1] += bv * 2;
	}
	for (size_t i = 0; i < trmlist.size(); i++) {
		double d = distance(trmlist[i], spt);
		double a2 = std::atan2(trmlist[i][1] - spt[1], trmlist[i][0] - spt[0]);
		trmlist[i][0] = spt[0] + d * std::cos(a2 + ang);
		trmlist[i][1] = spt[1] + d * std::sin(a2 + ang);
	}
	SArg sa;
	sa.wid = 0.8;
	sa.col = "rgba(100,100,100,0.5)";
	if (dep == 0) {
		stroke(p, trmlist, sa, [](double x) { return std::cos(0.5 * pi * x); });
	} else {
		stroke(p, trmlist, sa, [](double) { return 1.0; });
	}

	if (dep != 0) {
		double nben = ben + randChoice<double>({-1.0, 1.0}) * pi * 0.001 * dep * dep;
		if (rnd() < 0.5) {
			// JS evaluates args left to right: ang first, then len
			double angA = ang + ben + pi * randChoice<double>({normRand(-1, 0.5), normRand(0.5, 1)}) * 0.2;
			double lenA = len * normRand(0.8, 0.9);
			fracTree8(p, ept[0], ept[1], dep - 1, angA, lenA, nben);
			double angB = ang + ben + pi * randChoice<double>({normRand(-1, -0.5), normRand(0.5, 1)}) * 0.2;
			double lenB = len * normRand(0.8, 0.9);
			fracTree8(p, ept[0], ept[1], dep - 1, angB, lenB, nben);
		} else {
			fracTree8(p, ept[0], ept[1], dep - 1, ang + ben, len * normRand(0.8, 0.9), nben);
		}
	}
}

} // namespace

void Tree::tree06(Painter& p, double x, double y, double hei, double wid, const Color& col, double noi) {
	Painter tx, tw;
	Pts trmlist = fracTree6(tx, tw, x, y, 3, hei, wid, -pi / 2, 0);
	p.poly(trmlist, PArg{.xof = x, .yof = y, .fil = "white", .str = col.rgba(), .wid = 0});
	trmlist.erase(trmlist.begin());
	trmlist.erase(trmlist.end() - 1);
	SArg sa;
	sa.col = Color{100, 100, 100, 0.4 + rnd() * 0.1}.rgba();
	sa.wid = 2.5;
	sa.noi = 0.9;
	sa.out = 0;
	sa.xof = x;
	sa.yof = y;
	stroke(p, trmlist, sa, [](double) { return std::sin(1); });
	p.absorb(std::move(tx));
	p.absorb(std::move(tw));
}

void Tree::tree08(Painter& p, double x, double y, double hei, double wid, const Color& col, double noi) {
	double ang = normRand(-1, 1) * pi * 0.2;
	auto tr = branch(BranchArg{.hei = hei, .wid = wid, .ang = -pi / 2 + ang, .det = hei / 20, .ben = pi * 0.2});
	Pts trlist = tr.first;
	trlist.insert(trlist.end(), tr.second.rbegin(), tr.second.rend());

	Painter tw;
	for (size_t i = 0; i < trlist.size(); i++) {
		if (rnd() < 0.2) {
			// JS evaluates args left to right: dep first, then ang
			int dep8 = (int)std::floor(4 * rnd());
			double ang8 = -pi / 2 - ang * rnd();
			fracTree8(tw, x + trlist[i][0], y + trlist[i][1], dep8, ang8, 15, 0);
		} else if (i == (size_t)std::floor(trlist.size() / 2)) {
			fracTree8(tw, x + trlist[i][0], y + trlist[i][1], 3, -pi / 2 + ang, 15, 0);
		}
	}
	p.poly(trlist, PArg{.xof = x, .yof = y, .fil = "white", .str = col.rgba(), .wid = 0});
	SArg sa;
	sa.col = Color{100, 100, 100, 0.6 + rnd() * 0.1}.rgba();
	sa.wid = 2.5;
	sa.noi = 0.9;
	sa.out = 0;
	sa.xof = x;
	sa.yof = y;
	stroke(p, trlist, sa, [](double) { return std::sin(1); });
	p.absorb(std::move(tw));
}

} // namespace ss
