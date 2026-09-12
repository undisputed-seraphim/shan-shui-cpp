#include "draw/painter.h"
#include <format>

namespace ss {

void Painter::text(const TArg& t) {
	TextOp op;
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

namespace {

struct SvgWriter {
	std::string& s;

	void operator()(const PolyOp& op) const {
		s += "<polyline points='";
		for (size_t i = 0; i + 1 < op.pts.size(); i += 2) {
			s += ' ';
			appendFixed(s, op.pts[i] + op.xof, 1);
			s += ',';
			appendFixed(s, op.pts[i + 1] + op.yof, 1);
		}
		s += "' style='fill:";
		s += op.fil;
		s += ";stroke:";
		s += op.str;
		s += ";stroke-width:";
		appendNum(s, op.wid);
		s += "'/>";
	}

	void operator()(const TextOp& op) const {
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
};

} // namespace

std::string Painter::toSvg() const {
	std::string s;
	size_t est = 0;
	for (const auto& op : ops_) {
		if (auto* p = std::get_if<PolyOp>(&op))
			est += p->pts.size() * 9 + 64;
		else
			est += 256;
	}
	s.reserve(est);
	for (const auto& op : ops_)
		std::visit(SvgWriter{s}, op);
	return s;
}

} // namespace ss
