#include "gen/arch.h"
#include "core/noise.h"
#include "draw/shapes.h"
#include "gen/man.h"
#include <cmath>

namespace ss {

namespace {

Pts flip(Pts ptlist, double axis = 0) {
	for (auto& pt : ptlist)
		pt[0] = axis - (pt[0] - axis);
	return ptlist;
}

std::vector<Pts> flipNested(std::vector<Pts> ptlist, double axis = 0) {
	for (auto& row : ptlist)
		row = flip(row, axis);
	return ptlist;
}

void hut(Painter& p, double xoff, double yoff, double hei = 40, double wid = 180, int tex = 300) {
	const int reso0 = 10, reso1 = 10;
	std::vector<Pts> ptlist;
	for (int i = 0; i < reso0; i++) {
		double heir = hei + hei * 0.2 * rnd();
		Pts row;
		for (int j = 0; j < reso1; j++) {
			double nx = wid * ((double)i / (reso0 - 1) - 0.5) * std::pow((double)j / (reso1 - 1), 0.7);
			double ny = heir * ((double)j / (reso1 - 1));
			row.push_back({nx, ny});
		}
		ptlist.push_back(std::move(row));
	}
	Pts hull = ptlist[0];
	hull.pop_back();
	Pts last = ptlist[ptlist.size() - 1];
	last.pop_back();
	hull.insert(hull.end(), last.rbegin(), last.rend());
	p.poly(hull, PArg{.xof = xoff, .yof = yoff, .fil = "white", .str = "none"});
	p.poly(ptlist[0], PArg{.xof = xoff, .yof = yoff, .fil = "none", .str = "rgba(100,100,100,0.3)", .wid = 2});
	p.poly(
		ptlist[ptlist.size() - 1],
		PArg{.xof = xoff, .yof = yoff, .fil = "none", .str = "rgba(100,100,100,0.3)", .wid = 2});
	TArg2 ta;
	ta.xof = xoff;
	ta.yof = yoff;
	ta.tex = tex;
	ta.wid = 1;
	ta.len = 0.25;
	ta.col = [](double) { return "rgba(120,120,120," + toFixed(0.3 + rnd() * 0.3, 3) + ")"; };
	ta.dis = []() { return wtrand([](double a) { return a * a; }); };
	ta.noi = [](double) { return 5.0; };
	texture(p, ptlist, ta);
}

struct BoxArg {
	double hei = 20, wid = 120, rot = 0.7, per = 4;
	bool tra = true, bot = true;
	double wei = 3;
	std::function<std::vector<Pts>(const std::array<Pt, 4>&)> dec;
};

struct DecoRect {
	Pt pul, pur, pdl, pdr;
};

void box(Painter& p, double xoff, double yoff, const BoxArg& a) {
	double mid = -a.wid * 0.5 + a.wid * a.rot;
	double bmid = -a.wid * 0.5 + a.wid * (1 - a.rot);
	std::vector<Pts> ptlist;
	ptlist.push_back(div({{-a.wid * 0.5, -a.hei}, {-a.wid * 0.5, 0}}, 5));
	ptlist.push_back(div({{a.wid * 0.5, -a.hei}, {a.wid * 0.5, 0}}, 5));
	if (a.bot) {
		ptlist.push_back(div({{-a.wid * 0.5, 0}, {mid, a.per}}, 5));
		ptlist.push_back(div({{a.wid * 0.5, 0}, {mid, a.per}}, 5));
	}
	ptlist.push_back(div({{mid, -a.hei}, {mid, a.per}}, 5));
	if (a.tra) {
		if (a.bot) {
			ptlist.push_back(div({{-a.wid * 0.5, 0}, {bmid, -a.per}}, 5));
			ptlist.push_back(div({{a.wid * 0.5, 0}, {bmid, -a.per}}, 5));
		}
		ptlist.push_back(div({{bmid, -a.hei}, {bmid, -a.per}}, 5));
	}

	double surf = (a.rot < 0.5) * 2 - 1;
	if (a.dec) {
		std::array<Pt, 4> rect = {
			Pt{surf * a.wid * 0.5, -a.hei},
			Pt{mid, -a.hei + a.per},
			Pt{surf * a.wid * 0.5, 0},
			Pt{mid, a.per},
		};
		auto deco = a.dec(rect);
		ptlist.insert(ptlist.end(), deco.begin(), deco.end());
	}

	Pts polist = {{-a.wid * 0.5, -a.hei}, {a.wid * 0.5, -a.hei}, {a.wid * 0.5, 0}, {mid, a.per}, {-a.wid * 0.5, 0}};
	if (!a.tra) {
		p.poly(polist, PArg{.xof = xoff, .yof = yoff, .fil = "white", .str = "none"});
	}
	for (const auto& row : ptlist) {
		SArg sa;
		sa.col = "rgba(100,100,100,0.4)";
		sa.noi = 1;
		sa.wid = a.wei;
		sa.fun = [](double) { return 1.0; };
		Pts shifted;
		for (const auto& v : row)
			shifted.push_back({v[0] + xoff, v[1] + yoff});
		stroke(p, shifted, sa);
	}
}

// deco(style, rect-ish args): returns list of polylines
std::vector<Pts> deco(int style, const std::array<Pt, 4>& r, double hsp0, double hsp1, double vsp0, double vsp1) {
	const Pt& pul = r[0];
	const Pt& pur = r[1];
	const Pt& pdl = r[2];
	const Pt& pdr = r[3];
	std::vector<Pts> plist;
	Pts dl = div({pul, pdl}, vsp1);
	Pts dr = div({pur, pdr}, vsp1);
	Pts du = div({pul, pur}, hsp1);
	Pts dd = div({pdl, pdr}, hsp1);

	if (style == 1) {
		Pt mlu = du[(size_t)hsp0];
		Pt mru = du[du.size() - 1 - (size_t)hsp0];
		Pt mld = dd[(size_t)hsp0];
		Pt mrd = dd[du.size() - 1 - (size_t)hsp0];
		for (size_t i = (size_t)vsp0; i < dl.size() - (size_t)vsp0; i += (size_t)vsp0) {
			Pt mml = div({mlu, mld}, vsp1)[i];
			Pt mmr = div({mru, mrd}, vsp1)[i];
			plist.push_back(div({mml, dl[i]}, 5));
			plist.push_back(div({mmr, dr[i]}, 5));
		}
		plist.push_back(div({mlu, mld}, 5));
		plist.push_back(div({mru, mrd}, 5));
	} else if (style == 2) {
		for (size_t i = (size_t)hsp0; i < du.size() - (size_t)hsp0; i += (size_t)hsp0) {
			plist.push_back(div({du[i], dd[i]}, 5));
		}
	} else if (style == 3) {
		Pt mlu = du[(size_t)hsp0];
		Pt mru = du[du.size() - 1 - (size_t)hsp0];
		Pt mld = dd[(size_t)hsp0];
		Pt mrd = dd[du.size() - 1 - (size_t)hsp0];
		for (size_t i = (size_t)vsp0; i < dl.size() - (size_t)vsp0; i += (size_t)vsp0) {
			Pt mml = div({mlu, mld}, vsp1)[i];
			Pt mmr = div({mru, mrd}, vsp1)[i];
			Pt mmu = div({mlu, mru}, vsp1)[i];
			Pt mmd = div({mld, mrd}, vsp1)[i];
			plist.push_back(div({mml, mmr}, 5));
			plist.push_back(div({mmu, mmd}, 5));
		}
		plist.push_back(div({mlu, mld}, 5));
		plist.push_back(div({mru, mrd}, 5));
	}
	return plist;
}

struct RailArg {
	double hei = 20, wid = 180, rot = 0.7, per = 4;
	int seg = 4;
	double wei = 1;
	bool tra = true, fro = true;
};

void rail(Painter& p, double xoff, double yoff, double seed, const RailArg& a) {
	double mid = -a.wid * 0.5 + a.wid * a.rot;
	double bmid = -a.wid * 0.5 + a.wid * (1 - a.rot);
	std::vector<Pts> ptlist;
	if (a.fro) {
		ptlist.push_back(div({{-a.wid * 0.5, 0}, {mid, a.per}}, a.seg));
		ptlist.push_back(div({{mid, a.per}, {a.wid * 0.5, 0}}, a.seg));
	}
	if (a.tra) {
		ptlist.push_back(div({{-a.wid * 0.5, 0}, {bmid, -a.per}}, a.seg));
		ptlist.push_back(div({{bmid, -a.per}, {a.wid * 0.5, 0}}, a.seg));
	}
	if (a.fro) {
		ptlist.push_back(div({{-a.wid * 0.5, -a.hei}, {mid, -a.hei + a.per}}, a.seg));
		ptlist.push_back(div({{mid, -a.hei + a.per}, {a.wid * 0.5, -a.hei}}, a.seg));
	}
	if (a.tra) {
		ptlist.push_back(div({{-a.wid * 0.5, -a.hei}, {bmid, -a.hei - a.per}}, a.seg));
		ptlist.push_back(div({{bmid, -a.hei - a.per}, {a.wid * 0.5, -a.hei}}, a.seg));
	}
	if (a.tra) {
		int open = (int)std::floor(rnd() * ptlist.size());
		ptlist[open].pop_back();
		ptlist[(open + ptlist.size()) % ptlist.size()].pop_back();
	}

	for (size_t i = 0; i < ptlist.size() / 2; i++) {
		for (size_t j = 0; j < ptlist[i].size(); j++) {
			ptlist[i][j][1] += (nse((double)i, j * 0.5, seed) - 0.5) * a.hei;
			size_t ci = (ptlist.size() / 2 + i) % ptlist.size();
			ptlist[ci][j % ptlist[ci].size()][1] += (nse(i + 0.5, j * 0.5, seed) - 0.5) * a.hei;
			Pts ln = div({ptlist[i][j], ptlist[ci][j % ptlist[ci].size()]}, 2);
			ln[0][0] += (rnd() - 0.5) * a.hei * 0.5;
			p.poly(ln, PArg{.xof = xoff, .yof = yoff, .fil = "none", .str = "rgba(100,100,100,0.5)", .wid = 2});
		}
	}
	for (const auto& row : ptlist) {
		SArg sa;
		sa.col = "rgba(100,100,100,0.5)";
		sa.noi = 0.5;
		sa.wid = a.wei;
		sa.fun = [](double) { return 1.0; };
		Pts shifted;
		for (const auto& v : row)
			shifted.push_back({v[0] + xoff, v[1] + yoff});
		stroke(p, shifted, sa);
	}
}

struct RoofArg {
	double hei = 20, wid = 120, rot = 0.7, per = 4, cor = 5, wei = 3;
	std::array<int, 2> pla = {0, 0}; // pla[0]==1 -> plaque with text
	std::string plaText;
};

void roof(Painter& p, double xoff, double yoff, const RoofArg& a) {
	auto opf = [&](const Pts& pl) { return a.rot < 0.5 ? flip(pl) : pl; };
	double rrot = a.rot < 0.5 ? 1 - a.rot : a.rot;
	double mid = -a.wid * 0.5 + a.wid * rrot;
	double bmid = -a.wid * 0.5 + a.wid * (1 - rrot);
	(void)bmid;
	double quat = (mid + a.wid * 0.5) * 0.5 - mid;

	std::vector<Pts> ptlist;
	ptlist.push_back(
		div(opf(
				{{-a.wid * 0.5 + quat, -a.hei - a.per / 2},
				 {-a.wid * 0.5 + quat * 0.5, -a.hei / 2 - a.per / 4},
				 {-a.wid * 0.5 - a.cor, 0}}),
			5));
	ptlist.push_back(
		div(opf({{mid + quat, -a.hei}, {(mid + quat + a.wid * 0.5) / 2, -a.hei / 2}, {a.wid * 0.5 + a.cor, 0}}), 5));
	ptlist.push_back(
		div(opf({{mid + quat, -a.hei}, {mid + quat / 2, -a.hei / 2 + a.per / 2}, {mid + a.cor, a.per}}), 5));
	ptlist.push_back(div(opf({{-a.wid * 0.5 - a.cor, 0}, {mid + a.cor, a.per}}), 5));
	ptlist.push_back(div(opf({{a.wid * 0.5 + a.cor, 0}, {mid + a.cor, a.per}}), 5));
	ptlist.push_back(div(opf({{-a.wid * 0.5 + quat, -a.hei - a.per / 2}, {mid + quat, -a.hei}}), 5));

	Pts polist = opf(
		{{-a.wid * 0.5, 0},
		 {-a.wid * 0.5 + quat, -a.hei - a.per / 2},
		 {mid + quat, -a.hei},
		 {a.wid * 0.5, 0},
		 {mid, a.per}});
	p.poly(polist, PArg{.xof = xoff, .yof = yoff, .fil = "white", .str = "none"});

	for (const auto& row : ptlist) {
		SArg sa;
		sa.col = "rgba(100,100,100,0.4)";
		sa.noi = 1;
		sa.wid = a.wei;
		sa.fun = [](double) { return 1.0; };
		Pts shifted;
		for (const auto& v : row)
			shifted.push_back({v[0] + xoff, v[1] + yoff});
		stroke(p, shifted, sa);
	}

	if (a.pla[0] == 1) {
		Pts pp = opf({{mid + quat / 2, -a.hei / 2 + a.per / 2}, {-a.wid * 0.5 + quat * 0.5, -a.hei / 2 - a.per / 4}});
		if (pp[0][0] > pp[1][0])
			std::swap(pp[0], pp[1]);
		Pt mp = PolyTools::midPt(pp);
		double ang = std::atan2(pp[1][1] - pp[0][1], pp[1][0] - pp[0][0]);
		double adeg = (ang * 180) / PI;
		TArg t;
		t.fontSize = a.hei * 0.6;
		t.x = mp[0] + xoff;
		t.y = mp[1] + yoff;
		t.rot = adeg;
		t.content = a.plaText;
		p.text(t);
	}
}

struct PagroofArg {
	double hei = 20, wid = 120, rot = 0.7, per = 4, cor = 10;
	int sid = 4;
	double wei = 3;
};

void pagroof(Painter& p, double xoff, double yoff, const PagroofArg& a) {
	std::vector<Pts> ptlist;
	Pts polist = {{0, -a.hei}};
	for (int i = 0; i < a.sid; i++) {
		double fx = a.wid * ((double)i / (a.sid - 1) - 0.5);
		double fy = a.per * (1 - std::fabs((double)i / (a.sid - 1) - 0.5) * 2);
		double fxx = (a.wid + a.cor) * ((double)i / (a.sid - 1) - 0.5);
		if (i > 0) {
			ptlist.push_back({ptlist[ptlist.size() - 1][2], {fxx, fy}});
		}
		ptlist.push_back({{0, -a.hei}, {fx * 0.5, (-a.hei + fy) * 0.5}, {fxx, fy}});
		polist.push_back({fxx, fy});
	}
	p.poly(polist, PArg{.xof = xoff, .yof = yoff, .fil = "white", .str = "none"});
	for (const auto& row : ptlist) {
		SArg sa;
		sa.col = "rgba(100,100,100,0.4)";
		sa.noi = 1;
		sa.wid = a.wei;
		sa.fun = [](double) { return 1.0; };
		Pts shifted;
		for (const auto& v : div(row, 5))
			shifted.push_back({v[0] + xoff, v[1] + yoff});
		stroke(p, shifted, sa);
	}
}

} // namespace

void Arch::arch01(Painter& p, double xoff, double yoff, double seed, double hei, double wid, double rot, double per) {
	double pp = 0.4 + rnd() * 0.2;
	double h0 = hei * pp;
	double h1 = hei * (1 - pp);

	hut(p, xoff, yoff - hei, h0, wid);

	BoxArg ba;
	ba.hei = h1;
	ba.wid = (wid * 2) / 3;
	ba.per = per;
	ba.bot = false;
	box(p, xoff, yoff, ba);

	RailArg ra;
	ra.tra = true;
	ra.fro = false;
	ra.hei = 10;
	ra.wid = wid;
	ra.per = per * 2;
	ra.seg = (3 + rnd() * 3);
	rail(p, xoff, yoff, seed, ra);

	int mcnt = (int)randChoice<double>({0, 1, 1, 2});
	if (mcnt == 1) {
		// JS evaluates the position arg (normRand) before the args object (fli)
		double mx = xoff + normRand(-wid / 3, wid / 3);
		ManArg ma;
		ma.sca = 0.42;
		ma.fli = randChoice<bool>({true, false});
		Man::man(p, mx, yoff, ma);
	} else if (mcnt == 2) {
		ManArg ma;
		ma.sca = 0.42;
		ma.fli = false;
		Man::man(p, xoff + normRand(-wid / 4, -wid / 5), yoff, ma);
		ma.fli = true;
		Man::man(p, xoff + normRand(wid / 5, wid / 4), yoff, ma);
	}

	RailArg ra2;
	ra2.tra = false;
	ra2.fro = true;
	ra2.hei = 10;
	ra2.wid = wid;
	ra2.per = per * 2;
	ra2.seg = (3 + rnd() * 3);
	rail(p, xoff, yoff, seed, ra2);
}

void Arch::arch02(
	Painter& p,
	double xoff,
	double yoff,
	double seed,
	double hei,
	double wid,
	double rot,
	double per,
	int sto,
	int sty,
	bool rai) {
	double hoff = 0;
	for (int i = 0; i < sto; i++) {
		BoxArg ba;
		ba.tra = false;
		ba.hei = hei;
		ba.wid = wid * std::pow(0.85, i);
		ba.rot = rot;
		ba.wei = 1.5;
		ba.per = per;
		double hsp[4][2] = {{0, 0}, {1, 5}, {1, 5}, {1, 4}};
		double vsp[4][2] = {{0, 0}, {1, 2}, {1, 2}, {1, 3}};
		ba.dec = [sty, hsp, vsp](const std::array<Pt, 4>& r) {
			return deco(sty, r, hsp[sty][0], hsp[sty][1], vsp[sty][0], vsp[sty][1]);
		};
		box(p, xoff, yoff - hoff, ba);

		if (rai) {
			RailArg ra;
			ra.wid = wid * std::pow(0.85, i) * 1.1;
			ra.hei = hei / 2;
			ra.per = per;
			ra.rot = rot;
			ra.wei = 0.5;
			ra.tra = false;
			rail(p, xoff, yoff - hoff, i * 0.2, ra);
		}

		RoofArg rf;
		rf.hei = hei;
		rf.wid = wid * std::pow(0.9, i);
		rf.rot = rot;
		rf.wei = 1.5;
		rf.per = per;
		if (sto == 1 && rnd() < 1.0 / 3) {
			rf.pla[0] = 1;
			rf.plaText = "Pizza Hut";
		}
		roof(p, xoff, yoff - hoff - hei, rf);

		hoff += hei * 1.5;
	}
}

void Arch::arch03(
	Painter& p,
	double xoff,
	double yoff,
	double seed,
	double hei,
	double wid,
	double rot,
	double per,
	int sto) {
	double hoff = 0;
	for (int i = 0; i < sto; i++) {
		BoxArg ba;
		ba.tra = false;
		ba.hei = hei;
		ba.wid = wid * std::pow(0.85, i);
		ba.rot = rot;
		ba.wei = 1.5;
		ba.per = per / 2;
		ba.dec = [](const std::array<Pt, 4>& r) { return deco(1, r, 1, 4, 1, 2); };
		box(p, xoff, yoff - hoff, ba);

		RailArg ra;
		ra.seg = 5;
		ra.wid = wid * std::pow(0.85, i) * 1.1;
		ra.hei = hei / 2;
		ra.per = per / 2;
		ra.rot = rot;
		ra.wei = 0.5;
		ra.tra = false;
		rail(p, xoff, yoff - hoff, i * 0.2, ra);

		PagroofArg pa;
		pa.hei = hei * 1.5;
		pa.wid = wid * std::pow(0.9, i);
		pa.rot = rot;
		pa.wei = 1.5;
		pa.per = per;
		pagroof(p, xoff, yoff - hoff - hei, pa);

		hoff += hei * 1.5;
	}
}

void Arch::arch04(
	Painter& p,
	double xoff,
	double yoff,
	double seed,
	double hei,
	double wid,
	double rot,
	double per,
	int sto) {
	double hoff = 0;
	for (int i = 0; i < sto; i++) {
		BoxArg ba;
		ba.tra = true;
		ba.hei = hei;
		ba.wid = wid * std::pow(0.85, i);
		ba.rot = rot;
		ba.wei = 1.5;
		ba.per = per / 2;
		ba.dec = [](const std::array<Pt, 4>&) { return std::vector<Pts>{}; };
		box(p, xoff, yoff - hoff, ba);

		RailArg ra;
		ra.seg = 3;
		ra.wid = wid * std::pow(0.85, i) * 1.2;
		ra.hei = hei / 3;
		ra.per = per / 2;
		ra.rot = rot;
		ra.wei = 0.5;
		ra.tra = true;
		rail(p, xoff, yoff - hoff, i * 0.2, ra);

		PagroofArg pa;
		pa.hei = hei * 1;
		pa.wid = wid * std::pow(0.9, i);
		pa.rot = rot;
		pa.wei = 1.5;
		pa.per = per;
		pagroof(p, xoff, yoff - hoff - hei, pa);

		hoff += hei * 1.2;
	}
}

void Arch::boat01(Painter& p, double xoff, double yoff, double seed, double len, double sca, bool fli) {
	double dir = fli ? -1 : 1;
	ManArg ma;
	ma.ite = [](Painter& p2, const Pt& a, const Pt& b, bool f) { Man::stick01(p2, a, b, StickArg{.fli = f}); };
	ma.hat = [](Painter& p2, const Pt& a, const Pt& b, const HatArg& h) { Man::hat02(p2, a, b, h); };
	ma.sca = 0.5 * sca;
	ma.fli = !fli;
	ma.len = {0, 30, 20, 30, 10, 30, 30, 30, 30};
	Man::man(p, xoff + 20 * sca * dir, yoff, ma);

	Pts plist1, plist2;
	auto fun1 = [sca](double x) { return std::pow(std::sin(x * PI), 0.5) * 7 * sca; };
	auto fun2 = [sca](double x) { return std::pow(std::sin(x * PI), 0.5) * 10 * sca; };
	for (double i = 0; i < len * sca; i += 5 * sca) {
		plist1.push_back({i * dir, fun1(i / len)});
		plist2.push_back({i * dir, fun2(i / len)});
	}
	Pts plist = plist1;
	plist.insert(plist.end(), plist2.rbegin(), plist2.rend());
	p.poly(plist, PArg{.xof = xoff, .yof = yoff, .fil = "white"});
	SArg sa;
	sa.wid = 1;
	sa.fun = [](double x) { return std::sin(x * PI * 2); };
	sa.col = "rgba(100,100,100,0.4)";
	Pts shifted;
	for (const auto& v : plist)
		shifted.push_back({xoff + v[0], yoff + v[1]});
	stroke(p, shifted, sa);
}

void Arch::transmissionTower01(Painter& p, double xoff, double yoff, double seed, double hei, double wid) {
	auto toGlobal = [&](const Pt& v) { return Pt{v[0] + xoff, v[1] + yoff}; };
	auto quickstroke = [&](const Pts& pl) {
		Pts shifted;
		for (const auto& v : div(pl, 5))
			shifted.push_back(toGlobal(v));
		SArg sa;
		sa.wid = 1;
		sa.fun = [](double) { return 0.5; };
		sa.col = "rgba(100,100,100,0.4)";
		stroke(p, shifted, sa);
	};

	Pt p00 = {-wid * 0.05, -hei};
	Pt p01 = {wid * 0.05, -hei};
	Pt p10 = {-wid * 0.1, -hei * 0.9};
	Pt p11 = {wid * 0.1, -hei * 0.9};
	Pt p20 = {-wid * 0.2, -hei * 0.5};
	Pt p21 = {wid * 0.2, -hei * 0.5};
	Pt p30 = {-wid * 0.5, 0};
	Pt p31 = {wid * 0.5, 0};

	Pts bch = {{0.7, -0.85}, {1, -0.675}, {0.7, -0.5}};
	for (const auto& b : bch) {
		quickstroke({{-b[0] * wid, b[1] * hei}, {b[0] * wid, b[1] * hei}});
		quickstroke({{-b[0] * wid, b[1] * hei}, {0, (b[1] - 0.05) * hei}});
		quickstroke({{b[0] * wid, b[1] * hei}, {0, (b[1] - 0.05) * hei}});
		quickstroke({{-b[0] * wid, b[1] * hei}, {-b[0] * wid, (b[1] + 0.1) * hei}});
		quickstroke({{b[0] * wid, b[1] * hei}, {b[0] * wid, (b[1] + 0.1) * hei}});
	}

	Pts l10 = div({p00, p10, p20, p30}, 5);
	Pts l11 = div({p01, p11, p21, p31}, 5);
	for (size_t i = 0; i + 1 < l10.size(); i++) {
		quickstroke({l10[i], l11[i + 1]});
		quickstroke({l11[i], l10[i + 1]});
	}
	quickstroke({p00, p01});
	quickstroke({p10, p11});
	quickstroke({p20, p21});
	quickstroke({p00, p10, p20, p30});
	quickstroke({p01, p11, p21, p31});
}

} // namespace ss
