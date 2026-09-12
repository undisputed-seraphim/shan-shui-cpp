#pragma once
#include <cstdint>
#include <string>

namespace ss {

// xoshiro256** seeded from the seed string via FNV-1a + splitmix64.
// Integer-only: bit-identical results on every platform.
class Rng {
public:
	static Rng& inst();

	void seed(const std::string& x);
	double next(); // uniform in [0, 1)

private:
	Rng() = default;

	uint64_t s_[4] = {
		0x9e3779b97f4a7c15ULL, 0x243f6a8885a308d3ULL, 0x13198a2e03707344ULL, 0xa4093822299f31d0ULL};
};

inline double rnd() { return Rng::inst().next(); }

} // namespace ss
