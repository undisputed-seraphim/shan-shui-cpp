#include "core/scratch.h"

namespace ss {

namespace {
thread_local std::pmr::memory_resource* tls_arena = nullptr;
}

ScratchScope::ScratchScope(std::size_t size_hint)
	: arena_(size_hint)
	, prev_(tls_arena) {
	tls_arena = &arena_;
}

ScratchScope::~ScratchScope() { tls_arena = prev_; }

std::pmr::memory_resource* ScratchScope::current() { return tls_arena ? tls_arena : std::pmr::get_default_resource(); }

} // namespace ss
