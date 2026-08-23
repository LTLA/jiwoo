#ifndef JIWOO_EQUILENGTH_ARRAYS_HPP
#define JIWOO_EQUILENGTH_ARRAYS_HPP

#include <algorithm>
#include <cstddef>
#include <type_traits>

/**
 * @file EquilengthArrays.hpp
 * @brief Array of equilength arrays.
 */

namespace jiwoo {

/**
 * @brief Array of equilength arrays.
 *
 * @tparam Value_ Value of the inner arrays.
 *
 * An instance `x` of this class is such that `x[i][j]` yields a `Value_` for `i` in `[0, size)` and `j` in `[0, length)`.
 */
template<typename Value_>
class EquilengthArrays {
public:
    /**
     * Default constructor.
     * This does not perform any allocation and sets the size and length to zero.
     */
    EquilengthArrays() = default;

    /**
     * @param size Number of inner arrays.
     * @param length Length of each inner array.
     *
     * This constructor will not initialize the values of the inner arrays.
     * If an initial value is desired, consider using the overloaded constructor below.
     */
    EquilengthArrays(std::size_t size, std::size_t length) : my_size(size), my_length(length) {
        my_ptr = safely_allocate(
            size,
            length,
            [](std::size_t, Value_*) -> void {}
        );
    }

    /**
     * @param size Number of inner arrays.
     * @param length Length of each inner array.
     * @param fill Initial value with which to fill the inner arrays.
     */
    EquilengthArrays(std::size_t size, std::size_t length, Value_ fill) : my_size(size), my_length(length) {
        my_ptr = safely_allocate(
            size,
            length,
            [&](std::size_t, Value_* ptr) -> void {
                std::fill_n(ptr, length, fill);
            }
        );
    }

    /**
     * Copy constructor.
     *
     * @param other An `EquilengthArray` to be copied.
     */
    EquilengthArrays(const EquilengthArrays& other) : my_size(other.my_size), my_length(other.my_length) {
        my_ptr = safely_allocate(
            my_size,
            my_length,
            [&](std::size_t i, Value_* ptr) -> void {
                std::copy_n(other.my_ptr[i], my_length, ptr);
            }
        );
    }

    /**
     * Move constructor.
     *
     * @param other An `EquilengthArray` to move from.
     * On return, this has its size set to zero.
     */
    EquilengthArrays(EquilengthArrays&& other) noexcept : my_ptr(other.my_ptr), my_size(other.my_size), my_length(other.my_length) {
        other.my_size = 0;
        other.my_ptr = NULL;
    }

private:
    Value_** my_ptr = NULL;
    std::size_t my_size = 0;
    std::size_t my_length = 0;

    template<typename Extra_>
    static Value_** safely_allocate(std::size_t size, std::size_t length, Extra_ extra) {
        Value_** output = new Value_* [size];

        std::size_t allocated = 0;
        try {
            for (std::size_t i = 0; i < size; ++i) {
                output[i] = new Value_ [length];
                ++allocated;
                extra(i, output[i]);
            }
        } catch(...) { 
            for (std::size_t x = 0; x < allocated; ++x) {
                delete [] output[x];
            }
            delete[] output;
            throw;
        }

        return output;
    }

public:
    /**
     * Destructor.
     */
    ~EquilengthArrays() {
        if (my_ptr != NULL) {
            for (std::size_t i = 0; i < my_size; ++i) {
                delete [] my_ptr[i];
            }
            delete [] my_ptr;
        }
    }

public:
    /**
     * Copy assignment.
     * This will attempt to use as much of the current object's existing allocation as possible, 
     * otherwise it will perform a new allocation.
     * 
     * @param other An `EquilengthArray` to be copied.
     */
    EquilengthArrays& operator=(const EquilengthArrays& other) {
        // If the memory is already available and correctly sized, we can just copy it in.
        // We only attempt this if the copy assignment is nothrow, as this ensures we can't crash with a half-copied object.
        // Obviously no copying is necessary at all if it's a self assignment.
        if constexpr(std::is_nothrow_copy_assignable<Value_>::value) {
            if (other.my_size == my_size && other.my_length == my_length) {
                if (other.my_ptr != my_ptr) { 
                    for (std::size_t i = 0; i < my_size; ++i) {
                        std::copy_n(other.my_ptr[i], my_length, my_ptr[i]);
                    }
                }
                return *this;
            }
        }

        // One might be tempted to optimize for the situation where 'other.my_size == my_size', and just reallocate the inner arrays.
        // The problem is that, if there's an allocation error and the move assignment fails halfway through the inner arrays,
        // we'll end up with a frankenobject for which 'my_length' is not correct.

        // Copy and swap idiom.
        EquilengthArrays tmp(other);
        swap(tmp);
        return *this;
    }

    /**
     * Move assignment.
     *
     * @param other An `EquilengthArray` to be moved from.
     * On return, this will have its size and length set to zero.
     */
    EquilengthArrays& operator=(EquilengthArrays&& other) noexcept {
        // Copy and swap idiom.
        EquilengthArrays tmp(std::move(other));
        swap(tmp);
        return *this;
    }

    /**
     * @param other An `EquilengthArray` to swap contents with.
     * On return, `*this` will have the contents of `other` and vice versa.
     */
    void swap(EquilengthArrays& other) noexcept {
        std::swap(other.my_size, my_size);
        std::swap(other.my_length, my_length);
        std::swap(other.my_ptr, my_ptr);
    }

public:
    /** 
     * @return Number of inner arrays.
     */
    std::size_t size() const {
        return my_size;
    }

    /** 
     * @return Length of each inner array.
     */
    std::size_t length() const {
        return my_length;
    }

    /**
     * @param i Index of the inner array.
     * This should be non-negative and less than `size()`.
     * @return Pointer to the start of the `i`-th inner array.
     */
    Value_* operator[](std::size_t i) {
        return my_ptr[i];
    }

    /**
     * @param i Index of the inner array.
     * This should be non-negative and less than `size()`.
     * @return `const` pointer to the start of the `i`-th inner array.
     */
    const Value_* operator[](std::size_t i) const {
        return my_ptr[i];
    }

    /**
     * @return Pointer to the start of the array of pointers to all inner arrays.
     */
    Value_* const* get() {
        return my_ptr;
    }

    /**
     * @return `const` pointer to the start of the array of pointers to all inner arrays.
     */
    const Value_* const * get() const {
        return my_ptr;
    }
};

}

#endif
