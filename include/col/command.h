#pragma once

#include <col/control_flow.h>
#include <col/from_string.h>
#include <col/tuple.h>
#include <col/type_traits.h>

#include <cstddef>
#include <cstdint>

#include <algorithm>
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

    // 不明なエラー。
    struct UnknownError
    {};

    // 内部ロジックエラーの種類。
    enum class InternalLogicErrorKind : std::uint32_t
    {
        // 不正な関数戻り値。
        InvalidFunctionReturnType,
    };

    // 内部ロジックエラー。
    struct InternalLogicError
    {
        std::string_view name;
        InternalLogicErrorKind kind;
    };

    // ヘルプメッセージを表示する。
    struct ShowHelp
    {
        std::string help_message;
    };

    // 不明なオプション。
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

    // 不正な設定の種類。
    enum class InvalidConfigKind : std::uint32_t
    {
        // デフォルト値の設定方法が定まっていない。
        EmptyDefault,
        // パーサーの設定方法が定まっていない。
        EmptyParser,
    };

    // 不正な設定。
    struct InvalidConfiguration
    {
        std::string_view name;
        InvalidConfigKind kind;
    };

    // パーサーがエラーを返した。
    struct ValueParserError
    {
        std::string_view name;
        std::string_view arg;
    };

    // デフォルト値の生成時にエラーが発生した。
    struct DefaultValueError
    {
        std::string_view name;
    };

    // 必須のオプションが指定されなかった。
    struct MissingRequiredOption
    {
        std::string_view name;
    };

    // パーサーが返すエラー。
    using ParseError =
        std::variant<
            UnknownError,
            InternalLogicError,
            UnknownOption,
            ShowHelp,
            DuplicateOption,
            MissingOptionValue,
            ValueParserError,
            DefaultValueError,
            InvalidNumber,
            NotEnoughArgument,
            InvalidConfiguration,
            MissingRequiredOption
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
    auto format(const col::ShowHelp& err, std::format_context& ctx) const noexcept
    {
        return std::format_to(ctx.out(), "{}", err.help_message);
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
struct std::formatter<col::ValueParserError>
{
    constexpr auto parse(std::format_parse_context& ctx) const noexcept
    {
        return ctx.begin();
    }
    auto format(const col::ValueParserError& err, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(),
            "value parser failed: name='{}' arg='{}'", err.name, err.arg);
    }
};

template <>
struct std::formatter<col::DefaultValueError>
{
    constexpr auto parse(std::format_parse_context& ctx) const noexcept
    {
        return ctx.begin();
    }
    auto format(const col::DefaultValueError& err, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(),
            "failed to generate default value: name='{}'", err.name);
    }
};

template <>
struct std::formatter<col::MissingRequiredOption>
{
    constexpr auto parse(std::format_parse_context& ctx) const noexcept
    {
        return ctx.begin();
    }
    auto format(const col::MissingRequiredOption& err, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(),
            "missing required option: name='{}'", err.name);
    }
};

namespace col {

    // 空の型。
    struct blank
    {
        constexpr blank() noexcept = default;
    };

    // `T` として推論された型。
    template <class T>
    requires (!std::same_as<std::remove_cvref_t<T>, blank>)
    struct Deduced
    {
        using type = T;
    };

    // `T` が `col::Deduced` か判定する。
    template <class T>
    struct is_col_deduced : std::false_type {};
    // `T` が `col::Deduced` か判定する。
    template <class T>
    struct is_col_deduced<Deduced<T>> : std::true_type {};
    // `T` が `col::Deduced` であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr auto is_col_deduced_v = is_col_deduced<T>::value;

    // 型 `D` が `col::Arg` のデフォルト値として指定できる型であることを示すコンセプト。
    template <class D>
    concept default_value_type = (
        std::is_object_v<D> &&
        !std::same_as<std::remove_cvref_t<D>, blank> &&
        !is_col_deduced_v<std::remove_cvref_t<D>> &&
        (
            !std::invocable<D> ||
            (
                !std::is_void_v<std::invoke_result_t<D>> &&
                !std::same_as<std::remove_cvref_t<std::invoke_result_t<D>>, blank> &&
                !is_col_deduced_v<std::remove_cvref_t<std::invoke_result_t<D>>>
            )
        )
    );

    // デフォルト値の型 `D` から得られる型を推論し、メンバ型 `type` として定義する。
    template <default_value_type D>
    struct deduce_default_value_type
    {
        using type = D;
    };
    // デフォルト値の型 `D` から得られる型を推論し、メンバ型 `type` として定義する。
    template <default_value_type D>
    requires (
        std::invocable<D>
    )
    struct deduce_default_value_type<D>
    {
        using type = col::unwrap_ok_type_if_t<std::invoke_result_t<D>>;
    };
    // デフォルト値の型 `D` から得られる型。
    template <class D>
    using deduce_default_type_t = deduce_default_value_type<D>::type;


    // 型 `P` が `col::Arg` のパーサーとして指定できる型であることを示すコンセプト。
    template <class P>
    concept value_parser_type = (
        std::is_object_v<P> &&
        !std::same_as<std::remove_cvref_t<P>, blank> &&
        !is_col_deduced_v<std::remove_cvref_t<P>> &&
        std::invocable<P, const char*> &&
        !std::is_void_v<col::unwrap_ok_type_if_t<std::invoke_result_t<P, const char*>>> &&
        !std::same_as<std::remove_cvref_t<col::unwrap_ok_type_if_t<std::invoke_result_t<P, const char*>>>, blank> &&
        !is_col_deduced_v<std::remove_cvref_t<col::unwrap_ok_type_if_t<std::invoke_result_t<P, const char*>>>>
    );

    // パーサーの型 `P` から得られる型を推論し、メンバ型 `type` として定義する。
    template <value_parser_type P>
    struct deduce_value_parser_type
    {
        using type = col::unwrap_ok_type_if_t<std::invoke_result_t<P, const char*>>;
    };
    // パーサーの型 `P` から得られる型。
    template <class P>
    using deduce_parser_type_t = deduce_value_parser_type<P>::type;


    // 型 `D` と `P` の組がデフォルト値とパーサーとして `col::Arg` に指定されたときに適合することを示すコンセプト。
    template <class D, class P>
    concept default_and_parser_compatible = (
        (
            std::same_as<D, blank> &&
            value_parser_type<P>
        ) ||
        (
            default_value_type<D> &&
            std::same_as<P, blank>
        ) ||
        (
            default_value_type<D> &&
            value_parser_type<P> &&
            requires {
                typename std::common_type_t<
                    deduce_default_type_t<D>,
                    deduce_parser_type_t<P>
                >;
            }
        )
    );

    namespace detail {
        template <class D, class P>
        struct deduce_value_type : std::common_type<deduce_default_type_t<D>, deduce_parser_type_t<P>> {};
        template <class D>
        struct deduce_value_type<D, blank> : deduce_default_value_type<D> {};
        template <class P>
        struct deduce_value_type<blank, P> : deduce_value_parser_type<P> {};
        template <class D, class P>
        using deduce_value_type_t = deduce_value_type<D, P>::type;
    } // namespace detail

    // オプション名として適格な文字列。
    class OptionName
    {
        template <class>
        void invalid_name(){}
        struct too_short_name{};
        struct contains_invalid_character{};
        struct starts_with_invalid_character{};

        const std::string_view m_name;
    public:
        consteval OptionName(const char* name) noexcept
        : m_name{ name }
        {
            constexpr auto is_valid_char = [](char c) static noexcept
                {
                    return (('a' <= c && c <= 'z') ||
                        ('A' <= c && c <= 'Z') ||
                        ('0' <= c && c <= '9')) ||
                        c == '-' ||
                        c == '_';
                };
            if( m_name.size() < 2 )
            {
                invalid_name<too_short_name>();
            }
            else if( !std::ranges::all_of(std::ranges::cbegin(m_name), std::ranges::cend(m_name), is_valid_char) )
            {
                invalid_name<contains_invalid_character>();
            }
            else if( m_name[0] == '_' || m_name[0] == '-' )
            {
                invalid_name<starts_with_invalid_character>();
            }
        }

        consteval OptionName(std::string_view name) noexcept
        : OptionName{ name.data() }
        {}

        constexpr operator std::string_view() const noexcept
        {
            return m_name;
        }
    };

    // usage の表示におけるインデント幅の既定値。スペースの個数。
    inline constexpr std::size_t DefaultIndentWidthForUsage = 4ZU;

    
    namespace detail {

        template <class T, class, class>
        class CmdBase;
    }

    // コマンドライン引数の型。
    // `T` は、そのコマンドライン引数に対応する型。
    // `D` は、デフォルト値を得るための型。
    // `P` は、コマンドライン引数の文字列をパースして `T` を生成するための型。
    template <class T = blank, class D = blank, class P = blank>
    class [[nodiscard]] Arg
    {
        static_assert(std::is_object_v<T>);
        static_assert(std::same_as<D, blank> || default_value_type<D>);
        static_assert(std::same_as<P, blank> || value_parser_type<P>);

        const std::string_view m_name;
        const std::string_view m_help;

        template <class, class, class>
        friend class Arg;
        template <class, class, class>
        friend class detail::CmdBase;

        D m_default_value;
        P m_value_parser;
    public:
        // このコマンドライン引数に対応する型。
        using value_type = T;
        // デフォルト値を得るための型。
        using default_type = D;
        // 文字列をパースして `T` を生成するための型。
        using parser_type = P;

        constexpr explicit Arg(OptionName name, std::string_view help) noexcept
            requires (std::is_default_constructible_v<D> && std::is_default_constructible_v<P>)
        : m_name{ name }
        , m_help{ help }
        , m_default_value{}
        , m_value_parser{}
        {}

    private:
        template <class De, class Pr>
        requires (std::is_object_v<std::decay_t<De>> && std::is_object_v<std::decay_t<Pr>>)
        constexpr explicit Arg(OptionName name, std::string_view help, De&& de, Pr&& p) noexcept
        : m_name{ name }
        , m_help{ help }
        , m_default_value{ std::forward<De>(de) }
        , m_value_parser{ std::forward<Pr>(p) }
        {}

    public:
        // このコマンドライン引数の名前を得る
        [[nodiscard]] constexpr std::string_view get_name() const noexcept
        {
            return m_name;
        }
        // このコマンドライン引数のヘルプメッセージを得る
        [[nodiscard]] constexpr std::string_view get_help() const noexcept
        {
            return m_help;
        }
        // このコマンドライン引数のデフォルト値を生成する型の値を得る
        [[nodiscard]] constexpr const D& get_default() const noexcept
        {
            return m_default_value;
        }
        // このコマンドライン引数のパーサーの型の値を得る
        [[nodiscard]] constexpr const P& get_parser() const noexcept
        {
            return m_value_parser;
        }

        // usage 文字列を得る。
        // `indent_width` はインデント幅、 `help_column` はヘルプメッセージが開始される行頭からの位置。
        [[nodiscard]] constexpr std::string get_usage(std::size_t indent_with, std::size_t help_column) const
        {
            std::string usage{};
            usage.reserve(help_column + m_help.size());
            usage.append(indent_with, ' ');
            usage += "--";
            usage += m_name;
            if constexpr( !std::same_as<T, blank> && !std::same_as<T, bool> )
            {
                usage += " <";
                usage.append_range(m_name | std::ranges::views::transform([](char c) static noexcept
                    {
                        return ('A' <= c && c <= 'Z') ? static_cast<char>(c - ('a' - 'A')) : c;
                    }));
                usage += '>';
            }
            usage.append(help_column - usage.size(), ' ');
            usage += m_help;
            return usage;
        }

        // `T` の値を明示的に設定する。
        //
        // `T` が `col::blank` か、 `col::Deduced<T>` でなければならない。
        template <class Value>
        requires (
            std::is_object_v<Value> &&
            !std::same_as<std::remove_cvref_t<Value>, blank> && 
            !is_col_deduced_v<std::remove_cvref_t<Value>> &&
            ( std::same_as<D, blank> || std::convertible_to<deduce_default_type_t<D>, Value> ) &&
            ( std::same_as<P, blank> || std::convertible_to<deduce_parser_type_t<P>, Value> )
        )
        constexpr Arg<Value, D, P> set_value_type() &&
            requires (std::same_as<T, blank> || is_col_deduced_v<T>)
        {
            return Arg<Value, D, P>{
                m_name,
                m_help,
                std::move(m_default_value),
                std::move(m_value_parser)
            };
        }

        // デフォルト値を得る型を設定する。
        // `De` は、値そのものでも、引数なしで呼び出せる呼び出し可能オブジェクトでもよい。
        // `T` が `col::blank` の場合は、 `De` および `P` の型をもとに `T` が推論される。
        // 
        // `D` が `col::blank` でなければならない。
        template <class De>
        requires (
            default_value_type<std::decay_t<De>> &&
            (
                (
                    !std::same_as<T, blank> &&
                    !is_col_deduced_v<T> &&
                    std::convertible_to<deduce_default_type_t<std::decay_t<De>>, T>
                ) ||
                (
                    (std::same_as<T, blank> || is_col_deduced_v<T>) &&
                    default_and_parser_compatible<std::decay_t<De>, P>
                )
            )
        )
        constexpr auto set_default_value(De&& de) &&
            requires (std::same_as<D, blank>)
        {
            using Value = std::conditional_t<
                !std::same_as<T, blank> && !is_col_deduced_v<T>,
                T,
                Deduced<detail::deduce_value_type_t<std::decay_t<De>, P>>
            >;
            return Arg<Value, std::decay_t<De>, P>{
                m_name,
                m_help,
                std::forward<De>(de),
                std::move(m_value_parser)
            };
        }

        // パーサーを設定する。
        // `P` は、`const char*` を引数として呼び出せる呼び出し可能オブジェクトでなければならない。
        // `T` が `col::blank` の場合は、 `De` および `P` の型をもとに `T` が推論される。
        // 
        // `P` が `col::blank` でなければならない。
        template <class Pr>
        requires (
            value_parser_type<std::decay_t<Pr>> &&
            (
                (
                    !std::same_as<T, blank> &&
                    !is_col_deduced_v<T> &&
                    std::convertible_to<deduce_parser_type_t<std::decay_t<Pr>>, T>
                ) ||
                (
                    (std::same_as<T, blank> || is_col_deduced_v<T>) &&
                    default_and_parser_compatible<D, std::decay_t<Pr>>
                )
            )
        )
        constexpr auto set_value_parser(Pr&& p) &&
            requires (std::same_as<P, blank>)
        {
            using Value = std::conditional_t<
                !std::same_as<T, blank> && !is_col_deduced_v<T>,
                T,
                Deduced<detail::deduce_value_type_t<D, std::decay_t<Pr>>>
            >;
            return Arg<Value, D, std::decay_t<Pr>>{
                m_name,
                m_help,
                std::move(m_default_value),
                std::forward<Pr>(p)
            };
        }

    private:
        // コマンドライン引数を指しているイテレータ `I` およびその番兵 `S` を入力として、 `T` をパースする。
        // パースに成功した場合、イテレータは適切な数だけ進行する。
        //
        // `T` は、 `col::blank` であっても `col::Deduced<T>` であってもならない。
        template <class I, class S>
        requires (
            std::sentinel_for<S, I> &&
            std::convertible_to<col::iter_const_reference_t<I>, std::string_view>
        )
        [[nodiscard]] constexpr std::expected<T, col::ParseError> parse(I& iter, const S& s) const
            requires (!std::same_as<T, blank> && !is_col_deduced_v<T>)
        {
            if constexpr( std::same_as<T, bool> )
            {
                if constexpr( std::same_as<D, bool> )
                {
                    return !m_default_value;
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
                    const auto res = std::invoke(m_value_parser, a.data());
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
                                col::ValueParserError{
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

    // 推論ガイド。
    template <class T, class U>
    requires (std::convertible_to<T, OptionName> && std::convertible_to<U, std::string_view>)
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

            [[nodiscard]] constexpr std::string get_usage(std::string_view parent_cmd, std::size_t indent_width) const
            {
                std::string usage{m_help};

                // "Usage: cmd subcmd [OPTIONS] [COMMAND]"
                usage += "\n\nUsage: ";
                if( !parent_cmd.empty() ) // Cmd の場合は parent がない
                {
                    usage += parent_cmd;
                    usage += ' ';
                }
                usage += m_name;
                if constexpr( sizeof...(ArgTypes) > 0 )
                {
                    usage += " [OPTIONS]";
                }
                if constexpr( sizeof...(SubCmdTypes) > 0 )
                {
                    usage += " [COMMAND]";
                }
                usage += '\n';

                // Options:
                //    --name    help
                //    --str     help
                if constexpr( sizeof...(ArgTypes) > 0 )
                {
                    usage += "\nOptions:\n";
                    std::size_t max_option_name_length = 0ZU;
                    tuple_foreach([&max_option_name_length]<class ArgT>(const ArgT& arg) noexcept
                        {
                            auto length = arg.get_name().size();
                            if constexpr( !std::same_as<typename ArgT::value_type, blank> && !std::same_as<typename ArgT::value_type, bool> )
                            {
                                length += length + 3; // `' '`, "<>"
                            }
                            if( length > max_option_name_length )
                            {
                                max_option_name_length = length;
                            }
                        }, m_args);
                    const std::size_t help_indent = (indent_width * 2 + max_option_name_length + 2ZU); // <INDENT+1><`--`><option_name><INDENT><help>
                    tuple_foreach([&]<class ArgT>(const ArgT& arg)
                        {
                            usage += arg.get_usage(indent_width, help_indent) + "\n";
                        }, m_args);
                }

                if constexpr( sizeof...(SubCmdTypes) > 0 )
                {
                    usage += "\nCommands:\n";
                    std::size_t max_cmd_name_length = 0ZU;
                    tuple_foreach([&](const auto& sub)
                        {
                            const auto length = sub.get_name().size();
                            if( length > max_cmd_name_length )
                            {
                                max_cmd_name_length = length;
                            }
                        }, m_subs);
                    const std::size_t help_indent = (indent_width * 2 + max_cmd_name_length);
                    tuple_foreach([&]<class SubCmdT>(const SubCmdT& sub)
                        {
                            usage.append(indent_width, ' ');
                            usage += sub.get_name();
                            usage.append(help_indent - indent_width - sub.get_name().size(), ' ');
                            usage += sub.get_help();
                            usage += '\n';
                        }, m_subs);
                }

                return usage;
            }

            template <class Value, class Default, class Parser>
            constexpr auto add_arg_as_cmd(Arg<Value, Default, Parser>&& arg)
                noexcept(std::is_nothrow_move_constructible_v<Arg<Value, Default, Parser>>)
            {
                using CmdType = std::conditional_t<(sizeof...(SubCmdTypes) > 0),
                    Cmd<std::variant<SubCmdTypes...>, ArgTypes..., Arg<Value, Default, Parser>>,
                    Cmd<ArgTypes..., Arg<Value, Default, Parser>>
                >;
                return CmdType{
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
            template <class Value, class Default, class Parser>
            constexpr auto add_arg_as_cmd(Arg<Deduced<Value>, Default, Parser>&& arg)
                noexcept(std::is_nothrow_move_constructible_v<Arg<Deduced<Value>, Default, Parser>>)
            {
                return add_arg_as_cmd(std::move(arg).template set_value_type<Value>());
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
                using SubCmdType = std::conditional_t<(sizeof...(SubCmdTypes) > 0),
                    SubCmd<T, std::variant<SubCmdTypes...>, ArgTypes..., Arg<Value, Default, Parser>>,
                    SubCmd<T, ArgTypes..., Arg<Value, Default, Parser>>
                >;
                return SubCmdType{
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
            template <class Value, class Default, class Parser>
            constexpr auto add_arg_as_subcmd(Arg<Deduced<Value>, Default, Parser>&& arg)
                noexcept(std::is_nothrow_move_constructible_v<Arg<Deduced<Value>, Default, Parser>>)
            {
                return add_arg_as_subcmd(std::move(arg).template set_value_type<Value>());
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

            template <class Target = T, class I, class S>
            requires (std::sentinel_for<S, I>)
            constexpr std::expected<Target, col::ParseError> parse(std::string_view parent_cmd, I& iter, const S& sentinel) const
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
                                [&]<class SubCmdT>(const SubCmdT& sub)
                                    -> col::ControlFlow<std::optional<col::ParseError>>
                                {
                                    const std::string_view a{ *iter };
                                    if( a != sub.get_name() )
                                    {
                                        return col::Continue{};
                                    }
                                    std::ranges::advance(iter, 1);
                                    std::string parent{};
                                    if( !parent_cmd.empty() )
                                    {
                                        parent += parent_cmd;
                                        parent += ' ';
                                    }
                                    parent += get_name();
                                    const auto res = sub.parse(parent, iter, sentinel);
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
                            const Arg<ValueT, DefaultT, ParserT>& arg = std::get<0>(elem);
                            const std::string_view a{ *iter };

                            // TODO: `--help` の自動定義を選択可能にする
                            if( a == "--help" )
                            {
                                return col::Break{
                                    col::ShowHelp{
                                        .help_message = get_usage(parent_cmd, DefaultIndentWidthForUsage),
                                    }
                                };
                            }
                            else if( !a.starts_with("--") || a.size() <= 2 || a.substr(2) != arg.get_name() )
                            {
                                return col::Continue{};
                            }
                            std::optional<ValueT>& value = std::get<1>(elem);
                            if( value.has_value() )
                            {
                                return col::Break{
                                    col::DuplicateOption{
                                        .name = arg.get_name(),
                                    }
                                };
                            }
                            std::ranges::advance(iter, 1);
                            const auto parse_res = arg.parse(iter, sentinel);
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
                                            col::DefaultValueError {
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
                                                col::DefaultValueError {
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

    // サブコマンドの型。
    //
    // 型 `M` は、このサブコマンドのパース結果に対応させる型。
    // 型 `ArgTypes...` は、このサブコマンドのコマンドライン引数の型。
    template <class M, class ...ArgTypes>
    class [[nodiscard]] SubCmd : public detail::CmdBase<M, std::tuple<>, std::tuple<ArgTypes...>>
    {
        template <class, class, class>
        friend class detail::CmdBase;
        using detail::CmdBase<M, std::tuple<>, std::tuple<ArgTypes...>>::CmdBase;
    public:
        // このサブコマンドのパース結果に対応させる型。
        using value_type = M;

        // このサブコマンドにコマンドライン引数を追加する。
        template <class Value, class Default, class Parser>
        constexpr auto add(Arg<Value, Default, Parser>&& arg)
        {
            return detail::CmdBase<M, std::tuple<>, std::tuple<ArgTypes...>>::add_arg_as_subcmd(std::move(arg));
        }

        // このサブコマンドにサブコマンドを追加する。
        //
        // 追加するサブコマンドは、そのパース結果を生成するのに十分なコマンドライン引数の定義が完了していなければならない。
        template <class Map, class ...Args>
        requires (std::is_constructible_v<Map, typename Args::value_type...>)
        constexpr auto add(SubCmd<Map, Args...>&& sub)
        {
            return detail::CmdBase<M, std::tuple<>, std::tuple<ArgTypes...>>::add_subcmd_as_subcmd(std::move(sub));
        }

        // このサブコマンドにサブコマンドを追加する。
        //
        // 追加するサブコマンドは、そのパース結果を生成するのに十分なコマンドライン引数の定義が完了していなければならない。
        template <class Map, class ...Subs, class ...Args>
        requires (std::is_constructible_v<Map, std::variant<std::monostate, typename Subs::value_type...>, typename Args::value_type...>)
        constexpr auto add(SubCmd<Map, std::variant<Subs...>, Args...>&& sub)
        {
            return detail::CmdBase<M, std::tuple<>, std::tuple<ArgTypes...>>::add_subcmd_as_subcmd(std::move(sub));
        }
    };

    // サブコマンドの型。
    //
    // 型 `M` は、このサブコマンドのパース結果に対応させる型。
    // 型 `SubCmdTypes...` は、このサブコマンドのサブコマンドの型。
    // 型 `ArgTypes...` は、このサブコマンドのコマンドライン引数の型。
    template <class M, class ...SubCmdTypes, class ...ArgTypes>
    class [[nodiscard]] SubCmd<M, std::variant<SubCmdTypes...>, ArgTypes...>
        : public detail::CmdBase<M, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>
    {
        template <class, class, class>
        friend class detail::CmdBase;
        using detail::CmdBase<M, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::CmdBase;
    public:
        // このサブコマンドのパース結果に対応させる型。
        using value_type = M;

        // このコマンドにコマンドライン引数を追加する。
        template <class Value, class Default, class Parser>
        constexpr auto add(Arg<Value, Default, Parser>&& arg)
        {
            return detail::CmdBase<M, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::add_arg_as_subcmd(std::move(arg));
        }

        // このサブコマンドにサブコマンドを追加する。
        //
        // 追加するサブコマンドは、そのパース結果を生成するのに十分なコマンドライン引数の定義が完了していなければならない。
        template <class Map, class ...Args>
        requires (std::is_constructible_v<Map, typename Args::value_type...>)
        constexpr auto add(SubCmd<Map, Args...>&& sub)
        {
            return detail::CmdBase<M, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::add_subcmd_as_subcmd(std::move(sub));
        }

        // このサブコマンドにサブコマンドを追加する。
        //
        // 追加するサブコマンドは、そのパース結果を生成するのに十分なコマンドライン引数の定義が完了していなければならない。
        template <class Map, class ...Subs, class ...Args>
        requires (std::is_constructible_v<Map, std::variant<std::monostate, typename Subs::value_type...>, typename Args::value_type...>)
        constexpr auto add(SubCmd<Map, std::variant<Subs...>, Args...>&& sub)
        {
            return detail::CmdBase<M, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::add_subcmd_as_subcmd(std::move(sub));
        }
    };

    // コマンドの型。
    //
    // 型 `ArgTypes...` は、このコマンドのコマンドライン引数の型。
    template <class ...ArgTypes>
    class [[nodiscard]] Cmd : public detail::CmdBase<blank, std::tuple<>, std::tuple<ArgTypes...>>
    {
        template <class, class, class>
        friend class detail::CmdBase;
        using detail::CmdBase<blank, std::tuple<>, std::tuple<ArgTypes...>>::CmdBase;
    public:

        // usage 文字列を得る。
        [[nodiscard]] constexpr std::string get_usage() const
        {
            return get_usage(DefaultIndentWidthForUsage);
        }

        // usage 文字列を得る。
        // `indent_width` で指定したインデント幅をもとに出力される。
        [[nodiscard]] constexpr std::string get_usage(std::size_t indent_width) const
        {
            return detail::CmdBase<blank, std::tuple<>, std::tuple<ArgTypes...>>::get_usage("", indent_width);
        }

        // このコマンドにコマンドライン引数を追加する。
        template <class Value, class Default, class Parser>
        constexpr auto add(Arg<Value, Default, Parser>&& arg)
        {
            return detail::CmdBase<blank, std::tuple<>, std::tuple<ArgTypes...>>::add_arg_as_cmd(std::move(arg));
        }

        // このコマンドにサブコマンドを追加する。
        //
        // 追加するサブコマンドは、そのパース結果を生成するのに十分なコマンドライン引数の定義が完了していなければならない。
        template <class Map, class ...Args>
        requires (std::is_constructible_v<Map, typename Args::value_type...>)
        constexpr auto add(SubCmd<Map, Args...>&& sub)
        {
            return detail::CmdBase<blank, std::tuple<>, std::tuple<ArgTypes...>>::add_subcmd_as_cmd(std::move(sub));
        }

        // このコマンドにサブコマンドを追加する。
        //
        // 追加するサブコマンドは、そのパース結果を生成するのに十分なコマンドライン引数の定義が完了していなければならない。
        template <class Map, class ...Subs, class ...Args>
        requires (std::is_constructible_v<Map, std::variant<std::monostate, typename Subs::value_type...>, typename Args::value_type...>)
        constexpr auto add(SubCmd<Map, std::variant<Subs...>, Args...>&& sub)
        {
            return detail::CmdBase<blank, std::tuple<>, std::tuple<ArgTypes...>>::add_subcmd_as_cmd(std::move(sub));
        }

        // コマンドライン引数の範囲 `R` をパースして、指定した型 `T` を生成する。
        template <class T, class R>
        requires (
            !std::same_as<std::remove_cvref_t<T>, blank> &&
            std::ranges::viewable_range<R> &&
            std::convertible_to<col::range_const_reference_t<R>, std::string_view> &&
            std::is_constructible_v<T, typename ArgTypes::value_type...>
        )
        [[nodiscard]] constexpr std::expected<T, col::ParseError> parse(R r) const
        {
            const auto view = std::ranges::views::all(r);
            auto iter = std::ranges::cbegin(view);
            const auto sentinel = std::ranges::cend(view);
            return parse<T>(iter, sentinel);
        }

        // コマンドライン引数を指しているイテレータ `I` およびその番兵 `S` を入力として、指定した型 `T` をパースする。
        // パースに成功した場合、イテレータは適切な数だけ進行する。
        template <class T, class I, class S>
        requires (
            !std::same_as<std::remove_cvref_t<T>, blank> &&
            std::sentinel_for<S, I> &&
            std::convertible_to<col::iter_const_reference_t<I>, std::string_view> &&
            std::is_constructible_v<T, typename ArgTypes::value_type...>
        )
        [[nodiscard]] constexpr std::expected<T, col::ParseError> parse(I& iter, const S& sentinel) const
        {
            return detail::CmdBase<blank, std::tuple<>, std::tuple<ArgTypes...>>::template parse<T>("", iter, sentinel);
        }
    };

    // コマンドの型。
    // 
    // 型 `SubCmdTypes...` は、このコマンドのサブコマンドの型。
    // 型 `ArgTypes...` は、このコマンドのコマンドライン引数の型。
    template <class ...SubCmdTypes, class ...ArgTypes>
    class [[nodiscard]] Cmd<std::variant<SubCmdTypes...>, ArgTypes...>
        : public detail::CmdBase<blank, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>
    {
        template <class, class, class>
        friend class detail::CmdBase;
        using detail::CmdBase<blank, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::CmdBase;
    public:

        // usage 文字列を得る。
        [[nodiscard]] constexpr std::string get_usage() const
        {
            return get_usage(DefaultIndentWidthForUsage);
        }

        // usage 文字列を得る。
        // `indent_width` で指定したインデント幅をもとに出力される。
        [[nodiscard]] constexpr std::string get_usage(std::size_t indent_width) const
        {
            return detail::CmdBase<blank, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::get_usage("", indent_width);
        }

        // このコマンドにコマンドライン引数を追加する。
        template <class Value, class Default, class Parser>
        constexpr auto add(Arg<Value, Default, Parser>&& arg)
        {
            return detail::CmdBase<blank, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::add_arg_as_cmd(std::move(arg));
        }

        // このコマンドにサブコマンドを追加する。
        //
        // 追加するサブコマンドは、そのパース結果を生成するのに十分なコマンドライン引数の定義が完了していなければならない。
        template <class Map, class ...Args>
        requires (std::is_constructible_v<Map, typename Args::value_type...>)
        constexpr auto add(SubCmd<Map, Args...>&& sub)
        {
            return detail::CmdBase<blank, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::add_subcmd_as_cmd(std::move(sub));
        }

        // このコマンドにサブコマンドを追加する。
        //
        // 追加するサブコマンドは、そのパース結果を生成するのに十分なコマンドライン引数の定義が完了していなければならない。
        template <class Map, class ...Subs, class ...Args>
        requires (std::is_constructible_v<Map, std::variant<std::monostate, typename Subs::value_type...>, typename Args::value_type...>)
        constexpr auto add(SubCmd<Map, std::variant<Subs...>, Args...>&& sub)
        {
            return detail::CmdBase<blank, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::add_subcmd_as_cmd(std::move(sub));
        }

        // コマンドライン引数の範囲 `R` をパースして、指定した型 `T` を生成する。
        template <class T, class R>
        requires (
            !std::same_as<std::remove_cvref_t<T>, blank> &&
            std::ranges::viewable_range<R> &&
            std::convertible_to<col::range_const_reference_t<R>, std::string_view> &&
            std::is_constructible_v<T, std::variant<std::monostate, typename SubCmdTypes::value_type...>, typename ArgTypes::value_type...>
        )
        [[nodiscard]] constexpr std::expected<T, col::ParseError> parse(R r) const
        {
            const auto view = std::ranges::views::all(r);
            auto iter = std::ranges::cbegin(view);
            const auto sentinel = std::ranges::cend(view);
            return parse<T>(iter, sentinel);
        }

        // コマンドライン引数を指しているイテレータ `I` およびその番兵 `S` を入力として、指定した型 `T` をパースする。
        // パースに成功した場合、イテレータは適切な数だけ進行する。
        template <class T, class I, class S>
        requires (
            !std::same_as<std::remove_cvref_t<T>, blank> &&
            std::sentinel_for<S, I> &&
            std::convertible_to<col::iter_const_reference_t<I>, std::string_view> &&
            std::is_constructible_v<T, std::variant<std::monostate, typename SubCmdTypes::value_type...>, typename ArgTypes::value_type...>
        )
        [[nodiscard]] constexpr std::expected<T, col::ParseError> parse(I& iter, const S& sentinel) const
        {
            return detail::CmdBase<blank, std::tuple<SubCmdTypes...>, std::tuple<ArgTypes...>>::template parse<T>("", iter, sentinel);
        }
    };

    // 推論ガイド
    template <class T, class U>
    requires (std::convertible_to<T, std::string_view> && std::convertible_to<U, std::string_view>)
    Cmd(T, U) -> Cmd<>;


} // namespace col

