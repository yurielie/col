#include <col/arg_parser.h>

#include <expected>
#include <optional>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <variant>

int main(int argc, char** argv)
{
    struct Cli
    {
        bool help;
        std::string file;
        std::optional<std::string> dir;
    };

    constexpr auto ap = col::ArgParser{}
        .add_config(col::FlagConfig{"--help", "show help"})
        .add_config(col::OptionConfig<std::string>{"--file", "FILE", "path to .cpp file"}
            .set_required(true)
            .set_converter([](std::string_view file) static -> std::expected<std::string, std::string>
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

    const auto show_help = [&ap]()
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
        std::println("index = {}", res.error().index());
        std::println("error: {}", std::visit(
            [](const auto& err)
            {
                return std::format("{}", err);
            },
            res.error()));
        show_help();
    }
}
