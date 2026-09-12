#include "core/poly.h"
#include <algorithm>
#include <cmath>
#include <optional>

namespace ss {

static std::array<double, 2> lineExpr(const Pt& pt0, const Pt& pt1) {
	double den = pt1[0] - pt0[0];
	double m = den == 0 ? INFINITY : (pt1[1] - pt0[1]) / den;
	double k = pt0[1] - m * pt0[0];
	return {m, k};
}

static std::optional<Pt> intersect(const Pt* ln0, const Pt* ln1) {
	auto le0 = lineExpr(ln0[0], ln0[1]);
	auto le1 = lineExpr(ln1[0], ln1[1]);
	double den = le0[0] - le1[0];
	if (den == 0)
		return std::nullopt;
	double x = (le1[1] - le0[1]) / den;
	double y = le0[0] * x + le0[1];
	auto onSeg = [](const Pt& p, const Pt* ln) {
		return std::min(ln[0][0], ln[1][0]) <= p[0] && p[0] <= std::max(ln[0][0], ln[1][0]) &&
			   std::min(ln[0][1], ln[1][1]) <= p[1] && p[1] <= std::max(ln[0][1], ln[1][1]);
	};
	Pt p = {x, y};
	if (onSeg(p, ln0) && onSeg(p, ln1))
		return p;
	return std::nullopt;
}

static bool ptInPoly(const Pt& pt, const Pts& plist) {
	int scount = 0;
	for (size_t i = 0; i < plist.size(); i++) {
		const Pt& np = plist[i != plist.size() - 1 ? i + 1 : 0];
		Pt far = {pt[0] + 999, pt[1] + 999};
		Pt seg0[2] = {plist[i], np};
		Pt seg1[2] = {pt, far};
		if (intersect(seg0, seg1))
			scount++;
	}
	return scount % 2 == 1;
}

static bool lnInPoly(const Pt* ln, const Pts& plist) {
	double ep = 0.01;
	Pt lnc[2] = {
		{std::lerp(ln[0][0], ln[1][0], ep), std::lerp(ln[0][1], ln[1][1], ep)},
		{std::lerp(ln[1][0], ln[0][0], ep), std::lerp(ln[1][1], ln[0][1], ep)}};
	for (size_t i = 0; i < plist.size(); i++) {
		const Pt& np = plist[i != plist.size() - 1 ? i + 1 : 0];
		Pt seg[2] = {plist[i], np};
		if (intersect(lnc, seg))
			return false;
	}
	Pt mid = PolyTools::centroid({ln[0], ln[1]});
	if (!ptInPoly(mid, plist))
		return false;
	return true;
}

static std::vector<double> sidesOf(const Pts& plist) {
	std::vector<double> slist;
	slist.reserve(plist.size());
	for (size_t i = 0; i < plist.size(); i++) {
		const Pt& np = plist[i != plist.size() - 1 ? i + 1 : 0];
		double s = std::hypot(np[0] - plist[i][0], np[1] - plist[i][1]);
		slist.push_back(s);
	}
	return slist;
}

static double areaOf(const Pts& plist) {
	auto slist = sidesOf(plist);
	double a = slist[0], b = slist[1], c = slist[2];
	double s = (a + b + c) / 2;
	return std::sqrt(s * (s - a) * (s - b) * (s - c));
}

static double sliverRatio(const Pts& plist) {
	double A = areaOf(plist);
	auto slist = sidesOf(plist);
	double P = 0;
	for (double s : slist)
		P += s;
	return A / P;
}

// Ear clipping over the vertex ring without materializing rest-polygon
// copies. !optimize returns the first valid ear (ascending index);
// optimize picks the best sliver ratio, last maximum wins (as before).
static size_t bestEarIndex(const Pts& plist, bool convex, bool optimize) {
	const size_t n = plist.size();
	const size_t none = std::string::npos;
	if (!optimize) {
		for (size_t i = 0; i < n; i++) {
			const Pt& lp = plist[i != 0 ? i - 1 : n - 1];
			const Pt& np = plist[i != n - 1 ? i + 1 : 0];
			Pt ln[2] = {lp, np};
			if (convex || lnInPoly(ln, plist))
				return i;
		}
		return none;
	}
	size_t bestIdx = none;
	double bestRatio = 0;
	for (size_t i = 0; i < n; i++) {
		const Pt& lp = plist[i != 0 ? i - 1 : n - 1];
		const Pt& np = plist[i != n - 1 ? i + 1 : 0];
		Pt ln[2] = {lp, np};
		if (convex || lnInPoly(ln, plist)) {
			Pts tri = {lp, plist[i], np};
			double r = sliverRatio(tri);
			if (r >= bestRatio) {
				bestRatio = r;
				bestIdx = i;
			}
		}
	}
	return bestIdx;
}

// Recursive splitting turned into an explicit stack; the children are
// pushed so the pieces come out in the original depth-first order.
static std::vector<Pts> shatter(const Pts& plist, double a) {
	std::vector<Pts> out;
	std::vector<Pts> stack;
	stack.push_back(plist);
	while (!stack.empty()) {
		Pts cur = std::move(stack.back());
		stack.pop_back();
		if (cur.empty())
			continue;
		if (areaOf(cur) < a) {
			out.push_back(std::move(cur));
			continue;
		}
		auto slist = sidesOf(cur);
		size_t ind = 0;
		for (size_t i = 0; i < slist.size(); i++)
			if (slist[i] > slist[ind])
				ind = i;
		size_t nind = (ind + 1) % cur.size();
		size_t lind = (ind + 2) % cur.size();
		Pt mid = PolyTools::centroid({cur[ind], cur[nind]});
		stack.push_back({cur[lind], cur[nind], mid});
		stack.push_back({cur[ind], mid, cur[lind]});
	}
	return out;
}

Pt PolyTools::centroid(const Pts& plist) {
	Pt acc = {0, 0};
	for (const auto& v : plist) {
		acc[0] += v[0] / (double)plist.size();
		acc[1] += v[1] / (double)plist.size();
	}
	return acc;
}

Pt PolyTools::centroid(std::initializer_list<Pt> pts) {
	Pt acc = {0, 0};
	for (const auto& v : pts) {
		acc[0] += v[0] / (double)pts.size();
		acc[1] += v[1] / (double)pts.size();
	}
	return acc;
}

std::vector<Pts> PolyTools::triangulate(const Pts& plist, const TriArgs& args) {
	if (plist.size() <= 3)
		return shatter(plist, args.area);
	const size_t n = plist.size();
	size_t bestIdx = bestEarIndex(plist, args.convex, args.optimize);
	if (bestIdx == std::string::npos) {
		// no ear to clip: fall back to splitting alone (as before)
		return shatter(plist, args.area);
	}
	const Pt& lp = plist[bestIdx != 0 ? bestIdx - 1 : n - 1];
	const Pt& np = plist[bestIdx != n - 1 ? bestIdx + 1 : 0];
	Pts tri = {lp, plist[bestIdx], np};
	Pts rest;
	rest.reserve(n - 1);
	rest.insert(rest.end(), plist.begin(), plist.begin() + (long)bestIdx);
	rest.insert(rest.end(), plist.begin() + (long)bestIdx + 1, plist.end());
	auto r = shatter(tri, args.area);
	auto restTris = triangulate(rest, args);
	r.insert(r.end(), restTris.begin(), restTris.end());
	return r;
}

} // namespace ss
