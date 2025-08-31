#pragma once

#include <col/tuple.h>
#include <col/type_traits.h>

#include <cstddef>
#include <charconv>
#include <concepts>
#include <expected>
#include <format>
#include <functional>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>


namespace col {

    // 空文字ではない文字列
    class NonEmptyString
    {
        void invalid_empty_string() {}

        const char* m_str;
    public:
        // `NonEmptyString` のコンストラクタ
        // `str` が空文字であれば不適格。
        consteval NonEmptyString(const char* str) noexcept
        : m_str{ str }
        {
            if( std::string_view{str}.empty() )
            {
                invalid_empty_string();
            }
        }

        consteval NonEmptyString(std::string_view str) noexcept
        : NonEmptyString{ str.data() }
        {}

        constexpr operator std::string_view() const noexcept
        {
            return std::string_view{m_str};
        }
    };

    // ロングオプション名
    class LongOptionName
    {
        template <class T>
        void invalid_format() {}

        const char* m_name;
    public:
        // `LongOptionName` のコンストラクタ
        //  `name` が `--` で始まっていなければ不適格。
        consteval LongOptionName(const char* name) noexcept
        : m_name{ name }
        {
            std::string_view n{name};
            if( n.length() < 3 )
            {
                struct too_short_name{};
                invalid_format<too_short_name>();
            }
            else if( !n.starts_with("--") )
            {
                struct name_must_start_with_minusminus{};
                invalid_format<name_must_start_with_minusminus>();
            }
        }

        // `LongOptionName` のコンストラクタ
        // `name` が `--` で始まっていなければ不適格。
        consteval LongOptionName(std::string_view name) noexcept
        : LongOptionName{ name.data() }
        {}

        constexpr operator std::string_view() const noexcept
        {
            return std::string_view{m_name};
        }
    };

    // 不明なエラー
    struct UnknownError
    {};

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

    // パーサーが `std::nullopt` を返し失敗した。
    struct ParserFailedWithNullopt
    {
        std::string_view name;
        std::string_view arg;
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

    // パーサーがエラーを返した
    struct ParserConvertionError
    {
        std::string_view name;
        std::string_view arg;
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
            UnknownOption,
            ShowHelp,
            DuplicateArg,
            NoValueGivenForOption,
            ParserFailedWithNullopt,
            ParserConvertionError,
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
struct std::formatter<col::ParserFailedWithNullopt>
{
    constexpr auto parse(std::format_parse_context& ctx) const noexcept
    {
        return ctx.begin();
    }
    auto format(const col::ParserFailedWithNullopt& err, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(), "parser failed: option='{}' arg='{}'", err.name, err.arg);
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
struct std::formatter<col::ParserConvertionError>
{
    constexpr auto parse(std::format_parse_context& ctx) const noexcept
    {
        return ctx.begin();
    }
    auto format(const col::ParserConvertionError& err, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(),
            "parser failed to convert argument: name='{}' arg='{}'", err.name, err.arg);
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

    // `Arg` の `Parser` 型引数に指定できる変換関数であることを表すコンセプト
    template <class F, class T>
    concept value_parser_for =
        std::invocable<F, const char*> &&
        (
            requires {
                std::is_void_v<T>;
                std::is_object_v<unwrap_noneable_t<std::invoke_result_t<F, const char*>>>;
            } ||
            // T が std::optional か否かで T::value_type を参照できるかコンパイルエラーになるかが変わるので、
            // requires 節を分離する必要がある。
            requires {
                !std::is_void_v<T>;
                !is_std_optional_v<T>;
                std::is_constructible_v<T, unwrap_noneable_t<std::invoke_result_t<F, const char*>>>;
            } ||
            requires {
                !std::is_void_v<T>;
                is_std_optional_v<T>;
                typename T::value_type;
                std::is_constructible_v<typename T::value_type, unwrap_noneable_t<std::invoke_result_t<F, const char*>>>;
            }
        );

    namespace detail {

        template <class Current, class Parser>
        requires (std::invocable<Parser, const char*>)
        struct deduce_new_value_type_from_parser
        {
            using type = Current;
            static_assert(std::is_constructible_v<type, unwrap_noneable_t<std::invoke_result_t<Parser, const char*>>>);
        };
        template <class Parser>
        requires (std::invocable<Parser, const char*>)
        struct deduce_new_value_type_from_parser<void, Parser>
        {
            using type = std::remove_reference_t<unwrap_noneable_t<std::invoke_result_t<Parser, const char*>>>;
        };
        template <class Current, class Parser>
        requires (std::invocable<Parser, const char*>)
        struct deduce_new_value_type_from_parser<std::optional<Current>, Parser>
        {
            using type = std::optional<Current>;
            static_assert(std::is_constructible_v<type, unwrap_noneable_t<std::invoke_result_t<Parser, const char*>>>);
        };

        // `Current` と `Parser` の型から新しい `Current` の型を推論する。
        // `Parser` は `std::invocable<Parser, const char*>` を満たすこと。
        // `Parser` の戻り値の型そのものを、または戻り値の型が `std::optional` および `std::expected` であればその有効値の型を `R` とする。
        // 以下のように新しい `Current` の型を推論し、メンバ型 `type` として持つ。
        // - `Current` が `void` のとき、 `std::remove_cvref_t<R>` 。
        // - `Current` が `std::optional` のとき、 `Current` 。ただし、 `R` が `Current` で構築可能でなければ不適格。
        // - `Current` が非 `void` のとき、 `Current` 。ただし、 `R` が `Current` で構築可能でなければ不適格。
        template <class Current, class Parser>
        using deduce_new_value_type_from_parser_t = deduce_new_value_type_from_parser<Current, Parser>::type;

    } // namespace detail

    // コマンドの引数の定義
    // `T` は引数の型
    // `Default` は引数が指定されなかったとき、それを構築するための型。 `T` 、または引数なしで呼び出し可能で戻り値によって `T` を構築できるような型。
    // `T` がデフォルト構築可能なら `D` の初期値は `T` となる。
    // `Parser` はコマンドライン引数の文字列から `T` への変換関数
    template <
        class T = void,
        class Default = std::conditional_t<std::is_default_constructible_v<T>, T, void>,
        class Parser = void
    >
    class Arg
    {
        // 未確定の型パラメータを持つ状態から構築できるようにする。
        // すべて確定している状態からは構築できないが、何かを2度設定したことを意味するのでそもそも許容しない。
        friend class Arg<void, void, void>;
        friend class Arg<T, void, void>;
        friend class Arg<T, T, void>; // T を明示的に指定してかつ T がデフォルト構築可能なとき、 set_default(), set_parser() 内から参照可能にする
        friend class Arg<T, Default, void>;
        friend class Arg<T, void, Parser>;

        // void 型を保持できないので仮の実装
        template <class U>
        struct ValueHolder
        {
            std::optional<U> value;
        };
        template <>
        struct ValueHolder<void>
        {};

        template <class P>
        struct ParserHolder
        {
            P parser;
        };

        std::string_view m_name;
        std::string_view m_help;
        bool m_required;
        
        ValueHolder<Default> m_default;
        ValueHolder<Parser> m_parser;
    public:
        using value_type = T;
        using default_type = Default;
        using parser_type = Parser;

        // コンストラクタ
        // `name` は `"--"` で始まる文字列、 `help` は 1 文字以上の文字列
        constexpr Arg(LongOptionName name, NonEmptyString help)
        : m_name{ name }
        , m_help{ help }
        , m_required{ false }
        , m_default{}
        , m_parser{}
        {}

        constexpr std::string_view get_name() const noexcept
        {
            return m_name;
        }

        constexpr std::string_view get_help() const noexcept
        {
            return m_help;
        }

        constexpr bool get_required() const noexcept
        {
            return m_required;
        }

        constexpr const std::optional<Default>& get_default() const noexcept
            requires( !std::is_void_v<Default> )
        {
            return m_default.value;
        }

        constexpr bool get_default() const noexcept
            requires (std::is_void_v<T> && std::is_void_v<Default> && std::is_void_v<Parser>) // すべてが未確定なら bool とみなす
        {
            // デフォルト値も未指定なので一般的なフラグオプションとして使うはず
            return false;
        }

        constexpr const std::optional<Parser>& get_parser() const noexcept
            requires( !std::is_void_v<Parser> )
        {
            return m_parser.value;
        }

        // 必須引数かどうか指定する。
        [[nodiscard]]
        constexpr Arg set_required(bool required) &&
        {
            m_required = required;
            return std::move(*this);
        }

        // デフォルト値を `D` 型の値として設定し、設定後の `Arg` 型の値を返す。
        // `T` が `void` だったとき、 `D` から推論される。
        template <class D>
        requires (
            std::is_void_v<T> &&
            std::is_void_v<Parser> &&
            !std::invocable<std::decay_t<D>> &&
            std::is_object_v<std::decay_t<D>>)
        [[nodiscard]]
        constexpr auto set_default(D&& default_value) &&
        {
            Arg<std::decay_t<D>, std::decay_t<D>, void> arg{ m_name, m_help };
            arg.m_required = m_required;
            arg.m_default.value.emplace(std::forward<D>(default_value));
            // m_parser は元々 void なので操作不要
            return std::move(arg);
        }

        // デフォルト値を `D` 型で設定し、設定後の `Arg` 型の値を返す。
        // `D` から `T` へ変換可能でなければならない。
        template <class D>
        requires (
            std::is_object_v<T> &&
            !std::invocable<std::decay_t<D>> &&
            std::convertible_to<std::decay_t<D>, T>)
        [[nodiscard]]
        constexpr auto set_default(D&& default_value) &&
        {
            Arg<T, std::decay_t<D>, Parser> arg{ m_name, m_help };
            arg.m_required = m_required;
            arg.m_default.value.emplace(std::forward<D>(default_value));
            if constexpr( value_parser_for<Parser, T> ) // Parser が設定済みであれば引き継ぐ
            {
                if( m_parser.value.has_value() )
                {
                    arg.m_parser.value.emplace(std::move(*m_parser.value));
                }
            }
            return std::move(arg);
        }

        // `T` 型のデフォルト値を `Args...` で直接構築して設定し、設定後の `Arg` 型の値を返す。
        // `Args...` で `T` を構築可能でなければならない。
        template <class ...Args>
        requires (
            std::is_object_v<T> &&
            std::is_constructible_v<T, Args...>
        )
        [[nodiscard]]
        constexpr auto set_default(std::in_place_t, Args&& ...args) &&
        {
            Arg<T, T, Parser> arg{ m_name, m_help };
            arg.m_required = m_required;
            arg.m_default.value.emplace(std::forward<Args>(args)...);
            if constexpr( value_parser_for<Parser, T> ) // Parser が設定済みであれば引き継ぐ
            {
                if( m_parser.value.has_value() )
                {
                    arg.m_parser.value.emplace(std::move(*m_parser.value));
                }
            }
            return std::move(arg);
        }

        // `T` のデフォルト値を得る関数 `F` を設定し、設定後の `Arg` 型の値を返す。
        // `T` は `F` の戻り値型から推論される。
        template <class F>
        requires (
            std::is_void_v<T> &&
            std::invocable<F> &&
            std::is_object_v<std::decay_t<std::invoke_result_t<F>>> &&
            std::convertible_to<std::invoke_result_t<F>, std::decay_t<std::invoke_result_t<F>>>
        )
        [[nodiscard]]
        constexpr auto set_default(F&& f) &&
        {
            using NewT = std::decay_t<std::invoke_result_t<F>>;
            Arg<NewT, F, Parser> arg{ m_name, m_help };
            arg.m_required = m_required;
            arg.m_default.value.emplace(std::forward<F>(f));
            if constexpr( value_parser_for<Parser, NewT> )
            {
                if( m_parser.value.has_value() )
                {
                    arg.m_parser.value.emplace(std::move(*m_parser.value));
                }
            }
            return std::move(arg);
        }

        // `T` のデフォルト値を得る関数 `F` を設定し、設定後の `Arg` 型の値を返す。
        // `F` の戻り値型から `T` へ変換可能でなければならない。
        template <class F>
        requires (
            std::is_object_v<T> &&
            std::invocable<F> &&
            std::convertible_to<std::invoke_result_t<F>, T>
        )
        [[nodiscard]]
        constexpr auto set_default(F&& f) &&
        {
            Arg<T, F, Parser> arg{ m_name, m_help };
            arg.m_required = m_required;
            arg.m_default.value.emplace(std::forward<F>(f));
            if constexpr( value_parser_for<Parser, T> )
            {
                if( m_parser.value.has_value() )
                {
                    arg.m_parser.value.emplace(std::move(*m_parser.value));
                }
            }
            return std::move(arg);
        }


        // 引数の変換関数 `P` を設定し、設定後の `Arg` 型の値を返す。
        // `T` が `void` だったとき、 `T` の型は `P` の戻り値から推論される。
        template <class P>
        requires (value_parser_for<P, T>)
        [[nodiscard]]
        constexpr auto set_parser(P&& p) &&
        {
            using NewT = detail::deduce_new_value_type_from_parser_t<T, P>;
            Arg<NewT, Default, P> arg{ m_name, m_help }; // TODO: Default を引き継ぎ可能かを判定して、void に戻すかコンパイルエラーにする
            arg.m_required = m_required;
            arg.m_parser.value.emplace(std::forward<P>(p));
            if constexpr( std::conjunction_v<std::negation<std::is_void<Default>>,std::is_constructible<NewT, Default>> )
            {
                if( m_default.value.has_value() )
                {
                    arg.m_default.value.emplace(std::move(*m_default.value));
                }
            }
            else if constexpr( std::invocable<Default> )
            {
                if constexpr( std::is_constructible_v<NewT, unwrap_noneable_t<std::invoke_result_t<Default>>> )
                {
                    arg.m_default.value = m_default.value;
                }
            }
            return std::move(arg);
        }

    };

    // deduction guide
    Arg(const char*, const char*) -> Arg<>;

    // `T` が `col::Arg` か調べる。
    template <class T>
    struct is_col_arg : std::false_type {};
    // `T` が `col::Arg` か調べる。
    template <class T, class D, class P>
    struct is_col_arg<col::Arg<T, D, P>> : std::true_type {};
    // `T` が `col::Arg` であれば `true` 、でなければ `false` 。
    template <class T>
    inline constexpr bool is_col_arg_v = is_col_arg<T>::value;

    // コマンドラインパーサー
    // 指定した引数型 `ArgTypes...` をもとにコンストラクタ引数を作成し、パース結果を指定の型へマッピングする。
    template <class ...ArgTypes>
    requires (is_col_arg_v<ArgTypes> && ...)
    class Command
    {
        std::string_view m_name;
        std::tuple<ArgTypes...> m_args;
    public:
        constexpr Command(NonEmptyString name, ArgTypes&& ...args)
        : m_name{ name }
        , m_args{ std::forward<ArgTypes>(args)... }
        {}

        // 引数の定義を追加する。
        template <class T, class D, class P>
        constexpr Command<ArgTypes..., Arg<T, D, P>> add(Arg<T, D, P>&& arg) &&
        {
            return std::apply([this]<class ...Ts>(Ts&& ...args) {
                return Command<ArgTypes..., Arg<T, D, P>>{m_name, std::forward<Ts>(args)...};
            }, std::tuple_cat(std::move(m_args), std::tuple{std::move(arg)}));
        }

        // コマンドライン引数 `R` をパースして `T` にマッピングする。
        // `"--help"` がコマンドライン引数に含まれていた場合パースを中断し `ShowHelp` として `ParseError` を返す。
        template <class T, class R>
        requires (
            std::is_constructible_v<T, std::conditional_t<std::is_void_v<typename ArgTypes::value_type>, bool, unwrap_noneable_t<typename ArgTypes::value_type>>...> &&
            requires {
                std::ranges::viewable_range<R>;
                std::convertible_to<range_const_reference_t<R>, std::string_view>;
            })
        constexpr std::expected<T, ParseError> parse(const R& argv) const
        {
            return parse_impl<T>(true, argv);
        }

        // コマンドライン引数 `R` をパースして `T` にマッピングする。
        // `parse()` とは異なり、 `"--help"` を自動で検出しない。
        template <class T, class R>
        requires (
            std::is_constructible_v<T, std::conditional_t<std::is_void_v<typename ArgTypes::value_type>, bool, unwrap_noneable_t<typename ArgTypes::value_type>>...> &&
            requires {
                std::ranges::viewable_range<R>;
                std::convertible_to<range_const_reference_t<R>, std::string_view>;
            })
        constexpr std::expected<T, ParseError> parse_without_help(const R& argv) const
        {
            return parse_impl<T>(false, argv);
        }

        constexpr std::string get_help_message() const
        {
            const auto usage = make_usage_string();
            const auto desc = make_arg_descriptions();
            return usage + "\n\n" + desc;
        }
    private:

        template <class T, class R>
        constexpr std::expected<T, ParseError> parse_impl(bool use_help, const R& argv) const
        {
            std::tuple<std::optional<std::conditional_t<std::is_void_v<typename ArgTypes::value_type>, bool, typename ArgTypes::value_type>>...> init_argument{};
            auto arg_init_zipped = pack_tuples(std::as_const(m_args), init_argument);

            auto iter = std::ranges::cbegin(argv);
            const auto last = std::ranges::cend(argv);

            while( iter != last )
            {
                if( use_help && std::string_view{*iter} == "--help" )
                {
                    return std::unexpected{ShowHelp{}};
                }

                constexpr std::size_t ArgCount = sizeof...(ArgTypes);
                bool parsed = false;
                for( std::size_t i = 0; i < ArgCount; ++i )
                {
                    const auto res = col::invoke_per_tuple_elements(i, arg_init_zipped,
                        []<class T1, class T2, class I, class S>(std::tuple<T1, T2>& t, I& it, const S& l)
                            -> std::expected<bool, ParseError>
                        {
                            // T1 は Arg<T, Default, Parser>
                            // T2 は std::optional<typename T1::value_type> のはず
                            // I は decltype(std::ranges::begin(R))
                            // S は decltype(std::ranges::last(R))
                            const auto& cfg = std::get<0>(t);
                            using ArgT = std::decay_t<decltype(cfg)>;
                            using ValueT = unwrap_noneable_t<typename ArgT::value_type>;
                            const std::string_view arg{*it};
                            if( cfg.get_name() == arg )
                            {
                                auto& value = std::get<1>(t);
                                if( value.has_value() )
                                {
                                    return std::unexpected{DuplicateArg{
                                        .name = cfg.get_name(),
                                    }};
                                }
                                if constexpr( std::is_void_v<ValueT> || std::same_as<ValueT, bool> )
                                {
                                    if constexpr( !std::is_void_v<ValueT> )
                                    {
                                        if( const auto& default_value = cfg.get_default();
                                            default_value.has_value() )
                                        {
                                            value = !default_value.value();
                                        }
                                        else
                                        {
                                            value = true;
                                        }
                                    }
                                    else
                                    {
                                        value = true;
                                    }
                                    std::ranges::advance(it, 1);
                                    return true;
                                }
                                else
                                {
                                    std::ranges::advance(it, 1);
                                    if( it == l )
                                    {
                                        return std::unexpected{NoValueGivenForOption{
                                            .name = cfg.get_name(),
                                        }};
                                    }
                                    using ParserT = typename ArgT::parser_type;
                                    if constexpr( !std::is_void_v<ParserT> )
                                    {
                                        // Parser != void ならそれを優先
                                        const auto& parser = cfg.get_parser();
                                        if( !parser.has_value() )
                                        {
                                            return std::unexpected{InvalidConfiguration{
                                                .name = cfg.get_name(),
                                                .kind = InvalidConfigKind::EmptyParser,
                                            }};
                                        }
                                        const auto parseRes = std::invoke(*parser, std::string_view{*it}.data());
                                        using ParseResT = std::decay_t<decltype(parseRes)>;
                                        if constexpr( std::same_as<ParseResT, std::expected<ValueT, ParseError>> )
                                        {
                                            if( parseRes.has_value() )
                                            {
                                                value.emplace(std::move(*parseRes));
                                                std::ranges::advance(it, 1);
                                                return true;
                                            }
                                            else
                                            {
                                                return std::unexpected{std::move(parseRes.error())};
                                            }
                                        }
                                        else if constexpr( std::same_as<ParseResT, std::optional<ValueT>> )
                                        {
                                            if( parseRes.has_value() )
                                            {
                                                value.emplace(std::move(*parseRes));
                                                std::ranges::advance(it, 1);
                                                return true;
                                            }
                                            else
                                            {
                                                // TODO: optional のときに規定のエラーメッセージを出力する
                                                return std::unexpected{ParserFailedWithNullopt{
                                                    .name = cfg.get_name(),
                                                    .arg = std::string_view{*it},
                                                }};
                                            }
                                        }
                                        else
                                        {
                                            static_assert(std::convertible_to<ParseResT, ValueT>, "Parser must return T");
                                            value.emplace(std::move(parseRes));
                                            std::ranges::advance(it, 1);
                                            return true;
                                        }
                                    }
                                    else if constexpr( std::is_constructible_v<ValueT, const char*> )
                                    {
                                        // Parser が指定されていなくても、 ValueT が const char* で構築できる型であれば、それを使って初期化。
                                        // e.g. std::string
                                        value.emplace(*it);
                                        std::ranges::advance(it, 1);
                                        return true;
                                    }
                                    else if constexpr( std::is_integral_v<ValueT> )
                                    {
                                        // ValueT が整数型なら、 std::char_conv() を使って構築
                                        const std::string_view valueStr{*it};
                                        const int base = [](std::string_view s) static noexcept {
                                            if( s.starts_with("0x") )
                                            {
                                                return 16;
                                            }
                                            // TODO: 他の進数にも対応する
                                            return 10;
                                        }(valueStr);
                                        ValueT parsedValue{};
                                        const auto[ ptr, ec ] = std::from_chars(valueStr.cbegin(), valueStr.cend(), parsedValue, base);
                                        if( ptr == valueStr.cend() && ec == std::errc{} )
                                        {
                                            // 成功
                                            value.emplace(std::move(parsedValue));
                                            std::ranges::advance(it, 1);
                                            return true;
                                        }
                                        else
                                        {
                                            return std::unexpected{InvalidNumber{
                                                .name = cfg.get_name(),
                                                .arg = valueStr,
                                                .err = ec,
                                            }};
                                        }
                                    }
                                    else if constexpr( std::is_floating_point_v<ValueT> )
                                    {
                                        // ValueT が浮動小数点型なら、 std::char_conv() を使って構築
                                        const std::string_view valueStr{*it};
                                        ValueT parsedValue{};
                                        // TODO: general で失敗するなら別の書式を順番に試していくようにする
                                        const auto[ ptr, ec ] = std::from_chars(valueStr.cbegin(), valueStr.cend(), parsedValue, std::chars_format::general);
                                        if( ptr == valueStr.cend() && ec == std::errc{} )
                                        {
                                            // 成功
                                            value.emplace(std::move(parsedValue));
                                            std::ranges::advance(it, 1);
                                            return true;
                                        }
                                        else
                                        {
                                            return std::unexpected{InvalidNumber{
                                                .name = cfg.get_name(),
                                                .arg = valueStr,
                                                .err = ec,
                                            }};
                                        }
                                    }
                                    else if constexpr( std::is_void_v<typename ArgT::default_type> )
                                    {
                                        static_assert(false, "unsupported type");
                                    }
                                }
                            }
                            return false;
                        }, iter, last);
                    if( !res.has_value() )
                    {
                        return std::unexpected{ std::move(res.error()) };
                    }
                    else if( *res == true )
                    {
                        parsed = true;
                        break;
                    }
                }
                if( !parsed )
                {
                    return std::unexpected{UnknownOption{
                        .arg = std::string_view{*iter},
                    }};
                }
            }

            // fill default value
            for( std::size_t i = 0; i < sizeof...(ArgTypes); ++i )
            {
                const auto res = invoke_per_tuple_elements(i, arg_init_zipped, []<class T1, class T2>(std::tuple<T1, T2>& t, std::size_t index)
                    -> std::optional<ParseError>
                {
                    const auto& cfg = std::get<0>(t);
                    auto& value = std::get<1>(t);
                    if( value.has_value() )
                    {
                       return std::nullopt;
                    }
                    else if( cfg.get_required() )
                    {
                        return RequiredOption{
                            .name = cfg.get_name(),
                        };
                    }
                    using DefaultType = typename std::decay_t<T1>::default_type;
                    using ValueType = typename std::decay_t<T1>::value_type;
                    // D == void のとき、必ず !has_value() 。T がデフォルト構築可能ならそれで初期化。それもできないなら引数不足
                    // T == D であり自動導出された可能性があるなら、デフォルト値はない可能性がある
                    // T == D でも能動的に設定している可能性はあるので has_value() ならそれを使う
                    // T != D のとき、手動設定されているはずなので、!has_value() は何らかの設定エラー
                    // T != D のとき、has_value() のとき、
                    // D が関数ならそれを呼び出す
                    // T が値ならそれを使って構築する
                    if constexpr( !std::is_void_v<DefaultType> )
                    {
                        const auto& default_value = cfg.get_default();
                        if constexpr( std::is_invocable_v<DefaultType> )
                        {
                            if( default_value.has_value() )
                            {
                                value.emplace(std::invoke(*default_value));
                                return std::nullopt;
                            }
                        }
                        else if constexpr( std::is_default_constructible_v<ValueType> )
                        {
                            // デフォルト構築可能な T では、Arg の型推論で自動的に Default が埋まるが、
                            // 実際にデフォルト値が設定されているわけではないので「デフォルト値が指定されていないエラー」になってしまう。
                            // デフォルト構築可能であってもコンストラクタのコストは大きい可能性があるので、
                            // ここで処理して parse 実行時まで遅延させる。
                            if( default_value.has_value() )
                            {
                                value.emplace(*default_value);
                            }
                            else
                            {
                                value.emplace();
                            }
                            return std::nullopt;
                        }
                        else
                        {
                            if( default_value.has_value() )
                            {
                                value.emplace(*default_value);
                                return std::nullopt;
                            }
                        }

                        return InvalidConfiguration{
                            .name = cfg.get_name(),
                            .kind = InvalidConfigKind::EmptyDefault,
                        };
                    }
                    else if constexpr( std::is_default_constructible_v<ValueType> )
                    {
                        value.emplace();
                        return std::nullopt;
                    }
                    else
                    {
                        return NotEnoughArgument{
                            .index = index,
                            .name = cfg.get_name(),
                        };
                    }
                }, i);

                if( res.has_value() )
                {
                    return std::unexpected{*res};
                }
            }

            // TODO: none な optional のまま渡せる場合を考慮する
            return std::apply([]<class ...Ts>(std::optional<Ts>&& ...args) {
                return T { std::forward<Ts>(*args)... };
            }, std::move(init_argument));
        }

        constexpr std::string make_usage_string() const
        {
            constexpr auto make_usage = []<class T, class D, class P>(const Arg<T, D, P>& arg) {
                // flag: [--name]
                // option: [--name NAME]
                // option(required): --name NAME
                std::string usage{" "};
                if( !arg.get_required() )
                {
                    usage.append("[");
                }
                usage.append(arg.get_name());
                if constexpr( !std::same_as<T, bool> )
                {
                    usage.append(" ");
                    usage.append_range(arg.get_name().substr(2) | std::views::transform([](char c) {
                        if( 'a' <= c && c <= 'z' )
                        {
                            return static_cast<char>('A' + (c - 'a'));
                        }
                        else
                        {
                            return c;
                        }
                    }));
                }
                if( !arg.get_required() )
                {
                    usage.append("]");
                }
                return usage;
            };

            return std::apply([this, make_usage]<class ...Args>(Args& ...args) {
                return std::string{m_name} + ( make_usage(args) + ... );
            }, m_args);
        }

        constexpr std::string make_arg_descriptions() const
        {
            constexpr auto make_description = []<class T, class D, class P>(const Arg<T, D, P>& arg) {
                /*
                """
                    --name    help_message
                        (required)
                        default: <DEFAULT_VALUE>

                """
                */
                std::string desc{"    "};
                desc.append(arg.get_name());
                desc.append("    ");
                desc.append(arg.get_help());
                desc.append("\n");
                if( arg.get_required() )
                {
                    desc.append("        (required)\n");
                }
                if constexpr( !std::is_void_v<D> )
                {
                    const auto default_value = arg.get_default();
                    if( default_value.has_value() )
                    {
                        desc.append("        default: ");
                        if constexpr( std::is_convertible_v<D, std::string_view> )
                        {
                            std::string_view str{ *default_value };
                            desc.append(str);
                            desc.append("\n");
                        }
                        else
                        {
                            desc.append(" <DEFAULT_VALUE>\n");
                        }
                    }
                }
                desc.append("\n");
                return desc;
            };

            return std::apply([make_description]<class ...Args>(const Args& ...args) {
                return std::string{"options:\n"} + ( make_description(args) + ... );
            }, m_args);
        }
    };

    // コマンドラインパーサー
    // 指定した引数型 `ArgTypes...` をもとにコンストラクタ引数を作成し、パース結果を指定の型へマッピングする。
    template <>
    class Command<>
    {
        std::string_view m_name;
    public:
        constexpr Command(NonEmptyString name) noexcept
        : m_name{ name }
        {}

        // 引数の定義を追加する。
        template <class T, class D, class P>
        constexpr Command<Arg<T, D, P>> add(Arg<T, D, P>&& arg)
        {
            return Command<Arg<T, D, P>>{m_name, std::move(arg)};
        }
    };

    // deduction guide
    Command(const char*) -> Command<>;
    Command(std::string_view) -> Command<>;
    Command(NonEmptyString) -> Command<>;

    template <class ...ArgTypes>
    Command(ArgTypes&& ...) -> Command<ArgTypes...>;

}

