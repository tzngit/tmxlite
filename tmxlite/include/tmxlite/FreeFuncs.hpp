/*********************************************************************
Matt Marchant 2016 - 2021
http://trederia.blogspot.com

tmxlite - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

/*********************************************************************
base64_decode

Copyright (C) 2004-2008 René Nyffenegger
This source code is provided 'as-is', without any express or implied
warranty. In no event will the author be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this source code must not be misrepresented; you must not
claim that you wrote the original source code. If you use this source code
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original source code.
3. This notice may not be removed or altered from any source distribution.

René Nyffenegger rene.nyffenegger@adp-gmbh.ch
*********************************************************************/

#pragma once

#include <tmxlite/detail/Android.hpp>
#include <tmxlite/detail/Log.hpp>
#include <tmxlite/Config.hpp>
#include <tmxlite/Types.hpp>

#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <algorithm>

namespace tmx
{
    //using inline here just to supress unused warnings on gcc (TODO: can say "(void)x" instead)
    bool decompress(const char* source, std::vector<unsigned char>& dest, std::size_t inSize, std::size_t expectedSize);

    static inline std::string base64_decode(std::string const& encoded_string)
    {
        static const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";            
            
        std::function<bool(unsigned char)> is_base64 = 
            [](unsigned char c)->bool
        {
            return (isalnum(c) || (c == '+') || (c == '/'));
        };

        auto in_len = encoded_string.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::string ret;

        while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
        {
            char_array_4[i++] = encoded_string[in_]; in_++;
            if (i == 4)
            {
                for (i = 0; i < 4; i++)
                {
                    char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));
                }
                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                {
                    ret += char_array_3[i];
                }
                i = 0;
            }
        }

        if (i)
        {
            for (j = i; j < 4; j++)
            {
                char_array_4[j] = 0;
            }

            for (j = 0; j < 4; j++)
            {
                char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; (j < i - 1); j++)
            {
                ret += char_array_3[j];
            }
        }

        return ret;
    }

    static inline Colour colourFromString(std::string str)
    {
        //removes preceding #
        auto result = str.find_last_of('#');
        if (result != std::string::npos)
        {
            str = str.substr(result + 1);
        }

        if (str.size() == 6 || str.size() == 8)
        {
            unsigned int value, r, g, b;
            unsigned int a = 255;
            std::stringstream input(str);
            input >> std::hex >> value;

            r = (value >> 16) & 0xff;
            g = (value >> 8) & 0xff;
            b = value & 0xff;

            if (str.size() == 8)
            {
                a = (value >> 24) & 0xff;
            }

            return{ std::uint8_t(r), std::uint8_t(g), std::uint8_t(b), std::uint8_t(a) };
        }
        Logger::log(str + ": not a valid colour string", Logger::Type::Error);
        return{};
    }

    /*!
    \brief Splits 's' at each 'sep' and appends the parts to 'out'.
    */
    void splitStringInto(const std::string& s, char sep, std::vector<std::string>* out);

    /*!
    \brief Joins 'parts' with 'sep' and appends the result to 'out'.
    */
    void joinStringInto(const std::vector<std::string>& parts, char sep, std::string* out);

    /*!
    \brief Defaults to true only on Windows. May be changed for unit testing.
    */
    extern TMXLITE_CONSTINIT bool enableWindowsPathHandling;

    /*!
    \brief Returns whether 'path' is absolute, and optionally writes its
    prefix (e.g. '/' or 'c:\\') in 'prefix'.
    */
    bool isAbsoluteFilePath(const std::string& path, std::string* prefix = nullptr);

    /*!
    \brief Returns a path that refers to what 'path' would refer to if
    it were evaluated in 'workingDir'. Simplifies away '.' and '..'
    in most cases where it's possible.
    Both 'path' and 'workingDir' may be relative or absolute.
    */
    std::string resolveFilePath(std::string path, std::string workingDir);

    static inline std::string getFilePath(const std::string& path)
    {
        //TODO this doesn't actually check that there is a file at the
        //end of the path, or that it's even a valid path...

        static auto searchFunc = [](const char separator, const std::string& path)->std::string
        {
            std::size_t i = path.rfind(separator, path.length());
            if (i != std::string::npos)
            {
                return(path.substr(0, i + 1));
            }

            return "";
        };


        if (enableWindowsPathHandling)
        {
            std::string retVal = searchFunc('\\', path);
            if (!retVal.empty()) return retVal;
        }

        return searchFunc('/', path);
    }

    /*!
    \brief Appends the contents of a file into the given string.
    */
    bool readFileIntoString(const std::string& path, std::string* out);
} //namespacec tmx