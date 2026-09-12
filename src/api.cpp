#include "core/noise.h"
#include "core/rng.h"
#include "scene/scene.h"
#include <emscripten/bind.h>

namespace ss {

static Scene g_scene;

static void ssInit(const std::string& seed) {
	g_scene.reset();
	g_scene.seed(seed);
}

static std::string ssGetView(double cursx, double windx) { return g_scene.getView(cursx, windx); }

static void ssReset() { g_scene.reset(); }

static double ssNoise(double x, double y) { return Noise::inst().noise(x, y); }

EMSCRIPTEN_BINDINGS(shan_shui) {
	emscripten::function("ss_init", &ssInit);
	emscripten::function("ss_get_view", &ssGetView);
	emscripten::function("ss_reset", &ssReset);
	emscripten::function("ss_noise", &ssNoise);
}

} // namespace ss
