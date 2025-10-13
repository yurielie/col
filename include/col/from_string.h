#pragma once

#include <charconv>
#include <concepts>
#include <expected>
#include <string_view>
#include <system_error>
#include <utility>

namespace col {
    
    // 文字列から整数型 `T` に変換する。 `bool` 型は整数型に含まれない。
    // 10 進数または 16 進数表記を受け付ける。16進数表記の場合は `0x` で文字列が始まっている必要があり、16 進数の負値は変換できない。
    //
    // 文字列 `str` 全体が変換されなかった場合もエラーを返し、 `std::from_chars_result::ptr != str.cend()` となる。
    template <class T>
    requires (
        std::integral<T> &&
        !std::same_as<std::decay_t<T>, bool>
    )
    constexpr std::expected<T, std::from_chars_result> integral_from_string(std::string_view str) noexcept
    {
        const auto [s, base] = [&]() noexcept {
            if( str.starts_with("0x") )
            {
                return std::pair{ str.substr(2), 16 };
            }
            else
            {
                return std::pair{ str, 10 };
            }
        }();

        T value{};
        const auto res = std::from_chars(s.cbegin(), s.cend(), value, base);
        if( res.ptr == s.cend() && res.ec == std::errc{} )
        {
            return value;
        }
        else
        {
            return std::unexpected{ res };
        }
    }

    // 文字列から浮動小数点数型 `T` に変換する。
    // `std::chars_format::general` の変換に失敗したときは `std::chars_format::fixed` での変換も試みる。
    // ただし、そのときに返されるエラーは `general` で失敗したときのエラーとなる。
    template <std::floating_point T>
    constexpr std::expected<T, std::from_chars_result> float_from_string(std::string_view str) noexcept
    {
        T value{};
        const auto res = std::from_chars(str.cbegin(), str.cend(), value, std::chars_format::general);
        if( res.ptr == str.cend() && res.ec == std::errc{} )
        {
            return value;
        }
        else
        {
            const auto fixed_res = std::from_chars(str.cbegin(), str.cend(), value, std::chars_format::fixed);
            if( fixed_res.ptr == str.cend() && fixed_res.ec == std::errc{} )
            {
                return value;
            }
            else
            {
                // そもそも general で成功するべきであるのでそのエラーを返す。
                return std::unexpected{ res };
            }
        }
    }

    // 文字列から整数型または浮動小数点数型 `T` に変換する。
    template <class T>
    requires (
        (
            std::integral<T> &&
            !std::same_as<std::decay_t<T>, bool>
        ) ||
        std::floating_point<T>
    )
    constexpr std::expected<T, std::from_chars_result> number_from_string(std::string_view str) noexcept
    {
        if constexpr(std::integral<T>)
        {
            return integral_from_string<T>(str);
        }
        else
        {
            return float_from_string<T>(str);
        }
    }

} // namespace col
