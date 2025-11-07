#include <col/command.h>

#include <string>
#include <string_view>

namespace col {

    struct NotInvocable{};

    static_assert(std::is_object_v<blank>);
    static_assert(std::movable<blank>);
    static_assert(std::destructible<blank>);
    static_assert(std::invocable<blank> == false);

    [[maybe_unused]]
    inline int arg_default_int() { return 1; }
    static_assert(default_value_type<int>);
    static_assert(default_value_type<NotInvocable>);
    static_assert(default_value_type<decltype(&arg_default_int)>);
    static_assert(default_value_type<decltype([](){return 1;})>);
    
    [[maybe_unused]]
    inline void arg_default_void() {}
    [[maybe_unused]]
    inline blank arg_default_blank() { return blank{}; }
    static_assert(default_value_type<blank> == false);
    static_assert(default_value_type<int&> == false);
    static_assert(default_value_type<decltype(&arg_default_void)> == false);
    static_assert(default_value_type<decltype([](){})> == false);
    static_assert(default_value_type<decltype(&arg_default_blank)> == false);
    static_assert(default_value_type<decltype([](){return blank{};})> == false);



    [[maybe_unused]]
    inline int arg_parser_int_cstr(const char*) { return 1; }
    [[maybe_unused]]
    inline int arg_parser_int_strview(std::string_view) { return 1; }
    [[maybe_unused]]
    inline int arg_parser_int_str(std::string) { return 1; }
    static_assert(value_parser_type<decltype(&arg_parser_int_cstr)>);
    static_assert(value_parser_type<decltype(&arg_parser_int_strview)>);
    static_assert(value_parser_type<decltype(&arg_parser_int_str)>);
    static_assert(value_parser_type<decltype([](const char*){return 1;})>);
    static_assert(value_parser_type<decltype([](std::string_view){return 1;})>);
    static_assert(value_parser_type<decltype([](std::string){return 1;})>);

    [[maybe_unused]]
    inline int arg_parser_int_void() { return 1; }
    [[maybe_unused]]
    inline int arg_parser_int_int(int) { return 1; }
    [[maybe_unused]]
    inline void arg_parser_void_cstr(const char*) {}
    [[maybe_unused]]
    inline blank arg_parser_blank_cstr(const char*) { return blank{}; }
    static_assert(value_parser_type<int&> == false);
    static_assert(value_parser_type<blank> == false);
    static_assert(value_parser_type<NotInvocable> == false);
    static_assert(value_parser_type<decltype(&arg_parser_int_void)> == false);
    static_assert(value_parser_type<decltype(&arg_parser_int_int)> == false);
    static_assert(value_parser_type<decltype(&arg_parser_void_cstr)> == false);
    static_assert(value_parser_type<decltype(&arg_parser_blank_cstr)> == false);



    static_assert(default_and_parser_compatible<blank, decltype([](const char*){return 0;})>);
    static_assert(default_and_parser_compatible<int, blank>);
    static_assert(default_and_parser_compatible<int, decltype([](const char*){return 0;})>);
    static_assert(default_and_parser_compatible<const char*, decltype([](const char*){ return std::string{};})>);
    static_assert(default_and_parser_compatible<decltype([](){return "";}), decltype([](const char*){return "";})>);
    static_assert(default_and_parser_compatible<decltype([](){return "";}), decltype([](const char*){return std::string{};})>);

    static_assert(default_and_parser_compatible<blank, blank> == false);
    static_assert(default_and_parser_compatible<int&, blank> == false);
    static_assert(default_and_parser_compatible<blank, int&> == false);
    static_assert(default_and_parser_compatible<int, decltype([](const char*){return "";})> == false);
    static_assert(default_and_parser_compatible<decltype([](){return 0;}), decltype([](const char*){return "";})> == false);

} // namespace col

