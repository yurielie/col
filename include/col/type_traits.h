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

    namespace detail {

        // `T` が `std::expected` か調べる。
        template <class>
        struct is_std_expected : std::false_type {};
        // `T` が `std::expected` か調べる。
        template <class T, class E>
        struct is_std_expected<std::expected<T, E>> : std::true_type {};

        // `T` が `std::pair` か調べる。
        template <class>
        struct is_std_pair : std::false_type {};
        template <class T, class U>
        struct is_std_pair<std::pair<T, U>> : std::true_type {};

        // `T` が `std::tuple` か調べる。
        template <class T>
        struct is_std_tuple : std::false_type {};
        // `T` が `std::tuple` か調べる。
        template <class ...Args>
        struct is_std_tuple<std::tuple<Args...>> : std::true_type {};

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

        // `Cond` が `true` のとき `Trait1` を、 `false` のときは `Trait2` を実体化させて継承する。
        template <bool Cond, class Trait1, class Trait2>
        struct conditional_type : public Trait2 {};
        // `Cond` が `true` のとき `Trait1` を、 `false` のときは `Trait2` を実体化させて継承する。
        template <class Trait1, class Trait2>
        struct conditional_type<true, Trait1, Trait2> : public Trait1 {};

    } // namespace detail

    // `T` が `std::optional` か調べる。
    template <class T>
    struct is_std_optional : std::false_type {};
    template <class T>
    struct is_std_optional<std::optional<T>> : std::true_type {};

    // `T` が `std::optional` であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_optional_v = is_std_optional<T>::value;

    template <class T>
    struct is_std_variant : std::false_type {};
    template <class ...Types>
    struct is_std_variant<std::variant<Types...>> : std::true_type {};
    template <class T>
    inline constexpr bool is_std_variant_v = is_std_variant<T>::value;

    // `T` が `std::expected` か調べる。
    template <class T>
    struct is_std_expected : std::false_type {};
    template <class T, class E>
    struct is_std_expected<std::expected<T, E>> : std::true_type {};

    // `T` が `std::expected` であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_expected_v = is_std_expected<T>::value;


    // `T` が `std::pair` か調べる。
    template <class T>
    struct is_std_pair : std::false_type {};
    template <class T, class U>
    struct is_std_pair<std::pair<T, U>> : std::true_type {};

    // `T` が `std::pair` であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_pair_v = is_std_pair<T>::value;


    // `T` が `std::tuple` か調べる。
    template <class T>
    struct is_std_tuple : std::false_type {};
    template <class ...Ts>
    struct is_std_tuple<std::tuple<Ts...>> : std::true_type {};

    // `T` が `std::tuple` であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_tuple_v = is_std_tuple<T>::value;


    // `T` が `std::array` か調べる。
    template <class T>
    struct is_std_array : std::false_type {};
    template <class T, std::size_t N>
    struct is_std_array<std::array<T, N>> : std::true_type {};

    // `T` が `std::array` であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_array_v = is_std_array<T>::value;


    // `T` が `std::ranges::subrange` か調べる。
    template <class T>
    struct is_std_ranges_subrange : std::false_type {};
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

    template <class T>
    struct variant_traits {};

    template <class ...Types>
    struct variant_traits<std::variant<Types...>>
    {
        static constexpr std::size_t size = sizeof...(Types);
        using types = std::tuple<Types...>;

        template <class ...Ts>
        requires (std::destructible<Ts> && ... && true)
        using pushed_back_types = std::variant<Types..., Ts...>;
    };

    template <class T, class F>
    struct is_visitor_for : std::false_type {};
    template <class ...Ts, class F>
    requires (
        requires (Ts ts) {
            std::is_invocable_v<F, Ts>;
        } && ... && true
    )
    struct is_visitor_for<std::tuple<Ts...>, F> : std::true_type {};
    template <class ...Ts, class F>
    requires (
        requires (Ts ts) {
            std::is_invocable_r_v<F, Ts>;
        } && ... && true
    )
    struct is_visitor_for<std::variant<Ts...>, F> : std::true_type {};

    template <class T, class R, class F>
    struct is_visitor_for_r : std::false_type {};
    template <class ...Ts, class R, class F>
    requires (
        requires (Ts ts) {
            std::is_invocable_r_v<R, F, Ts>;
        } && ... && true
    )
    struct is_visitor_for_r<std::tuple<Ts...>, R, F> : std::true_type {};
    template <class ...Ts, class R, class F>
    requires (
        requires (Ts ts) {
            std::is_invocable_r_v<R, F, Ts>;
        } && ... && true
    )
    struct is_visitor_for_r<std::variant<Ts...>, R, F> : std::true_type {};

} // namespace col
