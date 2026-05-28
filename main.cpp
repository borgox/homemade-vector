#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <new>
#include <stdexcept>

template<typename T>
class Vector {
    T *data_;
    std::size_t size_;
    std::size_t capacity_;

    void destroyAt(size_t index) {
        if (data_ == nullptr) {
            return;
        }
        data_[index].~T();
    }
    void destroyElements() {
        if (data_ == nullptr) {
            return;
        }
        for (std::size_t i = 0; i < size_; i++) {
            destroyAt(i);
        }
    }
    void destroy() {
        destroyElements();
        free(data_);
        data_ = nullptr;
    }

public:
    // Constructor
    Vector() {
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    Vector(const Vector &other) {
        if (other.empty()) {
            data_ = nullptr;
            size_ = 0;
            capacity_ = 0;
        } else {
            size_ = other.size();
            capacity_ = other.capacity();
            data_ = static_cast<T *>(malloc(sizeof(T) * capacity_));
            if (data_ == nullptr) {
                throw std::runtime_error("Vector copy constructor: could not allocate memory");
            }
            for (std::size_t i = 0; i < other.size(); i++) {
                new (data_ + i) T(other.data()[i]);
            }
        }
    }

    Vector &operator=(const Vector &other) {
        if (this == &other) {
            return *this;
        }

        if (other.empty()) {
            destroy();
            size_ = 0;
            capacity_ = 0;
            return *this;
        }

        destroy();
        size_ = other.size();
        capacity_ = other.capacity();
        data_ = static_cast<T *>(malloc(sizeof(T) * capacity_));
        if (data_ == nullptr) {
            throw std::runtime_error("Vector copy assignment: could not allocate memory");
        }

        for (std::size_t i = 0; i < other.size(); i++) {
            new (data_ + i) T(other.data()[i]);
        }
        return *this;
    }

    Vector(Vector &&other) noexcept {
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
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

    // Accessors
    [[nodiscard]] T *data() noexcept { return data_; }
    [[nodiscard]] const T *data() const noexcept { return data_; }
    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }

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
        if (empty()) {
            return;
        }
        destroyElements();
        size_ = 0;
        capacity_ = 0;
    }

    void reserve(size_t n) {
        if (n <= capacity_) {
            return;
        }
        auto temp = static_cast<T *>(malloc(sizeof(T) * n));
        if (temp == nullptr) {
            throw std::runtime_error("Vector.push_back: Could not allocate memory");
        }
        for (std::size_t i = 0; i < size_; i++) {
            new (temp + i) T(data_[i]);
        }
        destroy();
        data_ = temp;
        capacity_ = n;
    }
    void resize(size_t n) {
        reserve(n);
        if (n > size_) {
            for (size_t i = size_; i < n; i++) {
                new (data_ + i) T();
            }
        } else if (n < size_) {
            for (size_t i = n; i < size_; i++) {
                destroyAt(i);
            }
        }
        size_ = n;
    }
    void insert(size_t index, const T& value) {
        if (index > size_) {
            throw std::out_of_range("Vector: Index out of bounds");
        }
        if (size_ == capacity_) {
            // same code as push_back. Might refactor into a func someday
            capacity_ = capacity_ != 0 ? capacity_ * 2 : 1;
            auto temp = static_cast<T *>(malloc(sizeof(T) * capacity_));
            if (temp == nullptr) {
                throw std::runtime_error("Vector.push_back: Could not allocate memory");
            }
            for (std::size_t i = 0; i < size_; i++) {
                new (temp + i) T(data_[i]);
            }
            destroy();
            data_ = temp;
        }
        // Shift
        for (size_t i = size_; i > index; i--) {
            if (i == size_) {
                new(&data_[i]) T(data_[i-1]);
            } else {
                data_[i] = data_[i-1];
            }
        }
        // Insert
        data_[index] = value;
        size_++;
    }
    void erase(size_t const index ) {
        if (index >= size_) {
            throw std::out_of_range("Vector: Index out of bounds");
        }
        destroyAt(index);
        for (size_t i = index; i < size_ - 1; i++) {
            if (i == size_) {
                new(&data_[i]) T(data_[i+1]);
            } else {
                data_[i] = data_[i + 1];
            }
         }
        size_--;
    }
    void push_back(const T& value) {
        if (size_ == capacity_) {
            reserve(capacity_ != 0 ? capacity_ * 2 : 1);
        }
        new (&data_[size_]) T(value);
        size_++;
    }

    T pop_back() {
        if (size_ == 0) {
            throw std::out_of_range("pop_back on empty_vector");
        }
        size_--;
        T val = data_[size_];
        data_[size_].~T();
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
        return data_ ? data_ + size_ : nullptr;
    }

    // Destructor
    ~Vector() {
        destroy();
    }
};

int main() {
    Vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    for (auto x : v) {
        std::cout << x << std::endl;
    }

    std::cout << "size = " << v.size() << std::endl;
    std::cout << "capacity = " << v.capacity() << std::endl;
    std::cout << "front = " << v.front() << std::endl;
    std::cout << "back = " << v.back() << std::endl;
    return 0;
}
