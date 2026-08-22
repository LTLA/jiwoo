#ifndef JIWOO_JIWOO_HPP
#define JIWOO_JIWOO_HPP

#include <optional>
#include <vector>
#include <type_traits>

/**
 * @file jiwoo.hpp
 * @brief Automatic memory management of containers of array pointers.
 */

/**
 * @namespace jiwoo
 * @brief Automatic memory management of containers of array pointers.
 */
namespace jiwoo {

/**
 * @brief Scope for freeing a container of array pointers.
 *
 * @tparam Container_ Any combination of `std::vector` and `std::optional` classes containing pointers to arrays, i.e., from `new[]`.
 *
 * Each instance of `Scope` should be bound to a separate instance of a `Container_`.
 * Once `Scope` goes out of scope, its destructor will traverse its bound `Container_` instance,
 * calling `delete[]` on all non-`NULL` array pointers that it can find. 
 */
template<typename Container_>
class Scope {
public:
    /**
     * @param x Instance of a `Container_` bound to this instance of `Scope`.
     * This should not be bound to any other instance of `Scope`.
     * The lifetime of `x` should exceed that of this instance of `Scope`.
     * Any non-`NULL` array pointers in `x` should not be used once this instance of `Scope` is destroyed.
     */
    Scope(Container_& x) : my_x(x) {}

    /**
     * @cond
     */
    ~Scope() {
        liberate(my_x);
    }
    /**
     * @endcond
     */

private:
    Container_& my_x;

    template<typename Inner_>
    static void liberate(const Inner_* x) {
        if (x) {
            delete [] x;
        }
    }

    template<typename Inner_>
    static void liberate(std::vector<Inner_>& x) {
        for (auto& y : x) {
            liberate(y);
        }
    }

    template<typename Inner_>
    static void liberate(std::optional<Inner_>& x) {
        if (x.has_value()) {
            liberate(*x);
        }
    }
};

/**
 * @cond
 */
template<typename Inner_>
void wipe(std::optional<Inner_>& x) noexcept {
    x.reset(); // noexcept in C++11
}

template<typename Inner_>
void wipe(std::vector<Inner_>& x) noexcept {
    x.clear(); // noexcept in C++11
}

template<typename Inner_>
void preallocate(Inner_* const&, Inner_*&) {}

template<typename Inner_>
void preallocate(const std::optional<Inner_>&, std::optional<Inner_>&);

template<typename Inner_>
void preallocate(const std::vector<Inner_>& from, std::vector<Inner_>& to) {
    const auto n = from.size();
    to.resize(n); // same size_type, so no need to worry about overflow.
    for (typename std::vector<Inner_>::size_type i = 0; i < n; ++i) {
        preallocate(from[i], to[i]);
    }
}

template<typename Inner_>
void preallocate(const std::optional<Inner_>& from, std::optional<Inner_>& to) {
    if (from.has_value()) {
        to.emplace();
        preallocate(*from, *to);
    }
}

template<typename Inner_>
void copy(Inner_* const& from, Inner_*& to) noexcept {
    to = from;
}

template<typename Inner_>
void copy(const std::optional<Inner_>&, std::optional<Inner_>&) noexcept;

template<typename Inner_>
void copy(const std::vector<Inner_>& from, std::vector<Inner_>& to) noexcept {
    const auto n = to.size();
    for (typename std::vector<Inner_>::size_type i = 0; i < n; ++i) {
        copy(from[i], to[i]);
    }
}

template<typename Inner_>
void copy(const std::optional<Inner_>& from, std::optional<Inner_>& to) noexcept {
    if (from.has_value()) {
        copy(*from, *to);
    }
}

template<bool manual_, typename Container_>
void transfer_internal(Container_& from, Container_& to) {
    if constexpr(std::is_nothrow_move_assignable<Container_>::value && !manual_) {
        to = std::move(from); // noexcept by definition.
    } else {
        preallocate(from, to); // this might throw, but no pointers have yet been transferred, so it's okay.
        copy(from, to); // noexcept
    }
    wipe(from); // also noexcept.
}
/**
 * @endcond
 */

/**
 * Transfer array pointers from one `Scope`-bound container to another.
 *
 * If this function throws an exception, none of the pointers will be transferred.
 * This ensures that each pointer will still be monitored by the `Scope` bound to `from`, so that it will not be freed twice.
 *
 * @tparam Container_ Any combination of `std::vector` and `std::optional`, see the template argument of the same name in `Scope`. 
 *
 * @param from Instance of a `Container_` from which to transfer pointers.
 * On return, this will not contain any pointers.
 * @param to Instance of a `Container_` to which the pointers to transferred.
 * On return, this will have the contents of the input `from`.
 */
template<typename Container_>
void transfer(Container_& from, Container_& to) {
    transfer_internal<false>(from, to);
}

}

#endif
