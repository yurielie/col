# col

Rust の `clap` ライクな C++ のコマンドラインパーサーライブラリです。

## Restrictions
- C++ 23 以降のみ

## Usage

ヘッダーオンリーライブラリです。

```cpp
#include <col/arg_parser.h>

#include <expected>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <print>

int main(int argc, char** argv)
{
    // パース結果を対応させる構造体を定義します。
    struct Cli
    {
        bool help;
        std::string file;
        std::optional<std::string> dir;
    };

    // パーサーを定義します。
    // Builder Pattern で定義できます。
    // 引数の定義順と、パース結果の構造体のメンバの定義順を対応させます。
    // OptionConfig には、デフォルト値や文字列から T への変換関数も指定できます。
    constexpr auto ap = col::ArgParser()
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

    // 対応させる構造体の型を明示的に指定してパースを実行します。
    // コマンドライン引数を std::ranges::borrowed_range として渡します。
    const auto res = ap.parse<Cli>(std::span{argv + 1, static_cast<std::size_t>(argc - 1)});

    // ヘルプメッセージの表示は能動的に行う必要があります。
    // usage とヘルプは個別に取得できます。また、これらは FlagConfig, OptionConfig<T> 単体でも取得できます。
    const auto show_help = [&ap]()
    {
        std::println("\nusage: ap {}\n{}", ap.get_usage_message(), ap.get_help_message());
    };
    
    // 戻り値は std::expected<T, E> です。
    // パースに成功していれば T が格納されています。
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

```

ビルド時には、インクルードパスを指定します。
```
$ clang++ -std=c++23 -stdlib=libc++ -I ./include -o ap ./main.cpp
```

上記のファイルの実行例です。
```
$ ./ap --help
error: args uninitialized

usage: ap [--help] --file FILE [--dir DIR]

  --help      show help
  --file FILE      path to .cpp file (required)
  --dir DIR      path to directory (default: ./build)
```

```
$ ./ap --file ./include/col/arg_parser.h 
error: not cpp file

usage: ap [--help] --file FILE [--dir DIR]

  --help      show help
  --file FILE      path to .cpp file (required)
  --dir DIR      path to directory (default: ./build)
```

```
$ ./ap --file ./src/main.cpp
file = ./src/main.cpp, dir = ./build
```

```
$ ./ap --file ./src/main.cpp --dir ./out
file = ./src/main.cpp, dir = ./out
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
      ii  libc++-19-dev:amd64             1:19.1.1-1ubuntu1~24.04.2                                       amd64        LLVM C++ Standard library (development files)
      ii  libc++abi-19-dev:amd64          1:19.1.1-1ubuntu1~24.04.2                                       amd64        LLVM low level support for a standard C++ library (development files)
      ```

## Future Works
- 不足している機能のサポート
  - `--help` オプションのデフォルト実装
  - short オプション
  - 位置引数
  - オプション引数における可変長引数
  - etc.
- エラーメッセージの拡充
  - 変換関数やパース結果におけるエラー型を適切に定義する


## License
MIT

