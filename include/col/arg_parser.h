#pragma once

#include <charconv>
#include <expected>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace col {

    template <class T>
    struct is_optional_type : std::false_type{};
    template <class T>
    struct is_optional_type<std::optional<T>> : std::true_type{};
    template <class T>
    inline constexpr bool is_optional_type_v = is_optional_type<T>::value;

    class FlagConfig
    {
        const char* m_name;
        const char* m_help;

    public:
        using value_type = bool;

        consteval explicit FlagConfig(const char* name, const char* help) noexcept
        : m_name{ name }
        , m_help{ help }
        {}

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
            return false;
        }

    };


    template <class T, class F = std::expected<T, std::string>(*)(std::string_view) noexcept>
    requires (std::same_as<std::expected<T, std::string>, std::invoke_result_t<F, const char*>>)
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

        consteval explicit OptionConfig(const char* name, const char* value_name, const char* help) noexcept
        : m_name{ name }
        , m_value_name{ value_name }
        , m_help{ help }
        , m_required{ false }
        , m_converter{ [](std::string_view arg) noexcept -> std::expected<T, std::string> {
            if constexpr (std::integral<T>) {
                const auto[ base, str ] = [&]() noexcept -> std::pair<int, std::string_view>
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
                const auto[ ptr, ec] = std::from_chars(std::ranges::cbegin(arg), std::ranges::cend(arg), value);
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
                return std::unexpected("unsupported type for default parsing.");
            }
        } }
        , m_default_value{ std::nullopt }
        {}

        template <class G = F>
        consteval explicit OptionConfig<T, F>(OptionConfig<T, G>&& config, F&& f) noexcept
        : m_name{ config.get_name() }
        , m_value_name{ config.get_value_name() }
        , m_help{ config.get_help() }
        , m_required{ config.is_required() }
        , m_converter{ std::forward<F>(f) }
        , m_default_value{ config.get_default_value() }
        {}

        constexpr OptionConfig(OptionConfig<T, F>&&) noexcept = default;
        constexpr OptionConfig<T, F>& operator=(OptionConfig<T, F>&&) noexcept = default; 

        
        consteval OptionConfig& set_required(bool required) & noexcept
        {
            m_required = required;
            return *this;
        }
        consteval OptionConfig&& set_required(bool required) && noexcept
        {
            m_required = required;
            return std::move(*this);
        }

        template <class ...Args>
        requires (std::is_constructible_v<T, Args...>)
        consteval OptionConfig& set_default_value(Args&& ...args) & noexcept
        {
            m_default_value.emplace(std::forward<Args>(args)...);
            return *this;
        }
        template <class ...Args>
        requires (std::is_constructible_v<T, Args...>)
        consteval OptionConfig&& set_default_value(Args&& ...args) && noexcept
        {
            m_default_value.emplace(std::forward<Args>(args)...);
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

        constexpr std::optional<T> get_default_value() const noexcept
        {
            return m_default_value;
        }

        template <class G = F>
        consteval OptionConfig<T, G> set_converter(G&& f) & noexcept
        {
            return OptionConfig<T, G>{ *this, std::forward<G>(f) };
        }

        template <class G = F>
        consteval OptionConfig<T, G> set_converter(G&& f) && noexcept
        {
            return OptionConfig<T, G>{ std::move(*this), std::forward<G>(f) };
        }

        constexpr auto call_converter(const char* arg) const noexcept -> decltype(std::invoke(m_converter, arg))
        {
            return std::invoke(m_converter, arg);
        }

    };


    template <class ...Configs, std::size_t ...Idx>
    constexpr auto init_args_tuple_impl(const std::tuple<Configs...>& configs, std::index_sequence<Idx...>) noexcept
    {
        return std::make_tuple(std::pair<std::optional<typename Configs::value_type>, const Configs&>(std::nullopt, std::cref(std::get<Idx>(configs)))...);
    }

    template <class ...Configs>
    constexpr std::tuple<std::pair<std::optional<typename Configs::value_type>, const Configs&>...> init_args_tuple(const std::tuple<Configs...>& configs) noexcept
    {
        return init_args_tuple_impl(configs, std::index_sequence_for<Configs...>{});
    }

    template <class T, class Config>
    constexpr void set_default_if_defined_one(std::pair<std::optional<T>, const Config&>& p) noexcept
    {
        auto& val = p.first;
        const auto& cfg = p.second;
        if constexpr( std::is_same_v<Config, col::FlagConfig> )
        {
            if( !val.has_value() )
            {
                val = cfg.get_default_value();
            }
        }
        else
        {
            if( !val.has_value() )
            {
                val = cfg.get_default_value();
            }
        }
    }

    template <std::size_t ...Idx, class ...Ts, class ...Configs>
    constexpr void set_default_if_defined(std::tuple<std::pair<std::optional<Ts>, const Configs&>...>& configs, std::index_sequence<Idx...>) noexcept
    {
        ( (set_default_if_defined_one(std::get<Idx>(configs))), ...);
    }

    template <class T, class Config>
    constexpr bool check_args_tuple_initialized_one(const std::optional<T>& arg, const Config& cfg) noexcept
    {
        if constexpr(std::is_same_v<Config, OptionConfig<T>>)
        {
            if constexpr(is_optional_type_v<T>)
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
    constexpr bool check_args_tuple_initialized(const std::tuple<std::pair<std::optional<Ts>, Configs>...>& t) noexcept
    {
        return std::apply(
            [](const auto& ...p) noexcept
            {
                return ( check_args_tuple_initialized_one(p.first, p.second) && ... && true);
            },
            t);
    }

    template <class ...Ts, class ...Configs>
    constexpr std::tuple<Ts...> make_init_tuple(std::tuple<std::pair<std::optional<Ts>, Configs>...>&& t) noexcept
    {
        return std::apply(
            [](auto&& ...p) noexcept
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
    ) noexcept -> std::expected<bool, std::string>
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
        std::index_sequence<Idx...>) noexcept
    {
        return matches_configs_impl<Idx...>(iter, last, init_args);
    }

    template <class ...Configs>
    class ArgParser
    {
        std::tuple<Configs...> m_configs;

    public:
        consteval explicit ArgParser() noexcept
        {}
        consteval explicit ArgParser(std::tuple<Configs...>&& configs) noexcept
        : m_configs(std::move(configs))
        {}

        template <class Config>
        requires (requires (Config c) { typename Config::value_type; })
        consteval ArgParser<Configs..., Config> add_config(Config&& config) && noexcept
        {
            return ArgParser<Configs..., Config>{ std::tuple_cat(std::move(m_configs), std::forward_as_tuple<Config>(std::forward<Config>(config))) };
        }

        template <class Config>
        requires (requires (Config c) { typename Config::value_type; })
        consteval ArgParser<Configs..., Config> add_config(Config&& config) & noexcept
        {
            return ArgParser<Configs..., Config>{ std::tuple_cat(m_configs, std::forward_as_tuple<Config>(std::forward<Config>(config))) };
        }


        template <class T, class R>
        requires (std::is_constructible_v<T, typename Configs::value_type...> && std::ranges::borrowed_range<R>)
        constexpr std::expected<T, std::string> parse(R range) const noexcept
        {
            auto init_args = init_args_tuple(m_configs);
            auto iter = std::ranges::cbegin(range);
            const auto last = std::ranges::cend(range);

            while( iter != last )
            {
                const auto res = matches_configs(iter, last, init_args, std::index_sequence_for<Configs...>{});
                if( !res.has_value() )
                {
                    return std::unexpected(res.error());
                }
                else if( *res == false )
                {
                    return std::unexpected("unexpected argument: any configrations matches it.");
                }
            }

            set_default_if_defined(init_args, std::index_sequence_for<Configs...>{});
            if( check_args_tuple_initialized(init_args) )
            {
                return std::expected<T, std::string>{ std::make_from_tuple<T>(make_init_tuple(std::move(init_args))) };
            }
            return std::unexpected("args uninitialized");
        }

    };
    template <class ...Configs>
    ArgParser(Configs...) -> ArgParser<Configs...>;



}
