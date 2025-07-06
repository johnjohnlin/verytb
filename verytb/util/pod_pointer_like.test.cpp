// Direct include
#include "util/pod_pointer_like.h"
// C system headers
// C++ standard library headers
// Other libraries' .h files.
#include "gtest/gtest.h"
// Your project's .h files.

using namespace verytb;

template<unsigned type_size>
void TestMode(detail::PointerMode expected_mode) {
	const auto mode = detail::PoolAllocationInfo<type_size>::kPointerMode;
	EXPECT_EQ(mode, expected_mode) << "for type size " << type_size;
}

TEST(pod_pointer_like, SizeModeCheck) {
	TestMode<detail::kPointerPoolThreshold - 1>(detail::PointerMode::eByValue);
	TestMode<detail::kPointerPoolThreshold>(detail::PointerMode::eByValue);

	TestMode<detail::kPointerPoolThreshold+1>(detail::PointerMode::eByPointerPool);
	TestMode<detail::kPointerThreshold>(detail::PointerMode::eByPointerPool);

	TestMode<detail::kPointerThreshold+1>(detail::PointerMode::eByPointer);
}

template<unsigned type_size>
void TestSize() {
	const unsigned pool_index = detail::PoolAllocationInfo<type_size>::kPoolIndex;
	const auto info = detail::PoolAllocationBlockInfo::IndexToBlockInfo(pool_index);
	EXPECT_GT(info.block_bytes, detail::kPointerPoolThreshold) << "for type size "<< type_size << ", pool index " << pool_index;
	EXPECT_LE(info.block_bytes, detail::kPointerThreshold) << "for type size " << type_size << ", pool index " << pool_index;
	EXPECT_GE(info.block_bytes, type_size) << "The resulting block size must be at least the type size for " << type_size << ", pool index " << pool_index;
	EXPECT_LT(info.block_bytes, type_size*3u/2u) << "The resulting block size wasted too much (more than 1.5x) space for " << type_size << ", pool index " << pool_index;
	EXPECT_GE(info.num_blocks_per_allocation, 4) << "for type size " << type_size;
}

TEST(pod_pointer_like, SizeCheck) {
	// Uncomment the following lines to see the mapping of sizes to pool indices and block sizes.
	// for (unsigned i = 129, prev = -1u; i <= 4096; ++i) {
	// 	const unsigned pool_index = detail::PoolAllocationBlockInfo::SizeToIndex(i);
	// 	if (prev == pool_index) continue;
	// 	prev = pool_index;
	// 	std::cout << i << ": " << pool_index << ", " << detail::PoolAllocationBlockInfo::IndexToBlockInfo(pool_index).block_bytes << std::endl;
	// }

	// Note: these values are hardcoded according to kPointerPoolThreshold and kPointerThreshold.
	// Only test values in (kPointerPoolThreshold, kPointerThreshold]
	TestSize<129>();
	TestSize<191>();
	TestSize<192>();
	TestSize<193>();
	TestSize<1023>();
	TestSize<1024>();
	TestSize<1025>();
	TestSize<1535>();
	TestSize<1536>();
	TestSize<1537>();
	TestSize<detail::kPointerThreshold-1>();
	TestSize<detail::kPointerThreshold>();
}
