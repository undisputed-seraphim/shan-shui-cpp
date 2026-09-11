#pragma once
#include <string>
#include <vector>
#include <map>

namespace ss {

struct Chunk {
  std::string tag;
  double x, y;
  std::string canv;
};

// The whole scene state (equivalent of MEM in the original)
struct Scene {
  std::string canv;
  std::vector<Chunk> chunks;
  double xmin = 0, xmax = 0;
  double cwid = 512;
  double cursx = 0;
  double lasttick = 0;
  double windx = 3000, windy = 800;
  std::map<int, double> planmtx; // JS: sparse array with NaN semantics

  void seed(const std::string& s);
  void reset();

  void chunkloader(double xmin, double xmax);
  void chunkrender(double xmin, double xmax);
  // mirrors JS update(): load chunks + render + wrap in <svg>
  std::string getView(double cursx, double windx);
  std::string calcViewBox() const;
};

// per-generator entry points for the test driver
std::string genTree04(double x, double y, double hei, double wid);
std::string genMountain(double x, double y, double seed, double hei, double wid, int tex);
std::string genArch01(double x, double y, double seed);
std::string genMan(double x, double y, double sca);

} // namespace ss
