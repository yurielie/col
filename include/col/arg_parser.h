#pragma once

#include <algorithm>
#include <charconv>
#include <concepts>
#include <expected>
#include <format>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace col {

    namespace detail {
        
        template <class T>
        struct is_optional_type : std::false_type{};
        template <class T>
        struct is_optional_type<std::optional<T>> : std::true_type{};
        template <class T>
        inline constexpr bool is_optional_type_v = is_optional_type<T>::value;

        template <class T>
        struct is_variant_type : std::false_type{};
        template <class ...Ts>
        struct is_variant_type<std::variant<Ts...>> : std::true_type{};
        template <class T>
        inline constexpr bool is_variant_type_v = is_variant_type<T>::value;

        template <class T, class CharT = std::string::value_type>
        constexpr std::basic_string<CharT> format_wrap(const T& value)
        {
            if constexpr( std::convertible_to<T, std::basic_string<CharT>> )
            {
                // std::basic_string<CharT> に変換可能なのであれば constexpr になる可能性がある
                return value;
            }
            else if constexpr( std::formattable<T, CharT> )
            {
                return std::format("{}", value);
            }
            else if constexpr( std::is_pointer_v<T> )
            {
                return "<pointer_type>";
            }
            else if constexpr( std::is_enum_v<T> )
            {
                return std::format("{}", std::to_underlying(value));
            }
            else if constexpr( ::col::detail::is_optional_type_v<T> )
            {
                return format_wrap<typename T::value_type>(*value);
            }
            else if constexpr( ::col::detail::is_variant_type_v<T> )
            {
                return std::visit([](const auto& v) static
                {
                    return format_wrap(v);
                }, value);
            }
            else
            {
                return "<?>";
            }
        }

    }

    class FlagConfig
    {
        const char* m_name;
        const char* m_help;
        bool m_default_value;

    public:
        using value_type = bool;

        constexpr explicit FlagConfig(const char* name, const char* help) noexcept
        : m_name{ name }
        , m_help{ help }
        , m_default_value{ false }
        {}

        constexpr FlagConfig& set_default_value(bool default_value) & noexcept
        {
            m_default_value = default_value;
            return *this;
        }
        constexpr FlagConfig&& set_default_value(bool default_value) && noexcept
        {
            m_default_value = default_value;
            return std::move(*this);
        }

        constexpr const char* get_name() const noexcept
        {
            return m_name;
        }
        constexpr const char* get_help() const noexcept
        {
            return m_help;
        }
        constexpr bool get_default_value() const noexcept
        {
            return m_default_value;
        }

        constexpr std::string get_usage_message() const
        {
            return std::string{"["} + m_name + "]";
        }

        constexpr std::string get_help_message() const
        {
            auto message = std::string{"\n  "} + m_name + "      " + m_help;
            if( m_default_value == true )
            {
                message += " (default: true)";
            }
            return message;
        }

    };

    template <class F, class T>
    concept converter_for =
        requires (T, F f, const char* arg) {
            { std::invoke(std::forward<F>(f), arg) } -> std::convertible_to<std::expected<T, std::string>>;
        } || requires (T, F f, const char* arg) {
            { std::invoke(std::forward<F>(f), arg) } -> std::convertible_to<T>;
        };

    template <class T, class F = std::expected<T, std::string>(*)(const char*)>
    requires (converter_for<F, T>)
    class OptionConfig
    {
        static_assert(std::is_reference_v<T> == false, "T must not be a reference type.");

        const char* m_name;
        const char* m_value_name;
        const char* m_help;
        bool m_required;

        F m_converter;
        std::optional<T> m_default_value;

    public:
        using value_type = T;

        constexpr explicit OptionConfig(const char* name, const char* value_name, const char* help) noexcept
        : m_name{ name }
        , m_value_name{ value_name }
        , m_help{ help }
        , m_required{ false }
        , m_converter{ [](const char* a) static -> std::expected<T, std::string> {
            const std::string_view arg{a};
            if constexpr (std::integral<T>) {
                const auto[ base, str ] = [&]() -> std::pair<int, std::string_view>
                {
                    if( arg.starts_with("0x") )
                    {
                        return {16, arg.substr(2)};
                    }
                    else if( arg.starts_with("0b") )
                    {
                        return {2, arg.substr(2)};
                    }
                    return { 10, arg };
                }();
                T value{};
                const auto[ ptr, ec] = std::from_chars(std::ranges::cbegin(str), std::ranges::cend(str), value);
                if( ec == std::errc{} )
                {
                    return std::expected<T, std::string>{ value };
                }
                else if( ec == std::errc::invalid_argument )
                {
                    return std::unexpected{"invalid argument"};
                }
                else if( ec == std::errc::result_out_of_range )
                {
                    return std::unexpected{"out of range"};
                }
                else
                {
                    return std::unexpected{"unexpected error"};
                }
            } else if constexpr(std::floating_point<T>) {
                T value{};
                const auto[ptr, ec] = std::from_chars(std::ranges::cbegin(arg), std::ranges::cend(arg), value);
                if( ec == std::errc{} )
                {
                    return std::expected<T, std::string>{ value };
                }
                else if( ec == std::errc::invalid_argument )
                {
                    return std::unexpected{"invalid argument"};
                }
                else if( ec == std::errc::result_out_of_range )
                {
                    return std::unexpected{"out of range"};
                }
                else
                {
                    return std::unexpected{"unexpected error"};
                }
            } else if constexpr( std::convertible_to<const char*, T> ) {
                return std::expected<T, std::string>{ arg.data() };
            } else {
                return std::unexpected{"unexpected error"};
            }
        } }
        , m_default_value{ std::nullopt }
        {}

        template <class G = F>
        requires (converter_for<F, T>)
        constexpr explicit OptionConfig<T, F>(const OptionConfig<T, G>& config, F&& f)
            noexcept(
                std::is_nothrow_constructible_v<F, decltype(std::forward<F>(f))>
                && std::is_nothrow_constructible_v<std::optional<T>> )
        : m_name{ config.get_name() }
        , m_value_name{ config.get_value_name() }
        , m_help{ config.get_help() }
        , m_required{ config.is_required() }
        , m_converter{ std::forward<F>(f) }
        , m_default_value{ config.get_default_value() }
        {}

        template <class G = F>
        requires (converter_for<F, T>)
        constexpr explicit OptionConfig<T, F>(OptionConfig<T, G>&& config, F&& f)
            noexcept(
                std::is_nothrow_constructible_v<F, decltype(std::forward<F>(f))>
                && std::is_nothrow_constructible_v<std::optional<T>> )
        : m_name{ config.get_name() }
        , m_value_name{ config.get_value_name() }
        , m_help{ config.get_help() }
        , m_required{ config.is_required() }
        , m_converter{ std::forward<F>(f) }
        , m_default_value{ config.get_default_value() }
        {}

        constexpr OptionConfig(OptionConfig<T, F>&&)
            noexcept(
                std::is_nothrow_move_constructible_v<F>
                && std::is_nothrow_move_constructible_v<std::optional<T>> )
        = default;

        constexpr OptionConfig<T, F>& operator=(OptionConfig<T, F>&&)
            noexcept(
                std::is_nothrow_move_constructible_v<F>
                && std::is_nothrow_move_constructible_v<std::optional<T>> )
        = default; 

        
        constexpr OptionConfig& set_required(bool required) & noexcept
        {
            m_required = required;
            return *this;
        }
        constexpr OptionConfig&& set_required(bool required) &&
            noexcept(std::is_nothrow_move_constructible_v<OptionConfig>)
        {
            m_required = required;
            return std::move(*this);
        }

        template <class ...Args>
        requires (std::is_constructible_v<T, Args...>)
        constexpr OptionConfig& set_default_value(Args&& ...args) &
            noexcept(std::is_nothrow_constructible_v<T, Args...>)
        {
            m_default_value.emplace(std::forward<Args>(args)...);
            m_required = false;
            return *this;
        }
        template <class ...Args>
        requires (std::is_constructible_v<T, Args...>)
        constexpr OptionConfig&& set_default_value(Args&& ...args) &&
            noexcept(std::is_nothrow_constructible_v<T, Args...> && std::is_nothrow_move_constructible_v<OptionConfig>)
        {
            m_default_value.emplace(std::forward<Args>(args)...);
            m_required = false;
            return std::move(*this);
        }

        constexpr const char* get_name() const noexcept
        {
            return m_name;
        }
        constexpr const char* get_value_name() const noexcept
        {
            return m_value_name;
        }
        constexpr const char* get_help() const noexcept
        {
            return m_help;
        }
        constexpr bool is_required() const noexcept
        {
            return m_required;
        }

        constexpr std::optional<T> get_default_value() const
            noexcept(std::is_nothrow_copy_constructible_v<std::optional<T>>)
        {
            return m_default_value;
        }

        template <class G = F>
        requires (converter_for<G, T>)
        constexpr OptionConfig<T, G> set_converter(G&& f) &
            noexcept(std::is_nothrow_constructible_v<OptionConfig<T, G>, const OptionConfig<T, F>&, G>)
        {
            return OptionConfig<T, G>{ *this, std::forward<G>(f) };
        }

        template <class G = F>
        requires (converter_for<G, T>)
        constexpr OptionConfig<T, G> set_converter(G&& f) &&
            noexcept(std::is_nothrow_constructible_v<OptionConfig<T, G>, OptionConfig<T, F>&&, G>)
        {
            return OptionConfig<T, G>{ std::move(*this), std::forward<G>(f) };
        }

        constexpr auto call_converter(const char* arg) const
            noexcept(
                std::is_nothrow_invocable_v<decltype(m_converter), decltype(arg)>
                && std::is_nothrow_constructible_v<
                        std::expected<T, std::string>, std::invoke_result_t<decltype(m_converter), decltype(arg)>>)
        -> decltype(std::invoke(m_converter, arg))
        {
            if constexpr( std::is_pointer_v<F> )
            {
                if( m_converter == nullptr )
                {
                    return std::unexpected("converter callback is nullptr.");
                }
            }
            if constexpr( std::same_as<std::invoke_result_t<decltype(m_converter), decltype(arg)>, std::expected<T, std::string>> )
            {
                return std::invoke(m_converter, arg);
            }
            else
            {
                return std::expected<T, std::string>{ std::invoke(m_converter, arg) };
            }
        }

        constexpr std::string get_usage_message() const
        {
            auto message = std::string{m_name} + " " + m_value_name;
            // 必須引数に指定されていたら(T が optional 型でも) `[]` で囲む。
            // T が optional 型ではないのにデフォルト値が指定されていないなら `[]` で囲む。
            return ( m_required || (!detail::is_optional_type_v<T> && !m_default_value.has_value()) )
                    ? message : "[" + message + "]";
        }

        constexpr std::string get_help_message() const
        {
            auto message = std::string{"\n  "} + m_name + " " + m_value_name + "      " + m_help;
            if( m_required )
            {
                message += " (required)";
            }
            if( m_default_value.has_value() )
            {
                message += " (default: " + detail::format_wrap(*m_default_value) + ")";
            }
            return message;
        }

    };


    namespace detail {

        template <class ...Configs, std::size_t ...Idx>
        constexpr auto init_args_tuple_impl(const std::tuple<Configs...>& configs, std::index_sequence<Idx...>)
        {
            return std::make_tuple(
                std::pair<std::optional<typename Configs::value_type>, const Configs&>{
                    std::nullopt, std::cref(std::get<Idx>(configs))
                }...);
        }

        template <class ...Configs>
        constexpr std::tuple<std::pair<std::optional<typename Configs::value_type>, const Configs&>...>
        init_args_tuple(const std::tuple<Configs...>& configs)
        {
            return init_args_tuple_impl(configs, std::index_sequence_for<Configs...>{});
        }

        template <class T, class Config>
        constexpr void set_default_if_defined_one(std::pair<std::optional<T>, const Config&>& p)
        {
            auto& val = p.first;
            const auto& cfg = p.second;
            if( !val.has_value() )
            {
                val = cfg.get_default_value();
            }
        }

        template <std::size_t ...Idx, class ...Ts, class ...Configs>
        constexpr void
        set_default_if_defined(std::tuple<std::pair<std::optional<Ts>, const Configs&>...>& configs, std::index_sequence<Idx...>)
        {
            ( (set_default_if_defined_one(std::get<Idx>(configs))), ...);
        }

        template <class T, class Config>
        constexpr bool check_args_tuple_initialized_one(const std::optional<T>& arg, const Config& cfg)
        {
            if constexpr(std::is_same_v<Config, OptionConfig<T>>)
            {
                if constexpr(detail::is_optional_type_v<T>)
                {
                    return !cfg.is_required() || (arg.has_value() && (*arg).has_value());
                }
                else
                {
                    return arg.has_value();
                }
            }
            else
            {
                return arg.has_value();
            }
        }

        template <class ...Ts, class ...Configs>
        constexpr bool check_args_tuple_initialized(const std::tuple<std::pair<std::optional<Ts>, Configs>...>& t)
        {
            return std::apply(
                [](const auto& ...p) static
                {
                    return ( check_args_tuple_initialized_one(p.first, p.second) && ... && true);
                },
                t);
        }

        template <class ...Ts, class ...Configs>
        constexpr std::tuple<Ts...> make_init_tuple(std::tuple<std::pair<std::optional<Ts>, Configs>...>&& t)
        {
            return std::apply(
                [](auto&& ...p) static
                {
                    return std::tuple<Ts...>{ std::move(*(p.first))... };
                },
                std::move(t));
        }
        
        template <std::size_t Index, std::size_t ...Idx, class I, class S, class ...Ts, class ...Configs>
        requires (std::input_iterator<I> && std::sentinel_for<S, I>)
        constexpr auto matches_configs_impl(
            I& iter, S last,
            std::tuple<std::pair<std::optional<Ts>, const Configs&>...>& init_args
        ) -> std::expected<bool, std::string>
        {
            auto& p = std::get<Index>(init_args);
            auto& val = p.first;
            const auto& config = p.second;

            const std::string_view arg{*iter};
            if( arg == config.get_name() )
            {
                std::ranges::advance(iter, 1);
                if constexpr( std::is_same_v<std::remove_cvref_t<decltype(config)>, FlagConfig> )
                {
                    val = true;
                    return std::expected<bool, std::string>{ true };
                }
                else
                {
                    if( iter == last )
                    {
                        return std::unexpected{ "no value given to option args" };
                    }
                    const auto res = config.call_converter(std::string_view{*iter}.data());
                    if( res.has_value() )
                    {
                        if( val.has_value() )
                        {
                            return std::unexpected("already initialized");
                        }
                        val = *res;
                        std::ranges::advance(iter, 1);
                        return std::expected<bool, std::string>{ true };
                    }
                    else
                    {
                        return std::unexpected{ res.error() };
                    }
                }
            }
            if constexpr(sizeof...(Idx) > 0 )
            {
                return matches_configs_impl<Idx...>(iter, last, init_args);
            }
            else
            {
                return std::expected<bool, std::string>{ false };
            }
        }

        template <class I, class S, class ...Ts, class ...Configs, std::size_t ...Idx>
        requires (std::input_iterator<I> && std::sentinel_for<S, I>)
        constexpr std::expected<bool, std::string> matches_configs(I& iter, S last,
            std::tuple<std::pair<std::optional<Ts>, const Configs&>...>& init_args,
            std::index_sequence<Idx...>)
        {
            return matches_configs_impl<Idx...>(iter, last, init_args);
        }

    } // namespace detail

    template <class T>
    struct is_flag_config : std::false_type{};
    template <>
    struct is_flag_config<::col::FlagConfig> : std::true_type{};
    template <class T>
    inline constexpr bool is_flag_config_v = is_flag_config<T>::value;

    template <class T>
    struct is_option_config : std::false_type{};
    template <class T, class F>
    struct is_option_config<::col::OptionConfig<T, F>> : std::true_type{};
    template <class T>
    inline constexpr bool is_option_config_v = is_option_config<T>::value;

    template <class T>
    struct is_config_type : std::disjunction<is_flag_config<T>, is_option_config<T>>{};
    template <class T>
    inline constexpr bool is_config_type_v = is_config_type<T>::value;

    template <class ...Configs>
    requires (is_config_type_v<Configs> && ...)
    class ArgParser
    {
        std::tuple<Configs...> m_configs;

    public:
        constexpr explicit ArgParser() noexcept
        {}
        constexpr explicit ArgParser(std::tuple<Configs...>&& configs)
            noexcept(std::is_nothrow_move_constructible_v<std::tuple<Configs...>>)
        : m_configs(std::move(configs))
        {}

        constexpr std::string get_usage_message() const
        {
            if constexpr( sizeof...(Configs) > 0 )
            {
                return std::apply([](const auto& ...configs) static
                {
                    std::array usages{ configs.get_usage_message()... };
                    return std::ranges::fold_left(usages | std::ranges::views::drop(1), usages[0],
                        [](auto lhs, auto rhs) static
                        {
                            return lhs + " " + rhs;
                        });
                }, m_configs);
            }
            else
            {
                return "";
            }
        }

        constexpr std::string get_help_message() const
        {
            if constexpr( sizeof...(Configs) > 0 )
            {
                return std::apply([](const auto& ...configs) static
                    {
                        return (configs.get_help_message() + ...);
                    }, m_configs);
            }
            else
            {
                return "";
            }
        }

        constexpr ArgParser<Configs..., FlagConfig> add_config(const FlagConfig& config) &
            noexcept(noexcept(ArgParser<Configs..., FlagConfig>{
                std::tuple_cat(m_configs, std::tuple<FlagConfig>{ config }) 
            }))
        {
            return ArgParser<Configs..., FlagConfig>{
                std::tuple_cat(m_configs, std::tuple<FlagConfig>{ config })
            };
        }

        constexpr ArgParser<Configs..., FlagConfig> add_config(const FlagConfig& config) &&
            noexcept(noexcept(ArgParser<Configs..., FlagConfig>{
                std::tuple_cat(std::move(m_configs), std::tuple<FlagConfig>{ config }) 
            }))
        {
            return ArgParser<Configs..., FlagConfig>{
                std::tuple_cat(std::move(m_configs), std::tuple<FlagConfig>{ config })
            };
        }

        constexpr ArgParser<Configs..., FlagConfig> add_config(FlagConfig&& config) &
            noexcept(noexcept(ArgParser<Configs..., FlagConfig>{std::tuple_cat(m_configs,
                    std::tuple<FlagConfig>{ std::move(config) })}))
        {
            return ArgParser<Configs..., FlagConfig>{
                std::tuple_cat(m_configs, std::tuple<FlagConfig>{ std::move(config) })
            };
        }

        constexpr ArgParser<Configs..., FlagConfig> add_config(FlagConfig&& config) &&
            noexcept(noexcept(ArgParser<Configs..., FlagConfig>{std::tuple_cat(std::move(m_configs),
                    std::tuple<FlagConfig>{ std::move(config) })}))
        {
            return ArgParser<Configs..., FlagConfig>{
                std::tuple_cat(std::move(m_configs), std::tuple<FlagConfig>{ std::move(config) })
            };
        }

        template <class T, class F>
        constexpr ArgParser<Configs..., OptionConfig<T, F>> add_config(const OptionConfig<T, F>& config) &
            noexcept(noexcept(ArgParser<Configs..., OptionConfig<T, F>>{
                std::tuple_cat(m_configs, std::tuple<OptionConfig<T, F>>{ config }) 
            }))
        {
            return ArgParser<Configs..., OptionConfig<T, F>>{
                std::tuple_cat(m_configs, std::tuple<OptionConfig<T, F>>{ config })
            };
        }

        template <class T, class F>
        constexpr ArgParser<Configs..., OptionConfig<T, F>> add_config(const OptionConfig<T, F>& config) &&
            noexcept(noexcept(ArgParser<Configs..., OptionConfig<T, F>>{
                std::tuple_cat(std::move(m_configs), std::tuple<OptionConfig<T, F>>{ config }) 
            }))
        {
            return ArgParser<Configs..., OptionConfig<T, F>>{
                std::tuple_cat(std::move(m_configs), std::tuple<OptionConfig<T, F>>{ config })
            };
        }

        template <class T, class F>
        constexpr ArgParser<Configs..., OptionConfig<T, F>> add_config(OptionConfig<T, F>&& config) &
            noexcept(noexcept(ArgParser<Configs..., OptionConfig<T, F>>{std::tuple_cat(m_configs,
                    std::tuple<OptionConfig<T, F>>{ std::move(config) })}))
        {
            return ArgParser<Configs..., OptionConfig<T, F>>{
                std::tuple_cat(m_configs, std::tuple<OptionConfig<T, F>>{ std::move(config) })
            };
        }

        template <class T, class F>
        constexpr ArgParser<Configs..., OptionConfig<T, F>> add_config(OptionConfig<T, F>&& config) &&
            noexcept(noexcept(ArgParser<Configs..., OptionConfig<T, F>>{std::tuple_cat(std::move(m_configs),
                    std::tuple<OptionConfig<T, F>>{ std::move(config) })}))
        {
            return ArgParser<Configs..., OptionConfig<T, F>>{
                std::tuple_cat(std::move(m_configs), std::tuple<OptionConfig<T, F>>{ std::move(config) })
            };
        }

        template <class T, class R>
        requires (std::is_constructible_v<T, typename Configs::value_type...> && std::ranges::input_range<R>)
        constexpr std::expected<T, std::string> parse(R range) const
        {
            auto init_args = detail::init_args_tuple(m_configs);
            auto iter = std::ranges::cbegin(range);
            const auto last = std::ranges::cend(range);

            while( iter != last )
            {
                const auto res = detail::matches_configs(iter, last, init_args, std::index_sequence_for<Configs...>{});
                if( !res.has_value() )
                {
                    return std::unexpected(res.error());
                }
                else if( *res == false )
                {
                    return std::unexpected("unexpected argument: any configrations matches it.");
                }
            }

            detail::set_default_if_defined(init_args, std::index_sequence_for<Configs...>{});
            if( detail::check_args_tuple_initialized(init_args) )
            {
                return std::expected<T, std::string>{ std::make_from_tuple<T>(detail::make_init_tuple(std::move(init_args))) };
            }
            return std::unexpected("args uninitialized");
        }

    };
    template <class ...Configs>
    ArgParser(Configs...) -> ArgParser<std::remove_cvref_t<Configs>...>;

} // namespace col
