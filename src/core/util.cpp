#include "core/util.h"
#include "core/poly.h"
#include <charconv>
#include <format>

namespace ss {

double distance(const Pt& p0, const Pt& p1) { return std::hypot(p0[0] - p1[0], p0[1] - p1[1]); }

double remap(double value, double istart, double istop, double ostart, double ostop) {
	return ostart + (ostop - ostart) * ((value - istart) * 1.0) / (istop - istart);
}

void seamless_noise(std::vector<double>& nslist) {
	double dif = nslist[nslist.size() - 1] - nslist[0];
	double bds[2] = {100, -100};
	for (size_t i = 0; i < nslist.size(); i++) {
		nslist[i] += (dif * (double)(nslist.size() - 1 - i)) / (double)(nslist.size() - 1);
		if (nslist[i] < bds[0])
			bds[0] = nslist[i];
		if (nslist[i] > bds[1])
			bds[1] = nslist[i];
	}
	std::ranges::transform(nslist, nslist.begin(), [&](double v) { return remap(v, bds[0], bds[1], 0, 1); });
}

double normRand(double m, double M) { return remap(rnd(), 0, 1, m, M); }

double randGaussian() {
	return rejection_sample([](double x) { return std::pow(e, -24 * std::pow(x - 0.5, 2)); }) * 2 - 1;
}

Pts bezier_mid_hull(const Pts& P_, double w) {
	Pts P = P_;
	if (P.size() == 2) {
		P.insert(P.begin() + 1, PolyTools::centroid({P[0], P[1]}));
	}
	Pts plist;
	plist.reserve((P.size() > 2 ? P.size() - 2 : 0) * 21);
	for (size_t j = 0; j + 2 < P.size(); j++) {
		Pt p0, p1, p2;
		if (j == 0)
			p0 = P[j];
		else
			p0 = PolyTools::centroid({P[j], P[j + 1]});
		p1 = P[j + 1];
		if (j == P.size() - 3)
			p2 = P[j + 2];
		else
			p2 = PolyTools::centroid({P[j + 1], P[j + 2]});
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

std::string Color::rgba() const { return std::format("rgba({},{},{},{:.3f})", r, g, b, a); }

void appendFixed(std::string& out, double v, int digits) {
	if (!std::isfinite(v)) {
		out += "-1000";
		return;
	}
	double scale = 1;
	for (int i = 0; i < digits; i++)
		scale *= 10;
	double r = std::round(v * scale) / scale;
	if (r == 0)
		r = 0; // normalize -0.0
	char buf[64];
	auto res = std::to_chars(buf, buf + sizeof buf, r, std::chars_format::fixed);
	out.append(buf, res.ptr);
}

std::string fmtFixed(double v, int digits) {
	std::string s;
	appendFixed(s, v, digits);
	return s;
}

void appendNum(std::string& out, double v) {
	if (!std::isfinite(v)) {
		out += "-1000";
		return;
	}
	char buf[64];
	auto res = std::to_chars(buf, buf + sizeof buf, v, std::chars_format::general);
	out.append(buf, res.ptr);
}

std::string fmtNum(double v) {
	std::string s;
	appendNum(s, v);
	return s;
}

} // namespace ss
