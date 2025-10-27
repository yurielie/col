#pragma once

#include <cstddef>
#include <concepts>
#include <memory>
#include <type_traits>

namespace col {

    // 非 nullptr が確実に格納されているポインタ型
    //
    // この型はポインタ型に対するアノテーション向けであり、ポインタが指すオブジェクトの寿命管理は行わない。
    //
    // ポインタ型からこの型を生成する場合、 `nullptr` でないことは呼び出し側が保証する。
    template <class T>
    requires (std::is_object_v<T> && !std::same_as<T, std::nullptr_t>)
    class NonNull
    {
        T* m_ptr;
    public:
        using value_type = T;

        // コンストラクタ
        //
        // 関数の引数へ渡すときにポインタ渡しの意図を呼び出し側で明示できるよう `explicit` を指定する。
        constexpr explicit NonNull(T& value) noexcept
        : m_ptr{ std::addressof(value) }
        {}

        constexpr operator T*() const noexcept
        {
            return m_ptr;
        }

        constexpr T& operator*() const noexcept
        {
            return *m_ptr;
        }

        constexpr T* operator->() const noexcept
        {
            return m_ptr;
        }

        // 内部で保持しているポインタを取得する
        constexpr T* get() const noexcept
        {
            return m_ptr;
        }
    };

    // 推論補助
    template <class T>
    NonNull(T&) -> NonNull<T>;

} // namespace col
