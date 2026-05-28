#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <vector>

#include "Vector.hpp"

// =====================================================
// ThrowOnCopy
// =====================================================

struct ThrowOnCopy {
    int value = 0;

    static inline int copy_count = 0;
    static inline int throw_after = 5;

    ThrowOnCopy() = default;

    explicit ThrowOnCopy(int v)
        : value(v) {}

    ThrowOnCopy(const ThrowOnCopy& other)
        : value(other.value)
    {
        ++copy_count;

        if (copy_count >= throw_after) {
            throw std::runtime_error("ThrowOnCopy exploded");
        }
    }

    ThrowOnCopy(ThrowOnCopy&& other) noexcept
        : value(other.value) {}

    ThrowOnCopy& operator=(const ThrowOnCopy&) = default;
    ThrowOnCopy& operator=(ThrowOnCopy&&) noexcept = default;

    bool operator==(const ThrowOnCopy& other) const {
        return value == other.value;
    }
};

// =====================================================
// MoveOnly
// =====================================================

struct MoveOnly {
    std::unique_ptr<int> ptr;

    explicit MoveOnly(int v)
        : ptr(std::make_unique<int>(v)) {}

    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;

    MoveOnly(MoveOnly&& other) noexcept
        : ptr(std::move(other.ptr)) {}

    MoveOnly& operator=(MoveOnly&& other) noexcept {
        if (this != &other) {
            ptr = std::move(other.ptr);
        }

        return *this;
    }

    bool operator==(const MoveOnly& other) const {
        return *ptr == *other.ptr;
    }
};

// =====================================================
// BigAligned
// =====================================================

struct alignas(64) BigAligned {
    char data[1024]{};

    BigAligned() {
        memset(data, 0xAB, sizeof(data));
    }

    bool operator==(const BigAligned& other) const {
        return memcmp(data, other.data, sizeof(data)) == 0;
    }
};

// =====================================================
// SelfRef
// =====================================================

struct SelfRef {
    int value = 0;
    int* self = nullptr;

    SelfRef()
        : value(0), self(&value) {}

    explicit SelfRef(int v)
        : value(v), self(&value) {}

    SelfRef(const SelfRef& other)
        : value(other.value), self(&value) {}

    SelfRef(SelfRef&& other) noexcept
        : value(other.value), self(&value) {}

    SelfRef& operator=(const SelfRef& other) {
        if (this != &other) {
            value = other.value;
            self = &value;
        }

        return *this;
    }

    SelfRef& operator=(SelfRef&& other) noexcept {
        if (this != &other) {
            value = other.value;
            self = &value;
        }

        return *this;
    }

    [[nodiscard]]
    bool valid() const {
        return self == &value;
    }

    bool operator==(const SelfRef& other) const {
        return value == other.value;
    }
};

// =====================================================
// LeakChecker
// =====================================================

struct LeakChecker {
    static inline int alive = 0;

    int value = 0;

    LeakChecker() {
        ++alive;
    }

    explicit LeakChecker(int v)
        : value(v)
    {
        ++alive;
    }

    LeakChecker(const LeakChecker& other)
        : value(other.value)
    {
        ++alive;
    }

    LeakChecker(LeakChecker&& other) noexcept
        : value(other.value)
    {
        ++alive;
    }

    LeakChecker& operator=(const LeakChecker&) = default;
    LeakChecker& operator=(LeakChecker&&) noexcept = default;

    ~LeakChecker() {
        --alive;
    }

    bool operator==(const LeakChecker& other) const {
        return value == other.value;
    }
};

// =====================================================
// ThrowMove
// =====================================================

struct ThrowMove {
    int value = 0;

    ThrowMove() = default;

    explicit ThrowMove(int v)
        : value(v) {}

    ThrowMove(const ThrowMove&) = default;

    ThrowMove(ThrowMove&&) noexcept = default;

    ThrowMove& operator=(const ThrowMove&) = default;
    ThrowMove& operator=(ThrowMove&&) = default;

    bool operator==(const ThrowMove& other) const {
        return value == other.value;
    }
};

// =====================================================
// Immutable
// =====================================================

struct Immutable {
    const int value;

    explicit Immutable(int v)
        : value(v) {}

    Immutable(const Immutable&) = default;
    Immutable(Immutable&&) noexcept = default;

    Immutable& operator=(const Immutable&) = delete;
    Immutable& operator=(Immutable&&) = delete;

    bool operator==(const Immutable& other) const {
        return value == other.value;
    }
};

// =====================================================
// Helpers
// =====================================================

template<typename T>
void check_same(
    const Vector<T>& mine,
    const std::vector<T>& ref)
{
    assert(mine.size() == ref.size());

    for (std::size_t i = 0; i < ref.size(); ++i) {
        assert(mine[i] == ref[i]);
    }
}

// =====================================================
// Fuzz
// =====================================================

void fuzz_test() {

    Vector<int> mine;
    std::vector<int> ref;

    std::random_device rd;
    std::mt19937 rng(rd());

    for (int iter = 0; iter < 100000; ++iter) {

        switch (rng() % 5U) {

            case 0U: {
                const int value =
                    static_cast<int>(rng());

                mine.push_back(value);
                ref.push_back(value);

                break;
            }

            case 1U: {

                if (!ref.empty()) {

                    const std::size_t index =
                        rng() % ref.size();

                    mine.erase(index);
                    ref.erase(
                        ref.begin()
                        + static_cast<std::ptrdiff_t>(index));
                }

                break;
            }

            case 2U: {

                if (!ref.empty()) {

                    const std::size_t index =
                        rng() % ref.size();

                    const int value =
                        static_cast<int>(rng());

                    mine.insert(index, value);

                    ref.insert(
                        ref.begin()
                        + static_cast<std::ptrdiff_t>(index),
                        value);
                }

                break;
            }

            case 3U: {

                const std::size_t size =
                    rng() % 256U;

                mine.resize(size);
                ref.resize(size);

                break;
            }

            case 4U: {

                if (!ref.empty()) {
                    mine.pop_back();
                    ref.pop_back();
                }

                break;
            }

            default:
                std::abort();
        }

        check_same(mine, ref);
    }
}

// =====================================================
// Main
// =====================================================

int main() {

    std::cout << "fuzz..." << std::endl;
    fuzz_test();

    std::cout << "self ref..." << std::endl;

    {
        Vector<SelfRef> v;

        for (int i = 0; i < 10000; ++i) {
            v.push_back(SelfRef(i));
        }

        for (const auto& x : v) {
            assert(x.valid());
        }
    }

    std::cout << "alignment..." << std::endl;

    {
        Vector<BigAligned> v;

        for (int i = 0; i < 1000; ++i) {
            v.push_back(BigAligned{});
        }

        for (const auto& x : v) {
            assert(
                reinterpret_cast<std::uintptr_t>(&x)
                % 64U == 0U);
        }
    }

    std::cout << "move only..." << std::endl;

    {
        Vector<MoveOnly> v;

        for (int i = 0; i < 100; ++i) {
            v.push_back(MoveOnly(i));
        }

        for (int i = 0; i < 100; ++i) {
            assert(*v[static_cast<std::size_t>(i)].ptr == i);
        }
    }

    std::cout << "leaks..." << std::endl;

    {
        Vector<LeakChecker> v;

        for (int i = 0; i < 10000; ++i) {
            v.push_back(LeakChecker(i));
        }

        v.clear();
    }

    assert(LeakChecker::alive == 0);

    std::cout << "throw copy..." << std::endl;

    try {

        ThrowOnCopy::copy_count = 0;
        ThrowOnCopy::throw_after = 4;

        Vector<ThrowOnCopy> v;

        for (int i = 0; i < 100; ++i) {
            ThrowOnCopy item(i);
            v.push_back(item);
        }

    } catch (const std::exception& e) {

        std::cout
            << "caught expected exception: "
            << e.what()
            << std::endl;
    }

    std::cout << "move only..." << std::endl;

    {
        Vector<ThrowMove> v;

        for (int i = 0; i < 100; ++i) {
            v.push_back(ThrowMove(i));
        }

        for (int i = 0; i < 100; ++i) {
            assert(v[static_cast<std::size_t>(i)].value == i);
        }
    }

    std::cout << "immutable..." << std::endl;

    {
        Vector<Immutable> v;

        v.push_back(Immutable(1));
        v.push_back(Immutable(2));
        v.push_back(Immutable(3));

        assert(v[0].value == 1);
        assert(v[1].value == 2);
        assert(v[2].value == 3);
    }

    std::cout << "\nALL TESTS PASSED\n";
}