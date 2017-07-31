#include <iostream>
#include <string>
#include <system_error>
#include <Windows.h>
#include <locale>
#include <boost/system/error_code.hpp>
#include <Windows.h>

// int main()
// {
//     SetLastError(ERROR_FILE_NOT_FOUND);

//     auto text = std::system_category().message(GetLastError());

//     std::cout << text << '\n';
// }
void print_locale()
{
    std::wcout << "User-preferred locale setting is " << std::locale("").name().c_str() << '\n';
    // on startup, the global locale is the "C" locale
    std::wcout << 1000.01 << '\n';
    // replace the C++ global locale as well as the C locale with the user-preferred locale
    std::locale::global(std::locale(""));
    // use the new global locale for future wide character output
    std::wcout.imbue(std::locale());
    // output the same number again
    std::wcout << 1000.01 << '\n';
}
int main()
{
    // setlocale(LC_ALL, "zh_TW.Big5");
    // setlocale(LC_ALL, "zh_TW.UTF-8");
    SetLastError(ERROR_FILE_NOT_FOUND);
    // auto text = boost::system::system_category().(
    //     GetLastError(), boost::system::system_category()).message();
    //or, just text = boost::system::system_category().message(GetLastError());
    auto text = boost::system::system_category().message(GetLastError());

    std::cout << text << '\n';
}
