#include "scene/scene.h"
#include "core/noise.h"
#include "core/rng.h"
#include "core/scratch.h"
#include "core/util.h"
#include "draw/painter.h"
#include "gen/arch.h"
#include "gen/man.h"
#include "gen/mount.h"
#include "gen/tree.h"
#include "gen/water.h"
#include <algorithm>
#include <cmath>
#include <format>

namespace ss {

namespace {

struct Plan {
	std::string tag;
	double x, y, h;
};

constexpr double kSamp = 0.03;
constexpr double kXstep = 5;
constexpr double kMwid = 200;

// planner helpers (free functions; see §10 for parallel generation)

template <typename F>
bool locMax(double x, double y, const F& f, int r) {
	double z0 = f(x, y);
	if (z0 <= 0.3)
		return false;
	for (int i = (int)(x - r); i < x + r; i++) {
		for (int j = (int)(y - r); j < y + r; j++) {
			if (f(i, j) > z0)
				return false;
		}
	}
	return true;
}

bool chadd(std::vector<Plan>& reg, const Plan& r, double mind = 10) {
	for (const auto& k : reg) {
		if (std::fabs(k.x - r.x) < mind)
			return false;
	}
	reg.push_back(r);
	return true;
}

double ns(double x, double y) { return std::max(nse(x * kSamp) - 0.55, 0.0) * 2; }
double yr(double x) { return nse(x * 0.01, pi); }

// mountplanner(xmin, xmax): returns planned features
std::vector<Plan> mountplanner(Scene& sc, double xmin, double xmax) {
	std::vector<Plan> reg;

	// per-slab occupancy: materialize this slab's cells so mountain spans
	// accumulate; cells outside the slab stay untouched (the JS version's
	// NaN marks there read back as 0, i.e. unoccupied)
	for (double i = xmin; i < xmax; i += kXstep)
		sc.planmtx[(int)std::floor(i / kXstep)] += 0;

	double jLast = 0;
	for (double i = xmin; i < xmax; i += kXstep) {
		for (double j = 0; j < yr(i) * 480; j += 30) {
			if (locMax(i, j, ns, 2)) {
				double xof = i + 2 * (rnd() - 0.5) * 500;
				double yof = j + 300;
				Plan r{"mount", xof, yof, ns(i, j)};
				if (chadd(reg, r)) {
					for (int k = (int)std::floor((xof - kMwid) / kXstep); k < (xof + kMwid) / kXstep; k++) {
						// mark the mountain's span; never-covered cells stay
						// unoccupied (JS wrote NaN there, which also read 0)
						auto it = sc.planmtx.find(k);
						if (it != sc.planmtx.end())
							it->second += 1;
					}
				}
			}
		}
		if (std::fabs(i) - 1000 * std::floor(std::fabs(i) / 1000) < std::max(1.0, kXstep - 1)) {
			Plan r{"distmount", i, 280 - rnd() * 50, ns(i, jLast)};
			chadd(reg, r);
		}
	}
	for (double i = xmin; i < xmax; i += kXstep) {
		if (sc.planmtx[(int)std::floor(i / kXstep)] == 0) {
			if (rnd() < 0.01) {
				for (double j = 0; j < 4 * rnd(); j++) {
					Plan r{"flatmount", i + 2 * (rnd() - 0.5) * 700, 700 - j * 50, ns(i, j)};
					chadd(reg, r);
				}
			}
		}
	}
	for (double i = xmin; i < xmax; i += kXstep) {
		if (rnd() < 0.2) {
			Plan r{"boat", i, 300 + rnd() * 390, 0};
			chadd(reg, r, 400);
		}
	}
	return reg;
}

} // namespace

void Scene::seed(const std::string& s) { Rng::inst().seed(s); }

void Scene::reset() {
	canv.clear();
	chunks.clear();
	xmin = 0;
	xmax = 0;
	cursx = 0;
	planmtx.clear();
}

void Scene::chunkloader(double vxmin, double vxmax) {
	auto add = [&](const Chunk& nch) {
		// chunks stay sorted by y; equal-y chunks at the current maximum
		// append after the run, elsewhere they insert before it
		if (chunks.empty() || nch.y >= chunks.back().y) {
			chunks.push_back(nch);
			return;
		}
		auto lb = std::lower_bound(
			chunks.begin(), chunks.end(), nch.y, [](const Chunk& c, double y) { return c.y < y; });
		chunks.insert(lb, nch);
	};

	while (vxmax > xmax - cwid || vxmin < xmin + cwid) {
		std::vector<Plan> plan;
		if (vxmax > xmax - cwid) {
			plan = mountplanner(*this, xmax, xmax + cwid);
			xmax = xmax + cwid;
		} else {
			plan = mountplanner(*this, xmin - cwid, xmin);
			xmin = xmin - cwid;
		}

		for (size_t i = 0; i < plan.size(); i++) {
			const Plan& pl = plan[i];
			ScratchScope scratch(1 << 20); // generator temporaries for this feature
			if (pl.tag == "mount") {
				Painter sp;
				Mount::mountain(sp, pl.x, pl.y, i * 2 * rnd());
				add(Chunk{pl.tag, pl.x, pl.y, sp.toSvg()});
				Painter sp2;
				water(sp2, pl.x, pl.y, i * 2);
				add(Chunk{"water", pl.x, pl.y - 10000, sp2.toSvg()});
			} else if (pl.tag == "flatmount") {
				Painter sp;
				Mount::FlatMountArg fa;
				double fseed = 2 * rnd() * pi;
				fa.wid = 600 + rnd() * 400;
				fa.hei = 100;
				fa.cho = 0.5 + rnd() * 0.2;
				Mount::flatMount(sp, pl.x, pl.y, fseed, fa);
				add(Chunk{pl.tag, pl.x, pl.y, sp.toSvg()});
			} else if (pl.tag == "distmount") {
				Painter sp;
				Mount::DistMountArg da;
				da.hei = 150;
				double seedv = rnd() * 100;
				da.len = (double)randChoice<double>({500.0, 1000.0, 1500.0});
				Mount::distMount(sp, pl.x, pl.y, seedv, da);
				add(Chunk{pl.tag, pl.x, pl.y, sp.toSvg()});
			} else if (pl.tag == "boat") {
				Painter sp;
				double bseed = rnd();
				bool bfli = randChoice<bool>({true, false});
				Arch::boat01(sp, pl.x, pl.y, bseed, 120, pl.y / 800, bfli);
				add(Chunk{pl.tag, pl.x, pl.y, sp.toSvg()});
			}
		}
	}
}

void Scene::chunkrender(double vxmin, double vxmax) {
	canv.clear();
	for (const auto& c : chunks) {
		if (vxmin - cwid < c.x && c.x < vxmax + cwid) {
			canv += c.canv;
		}
	}
}

std::string Scene::calcViewBox() const {
	double zoom = 1.142;
	return std::format("{} 0 {} {}", cursx, windx / zoom, windy / zoom);
}

std::string Scene::getView(double vcursx, double vwindx) {
	cursx = vcursx;
	windx = vwindx;
	chunkloader(cursx, cursx + windx);
	chunkrender(cursx, cursx + windx);
	return std::format(
		"<svg id='SVG' xmlns='http://www.w3.org/2000/svg' width='{}' height='{}'"
		" style='mix-blend-mode:multiply;'viewBox = '{}'>"
		"<g id='G' transform='translate(0,0)'>{}</g></svg>",
		windx,
		windy,
		calcViewBox(),
		canv);
}

// per-generator helpers for tests
std::string genTree04(double x, double y, double hei, double wid) {
	Painter sp;
	Tree::tree04(sp, x, y, hei, wid);
	return sp.toSvg();
}
std::string genMountain(double x, double y, double seed, double hei, double wid, int tex) {
	Painter sp;
	Mount::MountainArg ma;
	ma.hei = hei;
	ma.wid = wid;
	ma.tex = tex;
	Mount::mountain(sp, x, y, seed, ma);
	return sp.toSvg();
}
std::string genArch01(double x, double y, double seed) {
	Painter sp;
	Arch::arch01(sp, x, y, seed);
	return sp.toSvg();
}
std::string genMan(double x, double y, double sca) {
	Painter sp;
	ManArg ma;
	ma.sca = sca;
	Man::man(sp, x, y, ma);
	return sp.toSvg();
}

} // namespace ss
