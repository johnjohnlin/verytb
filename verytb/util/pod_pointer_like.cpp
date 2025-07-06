// Direct include
// C system headers
// C++ standard library headers
// Other libraries' .h files.
// Your project's .h files.
#include "util/pod_pointer_like.h"

namespace verytb::detail {

[[gnu::cold]]
void MemoryPool::LazyInitializeToPoolIndex(unsigned pool_index) {
	pools_.resize(pool_index + 1);
	for (unsigned i = 0; i <= pool_index; ++i) {
		auto& pool = pools_[i];
		if (pool.block_info.block_bytes == 0) {
			pool.block_info = PoolAllocationBlockInfo::IndexToBlockInfo(i);
		}
	}
}

[[gnu::cold]]
void MemoryPool::PoolWrapper::EnsureEnoughFree(unsigned pool_index) {
	// Allocate a new block array and split it into free blocks.
	memory_pool_.emplace_back(new char[block_info.block_bytes * block_info.num_blocks_per_allocation]);
	for (unsigned i = 0; i < block_info.num_blocks_per_allocation; ++i) {
		free_list.push_back(memory_pool_.back().get() + i * block_info.block_bytes);
	}
}

} // namespace verytb::detail
