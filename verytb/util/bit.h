#pragma once
// Direct include
// C system headers
// C++ standard library headers
// Other libraries' .h files.
// Your project's .h files.

namespace verytb::bit {

// a subset of C++20 bit operations or similar functionality

constexpr bool has_single_bit(unsigned x) {
	return x != 0 && (x & (x - 1)) == 0;
}

constexpr unsigned popcount(unsigned x) {
	// Count the number of set bits in x
	x -= (x >> 1) & 0x55555555u;
	x = (x & 0x33333333u) + ((x >> 2) & 0x33333333u);
	x = (x + (x >> 4)) & 0x0F0F0F0Fu;
	x += x >> 8;
	x += x >> 16; // for 32-bit integers
	return x & 0x3Fu; // max popcount for 32-bit integers is 32
}

constexpr unsigned digits(unsigned x) {
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16; // for 32-bit integers
	return popcount(x);
}

} // namespace verytb::bit
