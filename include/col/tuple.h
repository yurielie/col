#pragma once

#include <col/type_traits.h>

#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>


namespace col {

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
        constexpr auto pack_tuple_elements(Ts&& ...ts)
        {
            return std::forward_as_tuple(std::get<I>(ts)...);
        }

        template <std::size_t ...Idx, class ...Ts>
        constexpr auto pack_tuples_impl(std::index_sequence<Idx...>, Ts&& ...ts)
        {
            return std::tuple{ pack_tuple_elements<Idx>(ts...)... };
        }
        
        // tuple_size が 1 のとき、 tuple<T1, T2, ...>{ tuple<T1, T2, ...>{} } と呼び出されてコンストラクタにマッチし、 tuple の入れ子にならない。
        // 防止するために tuple_size == 1 のときを特殊化し、型を明示的に指定する。
        template <class ...Ts>
        constexpr auto pack_tuples_impl(std::index_sequence<0>, Ts&& ...ts)
        {
            return std::tuple<decltype(pack_tuple_elements<0>(ts...))>{ pack_tuple_elements<0>(ts...) };
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
        constexpr auto invoke_per_tuple_elements_impl_one(std::size_t index, T&& t, F&& f, Args&& ...args)
        {
            if constexpr( sizeof...(Idx) > 0 )
            {
                if( index > Index )
                {
                    return invoke_per_tuple_elements_impl_one<T, F, Idx...>(index, std::forward<T>(t), std::forward<F>(f), std::forward<Args>(args)...);
                }
            }
            return std::invoke(std::forward<F>(f), std::get<Index>(std::forward<T>(t)), std::forward<Args>(args)...);
        }

        template <class T, class F, std::size_t Index, std::size_t ...Idx, class ...Args>
        constexpr auto invoke_per_tuple_elements_impl(std::index_sequence<Index, Idx...>, std::size_t index, T&& t, F&& f, Args&& ...args)
        {
            return invoke_per_tuple_elements_impl_one<T, F, Index, Idx...>(index, std::forward<T>(t), std::forward<F>(f), std::forward<Args>(args)...);
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

        constexpr std::tuple<int> tu1{1};
        constexpr auto f2 = [](auto v, int i) {
            return v + i;
        };
        static_assert(invoke_per_tuple_elements(0, tu1, f2, 1) == 2);
    }
} // namespace col
