#include <col/command.h>

#include <array>
#include <string_view>

namespace col {

    template <class ArgT, class T = blank, class D = blank, class P = blank>
    inline constexpr auto is_expected_arg = std::same_as<std::remove_cvref_t<ArgT>, Arg<T, D, P>>;

    [[maybe_unused]]
    inline constexpr int default_fn_int() { return 1; }
    [[maybe_unused]]
    inline constexpr int value_parser_cstr_int(const char*) { return 1; }

    inline void arg_static_test() {
        // 型指定もデフォルト値指定も行わない場合
        constexpr Arg arg_empty{"name", "help"};
        static_assert(arg_empty.get_name() == "name");
        static_assert(arg_empty.get_help() == "help");
        static_assert(is_expected_arg<decltype(arg_empty)>);

        /* T を推論する場合 */

        // デフォルト値を値で指定する場合
        constexpr auto arg_int_default = Arg{"name", "help"}
            .set_default(10);
        static_assert(arg_int_default.get_default() == 10);
        static_assert(is_expected_arg<decltype(arg_int_default), Deduced<int>, int>);

        // デフォルト値を関数ポインタで指定する場合
        constexpr auto arg_int_default_fn_ptr = Arg{"name", "help"}
            .set_default(&default_fn_int);
        static_assert(is_expected_arg<decltype(arg_int_default_fn_ptr), Deduced<int>, int(*)()>);

        // デフォルト値を関数への参照で指定する場合
        constexpr auto arg_int_default_fn_ref = Arg{"name", "help"}
            .set_default(default_fn_int);
        static_assert(is_expected_arg<decltype(arg_int_default_fn_ref), Deduced<int>, int(*)()>);

        // パーサーを関数ポインタで指定する場合
        constexpr auto arg_int_parser_fn_ptr = Arg{"name", "help"}
            .set_parser(&value_parser_cstr_int);
        static_assert(arg_int_parser_fn_ptr.get_parser()("") == 1);
        static_assert(is_expected_arg<decltype(arg_int_parser_fn_ptr), Deduced<int>, blank, int(*)(const char*)>);

        // パーサーを関数への参照で指定する場合
        constexpr auto arg_int_parser_fn_ref = Arg{"name", "help"}
            .set_parser(value_parser_cstr_int);
        static_assert(arg_int_parser_fn_ref.get_parser()("") == 1);
        static_assert(is_expected_arg<decltype(arg_int_parser_fn_ref), Deduced<int>, blank, int(*)(const char*)>);

        // パーサーをラムダの左辺値参照で指定する場合
        constexpr auto int_parser_lambda = [](const char*) { return 1; };
        constexpr auto arg_int_parser_lambda = Arg{"name", "help"}
            .set_parser(int_parser_lambda);
        static_assert(arg_int_parser_lambda.get_parser()("") == 1);
        static_assert(is_expected_arg<decltype(arg_int_parser_lambda), Deduced<int>, blank, std::remove_cvref_t<decltype(int_parser_lambda)>>);

        // パーサーをラムダの右辺値参照で指定する場合
        constexpr auto arg_int_parser_lambda_rv = Arg{"name", "help"}
            .set_parser([](const char*){return 1;});
        static_assert(arg_int_parser_lambda_rv.get_parser()("") == 1);
        // lambda の型を得られないので一致の静的テストできない

        // パーサーが std::optional を返す場合
        constexpr auto arg_optional_int_parser = Arg{"name", "help"}
            .set_parser([](const char*){return std::optional{1};});
        static_assert(std::same_as<decltype(arg_optional_int_parser)::value_type, Deduced<int>>);

        // パーサーが std::expected を返す場合
        constexpr auto arg_expected_int_parser = Arg{"name", "help"}
            .set_parser([](const char*) -> std::expected<int, col::ParseError> { return 1; });
        static_assert(std::same_as<decltype(arg_expected_int_parser)::value_type, Deduced<int>>);

        /* デフォルト値もパーサーも指定する場合 かつ デフォルト値の型とパーサーの戻り値の型が同じ場合 */

        // デフォルト値を指定してからパーサーを指定する場合
        constexpr auto arg_int_default_int_parser = Arg{"name", "help"}
            .set_default(11)
            .set_parser(&value_parser_cstr_int);
        static_assert(is_expected_arg<decltype(arg_int_default_int_parser), Deduced<int>, int, int(*)(const char*)>);

        // パーサーを指定してからデフォルト値を指定する場合
        constexpr auto arg_int_parser_int_default = Arg{"name", "help"}
            .set_parser(&value_parser_cstr_int)
            .set_default(12);
        static_assert(is_expected_arg<decltype(arg_int_parser_int_default), Deduced<int>, int, int(*)(const char*)>);

        /* デフォルト値もパーサーも指定する場合 かつ デフォルト値の型とパーサーの戻り値の型が異なる場合 */

        // 共通の型で推論される
        constexpr auto arg_cstr_default_string_parser = Arg{"name", "help"}
            .set_default("foo")
            .set_parser([](const char*){return std::string{};});
        static_assert(std::same_as<decltype(arg_cstr_default_string_parser)::value_type, Deduced<std::string>>);

        // パーサーが std::optional を返す場合、その有効値との共通の型となる
        constexpr auto arg_cstr_default_optional_string_parser = Arg{"name", "help"}
            .set_default("foo")
            .set_parser([](const char*){return std::optional{std::string{}};});
        static_assert(std::same_as<decltype(arg_cstr_default_optional_string_parser)::value_type, Deduced<std::string>>);

        // パーサーが std::expected を返す場合、その有効値との共通の型となる
        constexpr auto arg_cstr_default_expected_string_parser = Arg{"name", "help"}
            .set_default("foo")
            .set_parser([](const char*) -> std::expected<std::string, col::ParseError> {return std::string{};});
        static_assert(std::same_as<decltype(arg_cstr_default_expected_string_parser)::value_type, Deduced<std::string>>);

        /* T を明示的に指定する場合 */

        // 明示的に型指定する場合
        constexpr auto arg_set_value_type = Arg{"name", "help"}
            .set_value_type<int>();
        static_assert(std::same_as<decltype(arg_set_value_type)::value_type, int>);
        constexpr auto arg_value_type_explicit = Arg<int>{"name", "help"};
        static_assert(std::same_as<decltype(arg_value_type_explicit)::value_type, int>);

        // T を明示的に指定しデフォルト値を値で指定する場合
        constexpr auto arg_explicit_int_default_int_value = Arg<int>{"name", "help"}
            .set_default(10);
        static_assert(std::same_as<decltype(arg_explicit_int_default_int_value)::value_type, int>);

        // T を明示的に指定しデフォルト値を関数で指定する場合
        constexpr auto arg_explicit_int_default_int_fn = Arg<int>{"name", "help"}
            .set_default(&default_fn_int);
        static_assert(std::same_as<decltype(arg_explicit_int_default_int_fn)::value_type, int>);

        // T を明示的に指定しパーサーを指定する場合
        constexpr auto arg_explicit_int_parser_cstr_int = Arg<int>{"name", "help"}
            .set_parser(&value_parser_cstr_int);
        static_assert(std::same_as<decltype(arg_explicit_int_parser_cstr_int)::value_type, int>);

        // T を明示的に指定し optional を返すパーサーを指定する場合
        constexpr auto arg_explicit_int_parser_cstr_optional_int = Arg<int>{"name", "help"}
            .set_parser([](const char*) { return std::optional{10}; });
        static_assert(std::same_as<decltype(arg_explicit_int_parser_cstr_optional_int)::value_type, int>);

        // T を明示的に指定し expected を返すパーサーを指定する場合
        constexpr auto arg_explicit_int_parser_cstr_expected_int = Arg<int>{"name", "help"}
            .set_parser([](const char*) -> std::expected<int, col::ParseError> { return 10; });
        static_assert(std::same_as<decltype(arg_explicit_int_parser_cstr_expected_int)::value_type, int>);

        // T を明示的に指定しデフォルト値を指定してからパーサーを指定する場合
        constexpr auto arg_explicit_int_default_int_fn_parser_expected_int = Arg<int>{"name", "help"}
            .set_default(&default_fn_int)
            .set_parser([](const char*) -> std::expected<int, col::ParseError> { return 1; });
        static_assert(std::same_as<decltype(arg_explicit_int_default_int_fn_parser_expected_int)::value_type, int>);

        // T を明示的に指定しデフォルト値を指定してからパーサーを指定する場合
        constexpr auto arg_explicit_optional_string_parser_optional_string_default_cstr = Arg<std::optional<std::string>>{"name", "help"}
            .set_parser([](std::string_view) { return std::optional{std::string{}}; })
            .set_default("");
        static_assert(std::same_as<decltype(arg_explicit_optional_string_parser_optional_string_default_cstr)::value_type, std::optional<std::string>>);

        /* Arg::parse() の静的テスト */

        // T = bool のとき、 parse は必ず成功する。パース結果の値はデフォルト値の設定に依存する。
        // 指定しない場合はデフォルト値が false なので、反転した true が返る。
        constexpr auto arg_bool_parse = []() {
            constexpr std::array<const char*, 0> argv{
            };
            auto iter = argv.cbegin();
            const auto s = argv.cend();
            return Arg<bool>{"flag", "help"} // 実際には Cmd, SubCmd に渡すと T = bool に設定されるので型指定は不要
                .parse(iter, s);
        }();
        static_assert(arg_bool_parse.has_value());
        static_assert(arg_bool_parse.value() == true);

        // デフォルト値が true のときは、反転して false が返る
        constexpr auto arg_bool_parse_default_false = []() {
            constexpr std::array<const char*, 0> argv{
            };
            auto iter = argv.cbegin();
            const auto s = argv.cend();
            return Arg{"flag", "help"}
                .set_default(true)
                .set_value_type<bool>() // 実際には Deduced<T> は外れる
                .parse(iter, s);
        }();
        static_assert(arg_bool_parse_default_false.has_value());
        static_assert(arg_bool_parse_default_false.value() == false);
    
        // 整数型の場合はパーサーを指定しなくても既定のパーサーがある
        constexpr auto arg_int_parse_trival_parser = []() {
            constexpr std::array argv{
                "10"
            };
            auto iter = argv.cbegin();
            const auto s = argv.cend();
            return Arg{"int", "help"}
                .set_default(1)
                .set_value_type<int>() // 実際には Deduced<T> は外れる
                .parse(iter, s);
        }();
        static_assert(arg_int_parse_trival_parser.has_value());
        static_assert(arg_int_parse_trival_parser.value() == 10);
    
        // 渡ってくる引数値がないとパースは失敗する
        constexpr auto arg_int_parse_failed_missing_option_value = []() {
            constexpr std::array<const char*, 0> argv{
            };
            auto iter = argv.cbegin();
            const auto s = argv.cend();
            return Arg{"int", "help"}
                .set_parser([](const char*) { return 2; })
                .set_value_type<int>() // 実際には Deduced<T> は外れる
                .parse(iter, s);
        }();
        static_assert(arg_int_parse_failed_missing_option_value.has_value() == false);
        static_assert(std::holds_alternative<col::MissingOptionValue>(arg_int_parse_failed_missing_option_value.error()));

        // パーサーは std::optional でパース結果を返せる
        constexpr auto arg_int_parse_optinal_ok = []() {
            constexpr std::array argv{
                "foo"
            };
            auto iter = argv.cbegin();
            const auto s = argv.cend();
            return Arg{"int", "help"}
                .set_parser([](const char*) { return std::optional{10}; })
                .set_value_type<int>() // 実際には Deduced<T> は外れる
                .parse(iter, s);
        }();
        static_assert(arg_int_parse_optinal_ok.has_value());
        static_assert(arg_int_parse_optinal_ok.value() == 10);

        // パーサーが std::nullopt を返した場合は失敗扱いになる
        constexpr auto arg_int_parse_failed_nullopt_convertion_error = []() {
            constexpr std::array argv{
                "foo"
            };
            auto iter = argv.cbegin();
            const auto s = argv.cend();
            return Arg{"int", "help"}
                .set_value_type<int>() // 実際には Deduced<T> は外れる
                .set_parser([](const char*) -> std::optional<int> { return std::nullopt; })
                .parse(iter, s);
        }();
        static_assert(arg_int_parse_failed_nullopt_convertion_error.has_value() == false);
        static_assert(std::holds_alternative<col::ConverterConvertionError>(arg_int_parse_failed_nullopt_convertion_error.error()));

        // パーサーが std::unexpected を返した場合は失敗扱いになる
        constexpr auto arg_int_parse_expected_ok = []() {
            constexpr std::array argv{
                "foo"
            };
            auto iter = argv.cbegin();
            const auto s = argv.cend();
            constexpr auto arg = Arg{"int", "help"}
                .set_value_type<int>() // 実際には Deduced<T> は外れる
                .set_parser([](const char*) -> std::expected<int, col::ParseError> {
                    return 10;
                });
            return arg
                .parse(iter, s);
            }();
        static_assert(arg_int_parse_expected_ok.has_value());
        static_assert(arg_int_parse_expected_ok.value() == 10);

        // パーサーが std::unexpected を返した場合は失敗扱いになる
        constexpr auto arg_int_parse_failed_unexpected_convertion_error = []() {
            constexpr std::array argv{
                "foo"
            };
            auto iter = argv.cbegin();
            const auto s = argv.cend();
            constexpr auto arg = Arg{"int", "help"}
                .set_parser([](const char* a) -> std::expected<int, col::ParseError> {
                    return std::unexpected{col::InvalidNumber{
                        .name = "int",
                        .arg = a,
                        .err = std::errc::invalid_argument
                    }};
                })
                .set_value_type<int>() // 実際には Deduced<T> は外れる
                ;
            return arg
                .parse(iter, s);
            }();
        static_assert(arg_int_parse_failed_unexpected_convertion_error.has_value() == false);
        static_assert(std::holds_alternative<col::InvalidNumber>(arg_int_parse_failed_unexpected_convertion_error.error()));
        static_assert(std::get<col::InvalidNumber>(arg_int_parse_failed_unexpected_convertion_error.error()).name == "int");
        static_assert(std::get<col::InvalidNumber>(arg_int_parse_failed_unexpected_convertion_error.error()).arg == "foo");
        static_assert(std::get<col::InvalidNumber>(arg_int_parse_failed_unexpected_convertion_error.error()).err == std::errc::invalid_argument);
    }


    inline void subcmd_static_test() {
        struct SubSubTest
        {
            bool flag;
            std::string str;
        };
        struct SubTest
        {
            std::variant<std::monostate, SubSubTest> subsub;
            bool flag;
            std::optional<std::string> optstr;
        };
        constexpr auto subsubcmd = SubCmd<SubSubTest>{"subsub", "help"}
            .add(Arg{"flag", "help"})
            .add(Arg{"str", "help"}
                .set_value_type<std::string>()
                .set_default("")
            );
        static_assert(subsubcmd.get_usage("", 2ZU) ==
            "help\n\nUsage: subsub [OPTIONS]\n\nOptions:\n  --flag       help\n  --str <str>  help\n");
        
        constexpr auto subcmd = SubCmd<SubTest>{"sub", "help"}
            .add(Arg{"flag", "help"})
            .add(Arg<std::optional<std::string>>{"optstr", "help"}
                .set_parser([](const char* s) { return std::optional<std::string>{s}; })
            )
            .add(SubCmd<SubSubTest>{"subsub", "help"}
                .add(Arg{"flag", "help"})
                .add(Arg{"str", "help"}
                    .set_value_type<std::string>()
                    .set_default("")
                )
            );
        static_assert(subcmd.get_usage("", 2ZU) == 
            "help\n\nUsage: sub [OPTIONS] [COMMAND]\n\nOptions:\n  --flag             help\n  --optstr <optstr>  help\n\nCommands:\n  subsub  help\n");
    }

    inline void subcmb_without_subsubcmd_static_test() {
        struct Test {
            bool foo;
            int bar;
        };
        constexpr auto res1 = []() static {
            constexpr std::array argv{
                "--foo", "--bar", "10"
            };
            constexpr auto sub = SubCmd<Test>{"test", "help"}
                .add(Arg{"foo", "help"})
                .add(Arg{"bar", "help"}.set_parser([](std::string_view) -> std::expected<int, col::ParseError>
                {
                    return std::unexpected{col::UnknownError{}};
                }));
            auto iter = std::ranges::cbegin(argv);
            const auto sentinel = std::ranges::cend(argv);
            return sub.parse("", iter, sentinel);
        }();
        static_assert(res1.has_value() == false);
        static_assert(std::holds_alternative<col::UnknownError>(res1.error()));


        constexpr auto res2 = []() static {
            constexpr std::array argv{
                "--foo", "--bar", "10"
            };
            constexpr auto sub = SubCmd<Test>{"test", "help"}
                .add(Arg{"foo", "help"})
                .add(Arg{"bar", "help"}.set_parser([](std::string_view) -> std::expected<int, col::ParseError>
                {
                    return 10;
                }));
            auto iter = std::ranges::cbegin(argv);
            const auto sentinel = std::ranges::cend(argv);
            return sub.parse("", iter, sentinel);
        }();
        static_assert(res2.has_value());
        constexpr auto res2_ok = *res2;
        static_assert(res2_ok.foo == true);
        static_assert(res2_ok.bar == 10);
    }


    inline void subcmd_with_subsubcmd_static_test() {
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
            constexpr auto sub = SubCmd<SubCmdTest>{"sub", "help"}
                .add(Arg{"foo", "help"})
                .add(SubCmd<SubSubCmdTest>{"subsub", "help"}
                    .add(Arg{"bar", "help"})
                );
            const auto r = std::ranges::views::all(argv);
            auto iter = std::ranges::cbegin(r);
            const auto sentinel = std::ranges::cend(r);
            return sub.parse("", iter, sentinel);
        }();
        static_assert(res1.has_value());
        constexpr auto res1_ok = *res1;
        static_assert(std::holds_alternative<SubSubCmdTest>(res1_ok.subcmd));
        static_assert(res1_ok.foo);
        constexpr auto res1_subsub = std::get<SubSubCmdTest>(res1_ok.subcmd);
        static_assert(res1_subsub.bar);
    }


    inline void cmd_without_subcmd_static_test() {
        struct CmdTest {
            bool foo;
        };
        constexpr auto res1 = []() static
        {
            constexpr std::array argv{
                "--foo",
            };
            constexpr auto cmd = Cmd{"cmd", "description"}
                .add(Arg{"foo", "help"});
            const auto r = std::ranges::views::all(argv);
            auto iter = std::ranges::cbegin(r);
            const auto sentinel = std::ranges::cend(r);
            return cmd.parse<CmdTest>(iter, sentinel);
        }();
        static_assert(res1.has_value());
        constexpr auto res1_ok = *res1;
        static_assert(res1_ok.foo);
    }

    inline void cmd_with_subcmd_static_test() {
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
            constexpr auto cmd = Cmd{"cmd", "description"}
                .add(Arg{"foo", "help"})
                .add(SubCmd<SubCmdTest>{"subsub", "help"}
                    .add(Arg{"bar", "help"})
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
        constexpr auto cmd_int_deduced = Cmd{"name", "cmd"}
            .add(Arg<Deduced<int>>{"name", "help"});
        static_assert(std::same_as<std::remove_cvref_t<decltype(cmd_int_deduced)>, Cmd<Arg<int>>>);
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
        constexpr auto cmd = col::Cmd{"cmd", "description"}
            .add(col::SubCmd<SubCmd1>{"subcmd1", "help"}
                .add(col::Arg{"num", "help"}.set_default(1))
                .add(col::SubCmd<SubSubCmd>{"subsubcmd", "help"}
                    .add(col::Arg<std::optional<std::string>>{"str", "help"}))
            )
            .add(col::SubCmd<SubCmd2>{"subcmd2", "help"}
                .add(col::Arg{"flag", "help"}))
            .add(col::Arg{"version", "help"})
            ;
        static_assert(cmd.get_name() == "cmd");
        static_assert(cmd.get_help() == "description");
        // static_assert(cmd.get_usage_detail(0ZU, 2ZU) == "cmd\n  descriotion");
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

    inline void cmd_failure_test() {
        struct SubCmdTest
        {
            std::string str;
        };
        struct CmdTest
        {
            std::variant<std::monostate, SubCmdTest> subcmd;
            std::string str;
        };
        constexpr auto cmd = Cmd{"cmd", ""}
            .add(Arg<std::string>{"str", "string"})
            .add(SubCmd<SubCmdTest>{"sub", ""}
                .add(Arg{"str", "string"}
                    .set_value_type<std::string>()));
        
        constexpr auto cmd_duperr = [&]() {
            constexpr std::array args{
                "--str", "s", "--str", "t"
            };
            return cmd.parse<CmdTest>(args);
        }();
        static_assert(cmd_duperr.has_value() == false);
        static_assert(std::holds_alternative<col::DuplicateOption>(cmd_duperr.error()));
        
        constexpr auto subcmd_duperr = [&]() {
            constexpr std::array args{
                "sub", "--str", "s", "--str", "t"
            };
            return cmd.parse<CmdTest>(args);
        }();
        static_assert(subcmd_duperr.has_value() == false);
        static_assert(std::holds_alternative<col::DuplicateOption>(subcmd_duperr.error()));
    }

} // namespace col
