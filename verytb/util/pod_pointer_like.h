#pragma once
// Direct include
// C system headers
// C++ standard library headers
#include <bit>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <stack>
#include <type_traits>
#include <vector>
// Other libraries' .h files.
// Your project's .h files.
#include "util/bit.h"

namespace verytb {

namespace detail {

enum class PointerMode {
	eByValue,
	eByPointerPool,
	eByPointer,
};
static constexpr unsigned kPointerPoolThreshold = 128;
static constexpr unsigned kPointerThreshold = 4096;
static constexpr unsigned kMaxBlocksPerAllocation = 128;
static_assert(bit::has_single_bit(kPointerPoolThreshold));
static_assert(bit::has_single_bit(kPointerThreshold));

struct PoolAllocationBlockInfo {
	unsigned block_bytes;
	unsigned num_blocks_per_allocation;

	constexpr static unsigned SizeToIndex(unsigned s) {
		// The logic here is to match the logic in IndexToBlockInfo.
		s -= 1;
		s /= (kPointerPoolThreshold/2);
		// we use -1 to keep the first 2 bits of s
		unsigned digits = bit::digits(s)-2;
		s >>= digits;
		// assert(s == 2 or s == 3);
		return (digits << 1) | (s == 3);
	}

	static constexpr PoolAllocationBlockInfo IndexToBlockInfo(unsigned index) {
		// This function maps an index to a block size and number of blocks per allocation.
		// index =
		//   0: 3<<6, 128 <-- 192 bytes, 128 blocks
		//   1: 4<<6, 128
		//   2: 3<<7, 64
		//   3: 4<<7, 64
		//   4: 3<<8, 32
		//   5: 4<<8, 32

		// Please refer to SizeCheck in pod_pointer_like.test.cpp for the test that verifies this logic.
		const bool is_even = (index & 1) == 0;
		index >>= 1;
		return {
			(is_even ? 3u : 4u) << (6 + index),
			(kMaxBlocksPerAllocation >> index)
		};
	}
};

template<unsigned type_size>
struct PoolAllocationInfo {
	constexpr static PointerMode kPointerMode = (
		type_size > kPointerThreshold ? PointerMode::eByPointer :
		type_size > kPointerPoolThreshold ? PointerMode::eByPointerPool :
		PointerMode::eByValue
	);
	static constexpr unsigned kPoolIndex = (
		kPointerMode != PointerMode::eByPointerPool ?
		-1u :
		PoolAllocationBlockInfo::SizeToIndex(type_size)
	);
};

class MemoryPool {
	MemoryPool(const MemoryPool&) = delete;
	MemoryPool& operator=(const MemoryPool&) = delete;
	MemoryPool(MemoryPool&&) = delete;
	MemoryPool& operator=(MemoryPool&&) = delete;

	void LazyInitializeToPoolIndex(unsigned pool_index);

public:

	friend class MemoryPoolDebugProxy; // only for testing and debugging purposes, define privately in cpp
	// Singleton instance
	static MemoryPool& Instance() {
		static MemoryPool instance;
		return instance;
	}
	MemoryPool() { pools_.reserve(32); } // you can still create a MemoryPool without singleton instance
	~MemoryPool() = default;

	void* Malloc(unsigned pool_index) {
		if (pool_index >= pools_.size()) [[unlikely]] {
			// move the content of this branch to a separate function to avoid code duplication
			// since Malloc is called frequently and this branch is very unlikely to be taken
			LazyInitializeToPoolIndex(pool_index);
		}
		return pools_[pool_index].malloc(pool_index);
	}

	void Free(void* ptr, unsigned pool_index) {
		pools_[pool_index].free(pool_index, ptr);
	}

	// This is public only for testing and debugging purposes.
	// Users should not rely on this interface.
	class PoolWrapper {
		void EnsureEnoughFree(unsigned pool_index);
	public:
		// store the actual memory pool (has ownership of the memory)
		std::vector<std::unique_ptr<char[]>> memory_pool_;
		// pointers to memory_pool_
		std::vector<void*> free_list;
		PoolAllocationBlockInfo block_info = {0, 0};

		void* malloc(unsigned pool_index) {
			if (free_list.empty()) [[unlikely]] {
				// move the content of this branch to a separate function to avoid code duplication
				// since malloc is called frequently and this branch is unlikely to be taken
				EnsureEnoughFree(pool_index);
			}
			void* ptr = free_list.back();
			free_list.pop_back();
			return ptr;
		}

		void free(unsigned pool_index, void* ptr) {
			free_list.push_back(ptr);
		}
	};
private:
	std::vector<PoolWrapper> pools_;
};

template<typename T, PointerMode mode = PointerMode::eByValue>
struct pointer_storage {
	T storage_;
	template<typename... Args>
	pointer_storage(Args&&... args) : storage_(std::forward<Args>(args)...) {}
	T* get() { return &storage_; }
	const T* get() const { return &storage_; }
	pointer_storage(pointer_storage&& rhs) : storage_(std::move(rhs.storage_)) {}
	pointer_storage& operator=(pointer_storage&& rhs) {
		storage_ = std::move(rhs.storage_);
		return *this;
	}
};

template<typename T>
struct pointer_storage<T, PointerMode::eByPointerPool> {
	static constexpr unsigned kPoolIndex = PoolAllocationInfo<sizeof(T)>::kPoolIndex;
	static_assert(kPoolIndex != -1u);
	T* storage_;
	template<typename... Args>
	pointer_storage(Args&&... args) : storage_(static_cast<T*>(MemoryPool::Instance().Malloc(kPoolIndex))) {
		new (storage_) T(std::forward<Args>(args)...);
	}
	~pointer_storage() {
		storage_->~T();
		MemoryPool::Instance().Free(static_cast<void*>(storage_), kPoolIndex);
	}
	T* get() { return storage_; }
	const T* get() const { return storage_; }
	pointer_storage(pointer_storage&& rhs) : storage_(rhs.storage_) {
		std::swap(rhs.storage_, storage_);
	}
	pointer_storage& operator=(pointer_storage&& rhs) {
		std::swap(storage_, rhs.storage_);
		return *this;
	}
};

template<typename T>
struct pointer_storage<T, PointerMode::eByPointer> {
	T* storage_;
	template<typename... Args>
	pointer_storage(Args&&... args) : storage_(new T(std::forward<Args>(args)...)) {}
	~pointer_storage() { delete storage_; }
	T* get() { return storage_; }
	const T* get() const { return storage_; }
	pointer_storage(pointer_storage&& rhs) : storage_(rhs.storage_) { rhs.storage_ = nullptr; }
	pointer_storage& operator=(pointer_storage&& rhs) {
		if (this != &rhs) {
			delete storage_;
			storage_ = rhs.storage_;
			rhs.storage_ = nullptr;
		}
		return *this;
	}
};

} // namespace detail


template<typename T>
class pointer_like {
	static constexpr detail::PointerMode kPointerMode = detail::PoolAllocationInfo<sizeof(T)>::kPointerMode;
	detail::pointer_storage<T, kPointerMode> storage_;

public:
	template<typename... Args>
	pointer_like(Args&&... args) : storage_(std::forward<Args>(args)...) {}
	T* get() { return storage_.get(); }
	T& operator*() { return *get(); }
	T* operator->() { return get(); }
	const T* get() const { return storage_.get(); }
	const T& operator*() const { return *get(); }
	const T* operator->() const { return get(); }
	pointer_like(const pointer_like&) = delete;
	pointer_like& operator=(const pointer_like&) = delete;
	pointer_like(pointer_like&& rhs) = default;
	pointer_like& operator=(pointer_like&& rhs) = default;
};

} // namespace verytb
