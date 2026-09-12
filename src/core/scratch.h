#pragma once
#include "core/poly.h"
#include <memory_resource>

namespace ss {

// Scratch containers for generator temporaries. Allocations go through the
// active ScratchScope's monotonic arena (heap when no scope is active).
// Must never outlive the scope they were allocated in.
using ScratchPts = std::pmr::vector<Pt>;
using ScratchPtsList = std::pmr::vector<ScratchPts>;

class ScratchScope {
public:
	explicit ScratchScope(std::size_t size_hint = 1 << 20);
	~ScratchScope();
	ScratchScope(const ScratchScope&) = delete;
	ScratchScope& operator=(const ScratchScope&) = delete;

	static std::pmr::memory_resource* current();

private:
	std::pmr::monotonic_buffer_resource arena_;
	std::pmr::memory_resource* prev_;
};

} // namespace ss
