#include "core/util.h"
#include "core/poly.h"
#include <cstdio>
#include <charconv>

namespace ss {

double distance(const Pt& p0, const Pt& p1) {
  return std::hypot(p0[0] - p1[0], p0[1] - p1[1]);
}

double mapval(double value, double istart, double istop, double ostart, double ostop) {
  return ostart +
         (ostop - ostart) * ((value - istart) * 1.0) / (istop - istart);
}

void loopNoise(std::vector<double>& nslist) {
  double dif = nslist[nslist.size() - 1] - nslist[0];
  double bds[2] = {100, -100};
  for (size_t i = 0; i < nslist.size(); i++) {
    nslist[i] += (dif * (double)(nslist.size() - 1 - i)) / (double)(nslist.size() - 1);
    if (nslist[i] < bds[0]) bds[0] = nslist[i];
    if (nslist[i] > bds[1]) bds[1] = nslist[i];
  }
  for (size_t i = 0; i < nslist.size(); i++) {
    nslist[i] = mapval(nslist[i], bds[0], bds[1], 0, 1);
  }
}

double normRand(double m, double M) { return mapval(rnd(), 0, 1, m, M); }

double wtrand(const std::function<double(double)>& func) {
  double x = rnd();
  double y = rnd();
  if (y < func(x)) return x;
  return wtrand(func);
}

double randGaussian() {
  return wtrand([](double x) { return std::pow(E, -24 * std::pow(x - 0.5, 2)); }) * 2 - 1;
}

Pts bezmh(const Pts& P_, double w) {
  Pts P = P_;
  if (P.size() == 2) {
    P.insert(P.begin() + 1, PolyTools::midPt({P[0], P[1]}));
  }
  Pts plist;
  for (size_t j = 0; j + 2 < P.size(); j++) {
    Pt p0, p1, p2;
    if (j == 0) p0 = P[j];
    else p0 = PolyTools::midPt({P[j], P[j + 1]});
    p1 = P[j + 1];
    if (j == P.size() - 3) p2 = P[j + 2];
    else p2 = PolyTools::midPt({P[j + 1], P[j + 2]});
    const int pl = 20;
    int count = pl + (j == P.size() - 3);
    for (int i = 0; i < count; i++) {
      double t = (double)i / pl;
      double u = std::pow(1 - t, 2) + 2 * t * (1 - t) * w + t * t;
      plist.push_back({
          (std::pow(1 - t, 2) * p0[0] + 2 * t * (1 - t) * p1[0] * w + t * t * p2[0]) / u,
          (std::pow(1 - t, 2) * p0[1] + 2 * t * (1 - t) * p1[1] * w + t * t * p2[1]) / u,
      });
    }
  }
  return plist;
}

std::string toFixed(double v, int digits) {
  if (!std::isfinite(v)) return "-1000";
  // round half away from zero like JS Number.prototype.toFixed
  double scale = 1;
  for (int i = 0; i < digits; i++) scale *= 10;
  double r = v >= 0 ? std::floor(v * scale + 0.5) : std::ceil(v * scale - 0.5);
  double out = r / scale;
  char buf[64];
  std::snprintf(buf, sizeof buf, "%.*f", digits, out);
  return buf;
}

std::string fmtNum(double v) {
  if (!std::isfinite(v)) return "-1000";
  char buf[64];
  auto res = std::to_chars(buf, buf + sizeof buf, v, std::chars_format::general);
  return std::string(buf, res.ptr);
}

std::string rgba(double r, double g, double b, double a) {
  return "rgba(" + std::to_string((int)r) + "," + std::to_string((int)g) + "," +
         std::to_string((int)b) + "," + toFixed(a, 3) + ")";
}

} // namespace ss
