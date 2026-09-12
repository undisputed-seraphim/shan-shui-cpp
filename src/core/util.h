#pragma once
#include "core/poly.h"
#include "core/rng.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
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

// Non-owning callback reference (replaces std::function for generator args).
// Binds lvalues only, so callables must outlive the reference.
template <typename Signature>
class FunctionRef;

template <typename R, typename... Args>
class FunctionRef<R(Args...)> {
public:
	FunctionRef() = default;

	template <typename F>
	FunctionRef(F& f)
		: obj_(std::addressof(f))
		, invoke_(invoker<F>) {}

	template <typename F>
	FunctionRef& operator=(F& f) {
		obj_ = std::addressof(f);
		invoke_ = invoker<F>;
		return *this;
	}

	explicit operator bool() const { return invoke_ != nullptr; }
	R operator()(Args... args) const { return invoke_(obj_, std::forward<Args>(args)...); }

private:
	template <typename F>
	static R invoker(const void* obj, Args... args) {
		return std::invoke(*static_cast<F*>(const_cast<void*>(obj)), std::forward<Args>(args)...);
	}

	const void* obj_ = nullptr;
	R (*invoke_)(const void*, Args...) = nullptr;
};

template <typename T>
T randChoice(const std::vector<T>& arr) {
	return arr[(size_t)std::floor((double)arr.size() * rnd())];
}

double normRand(double m, double M);
template <typename Func>
double rejection_sample(const Func& func) {
	for (;;) {
		double x = rnd();
		double y = rnd();
		if (y < func(x))
			return x;
	}
}
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
