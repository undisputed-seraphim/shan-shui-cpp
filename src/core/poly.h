#pragma once
#include <array>
#include <string>
#include <vector>

namespace ss {

using Pt = std::array<double, 2>;
using Pts = std::vector<Pt>;

struct TriArgs {
	double area = 100;
	bool convex = false;
	bool optimize = true;
};

struct PolyTools {
	static Pt centroid(const Pts& plist);
	static Pt centroid(std::initializer_list<Pt> pts);
	static std::vector<Pts> triangulate(const Pts& plist, const TriArgs& args);
};

} // namespace ss
