# col

C++ の色々なライブラリ。

## Restrictions
- C++ 23 以降のみ

## Usage

### <col/command.h>

Rust の `clap` ライクな C++ のコマンドラインパーサーライブラリです。
ヘッダーオンリーライブラリです。
このサンプルコードは [examples/col/command/main.cpp](./examples/col/command/main.cpp) にあります。

```cpp
#include <col/command.h>

#include <expected>
#include <optional>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <variant>

int main(int argc, char** argv)
{
    // パース結果を対応させる構造体を定義します。
    struct SubSubCmd
    {
        int num;
    };
    struct SubCmd1
    {
        // サブコマンドは構造体の先頭に std::variant<std::monostate, ...> として定義します。
        std::variant<std::monostate, SubSubCmd> subsubcmd;
        std::optional<std::string> str_opt;
    };
    struct SubCmd2
    {
        std::string str;
    };
    struct Cmd
    {
        // サブコマンドを持つ構造体も再帰的にサブコマンドに指定できます。
        std::variant<std::monostate, SubCmd1, SubCmd2> subcmd;
        bool version;
        bool verbose;
    };

    // コマンドを定義します。
    // Builder Pattern で定義できます。
    // それぞれの引数の型の値を定義順に並べたとき、パース結果の構造体を構築できる必要があります。
    constexpr auto parser = col::Cmd{"cmd", "sample command"}
        .add(col::Arg{"version", "show version"}) // 指定しなければ T = bool と見做されます。
        .add(col::Arg<bool>{"verbose", "show verbose"}) // 明示的に型を指定できます。
        .add(col::SubCmd<SubCmd1>{"subcmd1", "subcommand 1"} // サブコマンドに対しては、対応させる構造体の型を指定します。
            .add(col::SubCmd<SubSubCmd>{"subsubcmd", "subcommand of subcmd1"} // サブコマンドの入れ子が可能です。
                .add(col::Arg{"num", "number"}
                    .set_default(1) // 指定したデフォルト値から T が推論されます(この例では T = int)。
                ) // T が整数型・浮動小数点数型の場合、パーサーを指定しなければ std::from_chars() に基づく規定のパーサーが利用されます。
            )
            .add(col::Arg{"str_opt", "string option as std::optional<std::string>"}
                .set_value_type<std::optional<std::string>>() // set_value_type<T>() でも型を明示的に指定できます。
                .set_default(".") // T が指定されていても、T を構築できるデフォルト値であれば指定できます。
            )
        )
        .add(col::SubCmd<SubCmd2>{"subcmd2", "subcommand 2"}
            .add(col::Arg{"str", "string option as std::string"}
                // パーサーの戻り値から T が推論されます。 std::otional<T>, std::expected<T, E> の場合はその有効値になります(この例では T = std::string)。
                // パーサーを指定する場合は const char* で呼び出し可能である必要があります。
                // パーサーの戻り値型には T, std::optional<T>, std::expected<T, E> (requires std::convertible_to<E, col::ParseError>) が指定できます。
                .set_parser([](const char* arg) -> std::expected<std::string, col::ParseError> 
                {
                    if( std::string_view{arg} == "foo" )
                    {
                        return arg;
                    }
                    else
                    {
                        return std::unexpected{
                            col::ConverterConvertionError{
                                .name = "str",
                                .arg = arg,
                            }
                        };
                    }
                })
            )
        )
        ;

    // 対応させる構造体の型を明示的に指定してパースを実行します。
    // コマンドライン引数を std::ranges::viewable_range として渡します。
    const std::span args{argv + 1, argv + argc};
    const auto res = parser.parse<Cmd>(args);

    // 戻り値は std::expected<T, col::ParseError> です。
    // パースに成功していれば T が格納されています。
    if( res.has_value() )
    {
        const auto cmd = *res;
        std::println("[cmd] version = {}", cmd.version);
        switch( cmd.subcmd.index() )
        {
            case 1:
                {
                    const auto subcmd1 = std::get<SubCmd1>(cmd.subcmd); 
                    std::print("  [subcmd1]");
                    if( subcmd1.str_opt.has_value() )
                    {
                        std::print(" str_opt = {}", *subcmd1.str_opt);
                    }
                    std::println();
                    switch( subcmd1.subsubcmd.index() )
                    {
                        case 1:
                            {
                                const auto subsubcmd = std::get<SubSubCmd>(subcmd1.subsubcmd);
                                std::println("    [subsubcmd] num = {}", subsubcmd.num);
                            }
                            break;
                        default:
                            break;
                    }
                }
                break;
            case 2:
                {
                    const auto subcmd2 = std::get<SubCmd2>(cmd.subcmd);
                    std::print(" [subcmd2]");
                    std::println(" str= {} ", subcmd2.str);
                }
                break;
            default:
                break;
        }
    }
    else
    {
        // col::ParseError の実体は std::variant です。
        // 各エラー型は std::formatter を特殊化しており、文字列表示できます。
        // 
        // 各コマンドおよびサブコマンドにはヘルプオプション("--help") が自動実装されます。指定されると、 col::ShowHelp が返ります。
        // ヘルプメッセージの表示は std::format や col::ShowHelp::help_message を利用します。
        std::visit([](const auto& e) static
            {
                std::println("{}", e);
            }, res.error());
    }
}

```

ビルド時には、インクルードパスを指定します。
```
$ clang++ -std=c++23 -stdlib=libc++ -I ./include -o cmd ./examples/col/main.cpp
```

上記サンプルにおけるヘルプメッセージの例です。
```
$ ./cmd --help
sample command

Usage: cmd [OPTIONS] [COMMAND]

Options:
    --version    show version
    --verbose    show verbose

Commands:
    subcmd1    subcommand 1
    subcmd2    subcommand 2

```

## Suported platform

開発に使用している環境です。結果的にサポート対象です。

- OS
  - Ubuntu 24.04 (WSL)
    - ```
      $ uname -r
      5.15.167.4-microsoft-standard-WSL2
      ```
- Compiler
  - clang++-22
    - ```
      $ clang++-22 --version
      Ubuntu clang version 22.0.0 (++20251012081810+e5827e7b90d8-1~exp1~20251012082029.1222)
      Target: x86_64-pc-linux-gnu
      Thread model: posix
      InstalledDir: /usr/lib/llvm-22/bin
      ```
- libc++
  - libc++-22-dev
    - ```
      $ dpkg -l | grep libc++ | grep dev
      ii  libc++-22-dev     1:22~++20251012081810+e5827e7b90d8-1~exp1~20251012082029.1222   amd64    LLVM C++ Standard library (development files)
      ii  libc++abi-22-dev  1:22~++20251012081810+e5827e7b90d8-1~exp1~20251012082029.1222   amd64    LLVM low level support for a standard C++ library (development files)
      ```


## License
MIT
