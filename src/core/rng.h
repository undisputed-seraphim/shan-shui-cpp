#pragma once
#include <string>
#include <cmath>

namespace ss {

// Port of the original Blum-Blum-Shub style PRNG (index.html <script id="PRNG">).
// Must reproduce the JS double-precision behavior exactly: all state is double,
// s*s is computed as double, and modulus uses fmod.
class Rng {
public:
  static Rng& inst();

  void seed(const std::string& x);
  double next();


private:
  Rng() = default;
  double hash(const std::string& x) const;
  static double pow128(int i) { return std::ldexp(1.0, 7 * i); }

  double s = 1234;
  static constexpr double P = 999979.0;
  static constexpr double Q = 999983.0;
  double m = P * Q; // 999962000357.0
};

// "Math.random()"
inline double rnd() { return Rng::inst().next(); }

} // namespace ss
