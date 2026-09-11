#include "gen/man.h"
#include "draw/shapes.h"
#include "core/noise.h"
#include <cmath>
#include <algorithm>

namespace ss {

// Builds the ribbon around a polyline (like stroke, with end caps)
static std::pair<Pts, Pts> expand(const Pts& ptlist,
                                  const std::function<double(double)>& wfun) {
  Pts vtxlist0, vtxlist1;
  double n0 = rnd() * 10;
  for (size_t i = 1; i + 1 < ptlist.size(); i++) {
    double w = wfun((double)i / ptlist.size());
    double a1 = std::atan2(ptlist[i][1] - ptlist[i - 1][1],
                           ptlist[i][0] - ptlist[i - 1][0]);
    double a2 = std::atan2(ptlist[i][1] - ptlist[i + 1][1],
                           ptlist[i][0] - ptlist[i + 1][0]);
    double a = (a1 + a2) / 2;
    if (a < a2) a += PI;
    vtxlist0.push_back({ptlist[i][0] + w * std::cos(a),
                        ptlist[i][1] + w * std::sin(a)});
    vtxlist1.push_back({ptlist[i][0] - w * std::cos(a),
                        ptlist[i][1] - w * std::sin(a)});
  }
  size_t l = ptlist.size() - 1;
  double a0 = std::atan2(ptlist[1][1] - ptlist[0][1],
                         ptlist[1][0] - ptlist[0][0]) - PI / 2;
  double a1 = std::atan2(ptlist[l][1] - ptlist[l - 1][1],
                         ptlist[l][0] - ptlist[l - 1][0]) - PI / 2;
  double w0 = wfun(0);
  double w1 = wfun(1);
  vtxlist0.insert(vtxlist0.begin(),
                  {ptlist[0][0] + w0 * std::cos(a0), ptlist[0][1] + w0 * std::sin(a0)});
  vtxlist1.insert(vtxlist1.begin(),
                  {ptlist[0][0] - w0 * std::cos(a0), ptlist[0][1] - w0 * std::sin(a0)});
  vtxlist0.push_back({ptlist[l][0] + w1 * std::cos(a1),
                      ptlist[l][1] + w1 * std::sin(a1)});
  vtxlist1.push_back({ptlist[l][0] - w1 * std::cos(a1),
                      ptlist[l][1] - w1 * std::sin(a1)});
  return {vtxlist0, vtxlist1};
}

static Pts tranpoly(const Pt& p0, const Pt& p1, const Pts& ptlist) {
  Pts plist;
  plist.reserve(ptlist.size());
  for (const auto& v : ptlist) plist.push_back({-v[0], v[1]});
  double ang = std::atan2(p1[1] - p0[1], p1[0] - p0[0]) - PI / 2;
  double scl = distance(p0, p1);
  Pts qlist;
  qlist.reserve(plist.size());
  for (const auto& v : plist) {
    double d = distance(v, {0, 0});
    double a = std::atan2(v[1], v[0]);
    qlist.push_back({p0[0] + d * scl * std::cos(ang + a),
                     p0[1] + d * scl * std::sin(ang + a)});
  }
  return qlist;
}

static Pts flipper(const Pts& plist) {
  Pts r;
  r.reserve(plist.size());
  for (const auto& v : plist) r.push_back({-v[0], v[1]});
  return r;
}

void Man::hat01(Painter& p, const Pt& p0, const Pt& p1, const HatArg& a) {
  double seed = rnd();
  bool fli = a.fli;
  auto f = [fli](const Pts& pl) { return fli ? flipper(pl) : pl; };
  p.poly(tranpoly(p0, p1, f({{-0.3, 0.5}, {0.3, 0.8}, {0.2, 1}, {0, 1.1},
                            {-0.3, 1.15}, {-0.55, 1}, {-0.65, 0.5}})),
         PArg{.fil = "rgba(100,100,100,0.8)"});
  Pts qlist1;
  for (int i = 0; i < 10; i++) {
    qlist1.push_back({-0.3 - nse(i * 0.2, seed) * i * 0.1, 0.5 - i * 0.3});
  }
  p.poly(tranpoly(p0, p1, f(qlist1)),
         PArg{.str = "rgba(100,100,100,0.8)", .wid = 1});
}

void Man::hat02(Painter& p, const Pt& p0, const Pt& p1, const HatArg& a) {
  bool fli = a.fli;
  auto f = [fli](const Pts& pl) { return fli ? flipper(pl) : pl; };
  p.poly(tranpoly(p0, p1, f({{-0.3, 0.5}, {-1.1, 0.5}, {-1.2, 0.6}, {-1.1, 0.7},
                            {-0.3, 0.8}, {0.3, 0.8}, {1.0, 0.7}, {1.3, 0.6},
                            {1.2, 0.5}, {0.3, 0.5}})),
         PArg{.fil = "rgba(100,100,100,0.8)"});
}

void Man::stick01(Painter& p, const Pt& p0, const Pt& p1, const StickArg& a) {
  double seed = rnd();
  bool fli = a.fli;
  auto f = [fli](const Pts& pl) { return fli ? flipper(pl) : pl; };
  Pts qlist1;
  const int l = 12;
  for (int i = 0; i < l; i++) {
    qlist1.push_back({-nse(i * 0.1, seed) * 0.1 * std::sin(((double)i / l) * PI) * 5,
                      0 + i * 0.3});
  }
  p.poly(tranpoly(p0, p1, f(qlist1)),
         PArg{.str = "rgba(100,100,100,0.5)", .wid = 1});
}

void Man::man(Painter& p, double xoff, double yoff, const ManArg& a) {
  double sca = a.sca;
  // default hat: hat01; default ite: nothing
  auto hat = a.hat
      ? a.hat
      : [](Painter& p2, const Pt& a2, const Pt& b2, const HatArg& h) {
          Man::hat01(p2, a2, b2, h);
        };
  auto ite = a.ite;

  std::vector<double> ang;
  if (a.ang.empty()) {
    ang = {0, -PI / 2, normRand(0, 0), (PI / 4) * rnd(),
           ((PI * 3) / 4) * rnd(), (PI * 3) / 4, -PI / 4,
           (-PI * 3) / 4 - (PI / 4) * rnd(), -PI / 4};
  } else {
    ang = a.ang;
  }
  std::vector<double> len = a.len.empty()
      ? std::vector<double>{0, 30, 20, 30, 30, 30, 30, 30, 30}
      : a.len;
  for (auto& v : len) v *= sca;

  // joint tree: 0->{1->{2,5->{6},7->{8}},3->{4}}
  const int par[9] = {-1, 0, 1, 0, 3, 1, 5, 1, 7};

  auto gpar = [&](int ind) {
    std::vector<int> path;
    path.push_back(ind);
    while (path.back() != 0) path.push_back(par[path.back()]);
    std::reverse(path.begin(), path.end());
    return path;
  };
  auto grot = [&](int ind) {
    auto path = gpar(ind);
    double rot = 0;
    for (int k : path) rot += ang[k];
    return rot;
  };
  auto gpos = [&](int ind) {
    auto path = gpar(ind);
    Pt pos = {0, 0};
    for (int k : path) {
      double a2 = grot(k);
      pos[0] += len[k] * std::cos(a2);
      pos[1] += len[k] * std::sin(a2);
    }
    return pos;
  };

  std::vector<Pt> pts(ang.size());
  for (size_t i = 0; i < ang.size(); i++) pts[i] = gpos((int)i);
  yoff -= pts[4][1];

  auto toGlobal = [&](const Pt& v) {
    return Pt{(a.fli ? -1.0 : 1.0) * v[0] + xoff, v[1] + yoff};
  };

  auto cloth = [&](const Pts& plist, const std::function<double(double)>& fun) {
    Pts tlist = bezmh(plist, 2);
    auto [tlist1, tlist2] = expand(tlist, fun);
    // JS: poly(tlist1.concat(tlist2.reverse())...) - reverse() mutates tlist2!
    std::reverse(tlist2.begin(), tlist2.end());
    Pts poly1;
    poly1.reserve(tlist1.size() + tlist2.size());
    for (auto& v : tlist1) poly1.push_back(toGlobal(v));
    for (auto& v : tlist2) poly1.push_back(toGlobal(v));
    p.poly(poly1, PArg{.fil = "white"});
    Pts g1, g2;
    for (auto& v : tlist1) g1.push_back(toGlobal(v));
    for (auto& v : tlist2) g2.push_back(toGlobal(v));
    stroke(p, g1, SArg{.wid = 1, .col = "rgba(100,100,100,0.5)"});
    stroke(p, g2, SArg{.wid = 1, .col = "rgba(100,100,100,0.6)"});
  };

  auto fsleeve = [&](double x) {
    return sca * 8 * (std::sin(0.5 * x * PI) * std::pow(std::sin(x * PI), 0.1) +
                      (1 - x) * 0.4);
  };
  auto fbody = [&](double x) {
    return sca * 11 * (std::sin(0.5 * x * PI) * std::pow(std::sin(x * PI), 0.1) +
                       (1 - x) * 0.5);
  };
  auto fhead = [&](double x) {
    return sca * 7 * std::pow(0.25 - std::pow(x - 0.5, 2), 0.3);
  };

  if (ite) ite(p, toGlobal(pts[8]), toGlobal(pts[6]), a.fli);

  cloth({pts[1], pts[7], pts[8]}, fsleeve);
  cloth({pts[1], pts[0], pts[3], pts[4]}, fbody);
  cloth({pts[1], pts[5], pts[6]}, fsleeve);
  cloth({pts[1], pts[2]}, fhead);

  Pts hlist = bezmh({pts[1], pts[2]}, 2);
  auto [hlist1, hlist2] = expand(hlist, fhead);
  hlist1.erase(hlist1.begin(), hlist1.begin() + (size_t)std::floor(hlist1.size() * 0.1));
  hlist2.erase(hlist2.begin(), hlist2.begin() + (size_t)std::floor(hlist2.size() * 0.95));
  Pts hpoly;
  hpoly.reserve(hlist1.size() + hlist2.size());
  for (auto& v : hlist1) hpoly.push_back(toGlobal(v));
  for (auto it = hlist2.rbegin(); it != hlist2.rend(); ++it) hpoly.push_back(toGlobal(*it));
  p.poly(hpoly, PArg{.fil = "rgba(100,100,100,0.6)"});

  hat(p, toGlobal(pts[1]), toGlobal(pts[2]), {a.fli});
}

} // namespace ss
