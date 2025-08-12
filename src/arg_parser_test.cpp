#include <col/arg_parser.h>

#include <array>
#include <span>
#include <string>
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

    constexpr bool ok = []() noexcept
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
}


int main(int argc, char** argv) noexcept
{
    struct Cli {
        bool help;
        std::optional<std::string> file;
    };

    const auto res = col::ArgParser()
        .add_config(col::FlagConfig{"--help", "show help"})
        .add_config(col::OptionConfig<std::optional<std::string>>{"--file", "FILE", "input file"})
        .parse<Cli>(std::span{argv + 1, static_cast<std::size_t>(argc - 1)});
    
    if( res.has_value())
    {
        std::print("help = {}", res->help);
        if( res->file.has_value() )
        {
            std::print(", file = {}", *res->file);
        }
        std::println();
    }
    else
    {
        std::println("error: {}\n", res.error());
    }
}
