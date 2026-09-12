#include "core/rng.h"
#include "core/noise.h"

namespace ss {

static inline uint64_t rotl(uint64_t x, int k) { return (x << k) | (x >> (64 - k)); }

// FNV-1a over the seed string, then splitmix64 expansion for the 4 states.
static uint64_t hashSeed(const std::string& x) {
	uint64_t h = 0xcbf29ce484222325ULL;
	for (unsigned char c : x) {
		h ^= c;
		h *= 0x100000001b3ULL;
	}
	return h;
}

static uint64_t splitmix64(uint64_t& state) {
	uint64_t z = (state += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

Rng& Rng::inst() {
	static Rng r;
	return r;
}

void Rng::seed(const std::string& x) {
	uint64_t sm = hashSeed(x);
	for (auto& v : s_)
		v = splitmix64(sm);
	Noise::inst().rebuild(); // the perlin table derives from the stream
}

double Rng::next() {
	const uint64_t result = rotl(s_[1] * 5, 7) * 9;
	const uint64_t t = s_[1] << 17;
	s_[2] ^= s_[0];
	s_[3] ^= s_[1];
	s_[1] ^= s_[2];
	s_[0] ^= s_[3];
	s_[2] ^= t;
	s_[3] = rotl(s_[3], 45);
	return (result >> 11) * 0x1.0p-53; // [0, 1)
}

} // namespace ss
