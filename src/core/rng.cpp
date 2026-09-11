#include "core/rng.h"
#include <array>

namespace ss {
} // namespace ss

namespace ss {

// Minimal base64 encoder matching window.btoa for ASCII input.
static std::string b64encode(const std::string& in) {
  static const char* T =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string out;
  out.reserve(((in.size() + 2) / 3) * 4);
  size_t i = 0;
  while (i + 2 < in.size()) {
    unsigned v = (unsigned char)in[i] << 16 | (unsigned char)in[i + 1] << 8 |
                 (unsigned char)in[i + 2];
    out += T[(v >> 18) & 63];
    out += T[(v >> 12) & 63];
    out += T[(v >> 6) & 63];
    out += T[v & 63];
    i += 3;
  }
  if (i + 1 == in.size()) {
    unsigned v = (unsigned char)in[i] << 16;
    out += T[(v >> 18) & 63];
    out += T[(v >> 12) & 63];
    out += "==";
  } else if (i + 2 == in.size()) {
    unsigned v = (unsigned char)in[i] << 16 | (unsigned char)in[i + 1] << 8;
    out += T[(v >> 18) & 63];
    out += T[(v >> 12) & 63];
    out += T[(v >> 6) & 63];
    out += '=';
  }
  return out;
}

// JSON.stringify() for a string value (adds quotes and escapes).
static std::string jsonStringify(const std::string& s) {
  static const char* HEX = "0123456789abcdef";
  std::string r = "\"";
  for (unsigned char c : s) {
    switch (c) {
    case '"': r += "\\\""; break;
    case '\\': r += "\\\\"; break;
    case '\n': r += "\\n"; break;
    case '\r': r += "\\r"; break;
    case '\t': r += "\\t"; break;
    case '\b': r += "\\b"; break;
    case '\f': r += "\\f"; break;
    default:
      if (c < 0x20) {
        r += "\\u00";
        r += HEX[c >> 4];
        r += HEX[c & 15];
      } else {
        r += (char)c;
      }
    }
  }
  r += "\"";
  return r;
}

Rng& Rng::inst() {
  static Rng r;
  return r;
}

double Rng::hash(const std::string& x) const {
  std::string y = b64encode(jsonStringify(x));
  double z = 0;
  for (size_t i = 0; i < y.size(); i++) {
    z += (double)(unsigned char)y[i] * pow128((int)i);
  }
  return z;
}

void Rng::seed(const std::string& x) {
  double y = 0;
  double z = 0;
  auto redo = [&]() {
    y = std::fmod(hash(x) + z, m);
    z += 1;
  };
  while (std::fmod(y, P) == 0 || std::fmod(y, Q) == 0 || y == 0 || y == 1) {
    redo();
  }
  s = y;
  for (int i = 0; i < 10; i++) next();
}

double Rng::next() {
  s = std::fmod(s * s, m);
  return s / m;
}

} // namespace ss
