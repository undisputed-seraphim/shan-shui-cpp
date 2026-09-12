#include "scene/scene.h"
#include "core/noise.h"
#include "core/rng.h"
#include "core/util.h"
#include "draw/painter.h"
#include "gen/arch.h"
#include "gen/man.h"
#include "gen/mount.h"
#include "gen/tree.h"
#include "gen/water.h"
#include <algorithm>
#include <cmath>

namespace ss {

namespace {

struct Plan {
	std::string tag;
	double x, y, h;
};

// mountplanner(xmin, xmax): returns planned features
std::vector<Plan> mountplanner(Scene& sc, double xmin, double xmax) {
	auto locmax = [](double x, double y, const std::function<double(double, double)>& f, int r) {
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
	};

	std::vector<Plan> reg;
	auto chadd = [&reg](const Plan& r, double mind = 10) {
		for (const auto& k : reg) {
			if (std::fabs(k.x - r.x) < mind)
				return false;
		}
		reg.push_back(r);
		return true;
	};

	const double samp = 0.03;
	auto ns = [&](double x, double y) { return std::max(nse(x * samp) - 0.55, 0.0) * 2; };
	auto nns = [&](double x) { return 1 - nse(x * samp); };
	auto nnns = [&](double x, double y) { return std::max(nse(x * samp * 2, 2) - 0.55, 0.0) * 2; };
	auto yr = [&](double x) { return nse(x * 0.01, pi); };
	(void)nns;
	(void)nnns;

	const double xstep = 5;
	const double mwid = 200;
	for (double i = xmin; i < xmax; i += xstep) {
		int i1 = (int)std::floor(i / xstep);
		// JS: MEM.planmtx[i1] = MEM.planmtx[i1] || 0 (NaN -> 0)
		auto it = sc.planmtx.find(i1);
		double v = it == sc.planmtx.end() ? std::nan("") : it->second;
		sc.planmtx[i1] = (std::isnan(v) || v == 0) ? 0 : v;
	}

	double jLast = 0;
	for (double i = xmin; i < xmax; i += xstep) {
		for (double j = 0; j < yr(i) * 480; j += 30) {
			if (locmax(i, j, ns, 2)) {
				double xof = i + 2 * (rnd() - 0.5) * 500;
				double yof = j + 300;
				Plan r{"mount", xof, yof, ns(i, j)};
				if (chadd(r)) {
					for (int k = (int)std::floor((xof - mwid) / xstep); k < (xof + mwid) / xstep; k++) {
						// JS: planmtx[k] += 1 (undefined + 1 = NaN)
						auto it = sc.planmtx.find(k);
						if (it == sc.planmtx.end())
							sc.planmtx[k] = std::nan("");
						else
							it->second += 1;
					}
				}
			}
		}
		if (std::fabs(i) - 1000 * std::floor(std::fabs(i) / 1000) < std::max(1.0, xstep - 1)) {
			Plan r{"distmount", i, 280 - rnd() * 50, ns(i, jLast)};
			chadd(r);
		}
	}
	for (double i = xmin; i < xmax; i += xstep) {
		if (sc.planmtx[(int)std::floor(i / xstep)] == 0) {
			if (rnd() < 0.01) {
				for (double j = 0; j < 4 * rnd(); j++) {
					Plan r{"flatmount", i + 2 * (rnd() - 0.5) * 700, 700 - j * 50, ns(i, j)};
					chadd(r);
				}
			}
		}
	}
	for (double i = xmin; i < xmax; i += xstep) {
		if (rnd() < 0.2) {
			Plan r{"boat", i, 300 + rnd() * 390, 0};
			chadd(r, 400);
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
		if (chunks.empty()) {
			chunks.push_back(nch);
			return;
		}
		if (nch.y <= chunks[0].y) {
			chunks.insert(chunks.begin(), nch);
			return;
		}
		if (nch.y >= chunks[chunks.size() - 1].y) {
			chunks.push_back(nch);
			return;
		}
		for (size_t j = 0; j + 1 < chunks.size(); j++) {
			if (chunks[j].y <= nch.y && nch.y <= chunks[j + 1].y) {
				chunks.insert(chunks.begin() + (long)(j + 1), nch);
				return;
			}
		}
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
	return fmtNum(cursx) + " 0 " + fmtNum(windx / zoom) + " " + fmtNum(windy / zoom);
}

std::string Scene::getView(double vcursx, double vwindx) {
	cursx = vcursx;
	windx = vwindx;
	chunkloader(cursx, cursx + windx);
	chunkrender(cursx, cursx + windx);
	std::string s;
	s.reserve(canv.size() + 256);
	s += "<svg id='SVG' xmlns='http://www.w3.org/2000/svg' width='";
	s += fmtNum(windx);
	s += "' height='";
	s += fmtNum(windy);
	s += "' style='mix-blend-mode:multiply;'";
	s += "viewBox = '";
	s += calcViewBox();
	s += "'><g id='G' transform='translate(0,0)'>";
	s += canv;
	s += "</g></svg>";
	return s;
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
