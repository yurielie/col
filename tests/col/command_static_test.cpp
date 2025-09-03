#include <col/command.h>

#include <array>
#include <string>
#include <string_view>
#include <span>

namespace {

    static_assert(col::value_parser_for<decltype([](const char*) { return 0; }), int>);
    static_assert(col::value_parser_for<decltype([](std::string_view) { return 0; }), int>);
    static_assert(col::value_parser_for<decltype([](std::string) { return 0; }), int>);
    static_assert(col::value_parser_for<decltype([](const char*) { return 0; }), std::optional<int>>);
    static_assert(col::value_parser_for<decltype([](const char*) { return std::optional<int>{0}; }), int>);
    static_assert(col::value_parser_for<decltype([](const char*) { return std::optional<int>{0}; }), std::optional<int>>);
    static_assert(col::value_parser_for<decltype([](const char*) -> std::expected<int, col::ParseError> { return 0; }), int>);
    static_assert(col::value_parser_for<decltype([](const char*) -> std::expected<int, col::ParseError> { return 0; }), std::optional<int>>);

    [[maybe_unused]] void test() {
        struct Test
        {
            bool flag;
            bool flag_default;
            int opt_int_default;
            int opt_int_setdefault;
            int opt_int_setdefault_func;
            int opt_int_parser;
            int opt_int_parser_default;
            int opt_int_default_parser;
            int opt_int_default_func_parser;
            std::optional<int> opt_optional_int_deduced;
            std::optional<int> opt_optional_int_explicit;
            std::optional<int> opt_optional_int_parser;
            std::optional<int> opt_optional_int_parser_expected;
        };

        constexpr auto cmd = col::Command{"cmd"}
            .add(col::Arg{"--flag", "flag"})
            .add(col::Arg{"--flag_default", "flag default"}
                .set_default(true))
            .add(col::Arg<int>{"--opt_int_default", "opt int default"})
            .add(col::Arg{"--opt_int_setdefault", "opt int setdefault"}
                .set_default(1))
            .add(col::Arg{"--opt_int_setdefault_func", "opt int setdefault func"}
                .set_default([](){ return 1; }))
            .add(col::Arg{"--opt_int_parser", "opt int parser"}
                .set_parser([](const char*) { return 1; }))
            .add(col::Arg{"--opt_int_parser_default", "opt int parser default"}
                .set_parser([](const char*){ return 1; })
                .set_default(2))
            .add(col::Arg{"--opt_int_default_parser", "opt int default parser"}
                .set_default(1)
                .set_parser([](auto){ return 2; }))
            .add(col::Arg{"--opt_int_default_func_parser", "opt int default parser"}
                .set_default([](){ return 4; })
                .set_parser([](auto){ return 2; }))
            .add(col::Arg{"--opt_optional_int_deduced", "opt optional-int deduced"}
                .set_default([](){ return std::optional<int>{6}; }))
            .add(col::Arg<std::optional<int>>{"--opt_optional_int_explicit", "opt optional-int explicit"})
            .add(col::Arg<std::optional<int>>{"--opt_optional_int_parser", "opt optional-int parser"}
                .set_parser([](auto){ return 5; }))
            .add(col::Arg<std::optional<int>>{"--opt_optional_int_parser_expected", "opt optional-int parser_expected"}
                .set_parser([](auto) -> std::expected<int, col::ParseError> { return 5; }))
            ;

        constexpr auto res1 = [&]()
        {
            const char* args[] = {
                "--flag",
                "--opt_int_parser", "VALUE",
                "--opt_int_default_parser", "VALUIE",
                "--opt_optional_int_parser_expected", "VALUE"
            };

            return cmd.parse<Test>(std::span{args});
        }();
        static_assert(res1.has_value());
        static_assert(res1->flag);
        static_assert(res1->flag_default);
        static_assert(res1->opt_int_default == 0);
        static_assert(res1->opt_int_setdefault == 1);
        static_assert(res1->opt_int_setdefault_func == 1);
        static_assert(res1->opt_int_parser_default == 2);
        static_assert(res1->opt_int_default_parser == 2);
        static_assert(res1->opt_int_default_func_parser == 4);

        static_assert(res1->opt_optional_int_deduced.has_value());
        static_assert(res1->opt_optional_int_deduced.value() == 6);
        static_assert(!res1->opt_optional_int_explicit.has_value());
        static_assert(!res1->opt_optional_int_parser.has_value());
        static_assert(res1->opt_optional_int_parser_expected.has_value());
        static_assert(res1->opt_optional_int_parser_expected.value() == 5);

        {
            struct Test
            {
                std::optional<std::string> foo;
            };
            constexpr auto c = col::Command{"cmd"}
                .add(col::Arg<std::string>{"--foo", "foo"}
                    .set_default(std::in_place, 3, 'a'))
                ;
            const char* args[] = {
                "--foo", "FOO",
            };
            constexpr auto res = c.parse<Test>(std::span{args, 0});
            static_assert(res.has_value());
            static_assert(res->foo == "aaa");
        }

        {
            // Arg<void, void, void> が Arg<bool, bool, void> として扱われること、
            // デフォルト値が false になることを確認する。
            struct Test
            {
                bool foo;
            };
            constexpr auto res = col::Command{"cmd"}
                .add(col::Arg{"--foo", "foo"})
                .parse<Test>(std::array<const char*, 0>{});
            static_assert(res.has_value());
            static_assert(res->foo == false);
        }
    }

    [[maybe_unused]] void test2() {

        struct A{};
        struct B{
            int i;
            constexpr B(int n) : i{n} {}
        };
        struct C{};

        struct Res
        {
            bool v;
            A a;
            B b;
            C c;
            std::optional<C> c1;
        };
        constexpr auto res = []() static {
            constexpr auto cdd = col::Command{"cmd"}
                .add(col::Arg<bool>{"--v", "v"})
                .add(col::Arg{"--a", "a"}.set_parser([](const char*){ return A{}; }))
                .add(col::Arg{"--b", "b"}.set_default(B{1}))
                .add(col::Arg{"--c", "c"}.set_parser([](std::string_view arg) -> std::expected<C, col::ParseError>
                {
                    if( arg == "c" )
                    {
                        return C{};
                    }
                    else
                    {
                        return std::unexpected{col::ParserConvertionError{
                            .name = "--c",
                            .arg = arg,
                        }};
                    }
                }))
                .add(col::Arg<C>{"--c1", "c1"}.set_parser([](const char*)
                {
                    return std::optional{C{}};
                }))
                ;
            const char* args[] = {
                "--v",
            };
            return cdd.parse<Res>(std::span{args});
        }();
        static_assert(res.has_value());
        static_assert(res->v);
        static_assert(res->b.i == 1);
        static_assert(res->c1.has_value());
    }


} // namespace
