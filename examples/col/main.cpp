#include <col/command.h>

#include <cstddef>
#include <expected>
#include <format>
#include <optional>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <variant>

int main(int argc, char** argv)
{
    // パース結果を対応させる構造体を定義します。
    struct Cli
    {
        std::string file;
        std::optional<std::string> dir;
    };

    // コマンドを定義します。
    // Builder Pattern で定義できます。
    // 引数の定義順と、パース結果の構造体のメンバの定義順を対応させます。
    constexpr auto cmd = col::Command{"cmd"}
        .add(col::Arg{"--file", "path to .cpp file"}
            .set_required(true)
            .set_parser([](std::string_view file) static
                -> std::expected<std::string, col::ParseError>
            {
                if( file.length() > 4 && file.ends_with(".cpp") )
                {
                    return file.data();
                }
                else
                {
                    return std::unexpected{col::ParserConvertionError{
                        .name = "--file",
                        .arg = file,
                    }};
                }
            }))
        .add(col::Arg{"--dir", "path to directory"}
            .set_default("./build"));

    // 対応させる構造体の型を明示的に指定してパースを実行します。
    // コマンドライン引数を std::ranges::viewable_range として渡します。
    const std::span args{argv + 1, static_cast<std::size_t>(argc - 1)};
    const auto res = cmd.parse<Cli>(args);

    // 戻り値は std::expected<T, col::ParseError> です。
    // パースに成功していれば T が格納されています。
    if( res.has_value() )
    {
        std::print("file = {}", res->file);
        if( res->dir.has_value() )
        {
            std::print(", dir = {}", *res->dir);
        }
        std::println();
    }
    else
    {
        // col::ParseError の実体は std::variant のため、
        // std::visit を使って各エラー型に応じた処理をします。
        // 各エラー型は std::format() で文字列表現を得られます。
        const auto err = res.error();
        // もし "--help" が渡されていれば、エラーは col::ShowHelp になります。
        if( !std::holds_alternative<col::ShowHelp>(err) )
        {
            std::println("error: {}", std::visit([](const auto& e)
                {
                    return std::format("{}", e);
                }, err));
        }
        std::println("{}", cmd.get_help_message());
    }
}
