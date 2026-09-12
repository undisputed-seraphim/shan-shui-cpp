#pragma once
#include "draw/painter.h"

namespace ss {

namespace Mount {

struct MountainArg {
	double hei = -1; // default: 100 + rand*400
	double wid = -1; // default: 400 + rand*200
	int tex = 200;
	bool veg = true;
	int ret = 0;
};

void mountain(Painter& p, double xoff, double yoff, double seed, const MountainArg& a = {});

struct FlatMountArg {
	double hei = -1;
	double wid = -1;
	int tex = 80;
	double cho = 0.5;
	int ret = 0;
};

void flatMount(Painter& p, double xoff, double yoff, double seed, const FlatMountArg& a = {});

struct DistMountArg {
	double hei = 300;
	double len = 2000;
	int seg = 5;
};

void distMount(Painter& p, double xoff, double yoff, double seed, const DistMountArg& a = {});

void rock(
	Painter& p,
	double xoff,
	double yoff,
	double seed,
	double hei = 80,
	double wid = 100,
	int tex = 40,
	int ret = 0,
	double sha = 10);

} // namespace Mount

} // namespace ss
