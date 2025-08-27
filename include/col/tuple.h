#pragma once

#include <cstddef>
#include <array>
#include <functional>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

namespace col {

    namespace detail {
        template <class T>
        struct is_std_pair_impl : std::false_type {};
        template <class T, class U>
        struct is_std_pair_impl<std::pair<T, U>> : std::true_type {};
        template <class T>
        struct is_std_tuple_impl : std::false_type {};
        template <class ...Ts>
        struct is_std_tuple_impl<std::tuple<Ts...>> : std::true_type {};
        template <class T>
        struct is_std_array_impl : std::false_type {};
        template <class T, std::size_t N>
        struct is_std_array_impl<std::array<T, N>> : std::true_type {};
        template <class T>
        struct is_std_ranges_subrange_impl : std::false_type {};
        template <class I, class S, std::ranges::subrange_kind K>
        struct is_std_ranges_subrange_impl<std::ranges::subrange<I, S, K>> : std::true_type {};
    } // namespace detail

    // `T` が `std::pair` の特殊化か調べる
    template <class T>
    struct is_std_pair : detail::is_std_pair_impl<std::remove_cvref_t<T>>{};

    // `T` が `std::tuple` の特殊化か調べる
    template <class T>
    struct is_std_tuple : detail::is_std_tuple_impl<std::remove_cvref_t<T>>{};
    
    // `T` が `std::array` の特殊化か調べる
    template <class T>
    struct is_std_array : detail::is_std_array_impl<std::remove_cvref_t<T>>{};

    // `T` が `std::ranges::subrange` の特殊化か調べる
    template <class T>
    struct is_std_ranges_subrange : detail::is_std_ranges_subrange_impl<std::remove_cvref_t<T>>{};

    // `T` が `std::pair` の特殊化であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_pair_v = is_std_pair<T>::value;
    // `T` が `std::tuple` の特殊化であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_tuple_v = is_std_tuple<T>::value;
    // `T` が `std::array` の特殊化であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_array_v = is_std_array<T>::value;
    // `T` が `std::ranges::subrange` の特殊化であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_std_ranges_subrange_v = is_std_ranges_subrange<T>::value;

    // tuple-like な型を表すコンセプト
    template <class T>
    concept tuple_like = 
        is_std_pair_v<std::remove_cvref_t<T>> ||
        is_std_tuple_v<std::remove_cvref_t<T>> ||
        is_std_array_v<std::remove_cvref_t<T>> ||
        is_std_ranges_subrange_v<std::remove_cvref_t<T>>;
    
    namespace detail {
        template <class T>
        struct tuple_like_size_impl;
        template <class T>
        requires (is_std_pair_v<T>)
        struct tuple_like_size_impl<T> : std::integral_constant<std::size_t, 2ZU> {};
        template <class T>
        requires (is_std_tuple_v<T>)
        struct tuple_like_size_impl<T> : std::tuple_size<T> {};
        template <class T>
        requires (is_std_array_v<T>)
        struct tuple_like_size_impl<T> : std::tuple_size<T> {};
        template <class T>
        requires (is_std_ranges_subrange_v<T>)
        struct tuple_like_size_impl<T> : std::integral_constant<std::size_t, 2Z> {};
    } // namespace detail

    // tuple-like な型 `T` の要素数を取得する
    template <class T>
    requires (tuple_like<T>)
    struct tuple_like_size : detail::tuple_like_size_impl<std::remove_cvref_t<T>> {};

    // tuple-like な型 `T` の要素数
    template <class T>
    inline constexpr std::size_t tuple_like_size_v = tuple_like_size<T>::value;


    namespace detail {
        template <std::size_t I, class ...Ts>
        constexpr decltype(auto) pack_tuple_elements(Ts&& ...ts)
        {
            return std::forward_as_tuple(std::get<I>(ts)...);
        }

        template <std::size_t ...Idx, class ...Ts>
        constexpr decltype(auto) pack_tuples_impl(std::index_sequence<Idx...>, Ts&& ...ts)
        {
            return std::tuple{ pack_tuple_elements<Idx>(ts...)... };
        }

        template <class Idx, class T, class F, class ...Args>
        struct invocable_per_tuple_elements_impl : std::false_type {};
        template <class T, class F, class ...Args>
        struct invocable_per_tuple_elements_impl<std::index_sequence<>, T, F, Args...> : std::true_type {};
        template <std::size_t ...Idx, class T, class F, class ...Args>
        struct invocable_per_tuple_elements_impl<std::index_sequence<Idx...>, T, F, Args...>
            : std::conditional_t<
                ( requires (T t, F f) {
                        { std::invoke(std::forward<F>(f), std::get<Idx>(t)) } -> std::same_as<std::tuple_element_t<0, T>>;
                    } && ...)
            , std::true_type, std::false_type>{};
        
   
        template <class T, class F, std::size_t Index, std::size_t ...Idx, class ...Args>
        constexpr auto invoke_per_tuple_elements_impl(std::size_t index, T&& t, F&& f, Args&& ...args)
        {
            if constexpr( sizeof...(Idx) > 0 )
            {
                if( index > Index )
                {
                    return invoke_per_tuple_elements_impl<T, F, Idx...>(index, std::forward<T>(t), std::forward<F>(f), std::forward<Args>(args)...);
                }
            }
            return std::invoke(std::forward<F>(f), std::get<Index>(std::forward<T>(t)), std::forward<Args>(args)...);
        }

        template <class T, class F, class ...Args, std::size_t ...Idx>
        constexpr auto invoke_per_tuple_elements_impl(std::index_sequence<Idx...>, std::size_t index, T&& t, F&& f, Args&& ...args)
        {
            return invoke_per_tuple_elements_impl<T, F, Idx...>(index, std::forward<T>(t), std::forward<F>(f), std::forward<Args>(args)...);
        }
    } // namespace detail

    // 1 つ以上のタプルの各要素を前からタプルにした新たなタプルを作成する
    template <class T, class ...Ts>
    requires ((tuple_like_size_v<T> == tuple_like_size_v<Ts>) && ... )
    constexpr decltype(auto) pack_tuples(T&& t, Ts&& ...ts)
    {
        return detail::pack_tuples_impl(std::make_index_sequence<tuple_like_size_v<T>>(), std::forward<T>(t), std::forward<Ts>(ts)...);
    }

    // `F` が tuple-like な型 `T` のいずれの要素に対しても、その要素と `Args...` を引数として呼び出せる、呼び出し可能オブジェクトであることを表すコンセプト
    template <class F, class T, class ...Args>
    concept invocable_per_tuple_elements = requires {
        tuple_like<T>;
        detail::invocable_per_tuple_elements_impl<decltype(std::make_index_sequence<tuple_like_size_v<T>>()), T, F, Args...>::value;
    };

    // tuple-like な型 `T` の指定した位置の要素を利用して `F` を呼び出す。
    // `F` は `T` のどの要素型を引数としても呼び出せる必要があり、かつ戻り値の型はどの場合も同じでなければならない。
    template <class T, class F, class ...Args>
    requires (invocable_per_tuple_elements<F, T, Args...>)
    constexpr auto invoke_per_tuple_elements(std::size_t index, T&& t, F&& f, Args&& ...args)
    {
        return detail::invoke_per_tuple_elements_impl(std::make_index_sequence<tuple_like_size_v<T>>(), index, std::forward<T>(t), std::forward<F>(f), std::forward<Args>(args)...);
    }

    constexpr void tes() {
        constexpr std::tuple<char, int> tu{ 'a', 10 };
        constexpr auto f = [](auto v, int i) {
            return static_cast<int>(v) + static_cast<int>(1) + i;
        };
        static_assert(invoke_per_tuple_elements(0, tu, f, 1) == 99);
        static_assert(invoke_per_tuple_elements(1, tu, f, 2) == 13);
        static_assert(invoke_per_tuple_elements(2, tu, f, 3) == 14); // TODO: tuple_size 以上の index を指定すると末尾になる問題を解消する
    }
} // namespace col
