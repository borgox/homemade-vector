#pragma once

#include <cstddef>
#include <concepts>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

template<typename T, typename Allocator = std::allocator<T>>
class Vector {
	T *data_ = nullptr;
	std::size_t size_ = 0;
	std::size_t capacity_ = 0;

	void destroyBuffer(T *buffer, std::size_t count, std::size_t capacity) noexcept {
		if (buffer == nullptr) {
			return;
		}
		std::destroy_n(buffer, count);
		std::allocator_traits<Allocator>::deallocate(_allocator, buffer, capacity);
	}

	void destroyAt(size_t index) {
		std::allocator_traits<Allocator>::destroy(_allocator, data_ + index);
	}

	void destroyElements() {
		for (std::size_t i = 0; i < size_; i++) {
			destroyAt(i);
		}
	}

	void destroy() noexcept {
		if (data_ == nullptr) {
			size_ = 0;
			capacity_ = 0;
			return;
		}
		destroyElements();
		std::allocator_traits<Allocator>::deallocate(_allocator, data_, capacity_);
		data_ = nullptr;
		size_ = 0;
		capacity_ = 0;
	}

	[[no_unique_address]] Allocator _allocator{};

public:
	Vector() = default;

	explicit Vector(const Allocator& alloc)
		: data_(nullptr), size_(0), capacity_(0), _allocator(alloc) {}

	Vector(const Vector &other)
		requires std::copy_constructible<T>
		: data_(nullptr), size_(0), capacity_(0), _allocator(std::allocator_traits<Allocator>::select_on_container_copy_construction(other._allocator)) {
		if (other.empty()) {
			return;
		}
		T *buffer = std::allocator_traits<Allocator>::allocate(_allocator, other.capacity());
		std::size_t constructed = 0;
		try {
			for (; constructed < other.size(); ++constructed) {
				std::allocator_traits<Allocator>::construct(_allocator, buffer + constructed, other.data_[constructed]);
			}
		} catch (...) {
			destroyBuffer(buffer, constructed, other.capacity());
			throw std::runtime_error("Vector copy constructor: could not allocate or construct elements");
		}
		data_ = buffer;
		size_ = other.size();
		capacity_ = other.capacity();
	}

	Vector &operator=(const Vector &other)
		requires std::copy_constructible<T> {
		if (this == &other) {
			return *this;
		}

		if (other.empty()) {
			destroy();
			return *this;
		}

		using POC = std::allocator_traits<Allocator>::propagate_on_container_copy_assignment;
		Allocator alloc_for_new = _allocator;
		if constexpr (POC::value) {
			alloc_for_new = other._allocator;
		}

		T *buffer = std::allocator_traits<Allocator>::allocate(alloc_for_new, other.capacity());
		std::size_t constructed = 0;
		try {
			for (; constructed < other.size(); ++constructed) {
				std::allocator_traits<Allocator>::construct(alloc_for_new, buffer + constructed, other.data_[constructed]);
			}
		} catch (...) {
			std::destroy_n(buffer, constructed);
			std::allocator_traits<Allocator>::deallocate(alloc_for_new, buffer, other.capacity());
			throw std::runtime_error("Vector copy assignment: could not allocate or construct elements");
		}

		destroy();
		if constexpr (POC::value) {
			_allocator = other._allocator;
		}
		data_ = buffer;
		size_ = other.size();
		capacity_ = other.capacity();
		return *this;
	}

	Vector(Vector &&other) noexcept
		: data_(other.data_), size_(other.size_), capacity_(other.capacity_), _allocator(std::move(other._allocator)) {
		other.data_ = nullptr;
		other.size_ = 0;
		other.capacity_ = 0;
	}

	Vector &operator=(Vector &&other) noexcept {
		if (this == &other) {
			return *this;
		}

		destroy();
		data_ = other.data_;
		size_ = other.size_;
		capacity_ = other.capacity_;
		other.data_ = nullptr;
		other.size_ = 0;
		other.capacity_ = 0;
		return *this;
	}

	[[nodiscard]] T *data() noexcept { return data_; }
	[[nodiscard]] const T *data() const noexcept { return data_; }
	[[nodiscard]] std::size_t size() const noexcept { return size_; }
	[[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }
	[[nodiscard]] Allocator get_alloc() const noexcept { return _allocator; }

	T &operator[](std::size_t index) {
		return data_[index];
	}

	const T &operator[](std::size_t index) const {
		return data_[index];
	}

	[[nodiscard]] T &at(std::size_t index) {
		if (index >= size_) {
			throw std::out_of_range("Vector.at: index out of bounds");
		}
		return data_[index];
	}

	[[nodiscard]] const T &at(std::size_t index) const {
		if (index >= size_) {
			throw std::out_of_range("Vector.at: index out of bounds");
		}
		return data_[index];
	}

	[[nodiscard]] bool empty() const noexcept {
		return size_ == 0;
	}

	[[nodiscard]] T &front() {
		if (empty()) {
			throw std::out_of_range("Vector.front: cannot return first value from an empty vector");
		}
		return data_[0];
	}

	[[nodiscard]] const T &front() const {
		if (empty()) {
			throw std::out_of_range("Vector.front: cannot return first value from an empty vector");
		}
		return data_[0];
	}

	[[nodiscard]] T &back() {
		if (empty()) {
			throw std::out_of_range("Vector.back: cannot return last value from an empty vector");
		}
		return data_[size_ - 1];
	}

	[[nodiscard]] const T &back() const {
		if (empty()) {
			throw std::out_of_range("Vector.back: cannot return last value from an empty vector");
		}
		return data_[size_ - 1];
	}

	void clear() {
		destroyElements();
		size_ = 0;
	}

	void reserve(size_t n) {
		if (n <= capacity_) {
			return;
		}
		T *buffer = std::allocator_traits<Allocator>::allocate(_allocator, n);

		std::size_t constructed = 0;
		try {
			if constexpr (std::is_trivially_copyable_v<T>) {
				if (data_ != nullptr && size_ > 0) {
					memcpy(buffer, data_, size_ * sizeof(T));
				}
			} else {
				for (; constructed < size_; ++constructed) {
					std::allocator_traits<Allocator>::construct(_allocator, buffer + constructed, std::move_if_noexcept(data_[constructed]));
				}
			}
		} catch (...) {
			destroyBuffer(buffer, constructed, n);
			throw;
		}

		if (data_ != nullptr) {
			destroyElements();
			std::allocator_traits<Allocator>::deallocate(_allocator, data_, capacity_);
		}
		data_ = buffer;
		capacity_ = n;
	}

	void resize(size_t const n) {
		reserve(n);
		if (n > size_) {
			if constexpr (!std::is_default_constructible_v<T>) {
				throw std::logic_error("Vector.resize: T must be default constructible to grow the vector");
			}
			for (size_t i = size_; i < n; i++) {
				std::allocator_traits<Allocator>::construct(_allocator, data_ + i);
			}
		} else if (n < size_) {
			for (size_t i = n; i < size_; i++) {
				destroyAt(i);
			}
		}
		size_ = n;
	}

	void insert(size_t index, const T &value) {
		if (index > size_) {
			throw std::out_of_range("Vector: Index out of bounds");
		}
		if (size_ == capacity_) {
			reserve(capacity_ != 0 ? capacity_ * 2 : 1);
		}
		if (index == size_) {
			std::allocator_traits<Allocator>::construct(_allocator, data_ + size_, value);
			++size_;
			return;
		}

		std::allocator_traits<Allocator>::construct(_allocator, data_ + size_, std::move_if_noexcept(data_[size_ - 1]));
		for (size_t i = size_ - 1; i > index; --i) {
			data_[i] = std::move(data_[i - 1]);
		}
		data_[index] = value;
		++size_;
	}

	void erase(size_t const index) {
		if (index >= size_) {
			throw std::out_of_range("Vector: Index out of bounds");
		}
		for (size_t i = index; i + 1 < size_; ++i) {
			data_[i] = std::move(data_[i + 1]);
		}
		std::allocator_traits<Allocator>::destroy(_allocator, data_ + size_ - 1);
		--size_;
	}

	void push_back(const T &value) {
		if (size_ == capacity_) {
			reserve(capacity_ != 0 ? capacity_ * 2 : 1);
		}
		std::allocator_traits<Allocator>::construct(_allocator, data_ + size_, value);
		++size_;
	}

	void push_back(T &&value) {
		if (size_ == capacity_) {
			reserve(capacity_ != 0 ? capacity_ * 2 : 1);
		}
		std::allocator_traits<Allocator>::construct(_allocator, data_ + size_, std::move(value));
		++size_;
	}

	template<typename... Args>
	T &emplace_back(Args&&... args) {
		if (size_ == capacity_) {
			reserve(capacity_ != 0 ? capacity_ * 2 : 1);
		}
		std::allocator_traits<Allocator>::construct(_allocator, data_ + size_, std::forward<Args>(args)...);
		return data_[size_++];
	}

	T pop_back() {
		if (size_ == 0) {
			throw std::out_of_range("pop_back on empty vector");
		}
		--size_;
		T val = std::move(data_[size_]);
		std::allocator_traits<Allocator>::destroy(_allocator, data_ + size_);
		return val;
	}

	[[nodiscard]] T *begin() noexcept {
		return data_;
	}

	[[nodiscard]] T *end() noexcept {
		return data_ + size_;
	}

	[[nodiscard]] const T *begin() const noexcept {
		return data_;
	}

	[[nodiscard]] const T *end() const noexcept {
		return data_ + size_;
	}

	~Vector() {
		destroy();
	}
};
