# col

Rust の `clap` ライクな C++ のコマンドラインパーサーライブラリです。

## Restrictions
- C++ 23 以降のみ

## Usage

ヘッダーオンリーライブラリです。
このサンプルコードは [examples/col/main.cpp](./examples/col/main.cpp) にあります。

```cpp
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

```

ビルド時には、インクルードパスを指定します。
```
$ clang++ -std=c++23 -stdlib=libc++ -I ./include -o cmd ./examples/col/main.cpp
```

上記のファイルの実行例です。
```
$ ./cmd --help
cmd --file FILE [--dir DIR]

options:
  --file <FILE>             path to .cpp file
                            (required)

  --dir <DIR>               path to directory
                            default: ./build

```

```
$ ./cmd
error: required option was not given: name='--file'
cmd --file FILE [--dir DIR]

options:
  --file <FILE>             path to .cpp file
                            (required)

  --dir <DIR>               path to directory
                            default: ./build
```

```
$ ./cmd --file
error: no value for option name='--file'
cmd --file FILE [--dir DIR]

options:
  --file <FILE>             path to .cpp file
                            (required)

  --dir <DIR>               path to directory
                            default: ./build

```

```
$ ./cmd --file ./include/col/command.h 
error: parser failed to convert argument: name='--file' arg='./include/col/command.h'
cmd --file FILE [--dir DIR]

options:
  --file <FILE>             path to .cpp file
                            (required)

  --dir <DIR>               path to directory
                            default: ./build

```

```
$ ./cmd --file ./examples/col/main.cpp 
file = ./examples/col/main.cpp, dir = ./build
```

```
$ ./cmd --file ./examples/col/main.cpp --dir ./out
file = ./examples/col/main.cpp, dir = ./out
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
  - clang++-19
    - ```
      $ clang++-19 --version
      Ubuntu clang version 19.1.1 (1ubuntu1~24.04.2)
      Target: x86_64-pc-linux-gnu
      Thread model: posix
      InstalledDir: /usr/lib/llvm-19/bin
      ```
- libc++
  - libc++-19-dev
    - ```
      $ dpkg -l |grep libc++ | grep dev
      ii  libc++-19-dev:amd64       1:19.1.1-1ubuntu1~24.04.2   amd64    LLVM C++ Standard library (development files)
      ii  libc++abi-19-dev:amd64    1:19.1.1-1ubuntu1~24.04.2   amd64    LLVM low level support for a standard C++ library (development files)
      ```

## Future Works
- 不足している機能のサポート
  - オプション引数における可変長引数
  - etc.
- エラーメッセージの拡充
  - 変換関数やパース結果におけるエラー型を適切に定義する


## License
MIT
