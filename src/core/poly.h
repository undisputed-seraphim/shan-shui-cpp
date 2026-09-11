#pragma once
#include <vector>
#include <array>
#include <string>

namespace ss {

using Pt = std::array<double, 2>;
using Pts = std::vector<Pt>;

struct TriArgs {
  double area = 100;
  bool convex = false;
  bool optimize = true;
};

struct PolyTools {
  static Pt midPt(const Pts& plist);
  static Pt midPt(std::initializer_list<Pt> pts);
  static std::vector<Pts> triangulate(const Pts& plist, const TriArgs& args);
};

} // namespace ss
