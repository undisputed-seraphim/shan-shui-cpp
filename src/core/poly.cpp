#include "core/poly.h"
#include <cmath>
#include <algorithm>
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
  if (den == 0) return std::nullopt;
  double x = (le1[1] - le0[1]) / den;
  double y = le0[0] * x + le0[1];
  auto onSeg = [](const Pt& p, const Pt* ln) {
    return std::min(ln[0][0], ln[1][0]) <= p[0] &&
           p[0] <= std::max(ln[0][0], ln[1][0]) &&
           std::min(ln[0][1], ln[1][1]) <= p[1] &&
           p[1] <= std::max(ln[0][1], ln[1][1]);
  };
  Pt p = {x, y};
  if (onSeg(p, ln0) && onSeg(p, ln1)) return p;
  return std::nullopt;
}

static bool ptInPoly(const Pt& pt, const Pts& plist) {
  int scount = 0;
  for (size_t i = 0; i < plist.size(); i++) {
    const Pt& np = plist[i != plist.size() - 1 ? i + 1 : 0];
    Pt far = {pt[0] + 999, pt[1] + 999};
    Pt seg0[2] = {plist[i], np};
    Pt seg1[2] = {pt, far};
    if (intersect(seg0, seg1)) scount++;
  }
  return scount % 2 == 1;
}

static bool lnInPoly(const Pt* ln, const Pts& plist) {
  double ep = 0.01;
  Pt lnc[2] = {
      {ln[0][0] * (1 - ep) + ln[1][0] * ep, ln[0][1] * (1 - ep) + ln[1][1] * ep},
      {ln[0][0] * ep + ln[1][0] * (1 - ep), ln[0][1] * ep + ln[1][1] * (1 - ep)}};
  for (size_t i = 0; i < plist.size(); i++) {
    const Pt& np = plist[i != plist.size() - 1 ? i + 1 : 0];
    Pt seg[2] = {plist[i], np};
    if (intersect(lnc, seg)) return false;
  }
  Pt mid = PolyTools::midPt({ln[0], ln[1]});
  if (!ptInPoly(mid, plist)) return false;
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
  for (double s : slist) P += s;
  return A / P;
}

// Returns [triangle, rest] (best ear cut). If none found: [plist, []]
static std::pair<Pts, Pts> bestEar(const Pts& plist, bool convex, bool optimize) {
  std::vector<std::pair<Pts, Pts>> cuts;
  for (size_t i = 0; i < plist.size(); i++) {
    const Pt& lp = plist[i != 0 ? i - 1 : plist.size() - 1];
    const Pt& np = plist[i != plist.size() - 1 ? i + 1 : 0];
    Pts qlist = plist;
    qlist.erase(qlist.begin() + (long)i);
    Pt ln[2] = {lp, np};
    if (convex || lnInPoly(ln, plist)) {
      Pts tri = {lp, plist[i], np};
      if (!optimize) return {tri, qlist};
      cuts.push_back({tri, qlist});
    }
  }
  Pts best = plist;
  Pts bestq;
  double bestRatio = 0;
  for (auto& c : cuts) {
    double r = sliverRatio(c.first);
    if (r >= bestRatio) {
      best = c.first;
      bestq = c.second;
      bestRatio = r;
    }
  }
  return {best, bestq};
}

static std::vector<Pts> shatter(const Pts& plist, double a) {
  if (plist.empty()) return {};
  if (areaOf(plist) < a) return {plist};
  auto slist = sidesOf(plist);
  size_t ind = 0;
  for (size_t i = 0; i < slist.size(); i++)
    if (slist[i] > slist[ind]) ind = i;
  size_t nind = (ind + 1) % plist.size();
  size_t lind = (ind + 2) % plist.size();
  Pt mid = PolyTools::midPt({plist[ind], plist[nind]});
  auto r1 = shatter({plist[ind], mid, plist[lind]}, a);
  auto r2 = shatter({plist[lind], plist[nind], mid}, a);
  r1.insert(r1.end(), r2.begin(), r2.end());
  return r1;
}

Pt PolyTools::midPt(const Pts& plist) {
  Pt acc = {0, 0};
  for (const auto& v : plist) {
    acc[0] += v[0] / (double)plist.size();
    acc[1] += v[1] / (double)plist.size();
  }
  return acc;
}

Pt PolyTools::midPt(std::initializer_list<Pt> pts) {
  Pt acc = {0, 0};
  for (const auto& v : pts) {
    acc[0] += v[0] / (double)pts.size();
    acc[1] += v[1] / (double)pts.size();
  }
  return acc;
}

std::vector<Pts> PolyTools::triangulate(const Pts& plist, const TriArgs& args) {
  if (plist.size() <= 3) {
    return shatter(plist, args.area);
  } else {
    auto cut = bestEar(plist, args.convex, args.optimize);
    auto r = shatter(cut.first, args.area);
    auto rest = triangulate(cut.second, args);
    r.insert(r.end(), rest.begin(), rest.end());
    return r;
  }
}

} // namespace ss
