#pragma once

#include <col/from_string.h>
#include <col/tuple.h>
#include <col/type_traits.h>

#include <cstddef>
#include <cstdint>

#include <concepts>
#include <expected>
#include <format>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <string_view>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>


namespace col {

    // 不明なエラー
    struct UnknownError
    {};

    // 内部ロジックエラーの種類
    enum class InternalLogicErrorKind : std::uint32_t
    {
        // 不正な関数戻り値
        InvalidFunctionReturnType,
    };

    // 内部ロジックエラー
    struct InternalLogicError
    {
        std::string_view name;
        InternalLogicErrorKind kind;
    };

    // `--help` が指定された
    struct ShowHelp
    {};

    // 不明なオプション
    struct UnknownOption
    {
        std::string_view arg;
    };

    // 同じオプションに 2 度以上引数が与えられた。
    struct DuplicateArg
    {
        std::string_view name;
    };

    // オプションに対して引数が与えられなかった。
    struct NoValueGivenForOption
    {
        std::string_view name;
    };

    // 数値として不正な文字列を受け取った。
    struct InvalidNumber
    {
        std::string_view name;
        std::string_view arg;
        std::errc err;
    };

    // 構造体にマッピングするには引数が足りない。
    struct NotEnoughArgument
    {
        std::size_t index;
        std::string_view name;
    };

    // 不正な設定の種類
    enum class InvalidConfigKind : std::uint32_t
    {
        // デフォルト値の設定方法が定まっていない。
        EmptyDefault,
        // パーサーの設定方法が定まっていない。
        EmptyParser,
    };

    // 不正な設定
    struct InvalidConfiguration
    {
        std::string_view name;
        InvalidConfigKind kind;
    };

    // 変換関数がエラーを返した
    struct ConverterConvertionError
    {
        std::string_view name;
        std::string_view arg;
    };

    // デフォルト値の生成時にエラーが発生した
    struct DefaultGenerationError
    {
        std::string_view name;
    };

    // 必須のオプション
    struct RequiredOption
    {
        std::string_view name;
    };

    // パーサーが返すエラー
    using ParseError =
        std::variant<
            UnknownError,
            InternalLogicError,
            UnknownOption,
            ShowHelp,
            DuplicateArg,
            NoValueGivenForOption,
            ConverterConvertionError,
            DefaultGenerationError,
            InvalidNumber,
            NotEnoughArgument,
            InvalidConfiguration,
            RequiredOption
        >;
} // namespace col


// std::formatter の col::ParseError に対する特殊化

template <>
struct std::formatter<col::UnknownError> : std::formatter<const char*>
{
    auto format(const col::UnknownError&, std::format_context& ctx) const noexcept
    {
        return std::formatter<const char*>::format("unknown error", ctx);
    }
};

template <>
struct std::formatter<col::InternalLogicErrorKind> : std::formatter<const char*>
{
    static constexpr const char* kind_string[] = {
        "InvalidFunctionReturnType",
    };

    auto format(const col::InternalLogicErrorKind& kind, std::format_context& ctx) const noexcept
    {
        return std::formatter<const char*>::format(
            kind_string[static_cast<std::underlying_type_t<col::InternalLogicErrorKind>>(kind)], ctx);
    }
};

template <>
struct std::formatter<col::InternalLogicError>
{
    constexpr auto parse(std::format_parse_context& ctx) const noexcept
    {
        return ctx.begin();
    }
    auto format(const col::InternalLogicError& err, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(),
            "internal logic error: name='{}' kind='{}'", err.name, err.kind);
    }
};

template <>
struct std::formatter<col::UnknownOption>
{
    constexpr auto parse(std::format_parse_context& ctx) const noexcept
    {
        return ctx.begin();
    }
    auto format(const col::UnknownOption& err, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(), "unknown option: arg='{}'", err.arg);
    }
};

template <>
struct std::formatter<col::ShowHelp> : std::formatter<const char*>
{
    auto format(const col::ShowHelp&, std::format_context& ctx) const noexcept
    {
        return std::formatter<const char*>::format("show help", ctx);
    }
};


template <>
struct std::formatter<col::DuplicateArg>
{
    constexpr auto parse(std::format_parse_context& ctx) const noexcept
    {
        return ctx.begin();
    }
    auto format(const col::DuplicateArg& err, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(), "duplicate argument for name='{}'", err.name);
    }
};

template <>
struct std::formatter<col::NoValueGivenForOption>
{
    constexpr auto parse(std::format_parse_context& ctx) const noexcept
    {
        return ctx.begin();
    }
    auto format(const col::NoValueGivenForOption& err, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(), "no value for option name='{}'", err.name);
    }
};

template <>
struct std::formatter<col::InvalidNumber>
{
    constexpr auto parse(std::format_parse_context& ctx) const noexcept
    {
        return ctx.begin();
    }
    auto format(const col::InvalidNumber& err, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(),
            "invalid numver: option='{}' arg='{}' errc='{}'",
            err.name, err.arg, static_cast<std::underlying_type_t<decltype(err.err)>>(err.err));
    }
};

template <>
struct std::formatter<col::NotEnoughArgument>
{
    constexpr auto parse(std::format_parse_context& ctx) const noexcept
    {
        return ctx.begin();
    }
    auto format(const col::NotEnoughArgument& err, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(),
            "not enough argument: index='{}' name='{}'", err.index, err.name);
    }
};


template <>
struct std::formatter<col::InvalidConfigKind> : std::formatter<const char*>
{
    static constexpr const char* kind_string[] = {
        "EmptyDefault",
        "EmptyParser",
    };

    auto format(const col::InvalidConfigKind& kind, std::format_context& ctx) const noexcept
    {
        return std::formatter<const char*>::format(
            kind_string[static_cast<std::underlying_type_t<col::InvalidConfigKind>>(kind)], ctx);
    }
};

template <>
struct std::formatter<col::InvalidConfiguration>
{
    constexpr auto parse(std::format_parse_context& ctx) const noexcept
    {
        return ctx.begin();
    }
    auto format(const col::InvalidConfiguration& err, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(),
            "not enough argument: name='{}' kind='{}'", err.name, err.kind);
    }
};

template <>
struct std::formatter<col::ConverterConvertionError>
{
    constexpr auto parse(std::format_parse_context& ctx) const noexcept
    {
        return ctx.begin();
    }
    auto format(const col::ConverterConvertionError& err, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(),
            "callback failed to convert argument: name='{}' arg='{}'", err.name, err.arg);
    }
};

template <>
struct std::formatter<col::DefaultGenerationError>
{
    constexpr auto parse(std::format_parse_context& ctx) const noexcept
    {
        return ctx.begin();
    }
    auto format(const col::DefaultGenerationError& err, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(),
            "generator failed for default value: name='{}'", err.name);
    }
};

template <>
struct std::formatter<col::RequiredOption>
{
    constexpr auto parse(std::format_parse_context& ctx) const noexcept
    {
        return ctx.begin();
    }
    auto format(const col::RequiredOption& err, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(),
            "required option was not given: name='{}'", err.name);
    }
};

namespace col {

    struct blank
    {
        constexpr blank() noexcept = default;
    };


    template <class T = blank, class D = blank, class P = blank>
    class Arg
    {
        static_assert(std::is_object_v<T>);
        static_assert(std::is_object_v<D>);
        static_assert(std::is_object_v<P>);

        const std::string_view m_name;

        D m_default;
        P m_parser;
    public:
        using value_type = T;
        using default_type = D;
        using parser_type = P;

        constexpr explicit Arg(std::string_view name) noexcept
            requires (std::is_default_constructible_v<D> && std::is_default_constructible_v<P>)
        : m_name{ name }
        , m_default{}
        , m_parser{}
        {}

        template <class De, class Pr>
        requires (std::is_object_v<De> && std::is_object_v<Pr>)
        constexpr explicit Arg(std::string_view name, De&& de, Pr&& p) noexcept
        : m_name{ name }
        , m_default{ std::forward<De>(de) }
        , m_parser{ std::forward<Pr>(p) }
        {}

        constexpr std::string_view get_name() const noexcept
        {
            return m_name;
        }
        constexpr const D& get_default() const noexcept
        {
            return m_default;
        }
        constexpr const P& get_parser() const noexcept
        {
            return m_parser;
        }

        template <class ValueT>
        constexpr Arg<ValueT, D, P> set_value_type() &&
        {
            static_assert(!std::same_as<ValueT, blank>);
            return Arg<ValueT, D, P>{
                m_name,
                std::move(m_default),
                std::move(m_parser)
            };
        }

        template <class De>
        requires (
            std::is_object_v<std::decay_t<De>>
            // TODO: std::invocable<De> を満たすときの制約を追加する
        )
        constexpr auto set_default(De&& de) &&
        {
            static_assert(std::same_as<D, blank>, "D must be blank");

            using ValueT = std::conditional_t<
                !std::same_as<T, blank>,
                T,
                std::common_type_t<
                    std::decay_t<De>,
                    col::conditional_type_t<
                        std::invocable<P, const char*>,
                        col::unwrap_ok_type_if_t<std::invoke_result<P, const char*>>,
                        std::type_identity<std::decay_t<De>>
                    >
                >
            >;
            return Arg<ValueT, std::decay_t<De>, P>{
                m_name,
                std::forward<std::decay_t<De>>(de),
                std::move(m_parser)
            };
        }

        template <class Pr>
        requires (std::is_object_v<std::decay_t<Pr>> && requires {
            std::invocable<std::decay_t<Pr>, const char*>;
            // TODO: T が指定されている・いないときの Pr の制約を追加する
            !col::is_std_expected_v<std::remove_cvref_t<col::unwrap_ok_type_if_t<std::invoke_result_t<std::decay_t<Pr>, const char*>>>>
                || std::convertible_to<std::unexpected<col::ParseError>, std::remove_cvref_t<col::unwrap_ok_type_if_t<std::invoke_result_t<std::decay_t<Pr>, const char*>>>>;
                // TODO: P が指定されてなくても指定されていても T を生成できることを保証する
            })
        constexpr auto set_parser(Pr&& p) &&
        {
            static_assert(std::same_as<P, blank>, "P must be blank");

            using ValueT = std::conditional_t<
                !std::same_as<T, blank>,
                T,
                std::common_type_t<
                    col::unwrap_ok_type_if_t<std::invoke_result_t<std::decay_t<Pr>, const char*>>,
                    std::conditional_t<
                        !std::same_as<D, blank>,
                        D,
                        col::unwrap_ok_type_if_t<std::invoke_result_t<std::decay_t<Pr>, const char*>>
                    >
                >
            >;
            return Arg<ValueT, D, std::decay_t<Pr>>{
                m_name,
                std::move(m_default),
                std::forward<std::decay_t<Pr>>(p)
            };
        }

        template <class I, class S>
        requires (
            std::sentinel_for<S, I> &&
            std::convertible_to<col::iter_const_reference_t<I>, std::string_view>
        )
        constexpr std::expected<T, col::ParseError> parse(I& iter, const S& s) const
        {
            if constexpr( std::same_as<T, bool> )
            {
                if constexpr( std::same_as<D, bool> )
                {
                    return !m_default;
                }
                else
                {
                    return true;
                }
            }
            else
            {
                if( iter == s )
                {
                    return std::unexpected{
                        col::NoValueGivenForOption {
                            .name = m_name,
                        }
                    };
                }
                const std::string_view a{ *iter };
                std::ranges::advance(iter, 1);

                if constexpr( std::invocable<P, const char*> )
                {
                    const auto res = std::invoke(m_parser, a.data());
                    using R = std::remove_cvref_t<decltype(res)>;
                    if constexpr( std::same_as<R, T> )
                    {
                        return std::move(res);
                    }
                    else if constexpr( col::is_std_optional_v<R> )
                    {
                        if( res.has_value() )
                        {
                            return std::move(*res);
                        }
                        else
                        {
                            return std::unexpected{
                                col::ConverterConvertionError{
                                    .name = m_name,
                                    .arg = a,
                                }
                            };
                        }
                    }
                    else if constexpr( col::is_std_expected_v<R> )
                    {
                        if( res.has_value() )
                        {
                            return std::move(*res);
                        }
                        else
                        {
                            return std::unexpected{
                                std::move(res.error())
                            };
                        }
                    }
                }
                else if constexpr( std::convertible_to<const char*, T> )
                {
                    return static_cast<T>(a.data());
                }
                else if constexpr( std::is_integral_v<T> || std::is_floating_point_v<T> )
                {
                    const auto res = col::number_from_string<T>(a);
                    if( res.has_value() )
                    {
                        return *res;
                    }
                    else
                    {
                        return std::unexpected{
                            col::InvalidNumber{
                                .name = m_name,
                                .arg = a,
                                .err = std::move(res).error().ec,
                            }
                        };
                    }
                }
                else
                {
                    static_assert(false, "invalid parser type");
                }
            }
        }
    };

    template <class T>
    requires (std::convertible_to<T, std::string_view>)
    Arg(T) -> Arg<blank, blank, blank>;


    template <class T>
    struct is_col_arg : std::false_type {};
    template <class T, class D, class P>
    struct is_col_arg<Arg<T, D, P>> : std::true_type {};
    template <class T>
    inline constexpr auto is_col_arg_v = is_col_arg<T>::value;

    template <class M, class ...Args>
    class SubCmd;

    template <class T>
    struct is_col_subcmd : std::false_type {};
    template <class M, class ...Args>
    struct is_col_subcmd<SubCmd<M, Args...>> : std::true_type {};
    template <class T>
    inline constexpr auto is_col_subcmd_v = is_col_subcmd<T>::value;


    template <class M, class ...Args>
    class SubCmd
    {
        static_assert(((is_col_arg_v<Args> || is_col_subcmd_v<Args>) && ... && true));

        const std::string_view m_name;
        std::tuple<Args...> m_args;
    public:
        using value_type = M;

        constexpr explicit SubCmd(std::string_view name) noexcept
            requires (sizeof...(Args) == 0)
        : m_name{ name }
        , m_args{}
        {}

        constexpr explicit SubCmd(std::string_view name, std::tuple<Args...>&& args)
            noexcept((std::is_nothrow_move_constructible_v<Args> && ...))
        : m_name{ name }
        , m_args{ std::move(args) }
        {}

        constexpr std::string_view get_name() const noexcept
        {
            return m_name;
        }

        template <class T, class D, class P>
        constexpr auto add(Arg<T, D, P>&& arg) &&
            noexcept(std::is_nothrow_move_constructible_v<std::remove_cvref_t<decltype(arg)>>)
        {
            if constexpr( std::same_as<T, blank> )
            {
                return SubCmd<M, Args..., Arg<bool, D, P>>{
                    m_name,
                    std::tuple_cat(std::move(m_args), std::tuple{ std::move(arg).template set_value_type<bool>() } )
                };
            }
            else
            {
                return SubCmd<M, Args..., Arg<T, D, P>>{
                    m_name,
                    std::tuple_cat(std::move(m_args), std::tuple{ std::move(arg) } )
                };
            }
        }

        template <class MapT, class ...ArgTypes>
        constexpr SubCmd<M, std::variant<SubCmd<MapT, ArgTypes...>>, Args...> add(SubCmd<MapT, ArgTypes...>&& subcmd) &&
            requires (!is_col_subcmd_v<Args> && ... && true )
        {
            return SubCmd<M, std::variant<SubCmd<MapT, ArgTypes...>>, Args...>{
                m_name,
                std::tuple{ std::move(subcmd) },
                std::move(m_args)
            };
        }


        template <class I, class S>
        requires (
            std::sentinel_for<S, I> &&
            std::convertible_to<col::iter_const_reference_t<I>, std::string_view>
        )
        constexpr std::expected<M, col::ParseError> parse(I& iter, const S& sentinel) const
            noexcept(std::is_nothrow_constructible_v<M, typename Args::value_type...>)
            requires (std::is_constructible_v<M, typename Args::value_type...>)
        {
            std::tuple<std::optional<typename Args::value_type>...> init{};
            auto zipped = col::zip_tuples(m_args, init);

            while( iter != sentinel )
            {
                const auto res = col::tuple_try_foreach(
                    [&]<class ValueT, class DefaultT, class ParserT>
                        (std::tuple<const Arg<ValueT, DefaultT, ParserT>&, std::optional<ValueT>&>& elem)
                        -> col::ControlFlow<std::optional<col::ParseError>>
                    {
                        const Arg<ValueT, DefaultT, ParserT>& config = std::get<0>(elem);
                        const std::string_view a{ *iter };
                        if( a != config.get_name() )
                        {
                            return col::Continue{};
                        }
                        std::ranges::advance(iter, 1);
                        const auto parse_res = config.parse(iter, sentinel);
                        if( parse_res.has_value() )
                        {
                            std::optional<ValueT>& value = std::get<1>(elem);
                            value.emplace(std::move(*parse_res));
                            return col::Break{ std::nullopt };
                        }
                        else
                        {
                            return col::Break{
                                std::move(parse_res).error()
                            };
                        }
                    }, zipped);
                if( const auto& brk = res.to_break(); brk.has_value() )
                {
                    const auto e = *std::move(res).to_break();
                    return std::unexpected{
                        std::move(e)
                    };
                }
                else if( res.is_continue() )
                {
                    return std::unexpected{
                        col::UnknownOption{
                            .arg = std::string_view{ *iter },
                        }
                    };
                }
            }
            
            const auto default_init_res = col::tuple_try_foreach(
                [&]<class ValueT, class De, class Pr>(std::tuple<const Arg<ValueT, De, Pr>&, std::optional<ValueT>&>& elem)
                    -> col::ControlFlow<col::ParseError>
                {
                    std::optional<ValueT>& value = std::get<1>(elem);
                    if( value.has_value() )
                    {
                        return col::Continue{};
                    }
                    const Arg<ValueT, De, Pr>& config = std::get<0>(elem);
                    if constexpr( std::same_as<De, blank> )
                    {
                        if constexpr( std::is_default_constructible_v<std::remove_cvref_t<ValueT>> )
                        {
                            value.emplace();
                            return col::Continue{};
                        }
                        else
                        {
                            return col::Break{
                                col::InvalidConfiguration{
                                    .name = config.get_name(),
                                    .kind = col::InvalidConfigKind::EmptyDefault,
                                }
                            };
                        }
                    }
                    else
                    {
                        if constexpr( std::invocable<De> )
                        {
                            const auto invk_res = std::invoke(config.get_default());
                            using R = std::remove_cvref_t<decltype(invk_res)>;
                            if constexpr( std::convertible_to<R, ValueT> )
                            {
                                value.emplace(std::move(invk_res));
                                return col::Continue{};
                            }
                            else if constexpr( col::is_std_optional_v<R> )
                            {
                                if( invk_res.has_value() )
                                {
                                    value.emplace(std::move(*invk_res));
                                    return col::Continue{};
                                }
                                else
                                {
                                    return col::Break{
                                        col::DefaultGenerationError {
                                            .name = config.get_name(),
                                        }
                                    };
                                }
                            }
                            else if constexpr( col::is_std_expected_v<R> )
                            {
                                if( invk_res.has_value() )
                                {
                                    value.emplace(std::move(*invk_res));
                                    return col::Continue{};
                                }
                                else
                                {
                                    if constexpr( std::convertible_to<typename R::error_type, col::ParseError> )
                                    {
                                        return col::Break{
                                            std::unexpected{
                                                std::move(invk_res).error()
                                            }
                                        };
                                    }
                                    else
                                    {
                                        return col::Break{
                                            col::DefaultGenerationError {
                                                .name = config.get_name(),
                                            }
                                        };
                                    }
                                }
                            }
                            else
                            {
                                return col::Break{
                                    col::InternalLogicError{
                                        .name = config.get_name(),
                                        .kind = col::InternalLogicErrorKind::InvalidFunctionReturnType,
                                    }
                                };
                            }
                        }
                        else
                        {
                            value.emplace(config.get_default());
                            return col::Continue{};
                        }
                    }
                },
                zipped);
            if( default_init_res.is_break() )
            {
                return std::unexpected{
                    std::move(default_init_res).to_break()
                };
            }

            return std::apply([]<class ...Ts>(Ts&& ...ts) static
                {
                    return M{ (*std::forward<Ts>(ts))... };
                }, std::move(init));
        }
    };


    template <class M, class ...SubArgs, class ...Args>
    requires (is_col_subcmd_v<SubArgs> && ...)
    class SubCmd<M, std::variant<SubArgs...>, Args...>
    {
        static_assert(((is_col_arg_v<Args> || is_col_subcmd_v<Args>) && ... && true));

        const std::string_view m_name;
        std::tuple<SubArgs...> m_subs;
        std::tuple<Args...> m_args;
    public:
        using value_type = M;

        constexpr explicit SubCmd(std::string_view name, std::tuple<SubArgs...>&& subs, std::tuple<Args...>&& args)
            noexcept(
                std::is_nothrow_move_constructible_v<std::tuple<SubArgs...>>
                && std::is_nothrow_move_constructible_v<std::tuple<Args...>>)
        : m_name{ name }
        , m_subs{ std::move(subs) }
        , m_args{ std::move(args) }
        {}

        constexpr std::string_view get_name() const noexcept
        {
            return m_name;
        }

        template <class T, class D, class P>
        constexpr auto add(Arg<T, D, P>&& arg) &&
            noexcept(std::is_nothrow_move_constructible_v<std::remove_cvref_t<decltype(arg)>>)
        {
            if constexpr( std::same_as<T, blank> )
            {
                return SubCmd<M, std::variant<SubArgs...>, Args..., Arg<bool, D, P>>{
                    m_name,
                    std::move(m_subs),
                    std::tuple_cat(std::move(m_args), std::tuple{ std::move(arg).template set_value_type<bool>() } )
                };
            }
            else
            {
                return SubCmd<M, std::variant<SubArgs...>, Args..., Arg<T, D, P>>{
                    m_name,
                    std::move(m_subs),
                    std::tuple_cat(std::move(m_args), std::tuple{ std::move(arg) } )
                };
            }
        }

        template <class MapT, class ...ArgTypes>
        constexpr SubCmd<M, std::variant<SubArgs..., SubCmd<MapT, ArgTypes...>>, Args...> add(SubCmd<MapT, ArgTypes...>&& subcmd) &&
            requires (!is_col_subcmd_v<Args> && ... && true )
        {
            return SubCmd<M, std::variant<SubArgs..., SubCmd<MapT, ArgTypes...>>, Args...>{
                m_name,
                std::tuple_cat(
                    std::move(m_subs),
                    std::tuple{ std::move(subcmd) }
                ),
                std::move(m_args)
            };
        }


        template <class I, class S>
        requires (
            std::sentinel_for<S, I> &&
            std::convertible_to<col::iter_const_reference_t<I>, std::string_view>
        )
        constexpr std::expected<M, col::ParseError> parse(I& iter, const S& sentinel) const
            noexcept(std::is_nothrow_constructible_v<M, std::variant<std::monostate, typename SubArgs::value_type...>, typename Args::value_type...>)
            requires (std::is_constructible_v<M, std::variant<std::monostate, typename SubArgs::value_type...>, typename Args::value_type...>)
        {
            std::optional<std::variant<std::monostate, typename SubArgs::value_type...>> subsubcmd{};
            std::tuple<std::optional<typename Args::value_type>...> init{};
            auto zipped = col::zip_tuples(m_args, init);

            while( iter != sentinel )
            {
                const auto subsub_res = col::tuple_try_foreach(
                    [&]<class SubCmdCfg>(const SubCmdCfg& cfg)
                        -> col::ControlFlow<std::optional<col::ParseError>>
                    {
                        const std::string_view a{ *iter };
                        if( a != cfg.get_name() )
                        {
                            return col::Continue{};
                        }
                        std::ranges::advance(iter, 1);
                        const auto res = cfg.parse(iter, sentinel);
                        if( res.has_value() )
                        {
                            subsubcmd.emplace(std::move(*res));
                            return col::Break{ std::nullopt };
                        }
                        else
                        {
                            return col::Break{
                                std::move(res).error()
                            };
                        }
                    },
                    m_subs);
                if( subsub_res.is_break() )
                {
                    if( subsub_res.to_break().has_value() )
                    {
                        return std::unexpected{
                            *std::move(subsub_res).to_break()
                        };
                    }
                    else
                    {
                        // col::Break{std::nullopt} のとき、サブサブコマンドがパース成功したことを意味する。
                        // 残りの引数を続けてこのサブコマンドの引数として解釈できるが直感的ではないので、
                        // ここで break して残りの引数はエラーとする。
                        break;
                    }
                }

                const auto res = col::tuple_try_foreach(
                    [&]<class ValueT, class DefaultT, class ParserT>
                        (std::tuple<const Arg<ValueT, DefaultT, ParserT>&, std::optional<ValueT>&>& elem)
                        -> col::ControlFlow<std::optional<col::ParseError>>
                    {
                        const Arg<ValueT, DefaultT, ParserT>& config = std::get<0>(elem);
                        const std::string_view a{ *iter };
                        if( a != config.get_name() )
                        {
                            return col::Continue{};
                        }
                        std::ranges::advance(iter, 1);
                        const auto parse_res = config.parse(iter, sentinel);
                        if( parse_res.has_value() )
                        {
                            std::optional<ValueT>& value = std::get<1>(elem);
                            value.emplace(std::move(*parse_res));
                            return col::Break{ std::nullopt };
                        }
                        else
                        {
                            return col::Break{
                                std::move(std::move(parse_res).error())
                            };
                        }
                    }, zipped);
                if( res.is_break() )
                {
                    const auto brk = std::move(res).to_break();
                    if( brk.has_value() )
                    {
                        return std::unexpected{
                            std::move(*brk)
                        };
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    // どのサブサブコマンドでもオプションでもない
                    return std::unexpected{
                        col::UnknownOption{
                            .arg = *iter,
                        }
                    };
                }

            }

            if( iter != sentinel )
            {
                return std::unexpected{
                    col::UnknownOption{
                        .arg = *iter,
                    }
                };
            }

            if( !subsubcmd.has_value() )
            {
                subsubcmd.emplace(std::in_place_index<0>, std::monostate{});
            }

            const auto default_init_res = col::tuple_try_foreach(
                [&]<class ValueT, class De, class Pr>(std::tuple<const Arg<ValueT, De, Pr>&, std::optional<ValueT>&>& elem)
                    -> col::ControlFlow<col::ParseError>
                {
                    std::optional<ValueT>& value = std::get<1>(elem);
                    if( value.has_value() )
                    {
                        return col::Continue{};
                    }
                    const Arg<ValueT, De, Pr>& config = std::get<0>(elem);
                    if constexpr( std::same_as<De, blank> )
                    {
                        if constexpr( std::is_default_constructible_v<std::remove_cvref_t<ValueT>> )
                        {
                            value.emplace();
                            return col::Continue{};
                        }
                        else
                        {
                            return col::Break{
                                col::InvalidConfiguration{
                                    .name = config.get_name(),
                                    .kind = col::InvalidConfigKind::EmptyDefault,
                                }
                            };
                        }
                    }
                    else
                    {
                        if constexpr( std::invocable<De> )
                        {
                            const auto invk_res = std::invoke(config.get_default());
                            using R = std::remove_cvref_t<decltype(invk_res)>;
                            if constexpr( std::convertible_to<R, ValueT> )
                            {
                                value.emplace(std::move(invk_res));
                                return col::Continue{};
                            }
                            else if constexpr( col::is_std_optional_v<R> )
                            {
                                if( invk_res.has_value() )
                                {
                                    value.emplace(std::move(*invk_res));
                                    return col::Continue{};
                                }
                                else
                                {
                                    return col::Break{
                                        col::DefaultGenerationError {
                                            .name = config.get_name(),
                                        }
                                    };
                                }
                            }
                            else if constexpr( col::is_std_expected_v<R> )
                            {
                                if( invk_res.has_value() )
                                {
                                    value.emplace(std::move(*invk_res));
                                    return col::Continue{};
                                }
                                else
                                {
                                    if constexpr( std::convertible_to<typename R::error_type, col::ParseError> )
                                    {
                                        return col::Break{
                                            std::unexpected{
                                                std::move(invk_res).error()
                                            }
                                        };
                                    }
                                    else
                                    {
                                        return col::Break{
                                            col::DefaultGenerationError {
                                                .name = config.get_name(),
                                            }
                                        };
                                    }
                                }
                            }
                            else
                            {
                                return col::Break{
                                    col::InternalLogicError{
                                        .name = config.get_name(),
                                        .kind = col::InternalLogicErrorKind::InvalidFunctionReturnType,
                                    }
                                };
                            }
                        }
                        else
                        {
                            value.emplace(config.get_default());
                            return col::Continue{};
                        }
                    }
                },
                zipped);
            if( default_init_res.is_break() )
            {
                return std::unexpected{
                    std::move(default_init_res).to_break()
                };
            }

            return std::apply([&]<class ...Ts>(Ts&& ...ts)
                {
                    return M{ std::move(*subsubcmd), (*std::forward<Ts>(ts))... };
                }, std::move(init));
        }
    };


    template <class ...Args>
    class Cmd
    {
        static_assert(((is_col_arg_v<Args> || is_col_subcmd_v<Args>) && ... && true));

        const std::string_view m_name;
        std::tuple<Args...> m_args;
    public:

        constexpr explicit Cmd(std::string_view name) noexcept
            requires (sizeof...(Args) == 0)
        : m_name{ name }
        , m_args{}
        {}

        constexpr explicit Cmd(std::string_view name, std::tuple<Args...>&& args)
            noexcept((std::is_nothrow_move_constructible_v<Args> && ...))
        : m_name{ name }
        , m_args{ std::move(args) }
        {}

        constexpr std::string_view get_name() const noexcept
        {
            return m_name;
        }

        template <class T, class D, class P>
        constexpr auto add(Arg<T, D, P>&& arg) &&
            noexcept(std::is_nothrow_move_constructible_v<std::remove_cvref_t<decltype(arg)>>)
        {
            if constexpr( std::same_as<T, blank> )
            {
                return Cmd<Args..., Arg<bool, D, P>>{
                    m_name,
                    std::tuple_cat(std::move(m_args), std::tuple{ std::move(arg).template set_value_type<bool>() } )
                };
            }
            else
            {
                return Cmd<Args..., Arg<T, D, P>>{
                    m_name,
                    std::tuple_cat(std::move(m_args), std::tuple{ std::move(arg) } )
                };
            }
        }

        template <class MapT, class ...ArgTypes>
        constexpr Cmd<std::variant<SubCmd<MapT, ArgTypes...>>, Args...> add(SubCmd<MapT, ArgTypes...>&& subcmd) &&
            requires (!is_col_subcmd_v<Args> && ... && true )
        {
            return Cmd<std::variant<SubCmd<MapT, ArgTypes...>>, Args...>{
                m_name,
                std::tuple{ std::move(subcmd) },
                std::move(m_args)
            };
        }

        template <class T, class R>
        requires (
            std::ranges::viewable_range<R> &&
            std::convertible_to<col::range_const_reference_t<R>, std::string_view>
        )
        constexpr std::expected<T, col::ParseError> parse(R r) const
            noexcept(std::is_nothrow_constructible_v<T, typename Args::value_type...>)
            requires (std::is_constructible_v<T, typename Args::value_type...>)
        {
            auto v = std::ranges::views::all(r);
            auto iter = std::ranges::cbegin(v);
            const auto sentinel = std::ranges::cend(v);
            return parse<T>(iter, sentinel);
        }

        template <class T, class I, class S>
        requires (
            std::sentinel_for<S, I> &&
            std::convertible_to<col::iter_const_reference_t<I>, std::string_view>
        )
        constexpr std::expected<T, col::ParseError> parse(I& iter, const S& sentinel) const
            noexcept(std::is_nothrow_constructible_v<T, typename Args::value_type...>)
            requires (std::is_constructible_v<T, typename Args::value_type...>)
        {
            std::tuple<std::optional<typename Args::value_type>...> init{};
            auto zipped = col::zip_tuples(m_args, init);

            while( iter != sentinel )
            {
                const auto res = col::tuple_try_foreach(
                    [&]<class ValueT, class DefaultT, class ParserT>
                        (std::tuple<const Arg<ValueT, DefaultT, ParserT>&, std::optional<ValueT>&>& elem)
                        -> col::ControlFlow<std::optional<col::ParseError>>
                    {
                        const Arg<ValueT, DefaultT, ParserT>& config = std::get<0>(elem);
                        const std::string_view a{ *iter };
                        if( a != config.get_name() )
                        {
                            return col::Continue{};
                        }
                        std::ranges::advance(iter, 1);
                        const auto parse_res = config.parse(iter, sentinel);
                        if( parse_res.has_value() )
                        {
                            std::optional<ValueT>& value = std::get<1>(elem);
                            value.emplace(std::move(*parse_res));
                            return col::Break{ std::nullopt };
                        }
                        else
                        {
                            return col::Break{
                                std::move(parse_res).error()
                            };
                        }
                    }, zipped);
                if( const auto& brk = res.to_break(); brk.has_value() )
                {
                    const auto e = *std::move(res).to_break();
                    return std::unexpected{
                        std::move(e)
                    };
                }
                else if( res.is_continue() )
                {
                    return std::unexpected{
                        col::UnknownOption{
                            .arg = std::string_view{ *iter },
                        }
                    };
                }
            }

            if( iter != sentinel )
            {
                return std::unexpected{
                    col::UnknownOption{
                        .arg = *iter,
                    }
                };
            }
            
            const auto default_init_res = col::tuple_try_foreach(
                [&]<class ValueT, class De, class Pr>(std::tuple<const Arg<ValueT, De, Pr>&, std::optional<ValueT>&>& elem)
                    -> col::ControlFlow<col::ParseError>
                {
                    std::optional<ValueT>& value = std::get<1>(elem);
                    if( value.has_value() )
                    {
                        return col::Continue{};
                    }
                    const Arg<ValueT, De, Pr>& config = std::get<0>(elem);
                    if constexpr( std::same_as<De, blank> )
                    {
                        if constexpr( std::is_default_constructible_v<std::remove_cvref_t<ValueT>> )
                        {
                            value.emplace();
                            return col::Continue{};
                        }
                        else
                        {
                            return col::Break{
                                col::InvalidConfiguration{
                                    .name = config.get_name(),
                                    .kind = col::InvalidConfigKind::EmptyDefault,
                                }
                            };
                        }
                    }
                    else
                    {
                        if constexpr( std::invocable<De> )
                        {
                            const auto invk_res = std::invoke(config.get_default());
                            using R = std::remove_cvref_t<decltype(invk_res)>;
                            if constexpr( std::convertible_to<R, ValueT> )
                            {
                                value.emplace(std::move(invk_res));
                                return col::Continue{};
                            }
                            else if constexpr( col::is_std_optional_v<R> )
                            {
                                if( invk_res.has_value() )
                                {
                                    value.emplace(std::move(*invk_res));
                                    return col::Continue{};
                                }
                                else
                                {
                                    return col::Break{
                                        col::DefaultGenerationError {
                                            .name = config.get_name(),
                                        }
                                    };
                                }
                            }
                            else if constexpr( col::is_std_expected_v<R> )
                            {
                                if( invk_res.has_value() )
                                {
                                    value.emplace(std::move(*invk_res));
                                    return col::Continue{};
                                }
                                else
                                {
                                    if constexpr( std::convertible_to<typename R::error_type, col::ParseError> )
                                    {
                                        return col::Break{
                                            std::unexpected{
                                                std::move(invk_res).error()
                                            }
                                        };
                                    }
                                    else
                                    {
                                        return col::Break{
                                            col::DefaultGenerationError {
                                                .name = config.get_name(),
                                            }
                                        };
                                    }
                                }
                            }
                            else
                            {
                                return col::Break{
                                    col::InternalLogicError{
                                        .name = config.get_name(),
                                        .kind = col::InternalLogicErrorKind::InvalidFunctionReturnType,
                                    }
                                };
                            }
                        }
                        else
                        {
                            value.emplace(config.get_default());
                            return col::Continue{};
                        }
                    }
                },
                zipped);
            if( default_init_res.is_break() )
            {
                return std::unexpected{
                    std::move(default_init_res).to_break()
                };
            }

            return std::apply([]<class ...Ts>(Ts&& ...ts) static
                {
                    return T{ (*std::forward<Ts>(ts))... };
                }, std::move(init));
        }
    };
    
    template <class ...SubArgs, class ...Args>
    requires (is_col_subcmd_v<SubArgs> && ...)
    class Cmd<std::variant<SubArgs...>, Args...>
    {
        static_assert(((is_col_arg_v<Args> || is_col_subcmd_v<Args>) && ... && true));

        const std::string_view m_name;
        std::tuple<SubArgs...> m_subs;
        std::tuple<Args...> m_args;
    public:

        constexpr explicit Cmd(std::string_view name, std::tuple<SubArgs...>&& subs, std::tuple<Args...>&& args)
            noexcept(
                std::is_nothrow_move_constructible_v<std::tuple<SubArgs...>>
                && std::is_nothrow_move_constructible_v<std::tuple<Args...>>)
        : m_name{ name }
        , m_subs{ std::move(subs) }
        , m_args{ std::move(args) }
        {}

        constexpr std::string_view get_name() const noexcept
        {
            return m_name;
        }

        template <class T, class D, class P>
        constexpr auto add(Arg<T, D, P>&& arg) &&
            noexcept(std::is_nothrow_move_constructible_v<std::remove_cvref_t<decltype(arg)>>)
        {
            if constexpr( std::same_as<T, blank> )
            {
                return Cmd<std::variant<SubArgs...>, Args..., Arg<bool, D, P>>{
                    m_name,
                    std::move(m_subs),
                    std::tuple_cat(std::move(m_args), std::tuple{ std::move(arg).template set_value_type<bool>() } )
                };
            }
            else
            {
                return Cmd<std::variant<SubArgs...>, Args..., Arg<T, D, P>>{
                    m_name,
                    std::move(m_subs),
                    std::tuple_cat(std::move(m_args), std::tuple{ std::move(arg) } )
                };
            }
        }

        template <class MapT, class ...ArgTypes>
        constexpr Cmd<std::variant<SubArgs..., SubCmd<MapT, ArgTypes...>>, Args...> add(SubCmd<MapT, ArgTypes...>&& subcmd) &&
            requires (!is_col_subcmd_v<Args> && ... && true )
        {
            return Cmd<std::variant<SubArgs..., SubCmd<MapT, ArgTypes...>>, Args...>{
                m_name,
                std::tuple_cat(
                    std::move(m_subs),
                    std::tuple{ std::move(subcmd) }
                ),
                std::move(m_args)
            };
        }

        template <class T, class R>
        requires (
            std::ranges::viewable_range<R> &&
            std::convertible_to<col::range_const_reference_t<R>, std::string_view>
        )
        constexpr std::expected<T, col::ParseError> parse(R r) const
            noexcept(std::is_nothrow_constructible_v<T, std::variant<std::monostate, typename SubArgs::value_type...>, typename Args::value_type...>)
            requires (std::is_constructible_v<T, std::variant<std::monostate, typename SubArgs::value_type...>, typename Args::value_type...>)
        {
            auto v = std::ranges::views::all(r);
            auto iter = std::ranges::cbegin(v);
            const auto sentinel = std::ranges::cend(v);
            return parse<T>(iter, sentinel);
        }

        template <class T, class I, class S>
        requires (
            std::sentinel_for<S, I> &&
            std::convertible_to<col::iter_const_reference_t<I>, std::string_view>
        )
        constexpr std::expected<T, col::ParseError> parse(I& iter, const S& sentinel) const
            noexcept(std::is_nothrow_constructible_v<T, std::variant<std::monostate, typename SubArgs::value_type...>, typename Args::value_type...>)
            requires (std::is_constructible_v<T, std::variant<std::monostate, typename SubArgs::value_type...>, typename Args::value_type...>)
        {
            std::optional<std::variant<std::monostate, typename SubArgs::value_type...>> subsubcmd{};
            std::tuple<std::optional<typename Args::value_type>...> init{};
            auto zipped = col::zip_tuples(m_args, init);

            while( iter != sentinel )
            {
                const auto subsub_res = col::tuple_try_foreach(
                    [&]<class SubCmdCfg>(const SubCmdCfg& cfg)
                        -> col::ControlFlow<std::optional<col::ParseError>>
                    {
                        const std::string_view a{ *iter };
                        if( a != cfg.get_name() )
                        {
                            return col::Continue{};
                        }
                        std::ranges::advance(iter, 1);
                        const auto res = cfg.parse(iter, sentinel);
                        if( res.has_value() )
                        {
                            subsubcmd.emplace(std::move(*res));
                            return col::Break{ std::nullopt };
                        }
                        else
                        {
                            return col::Break{
                                std::move(res).error()
                            };
                        }
                    },
                    m_subs);
                if( subsub_res.is_break() )
                {
                    if( subsub_res.to_break().has_value() )
                    {
                        return std::unexpected{
                            *std::move(subsub_res).to_break()
                        };
                    }
                    else
                    {
                        // col::Break{std::nullopt} のとき、サブサブコマンドがパース成功したことを意味する。
                        // 残りの引数を続けてこのサブコマンドの引数として解釈できるが直感的ではないので、
                        // ここで break して残りの引数はエラーとする。
                        break;
                    }
                }

                const auto res = col::tuple_try_foreach(
                    [&]<class ValueT, class DefaultT, class ParserT>
                        (std::tuple<const Arg<ValueT, DefaultT, ParserT>&, std::optional<ValueT>&>& elem)
                        -> col::ControlFlow<std::optional<col::ParseError>>
                    {
                        const Arg<ValueT, DefaultT, ParserT>& config = std::get<0>(elem);
                        const std::string_view a{ *iter };
                        if( a != config.get_name() )
                        {
                            return col::Continue{};
                        }
                        std::ranges::advance(iter, 1);
                        const auto parse_res = config.parse(iter, sentinel);
                        if( parse_res.has_value() )
                        {
                            std::optional<ValueT>& value = std::get<1>(elem);
                            value.emplace(std::move(*parse_res));
                            return col::Break{ std::nullopt };
                        }
                        else
                        {
                            return col::Break{
                                std::move(std::move(parse_res).error())
                            };
                        }
                    }, zipped);
                if( res.is_break() )
                {
                    const auto brk = std::move(res).to_break();
                    if( brk.has_value() )
                    {
                        return std::unexpected{
                            std::move(*brk)
                        };
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    // どのサブサブコマンドでもオプションでもない
                    return std::unexpected{
                        col::UnknownOption{
                            .arg = *iter,
                        }
                    };
                }

            }

            if( iter != sentinel )
            {
                return std::unexpected{
                    col::UnknownOption{
                        .arg = *iter,
                    }
                };
            }

            if( !subsubcmd.has_value() )
            {
                subsubcmd.emplace(std::in_place_index<0>, std::monostate{});
            }

            const auto default_init_res = col::tuple_try_foreach(
                [&]<class ValueT, class De, class Pr>(std::tuple<const Arg<ValueT, De, Pr>&, std::optional<ValueT>&>& elem)
                    -> col::ControlFlow<col::ParseError>
                {
                    std::optional<ValueT>& value = std::get<1>(elem);
                    if( value.has_value() )
                    {
                        return col::Continue{};
                    }
                    const Arg<ValueT, De, Pr>& config = std::get<0>(elem);
                    if constexpr( std::same_as<De, blank> )
                    {
                        if constexpr( std::is_default_constructible_v<std::remove_cvref_t<ValueT>> )
                        {
                            value.emplace();
                            return col::Continue{};
                        }
                        else
                        {
                            return col::Break{
                                col::InvalidConfiguration{
                                    .name = config.get_name(),
                                    .kind = col::InvalidConfigKind::EmptyDefault,
                                }
                            };
                        }
                    }
                    else
                    {
                        if constexpr( std::invocable<De> )
                        {
                            const auto invk_res = std::invoke(config.get_default());
                            using R = std::remove_cvref_t<decltype(invk_res)>;
                            if constexpr( std::convertible_to<R, ValueT> )
                            {
                                value.emplace(std::move(invk_res));
                                return col::Continue{};
                            }
                            else if constexpr( col::is_std_optional_v<R> )
                            {
                                if( invk_res.has_value() )
                                {
                                    value.emplace(std::move(*invk_res));
                                    return col::Continue{};
                                }
                                else
                                {
                                    return col::Break{
                                        col::DefaultGenerationError {
                                            .name = config.get_name(),
                                        }
                                    };
                                }
                            }
                            else if constexpr( col::is_std_expected_v<R> )
                            {
                                if( invk_res.has_value() )
                                {
                                    value.emplace(std::move(*invk_res));
                                    return col::Continue{};
                                }
                                else
                                {
                                    if constexpr( std::convertible_to<typename R::error_type, col::ParseError> )
                                    {
                                        return col::Break{
                                            std::unexpected{
                                                std::move(invk_res).error()
                                            }
                                        };
                                    }
                                    else
                                    {
                                        return col::Break{
                                            col::DefaultGenerationError {
                                                .name = config.get_name(),
                                            }
                                        };
                                    }
                                }
                            }
                            else
                            {
                                return col::Break{
                                    col::InternalLogicError{
                                        .name = config.get_name(),
                                        .kind = col::InternalLogicErrorKind::InvalidFunctionReturnType,
                                    }
                                };
                            }
                        }
                        else
                        {
                            value.emplace(config.get_default());
                            return col::Continue{};
                        }
                    }
                },
                zipped);
            if( default_init_res.is_break() )
            {
                return std::unexpected{
                    std::move(default_init_res).to_break()
                };
            }

            return std::apply([&]<class ...Ts>(Ts&& ...ts)
                {
                    return T{ std::move(*subsubcmd), (*std::forward<Ts>(ts))... };
                }, std::move(init));
        }
    };
    template <class T>
    requires (std::convertible_to<T, std::string_view>)
    Cmd(T) -> Cmd<>;


} // namespace col

