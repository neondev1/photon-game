#include <chrono>
#include <random>

#include "head.hpp"

// ((sqrt5-1)/2) * 2^64
#define GOLDEN 0x9e3779b97f4a7c15ULL

unsigned mix32_rand(void) {
	std::random_device rd;
	static unsigned long long state = rd.entropy() ?
		rd() : std::chrono::high_resolution_clock::now().time_since_epoch().count();
	unsigned long long mix64 = (state += GOLDEN);
	mix64 = (mix64 ^ (mix64 >> 33)) * 0x62a9d9ed799705f5ULL;
	mix64 = (mix64 ^ (mix64 >> 28)) * 0xcb24d0a5c88c35b3ULL;
	return (unsigned)(mix64 >> 32);
}

// Returns a random number on [0, n)
unsigned mix32_rand(unsigned n) {
	unsigned long long limit = 0x100000000ULL - 0x100000000ULL % n;
	unsigned result;
	while ((result = mix32_rand()) > limit);
	return result % n;
}
