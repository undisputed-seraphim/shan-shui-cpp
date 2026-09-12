#pragma once
#include "core/noise.h"
#include "draw/painter.h"
#include "draw/shapes.h"
#include <functional>

namespace ss {

namespace Tree {

inline constexpr auto kDefaultBend = [](double) { return 0.0; };
inline constexpr auto kDefaultBend07 = [](double xx) { return std::sqrt(xx) * 0.2; };

void tree01(
	Painter& p,
	double x,
	double y,
	double hei = 50,
	double wid = 3,
	const Color& col = {},
	double noi = 0.5);
void tree02(
	Painter& p,
	double x,
	double y,
	double hei = 16,
	double wid = 8,
	int clu = 5,
	const Color& col = {},
	double noi = 0.5);
template <typename Ben = decltype(kDefaultBend)>
void tree03(
	Painter& p,
	double x,
	double y,
	double hei = 50,
	double wid = 5,
	const Ben& ben = {},
	const Color& col = {},
	double noi = 0.5) {
	const int reso = 10;
	std::vector<std::array<double, 2>> nslist;
	for (int i = 0; i < reso; i++) {
		nslist.push_back({nse(i * 0.5), nse(i * 0.5, 0.5)});
	}

	Pts line1, line2;
	Painter blobs;
	for (int i = 0; i < reso; i++) {
		double nx = x + ben((double)i / reso) * 100;
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
				ba.ang = ((rnd() - 0.5) * pi) / 6;
				ba.col = Color{col.r, col.g, col.b, rnd() * 0.2 + col.a}.rgba();
				blob(blobs, bx, by, ba);
			}
		}
		line1.push_back({nx + ((nslist[i][0] - 0.5) * wid - wid / 2) * (reso - i) / reso, ny});
		line2.push_back({nx + ((nslist[i][1] - 0.5) * wid + wid / 2) * (reso - i) / reso, ny});
	}
	Pts lc = line1;
	lc.insert(lc.end(), line2.rbegin(), line2.rend());
	p.poly(lc, PArg{.fil = "white", .str = col.rgba(), .wid = 1.5});
	p.absorb(std::move(blobs));
}
void tree04(
	Painter& p,
	double x,
	double y,
	double hei = 300,
	double wid = 6,
	const Color& col = {},
	double noi = 0.5);
void tree05(
	Painter& p,
	double x,
	double y,
	double hei = 300,
	double wid = 5,
	const Color& col = {},
	double noi = 0.5);
void tree06(
	Painter& p,
	double x,
	double y,
	double hei = 100,
	double wid = 6,
	const Color& col = {},
	double noi = 0.5);
template <typename Ben = decltype(kDefaultBend07)>
void tree07(
	Painter& p,
	double x,
	double y,
	double hei = 60,
	double wid = 4,
	const Ben& ben = {},
	const Color& col = {100, 100, 100, 1.0},
	double noi = 0.5) {
	const int reso = 10;
	std::vector<std::array<double, 2>> nslist;
	for (int i = 0; i < reso; i++) {
		nslist.push_back({nse(i * 0.5), nse(i * 0.5, 0.5)});
	}

	Pts line1, line2;
	std::vector<Pts> T;
	for (int i = 0; i < reso; i++) {
		double nx = x + ben((double)i / reso) * 100;
		double ny = y - ((double)i * hei) / reso;
		if (i >= reso / 4.0) {
			for (int j = 0; j < 1; j++) {
				double bx = nx + (rnd() - 0.5) * wid * 1.2 * (reso - i) * 0.5;
				double by = ny + (rnd() - 0.5) * wid * 0.5;
				BArg ba;
				ba.len = rnd() * 50 + 20;
				ba.wid = rnd() * 12 + 12;
				ba.ang = (-rnd() * pi) / 6;
				ba.col = Color{col.r, col.g, col.b, col.a}.rgba();
				ba.ret = 1;
				Pts bpl = blob_points(bx, by, ba, [](double xx) {
					return xx <= 1 ? 2.75 * xx * std::pow(1 - xx, 1 / 1.8)
								   : 2.75 * (xx - 2) * std::pow(xx - 1, 1 / 1.8);
				});
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
		Pt m = PolyTools::centroid(tri);
		int c = (int)(nse(m[0] * 0.02, m[1] * 0.02) * 200 + 50);
		std::string co = Color{c, c, c, 0.8}.rgba();
		p.poly(tri, PArg{.fil = co, .str = co, .wid = 0});
	}
}
void tree08(
	Painter& p,
	double x,
	double y,
	double hei = 80,
	double wid = 1,
	const Color& col = {},
	double noi = 0.5);
} // namespace Tree

} // namespace ss
