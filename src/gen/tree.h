#pragma once
#include "draw/painter.h"

namespace ss {

namespace Tree {
void tree01(Painter& p, double x, double y, double hei = 50, double wid = 3, const Color& col = {}, double noi = 0.5);
void tree02(
	Painter& p,
	double x,
	double y,
	double hei = 16,
	double wid = 8,
	int clu = 5,
	const Color& col = {},
	double noi = 0.5);
void tree03(
	Painter& p,
	double x,
	double y,
	double hei = 50,
	double wid = 5,
	const std::function<double(double)>& ben = {},
	const Color& col = {},
	double noi = 0.5);
void tree04(Painter& p, double x, double y, double hei = 300, double wid = 6, const Color& col = {}, double noi = 0.5);
void tree05(Painter& p, double x, double y, double hei = 300, double wid = 5, const Color& col = {}, double noi = 0.5);
void tree06(Painter& p, double x, double y, double hei = 100, double wid = 6, const Color& col = {}, double noi = 0.5);
void tree07(
	Painter& p,
	double x,
	double y,
	double hei = 60,
	double wid = 4,
	const std::function<double(double)>& ben = {},
	const Color& col = {100, 100, 100, 1.0},
	double noi = 0.5);
void tree08(Painter& p, double x, double y, double hei = 80, double wid = 1, const Color& col = {}, double noi = 0.5);
} // namespace Tree

} // namespace ss
