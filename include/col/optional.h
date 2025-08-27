#pragma once

#include <concepts>
#include <exception>
#include <functional>
#include <initializer_list>
#include <memory>
#include <new>
#include <optional>
#include <type_traits>
#include <utility>


namespace col {

    // `T` が `std::optional` の特殊化であるか調べる。
    template <class T>
    struct is_std_optional : std::false_type
    {};

    // `T` が `std::optional` の特殊化であるか調べる。
    template <class T>
    struct is_std_optional<std::optional<T>> : std::true_type
    {};

    // `T` が `std::optional` の特殊化であれば `true` 、そうでなければ `false` 。
    template <class T>
    inline constexpr bool is_std_optional_v = is_std_optional<T>::value;

    // 番兵ベースの Nullable 最適化が可能な型に対するトレイト型。
    template <class T>
    struct sentinel_nullable_traits{};

    // ポインタ型 `T` に対する `sentinel_nullable_traits` の特殊化。
    // 番兵値に `nullptr` を使用する。
    template <class T>
    requires (std::is_pointer_v<T>)
    struct sentinel_nullable_traits<T>
    {
        static constexpr T sentinel_value = nullptr;
    };

    // `std::optional` に対する `sentinel_nullable_traits` の特殊化。
    // 番兵値に `std::nullopt` を使用する。
    template <class T>
    requires (is_std_optional_v<T>)
    struct sentinel_nullable_traits<T>
    {
        static constexpr std::nullopt_t sentinel_value = std::nullopt;
    };

    // 番兵ベースの Nullable 最適化が可能な型であることを示すコンセプト。
    // 以下の型がこのコンセプトを満たす。
    // - ポインタ型
    // - `std::optional` 型
    // - 0 番目の候補型 が `std::monostate` であり、等値比較が可能な `std::variant` 型
    // - 以下を満たす型 `T`
    //   -- `T` と `sentinel_traits<T>::setinel_value` との間で等値比較可能
    //   -- `T` が `sentinel_traits<T>::setinel_value` で構築可能
    template <class T>
    concept sentinel_nullable_optimizable =
        std::is_pointer_v<T> ||
        is_std_optional_v<T> ||
        requires {
            sentinel_nullable_traits<T>::sentinel_value;
            std::equality_comparable_with<T, decltype(sentinel_nullable_traits<T>::sentinel_value)>;
            std::is_constructible_v<T, decltype(sentinel_nullable_traits<T>::sentinel_value)>; // TODO
        };

    // Nullable 最適化が可能な `optional` 型。
    // `std::optional` と互換のインターフェースを持つ。
    // `T` が `sentinel_nullable_optimizable` を満たす場合、無効値の領域が最適化される。
    // `Optimized` へ明示的に `false` を指定することで、 `T` によらず無効値の領域は最適化されない。
    template <class T, bool Optimized = sentinel_nullable_optimizable<T>>
    class optional;

    // `T` が `col::optional` の特殊化であるか調べる。
    template <class T>
    struct is_col_optional : std::false_type
    {};

    // `T` が `col::optional` の特殊化であるか調べる。
    template <class T>
    struct is_col_optional<col::optional<T>> : std::true_type
    {};

    // `T` が `col::optional` の特殊化であれば `true` 、そうでなければ `false` 。
    template <class T>
    inline constexpr bool is_col_optional_v = is_col_optional<T>::value;

    template <class T>
    struct unwrap_pointer_like
    {
        using type = std::type_identity_t<T>;
    };
    template <class T>
    requires (std::is_pointer_v<T> || is_std_optional_v<T>)
    struct unwrap_pointer_like<T>
    {
        using type = std::pointer_traits<T>::element_type;
    };
    template <class T>
    using unwrap_pointer_like_t = unwrap_pointer_like<T>::type;


    // Nullable 最適化されていない `optional` 型。
    template <class T>
    class optional<T, false>
    {
        std::optional<T> m_value;
    public:
        // 要素型
        using value_type = T;

        /* コンストラクタ */

        // 無効値を保持した状態で構築する
        constexpr optional() noexcept
        : m_value()
        {}

        // 無効値を保持した状態で構築する
        constexpr optional(std::nullopt_t) noexcept
        : m_value()
        {}

        // 有効値を `rhs` が保持していたらそれをコピーする
        constexpr optional(const optional& rhs)
            noexcept(std::is_nothrow_copy_constructible_v<T>)
            requires(
                std::is_copy_constructible_v<T> &&
                !std::is_trivially_copy_constructible_v<T>)
        : m_value(rhs.m_value)
        {}

        // `T` がコピー構築不可なら `delete` 定義
        constexpr optional(const optional& rhs)
            requires (!std::is_copy_constructible_v<T>)
        = delete;

        // `T` がトリビアルにコピー構築可能なら `default` 定義
        constexpr optional(const optional& rhs) noexcept
            requires(std::is_trivially_copy_constructible_v<T>)
        = default;

        // 有効値を `rhs` が保持していたらそれをムーブする。 `rhs.has_value()` は変更されない。
        constexpr optional(optional&& rhs)
            noexcept(std::is_nothrow_move_constructible_v<T>)
            requires(
                std::is_move_constructible_v<T> &&
                !std::is_trivially_move_constructible_v<T>)
        : m_value(std::move(rhs.m_value))
        {}

        // `T` がトリビアルにムーブ構築可能なら `default` 定義
        constexpr optional(optional<T>&& rhs) noexcept
            requires(std::is_trivially_move_constructible_v<T>)
        = default;
        
        // `T` がムーブ構築不可なら `delete` 定義
        constexpr optional(optional<T>&& rhs)
            requires (!std::is_move_constructible_v<T>)
        = delete;

        // 型 `T` のコンストラクタ引数を受け取って `T` の値を生成し有効値として保持する。
        template <class ...Args>
        requires (std::is_constructible_v<T, Args...>)
        explicit constexpr optional(std::in_place_t, Args&& ...args)
            noexcept(std::is_nothrow_constructible_v<T, Args...>) // TODO
        : m_value(std::in_place, std::forward<Args>(args)...)
        {}

        // 型 `T` のコンストラクタ引数として初期化子リストと任意個の引数を受け取って `T` の値を生成し有効値として保持する。
        template <class U, class ...Args>
        requires (std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
        explicit constexpr optional(std::in_place_t, std::initializer_list<U> il, Args&& ...args)
            noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>) // TODO
        : m_value(std::in_place, il, std::forward<Args>(args)...)
        {}

        // 型 `T` に変換可能な型 `U` をムーブして有効値として保持する。
        template <class U = T>
        requires requires (U u) {
            { T(std::move(u)) } -> std::same_as<T>;
        }
        explicit(!std::is_convertible_v<U, T>)
        constexpr optional(U&& rhs)
            noexcept(std::is_move_constructible_v<T>) // TODO
        : m_value(std::move(rhs))
        {}

        // 変換可能な `optional` からコピー構築する。
        template <class U>
        requires requires (U& u) {
            { T(u) } -> std::same_as<T>;
        }
        explicit(!std::is_convertible_v<U&, T>)
        constexpr optional(const optional<U>& rhs)
            noexcept(std::is_copy_constructible_v<T>) // TODO
        : m_value(rhs.m_value)
        {}

        // 変換可能な `optional` からムーブ構築する。
        template <class U>
        requires requires (U u) {
            { T(std::move(u)) } -> std::same_as<T>;
        }
        explicit(!std::is_convertible_v<U&&, T>)
        constexpr optional(optional<U>&& rhs)
            noexcept(std::is_move_constructible_v<T>) // TODO
        : m_value(std::move(rhs.m_value))
        {}

        /* デストラクタ */
        constexpr ~optional()
        {}

        /* 代入演算子 */

        // 無効値を代入する
        constexpr optional& operator=(std::nullopt_t) noexcept
        {
            m_value = std::nullopt;
            return *this;
        }

        // コピー代入
        constexpr optional& operator=(const optional& rhs)
            noexcept(std::is_nothrow_copy_assignable_v<T>)
        {
            m_value = rhs.m_value;
            return *this;
        }

        // `T` がトリビアルにコピー構築可能かつ、トリビアルにコピー代入可能かつ、トリビアルに破棄可能なら `default` 定義
        constexpr optional& operator=(const optional& rhs) noexcept
            requires(
                std::is_trivially_copy_constructible_v<T> &&
                std::is_trivially_copy_assignable_v<T> &&
                std::is_trivially_destructible_v<T>)
        = default;

        // `T` がコピー構築もコピー代入も不可能なら `delete` 定義
        constexpr optional& operator=(const optional& rhs)
            requires(
                !std::is_copy_constructible_v<T> &&
                !std::is_copy_assignable_v<T>)
        = delete;

        // ムーブ代入
        constexpr optional& operator=(optional&& rhs)
            noexcept(std::is_move_constructible_v<T>)
        {
            m_value = std::move(rhs.m_value);
            return *this;
        }

        // `T` がトリビアルにムーブ構築可能かつ、トリビアルにムーブ代入可能かつ、トリビアルに破棄可能なら `default` 定義
        constexpr optional& operator=(optional&& rhs) noexcept
            requires(
                std::is_trivially_move_constructible_v<T> && 
                std::is_trivially_move_assignable_v<T> &&
                std::is_trivially_destructible_v<T>)
        = default;

        // `T` がムーブ構築もムーブ代入も不可能なら `delete` 定義
        constexpr optional& operator=(optional&& rhs)
            requires(
                !std::is_move_constructible_v<T> &&
                !std::is_move_assignable_v<T>)
        = delete;

        // `T` に変換可能な型 `U` をムーブ代入
        template <class U = T>
        requires (std::is_convertible_v<U&&, T>)
        constexpr optional operator=(U&& rhs)
            noexcept(m_value.operator=(std::forward<U>(rhs))) // TODO
        {
            m_value = std::forward<U>(rhs);
            return *this;
        }
    
        // `optional<T>` に変換可能な型 `optional<U>` をコピー代入
        template <class U = T>
        requires (std::is_convertible_v<U&, T>)
        constexpr optional operator=(const optional<U>& rhs)
            noexcept(m_value.operator=(rhs.m_value)) // TODO
        {
            m_value = rhs.m_value;
            return *this;
        }
    
        // `optional<T>` に変換可能な型 `optional<U>` をムーブ代入
        template <class U = T>
        requires (std::is_convertible_v<U&&, T>)
        constexpr optional operator=(optional<U>&& rhs)
            noexcept(m_value.operator=(std::move(rhs.m_value))) // TODO
        {
            m_value = std::move(rhs.m_value);
            return *this;
        }

        // 要素型のコンストラクタ引数から直接構築し、構築された有効値への参照を返す。
        // `T` は `Args...` によって構築可能でなければならない。
        // 例外が発生した場合、 `*this` は有効値を含まない状態になり、元々保持していた有効値は破棄される。
        template <class ...Args>
        requires (std::is_constructible_v<T, Args...>)
        constexpr T& emplace(Args&& ...args)
            noexcept(false) // TODO
        {
            return m_value.emplace(std::forward<Args>(args)...);
        }

        // 要素型のコンストラクタ引数から直接構築し、構築された有効値への参照を返す。
        // このオーバーロードでは、コンテナをアロケーター付きで初期化子リスト代入できる。
        // `T` は `Args...` によって構築可能でなければならない。
        // 例外が発生した場合、 `*this` は有効値を含まない状態になり、元々保持していた有効値は破棄される。
        template <class U, class ...Args>
        requires (std::is_constructible_v<T, Args...>)
        constexpr T& emplace(std::initializer_list<U> il, Args&& ...args)
            noexcept(false) // TODO
        {
            return m_value.emplace(il, std::forward<Args>(args)...);
        }

        // 他の `optional` と値を入れ替える。
        // `T` が入れ替え可能かつムーブ構築可能でなければならない。
        // 例外が発生した場合、 `this->has_value()` と `rhs.has_value()` の状態は変わらない。
        // `this` と `rhs` 両方が有効値を保持しているとき、 `std::swap()` の例外安全性に準拠する。
        // `this` と `rhs` どちらか一方が有効値を保持しているとき、 `T` のムーブコンストラクタの例外安全性に準拠する。
        constexpr void swap(optional& rhs)
            noexcept(
                std::is_nothrow_move_constructible_v<T> &&
                std::is_nothrow_swappable_v<T>)
        {
            m_value.swap(rhs.m_value);
        }

        // 有効値を保持していない状態にする。
        // 有効値を保持していないとき、および `T` が非トリビアルなデストラクタを持っている場合は何もしない。
        constexpr void reset() noexcept
        {
            m_value.reset();
        }

        /* 値の観測 */

        // 有効値を保持しているか判定する
        constexpr bool has_value() const noexcept
        {
            return m_value.has_value();
        }

        // 有効値を取得する。有効値を保持していない場合は未定義動作を引き起こす。
        constexpr T& operator*() & noexcept
        {
            return *m_value;
        }

        // 有効値を取得する。有効値を保持していない場合は未定義動作を引き起こす。
        constexpr const T& operator*() const& noexcept
        {
            return *m_value;
        }

        // 有効値を取得する。有効値を保持していない場合は未定義動作を引き起こす。
        constexpr T&& operator*() && noexcept
        {
            return std::move(*m_value);
        }

        // 有効値を取得する。有効値を保持していない場合は未定義動作を引き起こす。
        constexpr const T&& operator*() const&& noexcept
        {
            return std::move(*m_value);
        }

        // 保持している有効値のメンバにアクセスする。有効値を保持していない場合は未定義動作を引き起こす。
        constexpr const T* operator->() const noexcept
        {
            return m_value.operator->();
        }

        // 保持している有効値のメンバにアクセスする。有効値を保持していない場合は未定義動作を引き起こす。
        constexpr T* operator->() noexcept
        {
            return m_value.operator->();
        }

        // 有効値を取得する。有効値を保持していない場合は例外が発生する。
        constexpr T& value() &
            noexcept(false) // TODO
        {
            return m_value.value();
        }

        // 有効値を取得する。有効値を保持していない場合は例外が発生する。
        constexpr const T& value() const&
            noexcept(false) // TODO
        {
            return m_value.value();
        }

        // 有効値を取得する。有効値を保持していない場合は例外が発生する。
        constexpr T&& value() &&
            noexcept(false) // TODO
        {
            return m_value.value();
        }

        // 有効値を取得する。有効値を保持していない場合は例外が発生する。
        constexpr const T&& value() const&&
            noexcept(false) // TODO
        {
            return m_value.value();
        }

        // 有効値を保持していればその有効値を、保持していなければ `v` を返す。
        // `v` はムーブ構築可能かつ `U&&` が `T` へ暗黙に変換可能な型でなければならない。
        template <class U>
        constexpr T value_or(U&& v) const&
            noexcept(false) // TODO
        {
            return m_value.value_or(std::forward<U>(v));
        }

        // 有効値を保持していればその有効値を、保持していなければ `v` を返す。
        // `v` はムーブ構築可能かつ `U&&` が `T` へ暗黙に変換可能な型でなければならない。
        template <class U>
        constexpr T value_or(U&& v) &&
            noexcept(false) // TODO
        {
            return m_value.value_or(std::forward<U>(v));
        }

        /* モナド操作 */

        // 有効値を保持していれば、その値に対して `f` を適用した結果を `optional` として返す。
        // 有効値を保持していなければ、 無効値を返す。
        // `F` は `T` を受け取り `optional` を返す関数でなければならない。
        template <class F>
        requires (std::invocable<F, T&>)
        constexpr auto and_then(F&& f) &
            noexcept(false) // TODO
        {
            using U = std::invoke_result_t<F, decltype(value())>;
            static_assert(
                col::is_col_optional_v<std::remove_cvref_t<U>>,
                "function's return type is not a specialized type of col::optional");
            if( has_value() )
            {
                return std::invoke(std::forward<F>(f), value());
            }
            else
            {
                return std::remove_cvref_t<U>();
            }
        }

        // 有効値を保持していれば、その値に対して `f` を適用した結果を `optional` として返す。
        // 有効値を保持していなければ、 無効値を返す。
        // `F` は `T` を受け取り `optional` を返す関数でなければならない。
        template <class F>
        requires (std::invocable<F, const T&>)
        constexpr auto and_then(F&& f) const&
        {
            using U = std::invoke_result_t<F, decltype(value())>;
            static_assert(
                col::is_col_optional_v<std::remove_cvref_t<U>>,
                "function's return type is not a specialized type of col::optional");
            if( has_value() )
            {
                return std::invoke(std::forward<F>(f), value());
            }
            else
            {
                return std::remove_cvref_t<U>();
            }
        }

        // 有効値を保持していれば、その値に対して `f` を適用した結果を `optional` として返す。
        // 有効値を保持していなければ、 無効値を返す。
        // `F` は `T` を受け取り `optional` を返す関数でなければならない。
        template <class F>
        requires (std::invocable<F, T&&>)
        constexpr auto and_then(F&& f) &&
        {
            using U = std::invoke_result_t<F, decltype(std::move(value()))>;
            static_assert(
                col::is_col_optional_v<std::remove_cvref_t<U>>,
                "function's return type is not a specialized type of col::optional");
            if( has_value() )
            {
                return std::invoke(std::forward<F>(f), std::move(value()));
            }
            else
            {
                return std::remove_cvref_t<U>();
            }
        }

        // 有効値を保持していれば、その値に対して `f` を適用した結果を `optional` として返す。
        // 有効値を保持していなければ、 無効値を返す。
        // `F` は `T` を受け取り `optional` を返す関数でなければならない。
        template <class F>
        requires (std::invocable<F, const T&&>)
        constexpr auto and_then(F&& f) const&&
        {
            using U = std::invoke_result_t<F, decltype(std::move(value()))>;
            static_assert(
                col::is_col_optional_v<std::remove_cvref_t<U>>,
                "function's return type is not a specialized type of col::optional");
            if( has_value() )
            {
                return std::invoke(std::forward<F>(f), std::move(value()));
            }
            else
            {
                return std::remove_cvref_t<U>();
            }
        }
        
        // 有効値を保持していれば、その値に対して `f` を適用した結果を `optional` に格納して返す。
        // 有効値を保持していなければ、無効値を返す。
        //
        // 戻り値の型を `U` とすると、
        // - `U` は、 `std::in_place_t` 、 `std::nullopt_t` いずれでもなく、非配列オブジェクト型であること。
        // - `U u(std::invoke(std::forward<F>(f), value()));` という宣言が妥当であること。
        template <class F>
        requires (std::invocable<F, T&>)
        constexpr auto transform(F&& f) &
            noexcept(false) // TODO
        {
            using U = std::invoke_result_t<F, decltype(value())>;
            static_assert(!std::same_as<U, std::in_place_t>);
            static_assert(!std::same_as<U, std::nullopt_t>);
            static_assert(!std::is_array_v<U>);
            static_assert(std::is_object_v<U>);
            static_assert(std::is_constructible_v<optional<U>, decltype(std::invoke(std::forward<F>(f), value()))>); // TODO

            if( has_value() )
            {
                return optional<U>(std::invoke(std::forward<F>(f), value()));
            }
            else
            {
                return optional<U>();
            }
        }

        // 有効値を保持していれば、その値に対して `f` を適用した結果を `optional` に格納して返す。
        // 有効値を保持していなければ、無効値を返す。
        //
        // 戻り値の型を `U` とすると、
        // - `U` は、 `std::in_place_t` 、 `std::nullopt_t` いずれでもなく、非配列オブジェクト型であること。
        // - `U u(std::invoke(std::forward<F>(f), value()));` という宣言が妥当であること。
        template <class F>
        requires (std::invocable<F, const T&>)
        constexpr auto transform(F&& f) const&
            noexcept(false) // TODO
        {
            using U = std::invoke_result_t<F, decltype(value())>;
            static_assert(!std::same_as<U, std::in_place_t>);
            static_assert(!std::same_as<U, std::nullopt_t>);
            static_assert(!std::is_array_v<U>);
            static_assert(std::is_object_v<U>);
            static_assert(std::is_constructible_v<optional<U>, decltype(std::invoke(std::forward<F>(f), value()))>); // TODO

            if( has_value() )
            {
                return optional<U>(std::invoke(std::forward<F>(f), value()));
            }
            else
            {
                return optional<U>();
            }
        }
        
        
        // 有効値を保持していれば、その値に対して `f` を適用した結果を `optional` に格納して返す。
        // 有効値を保持していなければ、無効値を返す。
        //
        // 戻り値の型を `U` とすると、
        // - `U` は、 `std::in_place_t` 、 `std::nullopt_t` いずれでもなく、非配列オブジェクト型であること。
        // - `U u(std::invoke(std::forward<F>(f), std::move(value())));` という宣言が妥当であること。
        template <class F>
        requires (std::invocable<F, T&&>)
        constexpr auto transform(F&& f) &&
            noexcept(false) // TODO
        {
            using U = std::invoke_result_t<F, decltype(std::move(value()))>;
            static_assert(!std::same_as<U, std::in_place_t>);
            static_assert(!std::same_as<U, std::nullopt_t>);
            static_assert(!std::is_array_v<U>);
            static_assert(std::is_object_v<U>);
            static_assert(std::is_constructible_v<optional<U>, decltype(std::invoke(std::forward<F>(f), std::move(value())))>); // TODO

            if( has_value() )
            {
                return optional<U>(std::invoke(std::forward<F>(f), std::move(value())));
            }
            else
            {
                return optional<U>();
            }
        }

        // 有効値を保持していれば、その値に対して `f` を適用した結果を `optional` に格納して返す。
        // 有効値を保持していなければ、無効値を返す。
        //
        // 戻り値の型を `U` とすると、
        // - `U` は、 `std::in_place_t` 、 `std::nullopt_t` いずれでもなく、非配列オブジェクト型であること。
        // - `U u(std::invoke(std::forward<F>(f), std::move(value())));` という宣言が妥当であること。
        template <class F>
        requires (std::invocable<F, const T&&>)
        constexpr auto transform(F&& f) const&&
            noexcept(false) // TODO
        {
            using U = std::invoke_result_t<F, decltype(std::move(value()))>;
            static_assert(!std::same_as<U, std::in_place_t>);
            static_assert(!std::same_as<U, std::nullopt_t>);
            static_assert(!std::is_array_v<U>);
            static_assert(std::is_object_v<U>);
            static_assert(std::is_constructible_v<optional<U>, decltype(std::invoke(std::forward<F>(f), std::move(value())))>); // TODO

            if( has_value() )
            {
                return optional<U>(std::invoke(std::forward<F>(f), std::move(value())));
            }
            else
            {
                return optional<U>();
            }
        }

        // 有効値を保持していれば、自身を返す。有効値を保持していなければ、 `F` の呼び出し結果を `optional` として返す。
        // `F` は `optional` を返す関数でなければならない。
        template <class F>
        requires (std::invocable<F> && std::copy_constructible<T>)
        constexpr optional or_else(F&& f) const&
            noexcept(false) // TODO
        {
            static_assert(std::same_as<std::remove_cvref_t<std::invoke_result_t<F>>, optional>);
            if( has_value() )
            {
                return *this;
            }
            else
            {
                return std::invoke(std::forward<F>(f));
            }
        }

        template <class F>
        requires (std::invocable<F> && std::move_constructible<T>)
        constexpr optional or_else(F&& f) &&
            noexcept(false) // TODO
        {
            static_assert(std::same_as<std::remove_cvref_t<std::invoke_result_t<F>>, optional>);
            if( has_value() )
            {
                return std::move(*this);
            }
            else
            {
                return std::invoke(std::forward<F>(f));
            }
        }
    };

    // `sentinel_nullable_optimizable` コンセプトを満たす型 `T` に対する `optinal` の特殊化。
    // Nullable 最適化により無効値の領域が最適化される。
    template <class T>
    requires (sentinel_nullable_optimizable<T>)
    class optional<T, true>
    {
        static_assert(std::is_reference_v<T> == false, "T must not be a reference type");
        static_assert(std::same_as<std::remove_cv_t<T>, std::in_place_t> == false, "T must not be std::in_place_t");
        static_assert(std::same_as<std::remove_cv_t<T>, std::nullopt_t> == false, "T must not be std::nullopt_t");
        static_assert(std::destructible<T>);

        T m_value;
    public:
        // 要素型
        // ポインタ型や `std::optional` についてはそれが扱う要素型。
        using value_type = unwrap_pointer_like_t<T>;

        // 番兵型
        using sentinel_type = decltype(sentinel_nullable_traits<T>::sentinel_value);

        // 番兵値
        static constexpr sentinel_type sentinel_value = sentinel_nullable_traits<T>::sentinel_value;
        

        /* コンストラクタ */

        // 無効値を保持した状態で構築する
        constexpr optional() noexcept
        : m_value(sentinel_value)
        {}

        // 無効値を保持した状態で構築する
        constexpr optional(std::nullopt_t) noexcept
        : m_value(sentinel_value)
        {}

        // 有効値を `rhs` が保持していたらそれをコピーする
        constexpr optional(const optional& rhs)
            noexcept(std::is_nothrow_copy_constructible_v<T>)
            requires(
                std::is_copy_constructible_v<T> &&
                !std::is_trivially_copy_constructible_v<T>)
        : m_value(rhs.m_value)
        {}

        // `T` がコピー構築不可なら `delete` 定義
        constexpr optional(const optional& rhs)
            requires (!std::is_copy_constructible_v<T>)
        = delete;

        // `T` がトリビアルにコピー構築可能なら `default` 定義
        constexpr optional(const optional& rhs) noexcept
            requires(std::is_trivially_copy_constructible_v<T>)
        = default;

        // 有効値を `rhs` が保持していたらそれをムーブする。`rhs.has_value()` は変更されないかは `T` のムーブコンストラクタに依存する。
        constexpr optional(optional&& rhs)
            noexcept(std::is_nothrow_move_constructible_v<T>)
            requires(
                std::is_move_constructible_v<T> &&
                !std::is_trivially_move_constructible_v<T>)
        : m_value(std::move(rhs.m_value))
        {}

        // `T` がトリビアルにムーブ構築可能なら `default` 定義
        constexpr optional(optional&& rhs) noexcept
            requires(std::is_trivially_move_constructible_v<T>)
        = default;
        
        // `T` がムーブ構築不可なら `delete` 定義
        constexpr optional(optional&& rhs)
            requires (!std::is_move_constructible_v<T>)
        = delete;

        // 型 `T` のコンストラクタ引数を受け取って `T` の値を生成し有効値として保持する。
        template <class ...Args>
        requires (std::is_constructible_v<T, Args...>)
        explicit constexpr optional(std::in_place_t, Args&& ...args)
            noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : m_value(std::forward<Args>(args)...)
        {}

        // 型 `T` のコンストラクタ引数として初期化子リストと任意個の引数を受け取って `T` の値を生成し有効値として保持する。
        template <class U, class ...Args>
        requires (std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
        explicit constexpr optional(std::in_place_t, std::initializer_list<U> il, Args&& ...args)
            noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>) // TODO
        : m_value(il, std::forward<Args>(args)...)
        {}

        // 型 `T` に変換可能な型 `U` をムーブして有効値として保持する。
        template <class U = T>
        requires requires (U u) {
            { T(std::move(u)) } -> std::same_as<T>;
        }
        explicit(!std::is_convertible_v<U, T>)
        constexpr optional(U&& rhs)
            noexcept(std::is_move_constructible_v<T>) // TODO
        : m_value(std::move(rhs))
        {}

        // 変換可能な `optional` からコピー構築する。
        template <class U>
        requires requires (U& u) {
            { T(u) } -> std::same_as<T>;
        }
        explicit(!std::is_convertible_v<U&, T>)
        constexpr optional(const optional<U>& rhs)
            noexcept(std::is_copy_constructible_v<T>) // TODO
        : m_value(rhs.m_value)
        {}

        // 変換可能な `optional` からムーブ構築する。
        template <class U>
        requires requires (U u) {
            { T(std::move(u)) } -> std::same_as<T>;
        }
        explicit(!std::is_convertible_v<U&&, T>)
        constexpr optional(optional<U>&& rhs)
            noexcept(std::is_move_constructible_v<T>) // TODO
        : m_value(std::move(rhs.m_value))
        {}

        /* デストラクタ */
        constexpr ~optional()
        {
            m_value.~T();
        }

        // `T` がトリビアルに破棄可能だった場合は `default` 実装。
        constexpr ~optional()
            requires(std::is_trivially_destructible_v<T>)
        = default;

        /* 代入演算子 */

        // 無効値を代入する
        constexpr optional& operator=(std::nullopt_t) noexcept
        {
            m_value = sentinel_value;
            return *this;
        }

        // コピー代入
        constexpr optional& operator=(const optional& rhs)
            noexcept(std::is_nothrow_copy_assignable_v<T>)
        {
            m_value = rhs.m_value;
            return *this;
        }

        // `T` がトリビアルにコピー構築可能かつ、トリビアルにコピー代入可能かつ、トリビアルに破棄可能なら `default` 定義
        constexpr optional& operator=(const optional& rhs) noexcept
            requires(
                std::is_trivially_copy_constructible_v<T> &&
                std::is_trivially_copy_assignable_v<T> &&
                std::is_trivially_destructible_v<T>)
        = default;

        // `T` がコピー構築もコピー代入も不可能なら `delete` 定義
        constexpr optional& operator=(const optional& rhs)
            requires(
                !std::is_copy_constructible_v<T> &&
                !std::is_copy_assignable_v<T>)
        = delete;

        // ムーブ代入
        constexpr optional& operator=(optional&& rhs)
            noexcept(std::is_move_constructible_v<T>)
        {
            m_value = std::move(rhs.m_value);
            return *this;
        }

        // `T` がトリビアルにムーブ構築可能かつ、トリビアルにムーブ代入可能かつ、トリビアルに破棄可能なら `default` 定義
        constexpr optional& operator=(optional&& rhs) noexcept
            requires(
                std::is_trivially_move_constructible_v<T> && 
                std::is_trivially_move_assignable_v<T> &&
                std::is_trivially_destructible_v<T>)
        = default;

        // `T` がムーブ構築もムーブ代入も不可能なら `delete` 定義
        constexpr optional& operator=(optional&& rhs)
            requires(
                !std::is_move_constructible_v<T> &&
                !std::is_move_assignable_v<T>)
        = delete;

        // `T` に変換可能な型 `U` をムーブ代入
        template <class U = T>
        requires (std::is_convertible_v<U&&, T>)
        constexpr optional operator=(U&& rhs)
            noexcept(m_value.operator=(std::forward<U>(rhs))) // TODO
        {
            m_value = std::forward<U>(rhs);
            return *this;
        }
    
        // `optional<T>` に変換可能な型 `optional<U>` をコピー代入
        template <class U = T>
        requires (std::is_convertible_v<U&, T>)
        constexpr optional operator=(const optional<U>& rhs)
            noexcept(m_value.operator=(rhs.m_value)) // TODO
        {
            m_value = rhs.m_value;
            return *this;
        }
    
        // `optional<T>` に変換可能な型 `optional<U>` をムーブ代入
        template <class U = T>
        requires (std::is_convertible_v<U&&, T>)
        constexpr optional operator=(optional<U>&& rhs)
            noexcept(m_value.operator=(std::move(rhs.m_value))) // TODO
        {
            m_value = std::move(rhs.m_value);
            return *this;
        }

        // 要素型のコンストラクタ引数から直接構築し、構築された有効値への参照を返す。
        // `T` は `Args...` によって構築可能でなければならない。
        // 例外が発生した場合、 `*this` は有効値を含まない状態になり、元々保持していた有効値は破棄される。
        template <class ...Args>
        requires (std::is_constructible_v<T, Args...>)
        constexpr T& emplace(Args&& ...args)
            noexcept(false) // TODO
        {
            m_value.~T();
            std::construct_at(&m_value, std::forward<Args>(args)...);
            return *std::launder(&m_value);
        }

        // 要素型のコンストラクタ引数から直接構築し、構築された有効値への参照を返す。
        // このオーバーロードでは、コンテナをアロケーター付きで初期化子リスト代入できる。
        // `T` は `Args...` によって構築可能でなければならない。
        // 例外が発生した場合、 `*this` は有効値を含まない状態になり、元々保持していた有効値は破棄される。
        template <class U, class ...Args>
        requires (std::is_constructible_v<T, Args...>)
        constexpr T& emplace(std::initializer_list<U> il, Args&& ...args)
            noexcept(false) // TODO
        {
            m_value.~T();
            std::construct_at(&m_value, il, std::forward<Args>(args)...);
            return *std::launder(*m_value);
        }

        // 他の `optional` と値を入れ替える。
        // `T` が入れ替え可能かつムーブ構築可能でなければならない。
        // 例外が発生した場合、 `this->has_value()` と `rhs.has_value()` の状態は変わらない。
        // `this` と `rhs` 両方が有効値を保持しているとき、 `std::swap()` の例外安全性に準拠する。
        // `this` と `rhs` どちらか一方が有効値を保持しているとき、 `T` のムーブコンストラクタの例外安全性に準拠する。
        constexpr void swap(optional& rhs)
            noexcept(
                std::is_nothrow_move_constructible_v<T> &&
                std::is_nothrow_swappable_v<T>)
        {
            const auto self_has = has_value();
            const auto rhs_has = rhs.has_value();
            if( self_has && rhs_has )
            {
                if( rhs_has )
                {
                    std::swap(value(), rhs.value());
                }
                else
                {
                    rhs.m_value = std::move(m_value);
                    reset();
                }
            }
            else if( rhs_has )
            {
                m_value = std::move(rhs.m_value);
                rhs.reset();
            }
        }

        // 有効値を保持していない状態にする。
        // 有効値を保持していないとき、および `T` が非トリビアルなデストラクタを持っている場合は何もしない。
        constexpr void reset() noexcept
        {
            m_value = sentinel_value;
        }

        /* 値の観測 */

        // 有効値を保持しているか判定する
        constexpr bool has_value() const noexcept
        {
            return m_value != sentinel_value;
        }

        // 有効値を取得する。有効値を保持していない場合は未定義動作を引き起こす。
        constexpr T& operator*() & noexcept
        {
            if constexpr(std::is_pointer_v<T> || is_std_optional_v<T>)
            {
                return *m_value;
            }
            else
            {
                return m_value;
            }
        }

        // 有効値を取得する。有効値を保持していない場合は未定義動作を引き起こす。
        constexpr const T& operator*() const& noexcept
        {
            if constexpr(std::is_pointer_v<T> || is_std_optional_v<T>)
            {
                return *m_value;
            }
            else
            {
                return m_value;
            }
        }

        // 有効値を取得する。有効値を保持していない場合は未定義動作を引き起こす。
        constexpr T&& operator*() && noexcept
        {
            if constexpr(std::is_pointer_v<T> || is_std_optional_v<T>)
            {
                return std::move(*m_value);
            }
            else
            {
                return m_value;
            }
        }

        // 有効値を取得する。有効値を保持していない場合は未定義動作を引き起こす。
        constexpr const T&& operator*() const&& noexcept
        {
            if constexpr(std::is_pointer_v<T> || is_std_optional_v<T>)
            {
                return std::move(*m_value);
            }
            else
            {
                return m_value;
            }
        }

        // 保持している有効値のメンバにアクセスする。有効値を保持していない場合は未定義動作を引き起こす。
        constexpr const T* operator->() const noexcept
        {
            if constexpr( std::is_pointer_v<T> )
            {
                return m_value;
            }
            else
            {
                return m_value.operator->();
            }
        }

        // 保持している有効値のメンバにアクセスする。有効値を保持していない場合は未定義動作を引き起こす。
        constexpr T* operator->() noexcept
        {
            if constexpr( std::is_pointer_v<T> )
            {
                return m_value;
            }
            else
            {
                return m_value.operator->();
            }
        }

        // 有効値を取得する。有効値を保持していない場合は `std::terminate()` が呼ばれる。
        constexpr T& value() &
            noexcept(false) // TODO
        {
            if( has_value() ) [[likely]]
            {
                return m_value;
            }
            else
            {
                std::terminate();
            }
        }

        // 有効値を取得する。有効値を保持していない場合は `std::terminate()` が呼ばれる。
        constexpr const T& value() const&
            noexcept(false) // TODO
        {
            if( has_value() ) [[likely]]
            {
                return m_value;
            }
            else
            {
                std::terminate();
            }
        }

        // 有効値を取得する。有効値を保持していない場合は `std::terminate()` が呼ばれる。
        constexpr T&& value() &&
            noexcept(false) // TODO
        {
            if( has_value() ) [[likely]]
            {
                return m_value;
            }
            else
            {
                std::terminate();
            }
        }

        // 有効値を取得する。有効値を保持していない場合は `std::terminate()` が呼ばれる。
        constexpr const T&& value() const&&
            noexcept(false) // TODO
        {
            if( has_value() ) [[likely]]
            {
                return m_value;
            }
            else
            {
                std::terminate();
            }
        }

        // 有効値を保持していればその有効値を、保持していなければ `v` を返す。
        // `v` はムーブ構築可能かつ `U&&` が `T` へ暗黙に変換可能な型でなければならない。
        template <class U>
        constexpr T value_or(U&& v) const&
            noexcept(false) // TODO
        {
            static_assert(std::is_move_constructible_v<T>);
            static_assert(std::is_convertible_v<U&&, T>);
            return has_value() ? value() : static_cast<T>(std::forward<U>(v));
        }

        // 有効値を保持していればその有効値を、保持していなければ `v` を返す。
        // `v` はムーブ構築可能かつ `U&&` が `T` へ暗黙に変換可能な型でなければならない。
        template <class U>
        constexpr T value_or(U&& v) &&
            noexcept(false) // TODO
        {
            static_assert(std::is_move_constructible_v<T>);
            static_assert(std::is_convertible_v<U&&, T>);
            return has_value() ? value() : static_cast<T>(std::forward<U>(v));
        }

        /* モナド操作 */

        // 有効値を保持していれば、その値に対して `f` を適用した結果を `optional` として返す。
        // 有効値を保持していなければ、 無効値を返す。
        // `F` は `T` を受け取り `optional` を返す関数でなければならない。
        template <class F>
        requires (std::invocable<F, T&>)
        constexpr auto and_then(F&& f) &
            noexcept(false) // TODO
        {
            using U = std::invoke_result_t<F, decltype(value())>;
            static_assert(
                col::is_col_optional_v<std::remove_cvref_t<U>>,
                "function's return type is not a specialized type of col::optional");
            if( has_value() )
            {
                return std::invoke(std::forward<F>(f), value());
            }
            else
            {
                return std::remove_cvref_t<U>();
            }
        }

        // 有効値を保持していれば、その値に対して `f` を適用した結果を `optional` として返す。
        // 有効値を保持していなければ、 無効値を返す。
        // `F` は `T` を受け取り `optional` を返す関数でなければならない。
        template <class F>
        requires (std::invocable<F, const T&>)
        constexpr auto and_then(F&& f) const&
        {
            using U = std::invoke_result_t<F, decltype(value())>;
            static_assert(
                col::is_col_optional_v<std::remove_cvref_t<U>>,
                "function's return type is not a specialized type of col::optional");
            if( has_value() )
            {
                return std::invoke(std::forward<F>(f), value());
            }
            else
            {
                return std::remove_cvref_t<U>();
            }
        }

        // 有効値を保持していれば、その値に対して `f` を適用した結果を `optional` として返す。
        // 有効値を保持していなければ、 無効値を返す。
        // `F` は `T` を受け取り `optional` を返す関数でなければならない。
        template <class F>
        requires (std::invocable<F, T&&>)
        constexpr auto and_then(F&& f) &&
        {
            using U = std::invoke_result_t<F, decltype(std::move(value()))>;
            static_assert(
                col::is_col_optional_v<std::remove_cvref_t<U>>,
                "function's return type is not a specialized type of col::optional");
            if( has_value() )
            {
                return std::invoke(std::forward<F>(f), std::move(value()));
            }
            else
            {
                return std::remove_cvref_t<U>();
            }
        }

        // 有効値を保持していれば、その値に対して `f` を適用した結果を `optional` として返す。
        // 有効値を保持していなければ、 無効値を返す。
        // `F` は `T` を受け取り `optional` を返す関数でなければならない。
        template <class F>
        requires (std::invocable<F, const T&&>)
        constexpr auto and_then(F&& f) const&&
        {
            using U = std::invoke_result_t<F, decltype(std::move(value()))>;
            static_assert(
                col::is_col_optional_v<std::remove_cvref_t<U>>,
                "function's return type is not a specialized type of col::optional");
            if( has_value() )
            {
                return std::invoke(std::forward<F>(f), std::move(value()));
            }
            else
            {
                return std::remove_cvref_t<U>();
            }
        }
        
        // 有効値を保持していれば、その値に対して `f` を適用した結果を `optional` に格納して返す。
        // 有効値を保持していなければ、無効値を返す。
        //
        // 戻り値の型を `U` とすると、
        // - `U` は、 `std::in_place_t` 、 `std::nullopt_t` いずれでもなく、非配列オブジェクト型であること。
        // - `U u(std::invoke(std::forward<F>(f), value()));` という宣言が妥当であること。
        template <class F>
        requires (std::invocable<F, T&>)
        constexpr auto transform(F&& f) &
            noexcept(false) // TODO
        {
            using U = std::invoke_result_t<F, decltype(value())>;
            static_assert(!std::same_as<U, std::in_place_t>);
            static_assert(!std::same_as<U, std::nullopt_t>);
            static_assert(!std::is_array_v<U>);
            static_assert(std::is_object_v<U>);
            static_assert(std::is_constructible_v<optional<U>, decltype(std::invoke(std::forward<F>(f), value()))>); // TODO

            if( has_value() )
            {
                return optional<U>(std::invoke(std::forward<F>(f), value()));
            }
            else
            {
                return optional<U>();
            }
        }

        // 有効値を保持していれば、その値に対して `f` を適用した結果を `optional` に格納して返す。
        // 有効値を保持していなければ、無効値を返す。
        //
        // 戻り値の型を `U` とすると、
        // - `U` は、 `std::in_place_t` 、 `std::nullopt_t` いずれでもなく、非配列オブジェクト型であること。
        // - `U u(std::invoke(std::forward<F>(f), value()));` という宣言が妥当であること。
        template <class F>
        requires (std::invocable<F, const T&>)
        constexpr auto transform(F&& f) const&
            noexcept(false) // TODO
        {
            using U = std::invoke_result_t<F, decltype(value())>;
            static_assert(!std::same_as<U, std::in_place_t>);
            static_assert(!std::same_as<U, std::nullopt_t>);
            static_assert(!std::is_array_v<U>);
            static_assert(std::is_object_v<U>);
            static_assert(std::is_constructible_v<optional<U>, decltype(std::invoke(std::forward<F>(f), value()))>); // TODO

            if( has_value() )
            {
                return optional<U>(std::invoke(std::forward<F>(f), value()));
            }
            else
            {
                return optional<U>();
            }
        }
        
        
        // 有効値を保持していれば、その値に対して `f` を適用した結果を `optional` に格納して返す。
        // 有効値を保持していなければ、無効値を返す。
        //
        // 戻り値の型を `U` とすると、
        // - `U` は、 `std::in_place_t` 、 `std::nullopt_t` いずれでもなく、非配列オブジェクト型であること。
        // - `U u(std::invoke(std::forward<F>(f), std::move(value())));` という宣言が妥当であること。
        template <class F>
        requires (std::invocable<F, T&&>)
        constexpr auto transform(F&& f) &&
            noexcept(false) // TODO
        {
            using U = std::invoke_result_t<F, decltype(std::move(value()))>;
            static_assert(!std::same_as<U, std::in_place_t>);
            static_assert(!std::same_as<U, std::nullopt_t>);
            static_assert(!std::is_array_v<U>);
            static_assert(std::is_object_v<U>);
            static_assert(std::is_constructible_v<optional<U>, decltype(std::invoke(std::forward<F>(f), std::move(value())))>); // TODO

            if( has_value() )
            {
                return optional<U>(std::invoke(std::forward<F>(f), std::move(value())));
            }
            else
            {
                return optional<U>();
            }
        }

        // 有効値を保持していれば、その値に対して `f` を適用した結果を `optional` に格納して返す。
        // 有効値を保持していなければ、無効値を返す。
        //
        // 戻り値の型を `U` とすると、
        // - `U` は、 `std::in_place_t` 、 `std::nullopt_t` いずれでもなく、非配列オブジェクト型であること。
        // - `U u(std::invoke(std::forward<F>(f), std::move(value())));` という宣言が妥当であること。
        template <class F>
        requires (std::invocable<F, const T&&>)
        constexpr auto transform(F&& f) const&&
            noexcept(false) // TODO
        {
            using U = std::invoke_result_t<F, decltype(std::move(value()))>;
            static_assert(!std::same_as<U, std::in_place_t>);
            static_assert(!std::same_as<U, std::nullopt_t>);
            static_assert(!std::is_array_v<U>);
            static_assert(std::is_object_v<U>);
            static_assert(std::is_constructible_v<optional<U>, decltype(std::invoke(std::forward<F>(f), std::move(value())))>); // TODO

            if( has_value() )
            {
                return optional<U>(std::invoke(std::forward<F>(f), std::move(value())));
            }
            else
            {
                return optional<U>();
            }
        }

        // 有効値を保持していれば、自身を返す。有効値を保持していなければ、 `F` の呼び出し結果を `optional` として返す。
        // `F` は `optional` を返す関数でなければならない。
        template <class F>
        requires (std::invocable<F> && std::copy_constructible<T>)
        constexpr optional or_else(F&& f) const&
            noexcept(false) // TODO
        {
            static_assert(std::same_as<std::remove_cvref_t<std::invoke_result_t<F>>, optional>);
            if( has_value() )
            {
                return *this;
            }
            else
            {
                return std::invoke(std::forward<F>(f));
            }
        }


        template <class F>
        requires (std::invocable<F> && std::move_constructible<T>)
        constexpr optional or_else(F&& f) &&
            noexcept(false) // TODO
        {
            static_assert(std::same_as<std::remove_cvref_t<std::invoke_result_t<F>>, optional>);
            if( has_value() )
            {
                return std::move(*this);
            }
            else
            {
                return std::invoke(std::forward<F>(f));
            }
        }
    
    };
    

    // 2 つの `col::optional<T>` の価を入れ替える。
    template <class T>
    requires (std::is_swappable_v<T> || std::is_move_constructible_v<T>)
    constexpr void swap(optional<T>& x, optional<T>& y)
        noexcept(noexcept(x.swap(y)))
    {
        x.swap(y);
    }

    // `optional` の等値比較を行う。
    // `x` と `y` のどちらも有効値を保持していれば、有効値同士を等値比較した結果を返す。
    // それ以外の場合は、どちらも無効値を保持していれば `true` を返し、そうでなければ `false` を返す。
    // `T` と `U` が `==` で比較可能であり、その戻り値が `bool` に暗黙変換可能でなければならない。
    template <class T, class U>
    requires (std::equality_comparable_with<T, U>)
    constexpr bool operator==(const optional<T>& x, const optional<U>& y)
        noexcept(false) // TODO
    {
        const auto x_has_value = x.has_value();
        const auto y_has_value = y.has_value();
        if( x_has_value && y_has_value )
        {
            return *x == *y;
        }
        else
        {
            return !x_has_value && !y_has_value;
        }
    }

    // `optional` の等値比較を行う。
    // `x` が無効値であれば `true` を返す。そうでなければ `false` を返す。
    template <class T>
    constexpr bool operator==(const optional<T>& x, std::nullopt_t) noexcept
    {
        return !x.has_value();
    }

    // `optional` の等値比較を行う。
    // `x` が有効値であれば、 `y` と等値比較した結果を返す。そうでなければ `false` を返す。
    template <class T, class U>
    requires (std::equality_comparable_with<T, U>)
    constexpr bool operator==(const optional<T>& x, const U& y)
        noexcept(false) // TODO
    {
        return x.has_value() ? *x == y : false;
    }

    // `optional` の等値比較を行う。
    // `y` が有効値であれば、 `x` と等値比較した結果を返す。そうでなければ `false` を返す。
    template <class T, class U>
    requires (std::equality_comparable_with<T, U>)
    constexpr bool operator==(const T& x, const optional<U>& y)
        noexcept(false) // TODO
    {
        return y == x;
    }

    // `optional` の非等値比較を行う。
    template <class T, class U>
    requires (std::equality_comparable_with<T, U>)
    constexpr bool operator!=(const optional<T>& x, const optional<U>& y)
    {
        return !(x == y);
    }

    // `optional` の非等値比較を行う。
    template <class T, class U>
    constexpr bool operator!=(const optional<T>& x, const U& y)
    {
        return !(x == y);
    }

    // `optional` の非等値比較を行う。
    template <class T, class U>
    constexpr bool operator!=(const T& x, const optional<U>& y)
    {
        return !(x == y);
    }

    // deduction guide
    template <class T>
    optional(T) -> optional<T, sentinel_nullable_optimizable<T>>;

} // namespace col
