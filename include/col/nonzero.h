#pragma once

#include <cstddef>
#include <cstdint>
#include <concepts>

#include <optional>

namespace col {

    // 非ゼロ値を確実に格納した整数型
    template <class T>
    requires (std::integral<T>)
    class NonZero
    {
        T m_value;

        constexpr NonZero(T value) noexcept
        : m_value{ value }
        {}

    public:

        // 整数型 `T` から `NonZero<T>` を生成する。 `value != 0` のときに有効値を返す。
        static constexpr std::optional<NonZero<T>> make_nonzero(T value) noexcept
        {
            if( value != 0 )
            {
                return NonZero<T>{ value };
            }
            return std::nullopt;
        }
        
        // 整数型 `T` から `NonZero<T>` を生成する。値を検査しない。
        static constexpr NonZero<T> make_nonzero_unchecked(T value) noexcept
        {
            return NonZero<T>{ value };
        }

        constexpr operator T() const noexcept
        {
            return m_value;
        }

        constexpr T get() const noexcept
        {
            return m_value;
        }

    };

    using NonZeroU8 = NonZero<std::uint8_t>;
    using NonZeroI8 = NonZero<std::int8_t>;
    using NonZeroU16 = NonZero<std::uint16_t>;
    using NonZeroI16 = NonZero<std::int16_t>;
    using NonZeroU32 = NonZero<std::uint32_t>;
    using NonZeroI32 = NonZero<std::int32_t>;
    using NonZeroU64 = NonZero<std::uint64_t>;
    using NonZeroI64 = NonZero<std::int64_t>;

    using NonZeroIsize = NonZero<std::ptrdiff_t>;
    using NonZeroUsize = NonZero<std::size_t>;

} // namespace col
