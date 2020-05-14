#pragma once

#include <string>
#include <type_traits>
#include <sstream>

/*  --------------------------------------
 */
template <class T>
std::enable_if_t<std::is_integral<T>::value, bool> isOdd(T num)
{
    return num & 1;
}

/*  --------------------------------------
 */
std::string trim(const std::string& s)
{
    size_t first = s.find_first_not_of(' ');
    if (first == std::string::npos)
        return "";

    size_t last = s.find_last_not_of(' ');
    return s.substr(first, (last - first + 1));
}

/*  --------------------------------------
 */
auto toUint(std::string& s)
{
    return std::stoul(trim(s));
}

/*  --------------------------------------
 */
auto toInt(std::string& s)
{
    return std::stoi(trim(s));
}


/*  --------------------------------------
 */
template <class Container>
void split(const std::string& s, Container& c, char d = ' ')
{
    std::stringstream ss(trim(s));
    std::string token;
    while (std::getline(ss, token, d))    
        c.emplace_back(token);
}

/*  --------------------------------------
 */
struct IntegralPairHash
{
    template <class T, typename = std::enable_if_t<std::is_integral<T>::value>>
    size_t operator()(const std::pair<T, T>& p) const {
        return p.first ^ p.second;
    }
};
