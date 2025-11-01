#pragma once

#include <col/control_flow.h>
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

    // 同じオプションが複数回指定された。
    struct DuplicateOption
    {
        std::string_view name;
    };

    // オプションに対して引数が与えられなかった。
    struct MissingOptionValue
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
            DuplicateOption,
            MissingOptionValue,
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
struct std::formatter<col::DuplicateOption>
{
    constexpr auto parse(std::format_parse_context& ctx) const noexcept
    {
        return ctx.begin();
    }
    auto format(const col::DuplicateOption& err, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(), "duplicate option: name='{}'", err.name);
    }
};

template <>
struct std::formatter<col::MissingOptionValue>
{
    constexpr auto parse(std::format_parse_context& ctx) const noexcept
    {
        return ctx.begin();
    }
    auto format(const col::MissingOptionValue& err, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(), "missing value for option: name='{}'", err.name);
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
            "invalid number: option='{}' arg='{}' errc='{}'",
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

    template <class D>
    concept arg_default_type = (
        std::is_object_v<D> &&
        !std::same_as<D, blank> &&
        (
            !std::invocable<D> ||
            (
                !std::is_void_v<std::invoke_result_t<D>> &&
                !std::same_as<std::remove_cvref_t<std::invoke_result_t<D>>, blank>
            )
        )
    );

    template <class P>
    concept arg_parser_type = (
        std::is_object_v<P> &&
        !std::same_as<P, blank> &&
        std::invocable<P, const char*> &&
        !std::is_void_v<std::invoke_result_t<P, const char*>> &&
        !std::same_as<std::remove_cvref_t<std::invoke_result_t<P, const char*>>, blank>
    );

    template <class D, class P>
    concept acceptable_default_and_parser_type = (
        (
            std::same_as<D, blank> &&
            arg_parser_type<P>
        ) ||
        (
            arg_default_type<D> &&
            std::same_as<P, blank>
        ) ||
        (
            !std::same_as<D, blank> &&
            !std::same_as<P, blank> &&
            (
                requires {
                    !std::invocable<D>;
                    typename std::common_type_t<D, std::invoke_result_t<P, const char*>>;
                } ||
                requires {
                    std::invocable<D>;
                    typename std::common_type_t<std::invoke_result_t<D>, std::invoke_result_t<P, const char*>>;
                }
            )
        )
    );

    template <class T = blank, class D = blank, class P = blank>
    class Arg
    {
        static_assert(std::is_object_v<T>);
        static_assert(std::same_as<D, blank> || arg_default_type<D>);
        static_assert(std::same_as<P, blank> || arg_parser_type<P>);

        const std::string_view m_name;
        const std::string_view m_help;

        D m_default;
        P m_parser;
    public:
        using value_type = T;
        using default_type = D;
        using parser_type = P;

        constexpr explicit Arg(std::string_view name, std::string_view help) noexcept
            requires (std::is_default_constructible_v<D> && std::is_default_constructible_v<P>)
        : m_name{ name }
        , m_help{ help }
        , m_default{}
        , m_parser{}
        {}

        template <class De, class Pr>
        requires (std::is_object_v<De> && std::is_object_v<Pr>)
        constexpr explicit Arg(std::string_view name, std::string_view help, De&& de, Pr&& p) noexcept
        : m_name{ name }
        , m_help{ help }
        , m_default{ std::forward<De>(de) }
        , m_parser{ std::forward<Pr>(p) }
        {}

        constexpr std::string_view get_name() const noexcept
        {
            return m_name;
        }
        constexpr std::string_view get_help() const noexcept
        {
            return m_help;
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
                m_help,
                std::move(m_default),
                std::move(m_parser)
            };
        }

        template <class De>
        requires (
            arg_default_type<std::decay_t<De>> &&
            acceptable_default_and_parser_type<std::decay_t<De>, P>
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
                m_help,
                std::forward<std::decay_t<De>>(de),
                std::move(m_parser)
            };
        }

        template <class Pr>
        requires (
            arg_parser_type<std::decay_t<Pr>> &&
            acceptable_default_and_parser_type<D, std::decay_t<Pr>>
        )
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
                m_help,
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
            requires (!std::same_as<T, blank>)
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
                        col::MissingOptionValue {
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
                    return std::unexpected{
                        col::InvalidConfiguration{
                            .name =  m_name,
                            .kind = col::InvalidConfigKind::EmptyParser,
                        }
                    };
                }
            }
        }
    };

    template <class T, class U>
    requires (std::convertible_to<T, std::string_view> && std::convertible_to<U, std::string_view>)
    Arg(T, U) -> Arg<blank, blank, blank>;


    template <class M, class ...Args>
    class SubCmd;
    template <class ...Args>
    class Cmd;

    namespace detail {

        template <class T, class, class>
        class CmdBase;
        template <class T, class ...SubCmdTypes, class ...ArgTypes>
        class CmdBase<T, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>
        {
            const std::string_view m_name;
            const std::string_view m_help;
            std::tuple<SubCmdTypes...> m_subs;
            std::tuple<ArgTypes...> m_args;

        public:
            constexpr CmdBase(std::string_view name, std::string_view help) noexcept
                requires (sizeof...(SubCmdTypes) == 0 && sizeof...(ArgTypes) == 0)
            : m_name{ name }
            , m_help{ help }
            , m_subs{}
            , m_args{}
            {}

            constexpr std::string_view get_name() const noexcept
            {
                return m_name;
            }

            constexpr std::string_view get_help() const noexcept
            {
                return m_help;
            }

        protected:
            constexpr CmdBase(
                std::string_view name, std::string_view help,
                std::tuple<SubCmdTypes...>&& subs, std::tuple<ArgTypes...>&& args)
                noexcept(
                    std::is_nothrow_move_constructible_v<std::tuple<SubCmdTypes...>> &&
                    std::is_nothrow_move_constructible_v<std::tuple<ArgTypes...>>
                )
            : m_name{ name }
            , m_help{ help }
            , m_subs{ std::move(subs) }
            , m_args{ std::move(args) }
            {}

            template <class Value, class Default, class Parser>
            constexpr auto add_arg_as_cmd(Arg<Value, Default, Parser>&& arg)
                noexcept(std::is_nothrow_move_constructible_v<Arg<Value, Default, Parser>>)
            {
                using NewCmd = std::conditional_t<(sizeof...(SubCmdTypes) > 0),
                    Cmd<std::variant<SubCmdTypes...>, ArgTypes..., Arg<Value, Default, Parser>>,
                    Cmd<ArgTypes..., Arg<Value, Default, Parser>>
                >;
                return NewCmd{
                    m_name,
                    m_help,
                    std::move(m_subs),
                    std::tuple_cat(std::move(m_args), std::tuple{ std::move(arg) })
                };
            }
            template <class Default, class Parser>
            constexpr auto add_arg_as_cmd(Arg<blank, Default, Parser>&& arg)
                noexcept(std::is_nothrow_move_constructible_v<Arg<blank, Default, Parser>>)
            {
                return add_arg_as_cmd(std::move(arg).template set_value_type<bool>());
            }
            
            template <class Map, class ...Args>
            constexpr auto add_subcmd_as_cmd(SubCmd<Map, Args...>&& sub)
                noexcept(std::is_nothrow_move_constructible_v<SubCmd<Map, Args...>>)
            {
                return Cmd<std::variant<SubCmdTypes..., SubCmd<Map, Args...>>, ArgTypes...>{
                    m_name,
                    m_help,
                    std::tuple_cat(std::move(m_subs), std::tuple{ std::move(sub) }),
                    std::move(m_args)
                };
            }

            template <class Value, class Default, class Parser>
            constexpr auto add_arg_as_subcmd(Arg<Value, Default, Parser>&& arg)
                noexcept(std::is_nothrow_move_constructible_v<Arg<Value, Default, Parser>>)
            {
                using NewSubCmd = std::conditional_t<(sizeof...(SubCmdTypes) > 0),
                    SubCmd<T, std::variant<SubCmdTypes...>, ArgTypes..., Arg<Value, Default, Parser>>,
                    SubCmd<T, ArgTypes..., Arg<Value, Default, Parser>>
                >;
                return NewSubCmd{
                    m_name,
                    m_help,
                    std::move(m_subs),
                    std::tuple_cat(std::move(m_args), std::tuple{ std::move(arg) })
                };
            }
            template <class Default, class Parser>
            constexpr auto add_arg_as_subcmd(Arg<blank, Default, Parser>&& arg)
                noexcept(std::is_nothrow_move_constructible_v<Arg<blank, Default, Parser>>)
            {
                return add_arg_as_subcmd(std::move(arg).template set_value_type<bool>());
            }

            template <class Map, class ...Args>
            constexpr auto add_subcmd_as_subcmd(SubCmd<Map, Args...>&& sub)
                noexcept(std::is_nothrow_move_constructible_v<SubCmd<Map, Args...>>)
            {
                return SubCmd<T, std::variant<SubCmdTypes..., SubCmd<Map, Args...>>, ArgTypes...>{
                    m_name,
                    m_help,
                    std::tuple_cat(std::move(m_subs), std::tuple{ std::move(sub) }),
                    std::move(m_args)
                };
            }

        public:
            template <class Target = T, class I, class S>
            requires (std::sentinel_for<S, I>)
            constexpr std::expected<Target, col::ParseError> parse(I& iter, const S& sentinel) const
                requires(
                    requires {
                        sizeof...(SubCmdTypes) > 0;
                        std::is_constructible_v<Target, std::variant<std::monostate, typename SubCmdTypes::value_type...>, typename ArgTypes::value_type...>;
                    } ||
                    requires {
                        sizeof...(SubCmdTypes) == 0;
                        std::is_constructible_v<Target, typename ArgTypes::value_type...>;
                    }
                )
            {
                using SubCmdVariantType = std::variant<std::monostate, typename SubCmdTypes::value_type...>;
                std::optional<SubCmdVariantType> subcommand{};
                std::tuple<std::optional<typename ArgTypes::value_type>...> parsed_arguments{};
                auto zipped = col::zip_tuples(m_args, parsed_arguments);

                while( iter != sentinel )
                {
                    if constexpr( sizeof...(SubCmdTypes) > 0 )
                    {
                        if( !subcommand.has_value() )
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
                                        subcommand.emplace(std::move(*res));
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
                            std::optional<ValueT>& value = std::get<1>(elem);
                            if( value.has_value() )
                            {
                                return col::Break{
                                    col::DuplicateOption{
                                        .name = config.get_name(),
                                    }
                                };
                            }
                            std::ranges::advance(iter, 1);
                            const auto parse_res = config.parse(iter, sentinel);
                            if( parse_res.has_value() )
                            {
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

                if( !subcommand.has_value() )
                {
                    subcommand.emplace(std::in_place_index<0>, std::monostate{});
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
                        if constexpr( sizeof...(SubCmdTypes) > 0 )
                        {
                            return Target{ std::move(*subcommand), (*std::forward<Ts>(ts))... };
                        }
                        else
                        {
                            return Target{ (*std::forward<Ts>(ts))... };
                        }
                    }, std::move(parsed_arguments));
            }
        };

    } // namespace detail


    template <class M, class ...ArgTypes>
    class SubCmd : public detail::CmdBase<M, std::tuple<>, std::tuple<ArgTypes...>>
    {
        template <class, class, class>
        friend class detail::CmdBase;
    public:
        using detail::CmdBase<M, std::tuple<>, std::tuple<ArgTypes...>>::CmdBase;
        using value_type = M;

        template <class Value, class Default, class Parser>
        constexpr auto add(Arg<Value, Default, Parser>&& arg)
        {
            return detail::CmdBase<M, std::tuple<>, std::tuple<ArgTypes...>>::add_arg_as_subcmd(std::move(arg));
        }

        template <class Map, class ...Args>
        requires (std::is_constructible_v<Map, typename Args::value_type...>)
        constexpr auto add(SubCmd<Map, Args...>&& sub)
        {
            return detail::CmdBase<M, std::tuple<>, std::tuple<ArgTypes...>>::add_subcmd_as_subcmd(std::move(sub));
        }

        template <class Map, class ...Subs, class ...Args>
        requires (std::is_constructible_v<Map, std::variant<std::monostate, typename Subs::value_type...>, typename Args::value_type...>)
        constexpr auto add(SubCmd<Map, std::variant<Subs...>, Args...>&& sub)
        {
            return detail::CmdBase<M, std::tuple<>, std::tuple<ArgTypes...>>::add_subcmd_as_subcmd(std::move(sub));
        }
    };

    template <class M, class ...SubCmdTypes, class ...ArgTypes>
    class SubCmd<M, std::variant<SubCmdTypes...>, ArgTypes...>
        : public detail::CmdBase<M, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>
    {
        template <class, class, class>
        friend class detail::CmdBase;
    public:
        using detail::CmdBase<M, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::CmdBase;
        using value_type = M;
    
        template <class Value, class Default, class Parser>
        constexpr auto add(Arg<Value, Default, Parser>&& arg)
        {
            return detail::CmdBase<M, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::add_arg_as_subcmd(std::move(arg));
        }

        template <class Map, class ...Args>
        requires (std::is_constructible_v<Map, typename Args::value_type...>)
        constexpr auto add(SubCmd<Map, Args...>&& sub)
        {
            return detail::CmdBase<M, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::add_subcmd_as_subcmd(std::move(sub));
        }

        template <class Map, class ...Subs, class ...Args>
        requires (std::is_constructible_v<Map, std::variant<std::monostate, typename Subs::value_type...>, typename Args::value_type...>)
        constexpr auto add(SubCmd<Map, std::variant<Subs...>, Args...>&& sub)
        {
            return detail::CmdBase<M, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::add_subcmd_as_subcmd(std::move(sub));
        }
    };

    template <class ...ArgTypes>
    class Cmd : public detail::CmdBase<blank, std::tuple<>, std::tuple<ArgTypes...>>
    {
        template <class, class, class>
        friend class detail::CmdBase;
    public:
        using detail::CmdBase<blank, std::tuple<>, std::tuple<ArgTypes...>>::CmdBase;

        template <class Value, class Default, class Parser>
        constexpr auto add(Arg<Value, Default, Parser>&& arg)
        {
            return detail::CmdBase<blank, std::tuple<>, std::tuple<ArgTypes...>>::add_arg_as_cmd(std::move(arg));
        }

        template <class Map, class ...Args>
        requires (std::is_constructible_v<Map, typename Args::value_type...>)
        constexpr auto add(SubCmd<Map, Args...>&& sub)
        {
            return detail::CmdBase<blank, std::tuple<>, std::tuple<ArgTypes...>>::add_subcmd_as_cmd(std::move(sub));
        }

        template <class Map, class ...Subs, class ...Args>
        requires (std::is_constructible_v<Map, std::variant<std::monostate, typename Subs::value_type...>, typename Args::value_type...>)
        constexpr auto add(SubCmd<Map, std::variant<Subs...>, Args...>&& sub)
        {
            return detail::CmdBase<blank, std::tuple<>, std::tuple<ArgTypes...>>::add_subcmd_as_cmd(std::move(sub));
        }

        template <class T, class R>
        requires (
            !std::same_as<T, blank> &&
            std::ranges::viewable_range<R> &&
            std::convertible_to<col::range_const_reference_t<R>, std::string_view> &&
            std::is_constructible_v<T, typename ArgTypes::value_type...>
        )
        constexpr std::expected<T, col::ParseError> parse(R r) const
        {
            const auto view = std::ranges::views::all(r);
            auto iter = std::ranges::cbegin(view);
            const auto sentinel = std::ranges::cend(view);
            return parse<T>(iter, sentinel);
        }

        template <class T, class I, class S>
        requires (
            !std::same_as<T, blank> &&
            std::sentinel_for<S, I> &&
            std::convertible_to<col::iter_const_reference_t<I>, std::string_view> &&
            std::is_constructible_v<T, typename ArgTypes::value_type...>
        )
        constexpr std::expected<T, col::ParseError> parse(I& iter, const S& sentinel) const
        {
            return detail::CmdBase<blank, std::tuple<>, std::tuple<ArgTypes...>>::template parse<T>(iter, sentinel);
        }
    };

    template <class ...SubCmdTypes, class ...ArgTypes>
    class Cmd<std::variant<SubCmdTypes...>, ArgTypes...>
        : public detail::CmdBase<blank, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>
    {
        template <class, class, class>
        friend class detail::CmdBase;
    public:
        using detail::CmdBase<blank, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::CmdBase;

        template <class Value, class Default, class Parser>
        constexpr auto add(Arg<Value, Default, Parser>&& arg)
        {
            return detail::CmdBase<blank, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::add_arg_as_cmd(std::move(arg));
        }

        template <class Map, class ...Args>
        requires (std::is_constructible_v<Map, typename Args::value_type...>)
        constexpr auto add(SubCmd<Map, Args...>&& sub)
        {
            return detail::CmdBase<blank, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::add_subcmd_as_cmd(std::move(sub));
        }

        template <class Map, class ...Subs, class ...Args>
        requires (std::is_constructible_v<Map, std::variant<std::monostate, typename Subs::value_type...>, typename Args::value_type...>)
        constexpr auto add(SubCmd<Map, std::variant<Subs...>, Args...>&& sub)
        {
            return detail::CmdBase<blank, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::add_subcmd_as_cmd(std::move(sub));
        }

        template <class T, class R>
        requires (
            !std::same_as<T, blank> &&
            std::ranges::viewable_range<R> &&
            std::convertible_to<col::range_const_reference_t<R>, std::string_view> &&
            std::is_constructible_v<T, std::variant<std::monostate, typename SubCmdTypes::value_type...>, typename ArgTypes::value_type...>
        )
        constexpr std::expected<T, col::ParseError> parse(R r) const
        {
            const auto view = std::ranges::views::all(r);
            auto iter = std::ranges::cbegin(view);
            const auto sentinel = std::ranges::cend(view);
            return parse<T>(iter, sentinel);
        }

        template <class T, class I, class S>
        requires (
            !std::same_as<T, blank> &&
            std::sentinel_for<S, I> &&
            std::convertible_to<col::iter_const_reference_t<I>, std::string_view> &&
            std::is_constructible_v<T, std::variant<std::monostate, typename SubCmdTypes::value_type...>, typename ArgTypes::value_type...>
        )
        constexpr std::expected<T, col::ParseError> parse(I& iter, const S& sentinel) const
        {
            return detail::CmdBase<blank, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::template parse<T>(iter, sentinel);
        }
    };

    template <class T, class U>
    requires (std::convertible_to<T, std::string_view> && std::convertible_to<U, std::string_view>)
    Cmd(T, U) -> Cmd<>;


} // namespace col

