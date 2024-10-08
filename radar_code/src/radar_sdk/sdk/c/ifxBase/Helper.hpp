/* ===========================================================================
** Copyright (C) 2021 Infineon Technologies AG
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** 1. Redistributions of source code must retain the above copyright notice,
**    this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. Neither the name of the copyright holder nor the names of its
**    contributors may be used to endorse or promote products derived from
**    this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
** ===========================================================================
*/

#ifndef IFX_BASE_STRING_HELPER_HPP
#define IFX_BASE_STRING_HELPER_HPP

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

namespace ifx {
namespace string {

/**
 * @brief Split string
 *
 * Return a vector of the words in the string, using delim as the delimiter
 * string.
 *
 * @param [in]  text    text string to split
 * @param [in]  delim   delimiter string
 * @retval vector of splitted input text
 */
inline std::vector<std::string> split(const std::string& text, const std::string& delim)
{
    size_t start = 0U;
    size_t end = text.find(delim);
    std::vector<std::string> parts;

    while (end != std::string::npos)
    {
        parts.push_back(text.substr(start, end - start));
        start = end + delim.length();
        end = text.find(delim, start);
    }

    parts.push_back(text.substr(start, end));
    return parts;
}

/**
 * @brief Left strip string
 *
 * Return a string without white spaces on the left side.
 *
 * @param [in]  text    text string to strip
 * @retval left stripped input text
 */
inline std::string lstrip(const std::string& text)
{
    size_t start = 0;

    for (size_t i = 0; i < text.length() && isspace(text[i]); ++i)
        ++start;

    return text.substr(start, std::string::npos);
}

/**
 * @brief Right strip string
 *
 * Return a string without white spaces on the right side.
 *
 * @param [in]  text    text string to strip
 * @retval right stripped input text
 */
inline std::string rstrip(const std::string& text)
{
    const size_t length = text.length();
    size_t spaces = 0;

    for (size_t i = 0; i < length && isspace(text[length - i - 1]); ++i)
        ++spaces;

    return text.substr(0, length - spaces);
}

/**
 * @brief Strip string
 *
 * Return a string without white spaces on the left and right side.
 *
 * @param [in]  text    text string to strip
 * @retval string without white spaces on begin or end
 */
inline std::string strip(const std::string& text)
{
    return lstrip(rstrip(text));
}

/**
 * @brief Lowercase string
 *
 * Return a string all in lowercase.
 *
 * @param [in]  text    text string to convert
 * @retval string converted to lowercase
 */
inline std::string lower(const std::string& text)
{
    std::string lower;
    transform(text.begin(), text.end(), back_inserter(lower), [](char c) { return tolower(c); });
    return lower;
}

/**
 * @brief Uppercase string
 *
 * Return a string all in uppercase.
 *
 * @param [in]  text    text string to convert
 * @retval string converted to uppercase
 */
inline std::string upper(const std::string& text)
{
    std::string upper;
    transform(text.begin(), text.end(), back_inserter(upper), [](char c) { return toupper(c); });
    return upper;
}

}  // namespace string
}  // namespace ifx

#endif /* #ifndef IFX_BASE_STRING_HELPER_HPP */
