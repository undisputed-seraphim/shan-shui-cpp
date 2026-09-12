#include "draw/painter.h"
#include <format>

namespace ss {

void Painter::poly(const Pts& pts, const PArg& a) {
	Op op;
	op.kind = 0;
	op.pts = pts;
	op.xof = a.xof;
	op.yof = a.yof;
	op.fil = a.fil;
	op.str = a.str.empty() ? a.fil : a.str;
	op.wid = a.wid;
	ops_.push_back(std::move(op));
}

void Painter::text(const TArg& t) {
	Op op;
	op.kind = 1;
	op.x = t.x;
	op.y = t.y;
	op.rot = t.rot;
	op.fontSize = t.fontSize;
	op.content = t.content;
	op.fill = t.fill;
	op.fontFamily = t.fontFamily;
	ops_.push_back(std::move(op));
}

void Painter::absorb(Painter&& o) {
	ops_.insert(ops_.end(), std::make_move_iterator(o.ops_.begin()), std::make_move_iterator(o.ops_.end()));
	o.ops_.clear();
}

std::string Painter::toSvg() const {
	std::string s;
	size_t est = 0;
	for (const auto& op : ops_)
		est += op.pts.size() * 18 + 64;
	s.reserve(est);
	for (const auto& op : ops_) {
		if (op.kind == 0) {
			s += "<polyline points='";
			for (const auto& p : op.pts) {
				s += ' ';
				appendFixed(s, p[0] + op.xof, 1);
				s += ',';
				appendFixed(s, p[1] + op.yof, 1);
			}
			s += "' style='fill:";
			s += op.fil;
			s += ";stroke:";
			s += op.str;
			s += ";stroke-width:";
			appendNum(s, op.wid);
			s += "'/>";
		} else {
			std::format_to(
				std::back_inserter(s),
				"<text font-size='{}' font-family='{}' style='fill:{}' "
				"text-anchor='middle' transform='translate({},{}) "
				"rotate({})'>{}</text>",
				op.fontSize,
				op.fontFamily,
				op.fill,
				op.x,
				op.y,
				op.rot,
				op.content);
		}
	}
	return s;
}

} // namespace ss
