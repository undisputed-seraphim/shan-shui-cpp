#pragma once
#include "draw/painter.h"

namespace ss {

struct HatArg {
	bool fli = false;
};

struct StickArg {
	bool fli = false;
};

struct ManArg {
	double sca = 0.5;
	std::function<void(Painter&, const Pt&, const Pt&, bool)> ite;
	bool fli = true;
	std::vector<double> ang; // optional override
	std::vector<double> len;
	std::function<void(Painter&, const Pt&, const Pt&, const HatArg&)> hat;
};

namespace Man {
void hat01(Painter& p, const Pt& p0, const Pt& p1, const HatArg& a = {});
void hat02(Painter& p, const Pt& p0, const Pt& p1, const HatArg& a = {});
void stick01(Painter& p, const Pt& p0, const Pt& p1, const StickArg& a = {});
void man(Painter& p, double xoff, double yoff, const ManArg& a = {});
} // namespace Man

} // namespace ss
