// This implementation of 'span' is heavily based on
//   https://github.com/tcbrindle/span
//
// I only simplified / stripped it down a bit:
// - Most extensions (compared to the std::span proposal) are removed.
// - Behavior on pre-condition violation is no longer configurable, this
//   implementation simply uses assert().
// - I dropped pre-C++14 support.
//
// Original copyright notice:
//
//    This is an implementation of std::span from P0122R7
//    http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0122r7.pdf
//
//             Copyright Tristan Brindle 2018.
//    Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file ../../LICENSE_1_0.txt or copy at
//             https://www.boost.org/LICENSE_1_0.txt)

#ifndef SPAN_HH
#define SPAN_HH

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>


constexpr size_t dynamic_extent = size_t(-1);

template<typename ElementType, size_t Extent = dynamic_extent>
class span;


namespace detail {

template<typename E, size_t S>
struct span_storage
{
    constexpr span_storage() noexcept = default;
    constexpr span_storage(E* ptr_, size_t /*unused*/) noexcept : ptr(ptr_) {}

    E* ptr = nullptr;
    static constexpr size_t size = S;
};

template<typename E>
struct span_storage<E, dynamic_extent>
{
    constexpr span_storage() noexcept = default;
    constexpr span_storage(E* ptr_, size_t size_) noexcept : ptr(ptr_), size(size_) {}

    E* ptr = nullptr;
    size_t size = 0;
};


// Reimplementation of C++17 std::size() and std::data()
template<typename C>
constexpr auto size(const C& c) -> decltype(c.size()) { return c.size(); }

template<typename T, size_t N>
constexpr auto size(const T (&)[N]) noexcept { return N; }


template<typename C>
constexpr auto data(C& c) -> decltype(c.data()) { return c.data(); }

template<typename C>
constexpr auto data(const C& c) -> decltype(c.data()) { return c.data(); }

template<typename T, size_t N>
constexpr T* data(T (&array)[N]) noexcept { return array; }

template<typename E>
constexpr const E* data(std::initializer_list<E> il) noexcept { return il.begin(); }


// Reimplementation of C++17 std::void_t
template<typename...>
using void_t = void;


template<typename>
struct is_span : std::false_type {};

template<typename T, size_t S>
struct is_span<span<T, S>> : std::true_type {};


template<typename>
struct is_std_array : std::false_type {};

template<typename T, size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};


template<typename, typename = void>
struct has_size_and_data : std::false_type {};

template<typename T>
struct has_size_and_data<T,
                         void_t<decltype(detail::size(std::declval<T>())),
                                decltype(detail::data(std::declval<T>()))>>
        : std::true_type {};


template<typename C, typename U = std::remove_cv_t<std::remove_reference_t<C>>>
struct is_container
{
    static constexpr bool value = !is_span<U>::value &&
                                  !is_std_array<U>::value &&
                                  !std::is_array<U>::value &&
                                  has_size_and_data<C>::value;
};


template<typename, typename, typename = void>
struct is_container_element_type_compatible : std::false_type {};

template<typename T, typename E>
struct is_container_element_type_compatible<T, E, void_t<decltype(detail::data(std::declval<T>()))>>
        : std::is_convertible<std::remove_pointer_t<decltype(detail::data(std::declval<T>()))> (*)[], E (*)[]>
{};


template<typename, typename = size_t>
struct is_complete : std::false_type {};

template<typename T>
struct is_complete<T, decltype(sizeof(T))> : std::true_type {};

} // namespace detail


template<typename ElementType, size_t Extent>
class span
{
    static_assert(Extent == dynamic_extent || Extent >= 0,
                  "A span must have an extent greater than or equal to zero, "
                  "or a dynamic extent");
    static_assert(std::is_object<ElementType>::value,
                  "A span's ElementType must be an object type (not a "
                  "reference type or void)");
    static_assert(detail::is_complete<ElementType>::value,
                  "A span's ElementType must be a complete type (not a forward "
                  "declaration)");
    static_assert(!std::is_abstract<ElementType>::value,
                  "A span's ElementType cannot be an abstract class type");

    using storage_type = detail::span_storage<ElementType, Extent>;

public:
    // constants and types
    using element_type = ElementType;
    using value_type = std::remove_cv_t<ElementType>;
    using index_type = size_t;
    using difference_type = ptrdiff_t;
    using pointer = ElementType*;
    using reference = ElementType&;
    using iterator = pointer;
    using const_iterator = const ElementType*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr index_type extent = Extent;

    // [span.cons], span constructors, copy, assignment, and destructor
    template<size_t E = Extent, std::enable_if_t<E <= 0, int> = 0>
    constexpr span() noexcept
    {
    }

    constexpr span(pointer ptr, index_type count)
        : storage(ptr, count)
    {
        assert(extent == dynamic_extent || count == extent);
    }

    constexpr span(pointer first_elem, pointer last_elem)
        : storage(first_elem, last_elem - first_elem)
    {
        assert(extent == dynamic_extent ||
               static_cast<index_type>(last_elem - first_elem) == extent);
    }

    template<size_t N,
             size_t E = Extent,
             std::enable_if_t<(E == dynamic_extent || N == E) &&
                              detail::is_container_element_type_compatible<element_type (&)[N],
                                                                           ElementType>::value,
                              int> = 0>
    constexpr span(element_type (&arr)[N]) noexcept
        : storage(arr, N)
    {
    }

    template<size_t N,
             size_t E = Extent,
             std::enable_if_t<(E == dynamic_extent || N == E) &&
                              detail::is_container_element_type_compatible<std::array<value_type, N>&,
                                                                           ElementType>::value,
                              int> = 0>
    /*constexpr*/ span(std::array<value_type, N>& arr) noexcept
        : storage(arr.data(), N)
    {
    }

    template<size_t N,
             size_t E = Extent,
             std::enable_if_t<(E == dynamic_extent || N == E) &&
                              detail::is_container_element_type_compatible<const std::array<value_type, N>&,
                                                                           ElementType>::value,
                      int> = 0>
    /*constexpr*/ span(const std::array<value_type, N>& arr) noexcept
        : storage(arr.data(), N)
    {
    }

    template<typename Container,
             std::enable_if_t<detail::is_container<Container>::value &&
                              detail::is_container_element_type_compatible<Container&, ElementType>::value,
                              int> = 0>
    constexpr span(Container& cont)
        : storage(detail::data(cont), detail::size(cont))
    {
        assert(extent == dynamic_extent || detail::size(cont) == extent);
    }

    template<typename Container,
             std::enable_if_t<detail::is_container<Container>::value &&
                              detail::is_container_element_type_compatible<const Container&, ElementType>::value,
                              int> = 0>
    constexpr span(const Container& cont)
        : storage(detail::data(cont), detail::size(cont))
    {
        assert(extent == dynamic_extent || detail::size(cont) == extent);
    }

    constexpr span(const span& other) noexcept = default;

    template<typename OtherElementType,
             size_t OtherExtent,
             std::enable_if_t<(Extent == OtherExtent || Extent == dynamic_extent) &&
                              std::is_convertible<OtherElementType (*)[], ElementType (*)[]>::value,
                              int> = 0>
    constexpr span(const span<OtherElementType, OtherExtent>& other) noexcept
      : storage(other.data(), other.size())
    {
    }

    ~span() noexcept = default;

    constexpr span& operator=(const span& other) noexcept = default;

    // [span.sub], span subviews
    template<size_t Count>
    constexpr span<element_type, Count> first() const
    {
        assert(Count >= 0 && Count <= size());
        return {data(), Count};
    }

    template<size_t Count>
    constexpr span<element_type, Count> last() const
    {
        assert(Count >= 0 && Count <= size());
        return {data() + (size() - Count), Count};
    }

    template<ptrdiff_t Offset, size_t Count = dynamic_extent>
    using subspan_return_t =
            span<ElementType,
                 Count != dynamic_extent ? Count
                                         : (Extent != dynamic_extent ? Extent - Offset : dynamic_extent)>;

    template<ptrdiff_t Offset, size_t Count = dynamic_extent>
    constexpr subspan_return_t<Offset, Count> subspan() const
    {
        assert((Offset >= 0 && Offset <= size()) &&
               (Count == dynamic_extent || (Count >= 0 && Offset + Count <= size())));
        return {data() + Offset,
                Count != dynamic_extent ? Count
                                        : (Extent != dynamic_extent ? Extent - Offset : size() - Offset)};
    }

    constexpr span<element_type, dynamic_extent> first(index_type count) const
    {
        assert(count >= 0 && count <= size());
        return {data(), count};
    }

    constexpr span<element_type, dynamic_extent> last(index_type count) const
    {
        assert(count >= 0 && count <= size());
        return {data() + (size() - count), count};
    }

    constexpr span<element_type, dynamic_extent> subspan(index_type offset,
                                                         index_type count = dynamic_extent) const
    {
        assert((offset >= 0 && offset <= size()) &&
               (count == dynamic_extent || (count >= 0 && offset + count <= size())));
        return {data() + offset, count == dynamic_extent ? size() - offset : count};
    }

    // [span.obs], span observers
    constexpr index_type size() const noexcept { return storage.size; }

    constexpr index_type size_bytes() const noexcept { return size() * sizeof(element_type); }

    constexpr bool empty() const noexcept { return size() == 0; }

    // [span.elem], span element access
    constexpr reference operator[](index_type idx) const
    {
        assert(idx >= 0 && idx < size());
        return *(data() + idx);
    }

    // Extension: not in P0122
    constexpr reference front() const
    {
        assert(!empty());
        return *data();
    }

    // Extension: not in P0122
    constexpr reference back() const
    {
        assert(!empty());
        return *(data() + (size() - 1));
    }

    constexpr pointer data() const noexcept { return storage.ptr; }

    // [span.iterators], span iterator support
    constexpr iterator begin() const noexcept { return data(); }
    constexpr iterator end()   const noexcept { return data() + size(); }
    constexpr const_iterator cbegin() const noexcept { return begin(); }
    constexpr const_iterator cend()   const noexcept { return end(); }

    /*constexpr*/ auto rbegin()  const noexcept { return reverse_iterator(end()); }
    /*constexpr*/ auto rend()    const noexcept { return reverse_iterator(begin()); }
    /*constexpr*/ auto crbegin() const noexcept { return const_reverse_iterator(cend()); }
    /*constexpr*/ auto crend()   const noexcept { return const_reverse_iterator(cbegin()); }

private:
    storage_type storage;
};


template<typename ElementType, size_t Extent>
span<const uint8_t, (Extent == dynamic_extent) ? dynamic_extent : (sizeof(ElementType) * Extent)>
as_bytes(span<ElementType, Extent> s) noexcept
{
    return {reinterpret_cast<const uint8_t*>(s.data()), s.size_bytes()};
}

template<typename ElementType,
         size_t Extent,
         std::enable_if_t<!std::is_const<ElementType>::value, int> = 0>
span<uint8_t, (Extent == dynamic_extent) ? dynamic_extent : (sizeof(ElementType) * Extent)>
as_writable_bytes( span<ElementType, Extent> s) noexcept
{
    return {reinterpret_cast<uint8_t*>(s.data()), s.size_bytes()};
}

#endif
