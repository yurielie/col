#include <col/arg_parser.h>

#include <expected>

namespace {

    void test([[maybe_unused]] int argc,[[maybe_unused]] char** argv) noexcept
    {
        col::OptionConfig<int> opt{"", "", ""};
        [[maybe_unused]] auto opt2 = opt.set_converter([](const char*) noexcept { return 0; });
    }

} // namespace <unnamed>

int main(int argc, char** argv)
{
    test(argc, argv);
    return 0;
}
