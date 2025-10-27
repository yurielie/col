#pragma once

#include <col/control_flow.h>
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
        is_std_pair_v<T> ||
        is_std_tuple_v<T> ||
        is_std_array_v<T> ||
        is_std_ranges_subrange_v<T>;


    namespace detail {
        template <std::size_t I, class ...Ts>
        constexpr auto zip_tuple_elements(Ts&& ...ts)
        {
            return std::forward_as_tuple(std::get<I>(ts)...);
        }

        template <std::size_t ...Idx, class ...Ts>
        constexpr auto zip_tuples_impl(std::index_sequence<Idx...>, Ts&& ...ts)
        {
            // tuple_size が 1 のとき、 tuple<T1, T2, ...>{ tuple<T1, T2, ...>{} } と呼び出されてコンストラクタにマッチしてしまう。
            // tuple の入れ子になるよう型を明示する。
            return std::tuple<decltype(zip_tuple_elements<Idx>(ts...))...>{ zip_tuple_elements<Idx>(ts...)... };
        }

        template <template <std::size_t I> class Trait, std::size_t Index, std::size_t ...Generated>
        consteval auto filter_index_sequence_impl(std::index_sequence<Index>, std::index_sequence<Generated...>)
        {
            if constexpr( Trait<Index>::value )
            {
                return std::index_sequence<Generated..., Index>();
            }
            else
            {
                return std::index_sequence<Generated...>();
            }
        }
        
        template <template <std::size_t I> class Trait, std::size_t Index, std::size_t ...Idx, std::size_t ...Generated>
        consteval auto filter_index_sequence_impl(std::index_sequence<Index, Idx...>, std::index_sequence<Generated...>)
        {
            if constexpr( Trait<Index>::value )
            {
                return filter_index_sequence_impl<Trait>(
                    std::index_sequence<Idx...>(),
                    std::index_sequence<Generated..., Index>()
                );
            }
            else
            {
                return filter_index_sequence_impl<Trait>(
                    std::index_sequence<Idx...>(),
                    std::index_sequence<Generated...>()
                );
            }
        }
    } // namespace detail

    // 1 つ以上のタプルの各要素を前からタプルにした新たなタプルを作成する
    template <class T, class ...Ts>
    requires (
        is_std_tuple_v<std::remove_cvref_t<T>> &&
        (is_std_tuple_v<std::remove_cvref_t<Ts>> && ...) &&
        ( (std::tuple_size_v<std::remove_cvref_t<T>> == std::tuple_size_v<std::remove_cvref_t<Ts>>) && ... )
    )
    constexpr decltype(auto) zip_tuples(T&& t, Ts&& ...ts)
    {
        return detail::zip_tuples_impl(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<T>>>(), std::forward<T>(t), std::forward<Ts>(ts)...);
    }


    // `T` が m 行 n 列 の行列様のタプル型 `std::tuple<std::tuple<A1, ..., An>, ..., std::tuple<M1, ..., Mn>>` であるか調べる。
    template <class T>
    struct is_tuple_matrix : std::false_type {};
    // `T` が m 行 n 列 の行列様のタプル型 `std::tuple<std::tuple<A1, ..., An>, ..., std::tuple<M1, ..., Mn>>` であるか調べる。
    template <class T, class ...Ts>
    requires (
        is_std_tuple_v<std::remove_cvref_t<T>> &&
        (is_std_tuple_v<std::remove_cvref_t<Ts>> && ... && true) &&
        ((std::tuple_size_v<std::remove_cvref_t<T>> == std::tuple_size_v<std::remove_cvref_t<Ts>>) && ... && true)
    )
    struct is_tuple_matrix<std::tuple<T, Ts...>> : std::true_type {};
    // `T` が m 行 n 列 の行列様のタプル型であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_tuple_matrix_v = is_tuple_matrix<T>::value;

    // `T` が m 行 n 列の行列様のタプル型 `std::tuple<std::tuple<A1, A2, ..., An>, ..., std::tuple<M1, M2, ..., Mn>>` であるときに、
    // これを `std::tuple<std::tuple<A1, ..., M1>, std::tuple<A2, ..., M2>, ..., std::tuple<An, ..., Mn>>` に転置する。
    //
    // ```cpp
    // std::tuple {                                     std::tuple {                           
    //     std::tuple<A1, A2, ..., An>{ ... },              std::tuple<A1, B1, ..., M1>{ ... },
    //     std::tuple<A1, A2, ..., An>{ ... },              std::tuple<A2, B2, ..., M2>{ ... },
    //     ...                                    ==>                                          
    //     std::tuple<A1, A2, ..., An>{ ... },              std::tuple<An, Bn, ..., Mn>{ ... },
    // }                                                }
    // ```                              
    template<class T>
    requires (
        is_tuple_matrix_v<std::remove_cvref_t<T>>
    )
    constexpr auto transpose_tuple(T&& t)
    {
        constexpr std::size_t Row = std::tuple_size_v<std::remove_cvref_t<T>>;
        constexpr std::size_t Col = std::tuple_size_v<std::tuple_element_t<0, std::remove_cvref_t<T>>>;
        return [&]<std::size_t ...C>(std::index_sequence<C...>)
        {
            return std::make_tuple(
                [&]<std::size_t I, std::size_t ...R>(std::integral_constant<std::size_t, I>, std::index_sequence<R...>)
                {
                    return std::make_tuple(std::get<I>(std::get<R>(t))...);
                }(std::integral_constant<std::size_t, C>{}, std::make_index_sequence<Row>{})...
            );
        }(std::make_index_sequence<Col>{});
    }

    template<class T>
    requires (
        is_tuple_matrix_v<T>
    )
    constexpr auto transpose_tuple_forward(T& t)
    {
        constexpr std::size_t Row = std::tuple_size_v<std::remove_cvref_t<T>>;
        constexpr std::size_t Col = std::tuple_size_v<std::tuple_element_t<0, std::remove_cvref_t<T>>>;
        return [&]<std::size_t ...C>(std::index_sequence<C...>)
        {
            return std::make_tuple(
                [&]<std::size_t I, std::size_t ...R>(std::integral_constant<std::size_t, I>, std::index_sequence<R...>)
                {
                    return std::forward_as_tuple(std::get<I>(std::get<R>(t))...);
                }(std::integral_constant<std::size_t, C>{}, std::make_index_sequence<Row>{})...
            );
        }(std::make_index_sequence<Col>{});
    }


    // メタ関数 `Trait` の条件を満たすインデックスのみにフィルタした `std::index_sequence` を返す
    template <template <std::size_t I> class Trait, std::size_t ...Idx>
    requires (
        requires {
            { Trait<Idx>::value } -> std::convertible_to<bool>;
        } && ...
    )
    consteval auto filter_index_sequence(std::index_sequence<Idx...>)
    {
        return detail::filter_index_sequence_impl<Trait>(std::index_sequence<Idx...>(), std::index_sequence<>());
    }

    // tuple-like な型 `Tuple` の指定した要素に `Trait` を適用するメタ関数をメンバ型 `apply<std::size_t>` として定義する。
    template <class Tuple, template <class> class Trait>
    requires (
        col::tuple_like<std::remove_cvref_t<Tuple>>
    )
    struct make_tuple_applyer
    {
        template <std::size_t I>
        struct apply : public Trait<std::tuple_element_t<I, Tuple>>
        {};
    };

    template <template <class> class Trait, class Tuple>
    requires (
        col::tuple_like<std::remove_cvref_t<Tuple>>
    )
    constexpr auto filter_tuple(Tuple& tuple) noexcept
    {
        return []<class T, std::size_t ...Idx>(T& t, std::index_sequence<Idx...>) {
            return std::forward_as_tuple( std::get<Idx>(t) ... );
        }(tuple, filter_index_sequence<make_tuple_applyer<Tuple, Trait>::template apply>(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>()));
    }

    namespace detail {
        template <class T, class F, std::size_t Index, std::size_t ...Idx>
        requires (is_visitor_for_v<F, T> && ( std::tuple_size_v<std::remove_cvref_t<T>> > 0 || requires {
            is_control_flow_v<std::remove_cvref_t<std::invoke_result_t<F, std::tuple_element_t<0, T>>>>;  
        }))
        constexpr auto tuple_try_foreach_impl(T&& t, F& f)
        {
            if constexpr( sizeof...(Idx) > 0 )
            {
                const auto res = std::invoke(f, std::get<Index>(t));
                if( res.is_break() )
                {
                    return std::move(res);
                }
                return tuple_try_foreach_impl<T, F, Idx...>(t, f);
            }
            else
            {
                return std::invoke(f, std::get<Index>(t));
            }
        }
    } // namespace detail

    template <class F, class T>
    requires (is_visitor_for_v<F, T>)
    constexpr auto tuple_try_foreach(F&& f, T&& t)
    {
        auto&& fn = std::forward<F>(f);
        return [&]<std::size_t ...Idx>(std::index_sequence<Idx...>)
        {
            return detail::tuple_try_foreach_impl<T, F, Idx...>(t, fn);
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<T>>>{});
    }

    inline void tuple_try_foreach_static_test() {
        constexpr std::tuple<int, const char*> t{ 10, "" };
        constexpr auto res = tuple_try_foreach([]<class T>(const T&) -> ControlFlow<int>
        {
            if constexpr( std::integral<T> )
            {
                return Continue{};
            }
            else
            {
                return Break{-1};
            }
        }, t);
        static_assert(res.is_continue() == false);
        static_assert(res == Break{-1});
    }

    namespace detail {

        template <template <class> class Pred, class T, std::size_t Index, std::size_t ...Idx>
        consteval std::optional<std::size_t> find_first_index_of_tuple(std::index_sequence<Index, Idx...>)
        {
            if constexpr( Pred<std::tuple_element_t<Index, T>>::value )
            {
                return Index;
            }
            else if constexpr( sizeof...(Idx) > 0 )
            {
                return find_first_index_of_tuple<Pred, T>(std::index_sequence<Idx...>{});
            }
            else
            {
                return std::nullopt;
            }
        }

    } // namespace detail

    template <template <class> class Pred, class T>
    struct find_first_index_of_tuple
    {
        static constexpr std::optional<std::size_t> value = std::nullopt;
    };
    template <template <class> class Pred, class ...Args>
    requires (
        ( requires {
            Pred<Args>::value;
            { Pred<Args>::value } -> std::convertible_to<bool>;
        } && ... && true )
    )
    struct find_first_index_of_tuple<Pred, std::tuple<Args...>>
    {
        static constexpr std::optional<std::size_t> value = detail::find_first_index_of_tuple<Pred, std::tuple<Args...>>(std::index_sequence_for<Args...>{});
    };

    template <template <class> class Pred, class T>
    requires (requires {
        find_first_index_of_tuple<Pred, T>::value;
    })
    constexpr auto tuple_split_by(T&& tuple)
    {
        constexpr auto FoundIndex = find_first_index_of_tuple<Pred, T>::value;
        if constexpr( FoundIndex.has_value() )
        {
            constexpr std::size_t Index = *FoundIndex;
            using TupleBind = std::conditional_t<std::is_lvalue_reference_v<T>, T, std::remove_reference_t<T>>;
            TupleBind tu = std::forward<T>(tuple);
            return std::make_pair(
                [&]<std::size_t ...Idx>(std::index_sequence<Idx...>)
                {
                    return std::tuple{ std::get<Idx>(tu)... };
                }(std::make_index_sequence<Index>{}),
                [&]<std::size_t ...Idx>(std::index_sequence<Idx...>)
                {
                    return std::tuple{ std::get<Idx + Index>(tu)... };
                }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<T>> - Index>{})
            );
        }
        else
        {
            return std::forward<T>(tuple);
        }
    }

    namespace detail {
        template <std::size_t Begin, std::size_t End>
        requires (End >= Begin)
        constexpr auto make_index_range() noexcept
        {
            return []<std::size_t ...Idx>(std::index_sequence<Idx...>) static noexcept
                ->  std::index_sequence<(Begin + Idx)...>
            {
                return std::index_sequence<(Begin + Idx)...>{};
            }(std::make_index_sequence<End - Begin>{});
        }
    } // namespace detail

    template <std::size_t Begin, std::size_t End>
    requires (End >= Begin)
    using make_index_range = decltype(detail::make_index_range<Begin, End>());

} // namespace col
