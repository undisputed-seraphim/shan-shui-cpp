#pragma once
#include "core/poly.h"
#include "draw/painter.h"
#include <functional>

namespace ss {

struct SArg {
	double xof = 0, yof = 0;
	double wid = 2;
	std::string col = "rgba(200,200,200,0.9)";
	double noi = 0.5;
	double out = 1;
	std::function<double(double)> fun = [](double x) { return std::sin(x * PI); };
};

struct BArg {
	double len = 20, wid = 5, ang = 0;
	std::string col = "rgba(200,200,200,0.9)";
	double noi = 0.5;
	int ret = 0;
	std::function<double(double)> fun = [](double x) {
		return x <= 1 ? std::pow(std::sin(x * PI), 0.5) : -std::pow(std::sin((x + 1) * PI), 0.5);
	};
};

void stroke(Painter& p, const Pts& ptlist, const SArg& a = {});
void blob(Painter& p, double x, double y, const BArg& a = {});
Pts blobPts(double x, double y, const BArg& a = {});
Pts div(const Pts& plist, double reso);

struct TArg2 { // texture args
	double xof = 0, yof = 0;
	int tex = 400;
	double wid = 1.5;
	double len = 0.2;
	double sha = 0;
	int ret = 0;
	std::function<double(double)> noi = [](double x) { return 30.0 / x; };
	std::function<std::string(double)> col = [](double) { return "rgba(100,100,100," + toFixed(rnd() * 0.3, 3) + ")"; };
	std::function<double()> dis = []() {
		if (rnd() > 0.5)
			return (1.0 / 3) * rnd();
		return (1.0 * 2) / 3 + (1.0 / 3) * rnd();
	};
};

void texture(Painter& p, const std::vector<Pts>& ptlist, const TArg2& a = {});

} // namespace ss
