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
#include <variant>

namespace col {

    // `T` が `std::optional` か調べる。
    template <class T>
    struct is_std_optional : std::false_type {};
    // `T` が `std::optional` か調べる。
    template <class T>
    struct is_std_optional<std::optional<T>> : std::true_type {};

    // `T` が `std::optional` であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_optional_v = is_std_optional<T>::value;


    // `T` が `std::variant` か調べる。
    template <class T>
    struct is_std_variant : std::false_type {};
    // `T` が `std::variant` か調べる。
    template <class ...Types>
    struct is_std_variant<std::variant<Types...>> : std::true_type {};

    // `T` が `std::variant` であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_variant_v = is_std_variant<T>::value;


    // `T` が `std::expected` か調べる。
    template <class T>
    struct is_std_expected : std::false_type {};
    // `T` が `std::expected` か調べる。
    template <class T, class E>
    struct is_std_expected<std::expected<T, E>> : std::true_type {};

    // `T` が `std::expected` であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_expected_v = is_std_expected<T>::value;


    // `T` が `std::pair` か調べる。
    template <class T>
    struct is_std_pair : std::false_type {};
    // `T` が `std::pair` か調べる。
    template <class T, class U>
    struct is_std_pair<std::pair<T, U>> : std::true_type {};

    // `T` が `std::pair` であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_pair_v = is_std_pair<T>::value;


    // `T` が `std::tuple` か調べる。
    template <class T>
    struct is_std_tuple : std::false_type {};
    // `T` が `std::tuple` か調べる。
    template <class ...Ts>
    struct is_std_tuple<std::tuple<Ts...>> : std::true_type {};

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
    struct unwrap_ok_type_if
    {
        using type = T;
    };
    // `T` が有効値または無効値を持つ型であれば有効値の型、そうでなければ `T` をメンバ型 `type` に持つ。
    template <class T>
    requires( is_std_optional_v<T> || is_std_expected_v<T> )
    struct unwrap_ok_type_if<T>
    {
        using type = std::remove_cvref_t<T>::value_type;
    };
    // `T` が有効値または無効値を持つ型であれば有効値の型、そうでなければ `T` 。
    template <class T>
    using unwrap_ok_type_if_t = unwrap_ok_type_if<T>::type;

    // 任意のイテレーター型 `It` から、そのイテレーターの要素への `const` 参照型を取得する。
    // C++23 で追加予定の実装の独自実装。
    template <std::indirectly_readable It>
    using iter_const_reference_t = std::common_reference_t<const std::iter_value_t<It>&&, std::iter_reference_t<It>>;

    // 任意の Range 型 `R` から、その Range の要素への `const` 参照型を取得する。
    // C++23 で追加予定の実装の独自実装。
    template <std::ranges::range R>
    using range_const_reference_t = iter_const_reference_t<std::ranges::iterator_t<R>>;


    namespace detail {
        template <bool Cond, class Trait1, class Trait2>
        struct conditional_type : public Trait2 {};
        template <class Trait1, class Trait2>
        struct conditional_type<true, Trait1, Trait2> : public Trait1 {};
    } // namespace detail

    // `Cond` が `true` であれば `Trait1` を、 `false` であれば `Trait2` を実体化させて継承する。
    // `Trait1` と `Trait2` はそれぞれ実体化されるときのみ `type` メンバ型を持てばよい。
    template <bool Cond, class Trait1, class Trait2>
    requires (
        (
            Cond &&
            requires {
                typename Trait1::type;
            }
        ) ||
        (
            !Cond &&
            requires {
                typename Trait2::type;
            }
        )
    )
    struct conditional_type : detail::conditional_type<Cond, Trait1, Trait2> {};

    // `Cond` が `true` であれば `Trait1::type` 、 `false` であれば `Trait2::type` 。
    // `Trait1` と `Trait2` はそれぞれ実体化されるときのみ `type` メンバ型を持てばよい。
    template <bool Cond, class Trait1, class Trait2>
    using conditional_type_t = conditional_type<Cond, Trait1, Trait2>::type;


    namespace detail {
        template <class F, class T, class>
        struct is_visitor_for_impl : std::false_type {};
        template <class F, class T, std::size_t ...Idx>
        requires (
            requires (F f, T t) {
                std::get<Idx>(t);
                { std::invoke(f, std::get<Idx>(t)) };
            } && ... && true
        )
        struct is_visitor_for_impl<F, T, std::index_sequence<Idx...>> : std::true_type {};
        template <class F, class T, class IdxSeq>
        inline constexpr auto is_visitor_for_impl_v = is_visitor_for_impl<F, T, IdxSeq>::value;
        
        template <class R, class F, class T, class>
        struct is_visitor_for_r_impl : std::false_type {};
        template <class R, class F, class T, std::size_t ...Idx>
        requires (
            requires (F f, T t) {
                std::get<Idx>(t);
                { std::invoke(f, std::get<Idx>(t)) } -> std::convertible_to<R>;
            } && ... && true
        )
        struct is_visitor_for_r_impl<R, F, T, std::index_sequence<Idx...>> : std::true_type {};
        template <class R, class F, class T, class IdxSeq>
        inline constexpr auto is_visitor_for_r_impl_v = is_visitor_for_r_impl<R, F, T, IdxSeq>::value;
    } // namespace detail

    // `F` が `T` の各要素型に対して Visitor パターンで呼び出せるか判定する。
    template <class F, class T>
    struct is_visitor_for : std::false_type {};
    // `F` が `T` の各要素型に対して Visitor パターンで呼び出せるか判定する。
    template <class F, class T>
    requires requires {
        std::tuple_size_v<std::remove_cvref_t<T>>;
        detail::is_visitor_for_impl_v<F, T, decltype(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<T>>>{})>;
    }
    struct is_visitor_for<F, T> : std::true_type {};
    // `F` が `T` の各要素型に対して Visitor パターンで呼び出せるか判定する。
    template <class F, class V>
    requires requires (F f, V v) {
        is_std_variant_v<std::remove_cvref_t<V>>;
        { std::visit(f, v) };
    }
    struct is_visitor_for<F, V> : std::true_type {};
    // `F` が `T` の各要素型に対して Visitor パターンで呼び出せれば `true` 、でなければ `false` 。
    template <class F, class T>
    inline constexpr auto is_visitor_for_v = is_visitor_for<F, T>::value;

    // `F` が `T` の各要素型に対して Visitor パターンで呼び出せて、その戻り値が `R` に変換可能か判定する。
    template <class R, class F, class T>
    struct is_visitor_for_r : std::false_type {};
    // `F` が `T` の各要素型に対して Visitor パターンで呼び出せて、その戻り値が `R` に変換可能か判定する。
    template <class R, class F, class T>
    requires requires {
        std::tuple_size_v<std::remove_cvref_t<T>>;
        detail::is_visitor_for_r_impl_v<R, F, T, decltype(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<T>>>{})>;
    }
    struct is_visitor_for_r<R, F, T> : std::true_type {};
    // `F` が `T` の各要素型に対して Visitor パターンで呼び出せて、その戻り値が `R` に変換可能か判定する。
    template <class R, class F, class V>
    requires requires (F f, V v) {
        is_std_variant_v<V>;
        { std::visit(f, v) } -> std::convertible_to<R>;
    }
    struct is_visitor_for_r<R, F, V> : std::true_type {};
    // `F` が `T` の各要素型に対して Visitor パターンで呼び出せて、その戻り値が `R` に変換可能であれば `true` 、でなければ `false` 。
    template <class R, class F, class T>
    inline constexpr auto is_visitor_for_r_v = is_visitor_for_r<R, F, T>::value;

} // namespace col
