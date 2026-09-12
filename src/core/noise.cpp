#include "core/noise.h"
#include "core/rng.h"
#include <cmath>
#include <cstdint>

namespace ss {

Noise& Noise::inst() {
	static Noise n;
	return n;
}

void Noise::ensure() {
	if (!perlin.empty())
		return;
	perlin.assign(SIZE + 1, 0.0);
	for (int i = 0; i < SIZE + 1; i++)
		perlin[i] = rnd();
}

static double scaledCosine(double i) { return 0.5 * (1.0 - std::cos(i * M_PI)); }

double Noise::noise(double x, double y, double z) {
	ensure();
	if (x < 0)
		x = -x;
	if (y < 0)
		y = -y;
	if (z < 0)
		z = -z;

	double xi = std::floor(x), yi = std::floor(y), zi = std::floor(z);
	double xf = x - xi, yf = y - yi, zf = z - zi;
	double rxf, ryf;
	double r = 0;
	double ampl = 0.5;
	double n1, n2, n3;

	for (int o = 0; o < perlinOctaves; o++) {
		// JS: of = xi + (yi << 4) + (zi << 8) with int32 semantics
		int32_t of = (int32_t)(xi + (double)((int32_t)yi << YWRAPB) + (double)((int32_t)zi << ZWRAPB));
		rxf = scaledCosine(xf);
		ryf = scaledCosine(yf);

		n1 = perlin[(uint32_t)of & SIZE];
		n1 += rxf * (perlin[((uint32_t)of + 1) & SIZE] - n1);
		n2 = perlin[((uint32_t)of + YWRAP) & SIZE];
		n2 += rxf * (perlin[((uint32_t)of + YWRAP + 1) & SIZE] - n2);
		n1 += ryf * (n2 - n1);

		of += ZWRAP;
		n2 = perlin[(uint32_t)of & SIZE];
		n2 += rxf * (perlin[((uint32_t)of + 1) & SIZE] - n2);
		n3 = perlin[((uint32_t)of + YWRAP) & SIZE];
		n3 += rxf * (perlin[((uint32_t)of + YWRAP + 1) & SIZE] - n3);
		n2 += ryf * (n3 - n2);

		n1 += scaledCosine(zf) * (n2 - n1);

		r += n1 * ampl;
		ampl *= perlinAmpFalloff;

		xi = (double)((int32_t)xi << 1);
		xf *= 2;
		yi = (double)((int32_t)yi << 1);
		yf *= 2;
		zi = (double)((int32_t)zi << 1);
		zf *= 2;

		if (xf >= 1.0) {
			xi++;
			xf--;
		}
		if (yf >= 1.0) {
			yi++;
			yf--;
		}
		if (zf >= 1.0) {
			zi++;
			zf--;
		}
	}
	return r;
}

void Noise::noiseDetail(double lod, double falloff) {
	if (lod > 0)
		perlinOctaves = (int)lod;
	if (falloff > 0)
		perlinAmpFalloff = falloff;
}

void Noise::noiseSeed(double seed) {
	// LCG as in the original: z = (a*z + c) % m with z = seed >>> 0
	const double m = 4294967296.0, a = 1664525.0, c = 1013904223.0;
	double z = std::fmod(std::fabs(seed), m);
	perlin.assign(SIZE + 1, 0.0);
	for (int i = 0; i < SIZE + 1; i++) {
		z = std::fmod(a * z + c, m);
		perlin[i] = z / m;
	}
}

} // namespace ss
