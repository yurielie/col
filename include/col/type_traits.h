#pragma once

#include <cstddef>
#include <array>
#include <expected>
#include <iterator>
#include <optional>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

namespace col {

    // `T` が `std::optional` か調べる。
    template <class>
    struct is_std_optional : std::false_type {};
    // `T` が `std::optional` か調べる。
    template <class T>
    struct is_std_optional<std::optional<T>> : std::true_type {};

    // `T` が `std::optional` であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_optional_v = is_std_optional<T>::value;
    

    // `T` が `std::expected` か調べる。
    template <class>
    struct is_std_expected : std::false_type {};
    // `T` が `std::expected` か調べる。
    template <class T, class E>
    struct is_std_expected<std::expected<T, E>> : std::true_type {};

    // `T` が `std::expected` であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_expected_v = is_std_expected<T>::value;

    // `T` が `std::pair` か調べる。
    template <class>
    struct is_std_pair : std::false_type {};
    template <class T, class U>
    struct is_std_pair<std::pair<T, U>> : std::true_type {};

    // `T` が `std::pair` であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_pair_v = is_std_pair<T>::value;


    // `T` が `std::tuple` か調べる。
    template <class T>
    struct is_std_tuple : std::false_type {};
    // `T` が `std::tuple` か調べる。
    template <class ...Args>
    struct is_std_tuple<std::tuple<Args...>> : std::true_type {};
    
    // `T` が `std::tuple` であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_tuple_v = is_std_tuple<T>::value;


    // `T` が `std::array` か調べる。
    template <class T>
    struct is_std_array : std::false_type {};
    // `T` が `std::array` か調べる。
    template <class T, std::size_t N>
    struct is_std_array<std::array<T, N>> : std::true_type {};

    // `T` が `std::array` であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_array_v = is_std_array<T>::value;


    // `T` が `std::ranges::subrange` か調べる。
    template <class T>
    struct is_std_ranges_subrange : std::false_type {};
    // `T` が `std::ranges::subrange` か調べる。
    template <class I, class S, std::ranges::subrange_kind K>
    struct is_std_ranges_subrange<std::ranges::subrange<I, S, K>> : std::true_type {};

    // `T` が `std::ranges::subrange` であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_ranges_subrange_v = is_std_ranges_subrange<T>::value;


    // `T` が有効値または無効値を持つ型であれば有効値の型、そうでなければ `T` をメンバ型 `type` に持つ。
    template <class T>
    struct unwrap_noneable
    {
        using type = T;
    };
    // `T` が有効値または無効値を持つ型であれば有効値の型、そうでなければ `T` をメンバ型 `type` に持つ。
    template <class T>
    struct unwrap_noneable<std::optional<T>>
    {
        using type = T;
    };
    // `T` が有効値または無効値を持つ型であれば有効値の型、そうでなければ `T` をメンバ型 `type` に持つ。
    template <class T, class E>
    struct unwrap_noneable<std::expected<T, E>>
    {
        using type = T;
    };
    // `T` が有効値または無効値を持つ型であれば有効値の型、そうでなければ `T` 。
    template <class T>
    using unwrap_noneable_t = unwrap_noneable<T>::type;

    // 任意のイテレーター型 `It` から、そのイテレーターの要素への `const` 参照型を取得する。
    // C++23 で追加予定の実装の独自実装。
    template <std::indirectly_readable It>
    using iter_const_reference_t = std::common_reference_t<const std::iter_value_t<It>&&, std::iter_reference_t<It>>;

    // 任意の Range 型 `R` から、その Range の要素への `const` 参照型を取得する。
    // C++23 で追加予定の実装の独自実装。
    template <std::ranges::range R>
    using range_const_reference_t = iter_const_reference_t<std::ranges::iterator_t<R>>;

} // namespace col
