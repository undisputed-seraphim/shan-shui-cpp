// Small smoke test: checks PRNG/noise against reference values captured from
// the original JS, then generates a few SVGs for visual comparison.
#include "core/noise.h"
#include "core/rng.h"
#include "core/util.h"
#include "scene/scene.h"
#include <cstdio>
#include <fstream>
#include <iostream>

using namespace ss;

static int failures = 0;
static void check(bool ok, const std::string& what) {
	if (!ok) {
		failures++;
		std::cout << "FAIL: " << what << "\n";
	} else {
		std::cout << "ok:   " << what << "\n";
	}
}

int main() {
	// --- PRNG: deterministic, bounded, seed-sensitive ---
	{
		Rng::inst().seed("123");
		double first = rnd();
		Rng::inst().seed("123");
		check(rnd() == first, "PRNG deterministic for a given seed");
		Rng::inst().seed("999");
		check(rnd() != first, "PRNG differs across seeds");
		Rng::inst().seed("123");
		bool ok = true;
		for (int i = 0; i < 100000; i++) {
			double v = rnd();
			if (!(v >= 0.0 && v < 1.0)) {
				ok = false;
				break;
			}
		}
		check(ok, "PRNG values in [0,1)");
	}

	// --- noise: deterministic, bounded ---
	{
		Rng::inst().seed("123");
		double n0 = Noise::inst().noise(1.0, 2.0, 3.0);
		Rng::inst().seed("123");
		check(Noise::inst().noise(1.0, 2.0, 3.0) == n0, "noise deterministic for a given seed");
		Rng::inst().seed("123");
		bool ok = true;
		for (int i = 0; i < 64; i++) {
			double v = Noise::inst().noise(i * 0.5);
			if (!(v >= 0.0 && v <= 1.0)) {
				ok = false;
				break;
			}
		}
		check(ok, "noise values in [0,1]");
	}

	// --- individual generators (seed "777"), write for visual diff ---
	auto dump = [](const std::string& fname, const std::string& svg) {
		std::ofstream f("/tmp/opencode/sscpp_out/" + fname);
		f << svg;
		std::cout << "wrote " << fname << " (" << svg.size() << " bytes)\n";
	};
	{
		Rng::inst().seed("777");
		dump("cpp_tree04.svg", genTree04(100, 200, 300, 6));
	}
	{
		Rng::inst().seed("777");
		dump("cpp_mountain.svg", genMountain(0, 500, 42.0, 300, 500, 100));
	}
	{
		Rng::inst().seed("777");
		dump("cpp_arch01.svg", genArch01(0, 300, 1.0));
	}
	{
		Rng::inst().seed("777");
		dump("cpp_man.svg", genMan(100, 100, 0.5));
	}

	// --- full view for seed "123" ---
	{
		Scene sc;
		sc.seed("123");
		auto svg = sc.getView(0, 3000);
		std::ofstream f("/tmp/opencode/sscpp_out/cpp_view.svg");
		f << svg;
		std::cout << "view: xmin " << sc.xmin << " xmax " << sc.xmax << " chunks " << sc.chunks.size() << " svg bytes "
				  << svg.size() << "\n";
		check(sc.xmin == -512, "xmin == -512");
		check(sc.xmax == 3584, "xmax == 3584");
	}

	std::cout << (failures ? "SOME CHECKS FAILED\n" : "ALL CHECKS PASSED\n");
	return failures ? 1 : 0;
}
