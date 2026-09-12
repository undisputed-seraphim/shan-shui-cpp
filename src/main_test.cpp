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
	// --- PRNG stream for seed "123" (from ref_prng.txt) ---
	{
		Rng::inst().seed("123");
		const char* expect[] = {
			"0.563566544187486",
			"0.321631826709592",
			"0.265661911199784",
			"0.977427078879056",
			"0.717024806592673",
			"0.017856904721004",
			"0.305112297321373",
			"0.429255656289695",
			"0.576170941272075",
			"0.628078658324792",
			"0.461861817647186",
			"0.649525784552932"};
		bool ok = true;
		std::string got;
		for (int i = 0; i < 12; i++) {
			std::string v = toFixed(rnd(), 15);
			if (v != expect[i]) {
				ok = false;
				got += v + " ";
			}
		}
		check(ok, "PRNG stream matches JS for seed '123'" + (ok ? "" : " (got " + got + ")"));
	}

	// --- noise head for seed "123" (from ref_noise.txt) ---
	{
		Rng::inst().seed("123");
		Noise::inst().noise(1.0, 2.0, 3.0); // build table lazily
		const char* expect[] = {
			"0.528343635",
			"0.379729339",
			"0.352870176",
			"0.455091527",
			"0.423446516",
			"0.424147296",
			"0.673503669",
			"0.591876120"};
		bool ok = true;
		std::string got;
		for (int i = 0; i < 8; i++) {
			std::string v = toFixed(Noise::inst().noise(i * 0.5), 9);
			if (v != expect[i]) {
				ok = false;
				got += v + " ";
			}
		}
		check(ok, "noise matches JS for seed '123'" + (ok ? "" : " (got " + got + ")"));
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
		check(sc.xmin == -512, "xmin == -512 (JS reference)");
		check(sc.xmax == 3584, "xmax == 3584 (JS reference)");
		check(sc.chunks.size() == 55, "chunk count == 55 (JS reference)");
		std::cout << "JS reference view svg bytes: 12118863\n";
	}

	std::cout << (failures ? "SOME CHECKS FAILED\n" : "ALL CHECKS PASSED\n");
	return failures ? 1 : 0;
}
