#pragma once
#include "core/poly.h"
#include "core/util.h"
#include <string>
#include <variant>
#include <vector>

namespace ss {

// Poly drawing arguments (mirrors JS poly()/stroke() args with defaults).
struct PArg {
	double xof = 0;
	double yof = 0;
	std::string fil = "rgba(0,0,0,0)";
	std::string str;
	double wid = 0;

	std::string effStr() const { return str.empty() ? fil : str; }
};

// Text drawing arguments (for Arch.roof plaques).
struct TArg {
	double x = 0, y = 0;
	double rot = 0; // degrees
	double fontSize = 12;
	std::string content;
	std::string fill = "rgba(100,100,100,0.9)";
	std::string fontFamily = "Verdana";
};

// A drawn polygon: coordinates flattened as x0,y0,x1,y1,...
struct PolyOp {
	std::vector<double> pts;
	double xof = 0, yof = 0;
	std::string fil, str;
	double wid = 0;
};

struct TextOp {
	double x = 0, y = 0;
	double fontSize = 0, rot = 0;
	std::string content, fontFamily, fill;
};

using Op = std::variant<PolyOp, TextOp>;

// Accumulator of drawing operations. Generators paint into this; the ops can
// be serialized to SVG today or consumed by a Canvas2D/WebGL renderer later.
class Painter {
public:
	// Copies points into op storage; accepts any point container
	// (std::vector or arena-backed std::pmr::vector).
	template <typename PtsT>
	void poly(const PtsT& pts, const PArg& a = {}) {
		PolyOp op;
		op.pts.reserve(pts.size() * 2);
		for (const auto& p : pts) {
			op.pts.push_back(p[0]);
			op.pts.push_back(p[1]);
		}
		op.xof = a.xof;
		op.yof = a.yof;
		op.fil = a.fil;
		op.str = a.str.empty() ? a.fil : a.str;
		op.wid = a.wid;
		ops_.push_back(std::move(op));
	}

	void text(const TArg& t);
	void absorb(Painter&& o); // move o's ops to the end (JS canv += txcanv)
	void clear() { ops_.clear(); }
	const std::vector<Op>& ops() const { return ops_; }
	size_t size() const { return ops_.size(); }
	std::string toSvg() const;

private:
	std::vector<Op> ops_;
};

} // namespace ss
