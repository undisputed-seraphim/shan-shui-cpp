#pragma once
#include "core/poly.h"
#include "core/rng.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <numbers>
#include <string>

namespace ss {

inline constexpr double pi = std::numbers::pi_v<double>;
inline constexpr double e = std::numbers::e_v<double>;

// Drawing color. Replaces the JS-style "rgba(r,g,b,a)" string plumbing;
// formatting happens only at paint time.
struct Color {
	int r = 100, g = 100, b = 100;
	double a = 0.5;
	std::string rgba() const;
};

double distance(const Pt& p0, const Pt& p1);
double remap(double value, double istart, double istop, double ostart, double ostop);
void seamless_noise(std::vector<double>& nslist);

template <typename T>
T randChoice(const std::vector<T>& arr) {
	return arr[(size_t)std::floor((double)arr.size() * rnd())];
}

double normRand(double m, double M);
double rejection_sample(const std::function<double(double)>& func);
double randGaussian();
Pts bezier_mid_hull(const Pts& P, double w = 1);

// number formatting helpers
std::string fmtFixed(double v, int digits);				  // fixed `digits` decimals, normal C++ rounding
void appendFixed(std::string& out, double v, int digits); // fmtFixed into out (no temporaries)
std::string fmtNum(double v);							  // shortest round-trip
void appendNum(std::string& out, double v);				  // fmtNum into out (no temporaries)

// returns `pts` translated by (xof, yof)
inline Pts offset(const Pts& pts, double xof, double yof) {
	Pts out;
	out.reserve(pts.size());
	std::ranges::transform(
		pts, std::back_inserter(out), [xof, yof](const Pt& v) { return Pt{v[0] + xof, v[1] + yof}; });
	return out;
}

} // namespace ss
