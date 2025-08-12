# col

Rust の `clap` ライクな C++ のコマンドラインパーサーライブラリです。

## Restrictions
- C++ 23 以降のみ

## Usage

ヘッダーオンリーライブラリです。

```cpp
#include <col/arg_parser.h>

#include <optional>
#include <span>
#include <string>
#include <print>

int main(int argc, char** argv)
{
    // パース結果を対応させる構造体を定義します。
    struct Cli {
        bool help;
        std::optional<std::string> file;
    };

    // パーサーを定義します。
    // Builder Pattern で定義できます。
    // OptionConfig には、デフォルト値や文字列から T への変換関数も指定できます。
    const auto res = col::ArgParser()
        .add_config(col::FlagConfig{"--help", "show help"})
        .add_config(col::OptionConfig<std::optional<std::string>>{"--file", "FILE", "input file"})
        // パースするコマンドライン引数を std::ranges::borrowed_range として渡します。
        .parse<Cli>(std::span{argv + 1, static_cast<std::size_t>(argc - 1)});
    
    // 戻り値は std::expected<T, E> です。
    // パースが成功していれば T が格納されています。
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
        std::println("error: {}", res.error());
    }
}

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
  - ヘルプメッセージの表示機能
  - short オプション
  - 位置引数
  - オプション引数における可変長引数
  - etc.
- エラーメッセージの拡充
  - (無駄に) constexpr なので、まだ柔軟なエラーメッセージが出せません。


## License
MIT

