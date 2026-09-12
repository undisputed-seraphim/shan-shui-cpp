#pragma once
#include "draw/painter.h"

namespace ss {

namespace Arch {
void arch01(
	Painter& p,
	double xoff,
	double yoff,
	double seed,
	double hei = 70,
	double wid = 180,
	double rot = 0.7,
	double per = 5);
void arch02(
	Painter& p,
	double xoff,
	double yoff,
	double seed,
	double hei = 10,
	double wid = 50,
	double rot = 0.3,
	double per = 5,
	int sto = 3,
	int sty = 1,
	bool rai = false);
void arch03(
	Painter& p,
	double xoff,
	double yoff,
	double seed,
	double hei = 10,
	double wid = 50,
	double rot = 0.7,
	double per = 5,
	int sto = 7);
void arch04(
	Painter& p,
	double xoff,
	double yoff,
	double seed,
	double hei = 15,
	double wid = 30,
	double rot = 0.7,
	double per = 5,
	int sto = 2);
void boat01(Painter& p, double xoff, double yoff, double seed, double len = 120, double sca = 1, bool fli = false);
void transmissionTower01(Painter& p, double xoff, double yoff, double seed, double hei = 100, double wid = 20);
} // namespace Arch

} // namespace ss
