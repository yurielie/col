#include <col/optional.h>


struct MyType
{
    int m_i;
    constexpr MyType(int i) : m_i{i}{}

    operator int() const noexcept
    {
        return m_i;
    }
};

constexpr bool operator==(const MyType& x, const MyType& y) {
    return x.m_i == y.m_i;
}
constexpr bool operator==(const MyType& x, int y) {
    return x.m_i == y;
}

namespace col {
    template <>
    struct sentinel_nullable_traits<MyType>
    {
        static constexpr int sentinel_value = 0;
    };
}

namespace {

    static_assert(std::same_as<col::optional<int>::value_type, int>);
    static_assert(std::same_as<col::optional<int*>::value_type, int>);
    static_assert(std::same_as<col::optional<MyType>::value_type, MyType>);

    static_assert(sizeof(col::optional<int>) > sizeof(int));
    static_assert(sizeof(col::optional<int*>) == sizeof(int*));
    static_assert(sizeof(col::optional<MyType>) == sizeof(MyType));
    static_assert(sizeof(col::optional<MyType, false>) == sizeof(std::optional<MyType>));

    consteval bool test_optimized_ptr() {
        int i = 1;
        col::optional opt{&i};
        int j = 2;
        opt.emplace(&j);
        opt.reset();
        auto res = opt
            .or_else([](){ return col::optional<int*, true>{nullptr}; })
            .transform([](int* v) {
                *v *= 2;
                return v;
            })
            .and_then([](int* v){ return col::optional(static_cast<double>(*v)); });

        return res.has_value() == false;
    }
    static_assert(test_optimized_ptr());

    consteval bool test_optimized_mytype() {
        col::optional<MyType> opt{};
        if( opt.has_value() )
        {
            return false;
        }
        opt.emplace(10);
        if( !opt.has_value() )
        {
            return false;
        }

        const auto v = opt.value();
        if( v.m_i != 10 )
        {
            return false;
        }
        const auto res = opt
            .and_then([](MyType mt){ return col::optional{MyType(mt.m_i * 2)}; })
            .transform([](MyType mt){ return mt.m_i * 2; });

        return res.has_value() && *res == 40;
    }
    static_assert(test_optimized_mytype());


    consteval bool test_non_optimized() {
        col::optional opt{10};
        opt.emplace(100);
        opt.reset();
        auto res = opt
            .or_else([](){ return col::optional{10}; })
            .transform([](int i) { return i * 2; })
            .and_then([](int i){ return col::optional(static_cast<double>(i)); });

        return res.has_value() && res.value() == 20.0;
    }
    static_assert(test_non_optimized());
}

