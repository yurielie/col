#include <col/control_flow.h>

#include <type_traits>
#include <variant>

namespace {

    void control_flow_static_test() {
        constexpr auto brk = col::Break{10};
        static_assert(std::same_as<std::remove_cvref_t<decltype(brk)>, col::Break<int>>);
        static_assert(brk.get() == 10);

        constexpr auto cf1 = col::ControlFlow{brk};
        static_assert(std::same_as<std::remove_cvref_t<decltype(cf1)>, col::ControlFlow<int, std::monostate>>);
        static_assert(cf1.is_break() == true);
        static_assert(cf1.is_continue() == false);
        static_assert(cf1 == col::Break{10});
        static_assert(cf1 != col::Break{9});
        static_assert(cf1 != col::Continue{});
        static_assert(std::get<1>(std::move(cf1).get()) == 10);
        static_assert(cf1.to_break() == 10);

        constexpr auto con = col::Continue{5};
        static_assert(std::same_as<std::remove_cvref_t<decltype(con)>, col::Continue<int>>);
        static_assert(con.get() == 5);

        constexpr col::ControlFlow<int, int> cf2{con};
        static_assert(cf2 == col::Continue{5});
        static_assert(cf2 != col::Continue{4});
        static_assert(cf2 != col::Break{5});
        static_assert(std::get<0>(cf2.get()) == 5);
        static_assert(cf2.to_continue() == 5);

        constexpr auto cf3 = []() -> col::ControlFlow<int> {
            return col::Continue{};
        }();
        static_assert(std::same_as<std::remove_cvref_t<decltype(cf3)>, col::ControlFlow<int, std::monostate>>);
        static_assert(cf3 == col::Continue{});

        constexpr auto cf4 = []() -> col::ControlFlow<int> {
            return col::Break{-1};
        }();
        static_assert(std::same_as<std::remove_cvref_t<decltype(cf4)>, col::ControlFlow<int, std::monostate>>);
        static_assert(cf4 == col::Break{-1});
    }

}

int main() {
    control_flow_static_test();
}

