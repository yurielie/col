#include <col/command.h>

#include <expected>
#include <print>
#include <string>
#include <vector>

namespace {

    #define COL_TO_STRING_IMPL(x) #x
    #define COL_TO_STRING(x) COL_TO_STRING_IMPL(x)

    #define COL_ASSERT(cond) \
        do { \
            if( (cond) == false ) [[unlikely]] { \
                std::println("[FAILED]: at " COL_TO_STRING( __LINE__ ) " of " COL_TO_STRING(__FILE__) ":\n    expected \"" COL_TO_STRING( cond ) "\" is true\n"); \
                return 1; \
            } \
        } while(false)

    #define COL_EXPECT(cond) \
        do { \
            if( (cond) == false ) [[unlikely]] { \
                std::println("[FAILED]: at " COL_TO_STRING( __LINE__ ) " of " COL_TO_STRING(__FILE__) ":\n    expected \"" COL_TO_STRING( cond ) "\" is true\n"); \
            } \
        } while(false)

    int test()
    {
        {
            struct Test
            {
                bool flag;
                int value;
            };
            constexpr auto cmd = col::Command{"cmd"}
                .add(col::Arg{"--flag", "flag"})
                .add(col::Arg{"--value", "int"}
                    .set_default(1)
                    .set_parser([](const char*) -> std::expected<int, col::ParseError>
                    {
                        return 2;
                    }))
                ;
            std::println("constructed help message:\n{}", cmd.get_help_message());

            std::vector<std::string> argv {
                "--flag", "--value", "VALUE"
            };
            const auto res = cmd.parse<Test>(argv);
            COL_ASSERT(res.has_value());
            COL_EXPECT(res->flag == true);
            COL_EXPECT(res->value == 2);
        }

        {
            struct Test
            {
                bool required;
            };
            constexpr auto cmd = col::Command{"cmd"}
                .add(col::Arg{"--required", "required"}.set_required(true))
                ;
            std::vector<std::string> empty{};
            const auto res = cmd.parse<Test>(empty);
            COL_ASSERT(!res.has_value());
            const auto err = res.error();
            COL_ASSERT(std::holds_alternative<col::RequiredOption>(err));
            const auto requiredErr = std::get<col::RequiredOption>(err);
            COL_ASSERT(requiredErr.name == "--required");
        }

        return 0;
    }
}

int main(int, char**)
{
    return test();
}
