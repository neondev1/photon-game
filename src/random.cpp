#include <chrono>
#include <cstdint>
#include <random>

#include "head.hpp"

// Based on Steele et al. (2014)
// Uses Variant 4 from http://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html
static constexpr uint32_t mix64_32(uint64_t input) {
	input = (input ^ (input >> 33)) * 0x62a9d9ed799705f5ULL;
	input = (input ^ (input >> 28)) * 0xcb24d0a5c88c35b3ULL;
	return (uint32_t)(input >> 32);
}

// Returns a random number on [0, UINT32_MAX]
uint32_t mix32_rand(void) {
	static std::random_device rd;
	static std::chrono::high_resolution_clock::duration time = std::chrono::high_resolution_clock::now().time_since_epoch();
	static uint64_t state = rd.entropy() ? rd()
		: std::chrono::duration_cast<std::chrono::seconds>(time).count() ^ std::chrono::duration_cast<std::chrono::milliseconds>(time).count()
		^ std::chrono::duration_cast<std::chrono::microseconds>(time).count() ^ std::chrono::duration_cast<std::chrono::nanoseconds>(time).count();
	return mix64_32(state += 0x9e3779b97f4a7c15ULL); // ((sqrt5-1)/2) * 2^64
}

// Returns a random number on [0, n)
uint32_t mix32_rand(uint32_t n) {
	uint32_t limit = (uint32_t)((UINT32_MAX + 1ULL) - (UINT32_MAX + 1ULL) % n - 1ULL);
	uint32_t result;
	while ((result = mix32_rand()) > limit);
	return result % n;
}
