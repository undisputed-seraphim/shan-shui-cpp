#pragma once
#include <vector>

namespace ss {

// Perlin noise (p5.js-style). The value table derives from the global PRNG
// and is rebuilt whenever Rng::seed() reseeds the stream.
class Noise {
public:
	static Noise& inst();

	double noise(double x, double y = 0, double z = 0);
	void noiseDetail(double lod, double falloff);
	void rebuild();

private:
	Noise() = default;

	static constexpr int YWRAPB = 4;
	static constexpr int YWRAP = 1 << YWRAPB;
	static constexpr int ZWRAPB = 8;
	static constexpr int ZWRAP = 1 << ZWRAPB;
	static constexpr int SIZE = 4095;

	int perlinOctaves = 4;
	double perlinAmpFalloff = 0.5;
	std::vector<double> perlin;
};

inline double nse(double x, double y = 0, double z = 0) { return Noise::inst().noise(x, y, z); }

} // namespace ss
