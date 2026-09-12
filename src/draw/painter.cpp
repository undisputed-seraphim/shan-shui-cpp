#include "draw/painter.h"
#include <cstdio>

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
		est += op.pts.size() * 16 + 64;
	s.reserve(est);
	for (const auto& op : ops_) {
		if (op.kind == 0) {
			s += "<polyline points='";
			for (const auto& p : op.pts) {
				s += " ";
				s += toFixed(p[0] + op.xof, 1);
				s += ",";
				s += toFixed(p[1] + op.yof, 1);
			}
			s += "' style='fill:";
			s += op.fil;
			s += ";stroke:";
			s += op.str;
			s += ";stroke-width:";
			s += fmtNum(op.wid);
			s += "'/>";
		} else {
			char buf[512];
			std::snprintf(
				buf,
				sizeof buf,
				"<text font-size='%s' font-family='%s' style='fill:%s' "
				"text-anchor='middle' transform='translate(%s,%s) "
				"rotate(%s)'>%s</text>",
				fmtNum(op.fontSize).c_str(),
				op.fontFamily.c_str(),
				op.fill.c_str(),
				fmtNum(op.x).c_str(),
				fmtNum(op.y).c_str(),
				fmtNum(op.rot).c_str(),
				op.content.c_str());
			s += buf;
		}
	}
	return s;
}

} // namespace ss
