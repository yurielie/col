#pragma once

#include <concepts>
#include <type_traits>
#include <utility>
#include <variant>

namespace col {

    template <class B>
    requires (std::destructible<B>)
    class Break
    {
        B m_value;
    public:
        template <class Br = B>
        requires (std::convertible_to<Br, B>)
        constexpr explicit Break(Br&& b) noexcept(std::is_nothrow_constructible_v<B, Br>)
        : m_value{ std::forward<Br>(b) }
        {}

        constexpr B& get() & noexcept
        {
            return m_value;
        }
        constexpr const B& get() const& noexcept
        {
            return m_value;
        }
        constexpr B&& get() && noexcept
        {
            return std::move(m_value);
        }
        constexpr const B&& get() const&& noexcept
        {
            return std::move(m_value);
        }
    };
    template <class B>
    Break(B) -> Break<std::remove_cvref_t<B>>;

    template <class C = std::monostate>
    requires (std::destructible<C>)
    class Continue
    {
        C m_value;
    public:
        template <class Con = C>
        requires (std::convertible_to<Con, C>)
        constexpr explicit Continue(Con&& con) noexcept(std::is_nothrow_constructible_v<C, Con>)
        : m_value{ std::forward<Con>(con) }
        {}

        constexpr explicit Continue() noexcept(std::is_nothrow_default_constructible_v<C>)
            requires (std::is_default_constructible_v<C>)
        : m_value{}
        {}

        constexpr C& get() &
        {
            return m_value;
        }
        constexpr const C& get() const&
        {
            return m_value;
        }
        constexpr C&& get() &&
        {
            return std::move(m_value);
        }
        constexpr const C&& get() const&&
        {
            return std::move(m_value);
        }
    };
    Continue() -> Continue<std::monostate>;
    template <class C>
    Continue(C) -> Continue<std::remove_cvref_t<C>>;

    template <class B, class C = std::monostate>
    class ControlFlow
    {
        std::variant<C, B> m_value;
    public:
        template <class Br = B>
        requires (std::convertible_to<Br, B>)
        constexpr ControlFlow(Break<Br> b) noexcept(std::is_nothrow_move_constructible_v<B>)
        : m_value{ std::in_place_index<1>, std::move(b).get() }
        {}

        template <class Con = C>
        requires (std::convertible_to<Con, C>)
        constexpr ControlFlow(Continue<Con> c) noexcept(std::is_nothrow_move_constructible_v<C>)
        : m_value{ std::in_place_index<0>, std::move(c).get() }
        {}

        constexpr bool is_continue() const noexcept
        {
            return m_value.index() == 0ZU;
        }
        constexpr bool is_break() const noexcept
        {
            return !is_continue();
        }

        constexpr std::variant<C, B>& get() & noexcept
        {
            return m_value;
        }

        constexpr const std::variant<C, B>& get() const& noexcept
        {
            return m_value;
        }

        constexpr std::variant<C, B>&& get() && noexcept
        {
            return std::move(m_value);
        }

        constexpr const std::variant<C, B>&& get() const&& noexcept
        {
            return std::move(m_value);
        }

        constexpr B& to_break() & noexcept
        {
            return *std::get_if<1>(&m_value);
        }
        constexpr const B& to_break() const& noexcept
        {
            return *std::get_if<1>(&m_value);
        }
        constexpr B&& to_break() && noexcept
        {
            return std::move(*std::get_if<1>(&m_value));
        }
        constexpr const B&& to_break() const&& noexcept
        {
            return std::move(*std::get_if<1>(&m_value));
        }

        constexpr C& to_continue() & noexcept
        {
            return *std::get_if<0>(&m_value);
        }
        constexpr const C& to_continue() const& noexcept
        {
            return *std::get_if<0>(&m_value);
        }
        constexpr C&& to_continue() && noexcept
        {
            return std::move(*std::get_if<0>(&m_value));
        }
        constexpr const C&& to_continue() const&& noexcept
        {
            return std::move(*std::get_if<0>(&m_value));
        }

        constexpr bool operator==(const Break<B>& b) const noexcept
            requires (std::equality_comparable<B>)
        {
            return is_break() && (*std::get_if<1>(&m_value) == b.get());
        }
        constexpr bool operator==(const Continue<C>& c) const noexcept
            requires (std::equality_comparable<C>)
        {
            return is_continue() && (*std::get_if<0>(&m_value) == c.get());
        }
    };
    template <class B>
    ControlFlow(Break<B>) -> ControlFlow<B>;

    template <class B>
    constexpr bool operator==(const ControlFlow<B, std::monostate>& cf, const Continue<std::monostate>&) noexcept
    {
        return cf.is_continue();
    }
    template <class C>
    constexpr bool operator==(const ControlFlow<std::monostate, C>& cf, const Break<std::monostate>&) noexcept
    {
        return cf.is_break();
    }

    template <class T>
    struct is_control_flow : std::false_type {};
    template <class B, class C>
    struct is_control_flow<ControlFlow<B, C>> : std::true_type {};
    template <class T>
    inline constexpr bool is_control_flow_v = is_control_flow<T>::value;

} // namespace col
