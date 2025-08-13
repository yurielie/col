#include <col/arg_parser.h>

#include <array>
#include <expected>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <print>

constexpr void test() noexcept
{
    enum Res {
        Err,
        Ok,
    };
    struct Sample
    {
        bool verbose;
        int count;
        Res res;
    };

    constexpr bool ok = []() static noexcept
    {
        const auto ap = col::ArgParser{}
            .add_config(col::FlagConfig{"--verbose", "show verbose"})
            .add_config(col::OptionConfig<int>{"--count", "N", "number"}
                .set_default_value(10))
            .add_config(col::OptionConfig<Res>{"--res", "RES", "result"}
                .set_converter([](std::string_view arg) noexcept -> std::expected<Res, std::string> {
                    if( arg == "ok" ) {
                        return Res::Ok;
                    } else if( arg == "err") {
                        return Res::Err;
                    } else {
                        return std::unexpected("invalid Enu");
                    }
                })
            )
            ;
        std::array raw_args{
            "--verbose",
            "--res", "ok"
        };
        std::span args{raw_args};
        
        const auto res = ap.parse<Sample>(args);

        return res.has_value() && res->verbose == true && res->count == 10 && res->res == Res::Ok;
    }();
    static_assert(ok);

    constexpr bool null_callback = []() static noexcept
    {
        constexpr auto ap = col::ArgParser{}
            .add_config(col::OptionConfig<int>{"--num", "N", ""}
                .set_converter<std::expected<int, std::string>(*)(const char*)>(nullptr));
        std::array raw_args{
            "--num", "0"
        };
        std::span args{raw_args};
        const auto res = ap.parse<int>(args);
        return !res.has_value() && res.error() == std::string{"converter callback is nullptr."};
    }();
    static_assert(null_callback);

    constexpr bool default_values = []() static noexcept
    {
        struct Sample
        {
            bool flag;
            int opt;
        };
        constexpr auto ap = col::ArgParser{}
            .add_config(col::FlagConfig{"--flag", "flag 1"}
                .set_default_value(true))
            .add_config(col::OptionConfig<int>{"--opt", "N", "opt"}
                .set_default_value(1));
        std::array<const char*, 0> args{};
        const auto res = ap.parse<Sample>(std::span{args});
        return res.has_value() && res->flag == true && res->opt == 1;
    }();
    static_assert(default_values);

    static_assert(col::ArgParser{}.get_usage_message().length() == 0);
    static_assert(col::ArgParser{}.get_help_message().length() == 0);
    static_assert(col::ArgParser{}.add_config(col::FlagConfig{"--help", "show help"}).get_usage_message().length() > 0);
    static_assert(col::ArgParser{}.add_config(col::FlagConfig{"--help", "show help"}).get_help_message().length() > 0);
    static_assert(col::ArgParser{}.add_config(col::OptionConfig<std::string>{"--opt", "OPT", "opt"}).get_usage_message().length() > 0);
    static_assert(col::ArgParser{}.add_config(col::OptionConfig<const char*>{"--opt", "OPT", "opt"}).get_help_message().length() > 0);

    static_assert(col::OptionConfig<std::string>{"name", "value", ""}.get_usage_message() == "name value");
    static_assert(col::OptionConfig<std::string>{"name", "value", ""}.set_required(true).get_usage_message() == "name value");
    static_assert(col::OptionConfig<std::string>{"name", "value", ""}.set_default_value("").get_usage_message() == "[name value]");
    static_assert(col::OptionConfig<std::string>{"name", "value", ""}.set_required(true).set_default_value("").get_usage_message() == "name value");
    static_assert(col::OptionConfig<std::optional<std::string>>{"name", "value", ""}.get_usage_message() == "[name value]");
    static_assert(col::OptionConfig<std::optional<std::string>>{"name", "value", ""}.set_required(true).get_usage_message() == "name value");
    static_assert(col::OptionConfig<std::optional<std::string>>{"name", "value", ""}.set_default_value("").get_usage_message() == "[name value]");
    static_assert(col::OptionConfig<std::optional<std::string>>{"name", "value", ""}.set_required(true).set_default_value("").get_usage_message() == "name value");
}


int main(int argc, char** argv) noexcept
{
    struct Cli
    {
        bool help;
        std::string file;
        std::optional<std::string> dir;
    };

    constexpr auto ap = col::ArgParser()
        .add_config(col::FlagConfig{"--help", "show help"})
        .add_config(col::OptionConfig<std::string>{"--file", "FILE", "path to .cpp file"}
            .set_required(true)
            .set_converter([](std::string_view file) static noexcept -> std::expected<std::string, std::string>
            {
                if( file.length() > 4 && file.ends_with(".cpp") )
                {
                    return file.data();
                }
                else
                {
                    return std::unexpected{"not .cpp file"};
                }
            }))
        .add_config(col::OptionConfig<std::optional<std::string>>{"--dir", "DIR", "path to directory"}
            .set_default_value("./build"));

    const auto res = ap.parse<Cli>(std::span{argv + 1, static_cast<std::size_t>(argc - 1)});

    const auto show_help = [&ap]() noexcept
    {
        std::println("\nusage: ap {}\n{}", ap.get_usage_message(), ap.get_help_message());
    };
    
    if( res.has_value() )
    {
        if( res->help )
        {
            show_help();
            return 0;
        }
        std::print("file = {}", res->file);
        if( res->dir.has_value() )
        {
            std::print(", dir = {}", *res->dir);
        }
        std::println();
    }
    else
    {
        std::println("error: {}", res.error());
        show_help();
    }
}
