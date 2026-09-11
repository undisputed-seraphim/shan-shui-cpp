#pragma once
#include "core/poly.h"
#include "core/rng.h"
#include <functional>
#include <string>
#include <cmath>

namespace ss {

constexpr double PI = 3.14159265358979323846;
constexpr double E = 2.71828182845904523536;

double distance(const Pt& p0, const Pt& p1);
double mapval(double value, double istart, double istop, double ostart, double ostop);
void loopNoise(std::vector<double>& nslist);

template <typename T> T randChoice(const std::vector<T>& arr) {
  return arr[(size_t)std::floor((double)arr.size() * rnd())];
}

double normRand(double m, double M);
double wtrand(const std::function<double(double)>& func);
double randGaussian();
Pts bezmh(const Pts& P, double w = 1);

// number formatting helpers
std::string toFixed(double v, int digits);       // JS toFixed-ish (%.*f)
std::string fmtNum(double v);                    // JS Number->string (shortest)
std::string rgba(double r, double g, double b, double a); // "rgba(r,g,b,a)" a: 3 decimals

} // namespace ss
