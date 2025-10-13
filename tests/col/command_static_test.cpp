#include <col/command.h>

#include <array>
#include <string_view>

namespace col {

    inline void argconfig_static_test() {
        constexpr Arg c1{"name"};
        static_assert(c1.get_name() == "name");
        constexpr auto c2 = Arg{""}
            .set_default(10);
        static_assert(c2.get_default() == 10);
        constexpr auto c3 = Arg{""}
            .set_parser([](const char*) { return 1; });
        static_assert(c3.get_parser()("") == 1);
        constexpr auto c4 = Arg{""}
            .set_default(11)
            .set_parser([](const char*) { return 2; });
        static_assert(c4.get_default() == 11);
        static_assert(c4.get_parser()("") == 2);
        constexpr auto c5 = Arg{""}
            .set_parser([](const char*) { return 3; })
            .set_default(12);
        static_assert(c5.get_default() == 12);
        static_assert(c5.get_parser()("") == 3);
        
        constexpr auto res1 = []() {
            constexpr std::array<const char*, 0> argv{
            };
            auto iter = argv.cbegin();
            const auto s = argv.cend();
            return Arg<bool>{"--flag"}
                .parse(iter, s);
        }();
        static_assert(res1.has_value());
        static_assert(res1.value() == true);
    
        constexpr auto res2 = []() {
            constexpr std::array argv{
                "10"
            };
            auto iter = argv.cbegin();
            const auto s = argv.cend();
            return Arg{"--int"}
                .set_default(1)
                .parse(iter, s);
        }();
        static_assert(res2.has_value());
        static_assert(res2.value() == 10);
    
        constexpr auto res3 = []() {
            constexpr std::array<const char*, 0> argv{
            };
            auto iter = argv.cbegin();
            const auto s = argv.cend();
            return Arg{"--int"}
                .set_parser([](const char*) { return 2; })
                .parse(iter, s);
        }();
        static_assert(res3.has_value() == false);
        static_assert(std::holds_alternative<col::NoValueGivenForOption>(res3.error()));
    
        constexpr auto res4 = []() {
            constexpr std::array argv{
                "foo"
            };
            auto iter = argv.cbegin();
            const auto s = argv.cend();
            return Arg{"--int"}
                .set_parser([](const char*) -> std::optional<int> { return std::nullopt; })
                .parse(iter, s);
        }();
        static_assert(res4.has_value() == false);
        static_assert(std::holds_alternative<col::ConverterConvertionError>(res4.error()));

        constexpr auto res5 = []() {
            constexpr std::array argv{
                "foo"
            };
            auto iter = argv.cbegin();
            const auto s = argv.cend();
            constexpr auto arg = Arg{"--int"}
                .set_parser([](const char* a) -> std::expected<int, col::ParseError> {
                    if( std::string_view{a} == "foo" )
                    {
                        return std::unexpected{col::InvalidNumber{
                            .name = "--int",
                            .arg = a,
                            .err = std::errc::invalid_argument
                        }};
                    }
                    else
                    {
                        return 10;
                    }
                });
            return arg
                .parse(iter, s);
            }();
        static_assert(res5.has_value() == false);
        constexpr auto res5_err = res5.error();
        static_assert(std::holds_alternative<col::InvalidNumber>(res5_err));
        constexpr auto res5_err_raw = std::get<col::InvalidNumber>(res5_err);
        static_assert(res5_err_raw.name == "--int");
        static_assert(res5_err_raw.arg == "foo");
        static_assert(res5_err_raw.err == std::errc::invalid_argument);
    }

    inline void subcmbconfig_without_subsubcommand_static_test() {
        struct Test {
            bool foo;
            int bar;
        };
        constexpr auto res1 = []() static {
            constexpr std::array argv{
                "--foo", "--bar", "10"
            };
            constexpr auto sub = SubCmd<Test>{"test"}
                .add(Arg{"--foo"})
                .add(Arg{"--bar"}.set_parser([](std::string_view) -> std::expected<int, col::ParseError>
                {
                    return std::unexpected{col::UnknownError{}};
                }));
            auto iter = std::ranges::cbegin(argv);
            const auto sentinel = std::ranges::cend(argv);
            return sub.parse(iter, sentinel);
        }();
        static_assert(res1.has_value() == false);
        static_assert(std::holds_alternative<col::UnknownError>(res1.error()));


        constexpr auto res2 = []() static {
            constexpr std::array argv{
                "--foo", "--bar", "10"
            };
            constexpr auto sub = SubCmd<Test>{"test"}
                .add(Arg{"--foo"})
                .add(Arg{"--bar"}.set_parser([](std::string_view) -> std::expected<int, col::ParseError>
                {
                    return 10;
                }));
            auto iter = std::ranges::cbegin(argv);
            const auto sentinel = std::ranges::cend(argv);
            return sub.parse(iter, sentinel);
        }();
        static_assert(res2.has_value());
        constexpr auto res2_ok = *res2;
        static_assert(res2_ok.foo == true);
        static_assert(res2_ok.bar == 10);
    }


    inline void subcmdconfig_with_subsubcommand_static_test() {
        struct SubSubCmdTest{
            bool bar;
        };
        struct SubCmdTest {
            std::variant<std::monostate, SubSubCmdTest> subcmd;
            bool foo;
        };
        constexpr auto res1 = []() static
        {
            constexpr std::array argv{
                "--foo", "subsub", "--bar"
            };
            constexpr auto sub = SubCmd<SubCmdTest>{"sub"}
                .add(Arg{"--foo"})
                .add(SubCmd<SubSubCmdTest>{"subsub"}
                    .add(Arg{"--bar"})
                );
            const auto r = std::ranges::views::all(argv);
            auto iter = std::ranges::cbegin(r);
            const auto sentinel = std::ranges::cend(r);
            return sub.parse(iter, sentinel);
        }();
        static_assert(res1.has_value());
        constexpr auto res1_ok = *res1;
        static_assert(std::holds_alternative<SubSubCmdTest>(res1_ok.subcmd));
        static_assert(res1_ok.foo);
        constexpr auto res1_subsub = std::get<SubSubCmdTest>(res1_ok.subcmd);
        static_assert(res1_subsub.bar);
    }


    inline void cmdconfig_without_subcommand_static_test() {
        struct CmdTest {
            bool foo;
        };
        constexpr auto res1 = []() static
        {
            constexpr std::array argv{
                "--foo",
            };
            constexpr auto cmd = Cmd{"cmd"}
                .add(Arg{"--foo"});
            const auto r = std::ranges::views::all(argv);
            auto iter = std::ranges::cbegin(r);
            const auto sentinel = std::ranges::cend(r);
            return cmd.parse<CmdTest>(iter, sentinel);
        }();
        static_assert(res1.has_value());
        constexpr auto res1_ok = *res1;
        static_assert(res1_ok.foo);
    }

    inline void cmdconfig_with_subcommand_static_test() {
        struct SubCmdTest{
            bool bar;
        };
        struct CmdTest {
            std::variant<std::monostate, SubCmdTest> subcmd;
            bool foo;
        };
        constexpr auto res1 = []() static
        {
            constexpr std::array argv{
                "--foo", "subsub", "--bar"
            };
            constexpr auto cmd = Cmd{"cmd"}
                .add(Arg{"--foo"})
                .add(SubCmd<SubCmdTest>{"subsub"}
                    .add(Arg{"--bar"})
                );
            const auto r = std::ranges::views::all(argv);
            auto iter = std::ranges::cbegin(r);
            const auto sentinel = std::ranges::cend(r);
            return cmd.parse<CmdTest>(iter, sentinel);
        }();
        static_assert(res1.has_value());
        constexpr auto res1_ok = *res1;
        static_assert(std::holds_alternative<SubCmdTest>(res1_ok.subcmd));
        static_assert(res1_ok.foo);
        constexpr auto res1_subsub = std::get<SubCmdTest>(res1_ok.subcmd);
        static_assert(res1_subsub.bar);
    }

    inline void cmd_test() {
        struct SubSubCmd {
            std::optional<std::string> str;
        };
        struct SubCmd1 {
            std::variant<std::monostate, SubSubCmd> subsub;
            int num;
        };
        struct SubCmd2 {
            bool flag;
        };
        struct Cmd {
            std::variant<std::monostate, SubCmd1, SubCmd2> subcmd;
            bool version;
        };
        constexpr auto cmd = col::Cmd{"cmd"}
            .add(col::SubCmd<SubCmd1>{"subcmd1"}
                .add(col::Arg{"--num"}.set_default(1))
                .add(col::SubCmd<SubSubCmd>{"subsubcmd"}
                    .add(col::Arg<std::optional<std::string>>{"str"}))
            )
            .add(col::SubCmd<SubCmd2>{"subcmd2"}
                .add(col::Arg{"--flag"}))
            .add(col::Arg{"--version"})
            ;
        constexpr auto res1 = [&]()
        {
            constexpr std::array argv{
                "subcmd1", "--num", "10", "subsubcmd"
            };

            return cmd.parse<Cmd>(argv);
        }();

        static_assert(res1.has_value());
        constexpr auto cmd_value = *res1;
        static_assert(cmd_value.version == false);
        static_assert(std::holds_alternative<SubCmd1>(cmd_value.subcmd));
        constexpr auto subcmd = std::get<SubCmd1>(cmd_value.subcmd);
        static_assert(subcmd.num == 10);
        static_assert(std::holds_alternative<SubSubCmd>(subcmd.subsub));
        constexpr auto subsubcmd = std::get<SubSubCmd>(subcmd.subsub);
        static_assert(subsubcmd.str.has_value() == false);
    }

} // namespace col
