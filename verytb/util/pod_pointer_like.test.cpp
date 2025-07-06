// Direct include
#include "util/pod_pointer_like.h"
// C system headers
// C++ standard library headers
#include <algorithm>
#include <vector>
// Other libraries' .h files.
#include "gtest/gtest.h"
// Your project's .h files.

using namespace verytb;
using namespace std;

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
	EXPECT_GE(info.block_bytes, type_size) << "The resulting block size must be at least " << type_size << ", pool index " << pool_index;
	EXPECT_LT(info.block_bytes, type_size*3u/2u) << "The resulting block size wasted too much (more than 1.5x) space for " << type_size << ", pool index " << pool_index;
	EXPECT_GE(info.num_blocks_per_allocation, 4) <<
		"If block per allocation is less than 4 (heuristic), "
		"then we believe there is no points in using pool allocator, "
		"please try to increase kMaxBlocksPerAllocation";
}

TEST(pod_pointer_like, SizeCheck) {
	// Uncomment the following lines to see the mapping of sizes to pool indices and block sizes.
	// for (unsigned i = 129, prev = -1u; i <= 4096; ++i) {
	// 	const unsigned pool_index = detail::PoolAllocationBlockInfo::SizeToIndex(i);
	// 	if (prev == pool_index) continue;
	// 	prev = pool_index;
	// 	cout << i << ": " << pool_index << ", " << detail::PoolAllocationBlockInfo::IndexToBlockInfo(pool_index).block_bytes << endl;
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

namespace verytb::detail {

struct MemoryPoolDebugProxy {
	MemoryPool& pool_;

	MemoryPoolDebugProxy(MemoryPool& pool) : pool_(pool) {}

	auto& pools() {
		return pool_.pools_;
	}
};

} // namespace verytb::detail

TEST(pod_pointer_like, MemoryPoolInternal) {
	// This tests the internal implementation of MemoryPool
	auto& pool_ = detail::MemoryPool::Instance();
	auto proxy = detail::MemoryPoolDebugProxy(pool_);
	auto& pool_memory = proxy.pools();

	// If we Malloc(0), we get contagious memory blocks with
	// spacing detail::PoolAllocationBlockInfo::IndexToBlockInfo(pool_index).block_bytes
	{
		auto& pool0 = pool_memory[0];

		// Consume the whole blocks of the first unique_ptr<char[]> allocation
		vector<char*> allocated;
		const unsigned kBlockSize = detail::PoolAllocationBlockInfo::IndexToBlockInfo(0).block_bytes;
		for (unsigned i = 0; i < detail::kMaxBlocksPerAllocation; ++i) {
			allocated.push_back((char*)pool_.Malloc(0));
			ASSERT_EQ(pool0.memory_pool_.size(), 1);
			ASSERT_EQ(pool0.free_list.size(), detail::kMaxBlocksPerAllocation - i - 1);
		}

		// Check that the allocated blocks are contiguous
		sort(allocated.begin(), allocated.end());
		char* base = pool0.memory_pool_[0].get();
		for (unsigned i = 0; i < allocated.size(); ++i) {
			EXPECT_EQ(allocated[i] - base, i * kBlockSize);
		}

		// return the first 3 blocks to the pool
		for (unsigned i = 0; i < 3; ++i) {
			pool_.Free(allocated[i], 0);
		}
		ASSERT_EQ(pool0.free_list.size(), 3);
		// and retrive the first 3 blocks again
		for (unsigned i = 0; i < 3; ++i) {
			pool_.Malloc(0);
			ASSERT_EQ(pool0.memory_pool_.size(), 1);
		}
		// now allocate 1 more block, which should trigger a new allocation
		allocated.push_back((char*)pool_.Malloc(0));
		ASSERT_EQ(pool0.memory_pool_.size(), 2);
	}
	{
		// After Malloc(5), we should have 6 pools in total.
		// Each pool should have its block information lazily initialized.
		const unsigned kTargetPoolIndex = 5;
		pool_.Malloc(kTargetPoolIndex);
		ASSERT_EQ(pool_memory.size(), kTargetPoolIndex + 1);
		for (unsigned i = 0; i <= kTargetPoolIndex; ++i) {
			auto& fixed_size_pool = pool_memory[i];
			const auto expected_info = detail::PoolAllocationBlockInfo::IndexToBlockInfo(i);
			const auto& info = fixed_size_pool.block_info;
			EXPECT_EQ(info.block_bytes, expected_info.block_bytes) << "for pool index " << i;
			EXPECT_EQ(info.num_blocks_per_allocation, expected_info.num_blocks_per_allocation) << "for pool index " << i;
		}
	}
}
