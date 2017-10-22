#ifndef R_STD_COLOR_HPP
#define R_STD_COLOR_HPP

#include <iostream>
#include <string>
#include <stdexcept>
#include <exception>
using std::string;
using std::basic_ostream;

namespace rlib {
    enum class color_t {color_unset = 10, black = 0, red, green, brown, blue, magenta, cyan, lightgray};
    enum class font_t {font_unset = 0, bold = 1, underline = 4, dark = 2, background = 7, striked = 9}; //Edit line53 if (int)font_t may >= 10 !!
    class clear_t {} clear;

    class fontInfo
    {
    public:
        fontInfo(color_t text_color) : textColor(text_color) {}
        fontInfo(font_t font_type) : fontType(font_type) {}
        fontInfo(color_t text_color, font_t font_type) : textColor(text_color), fontType(font_type) {}
        fontInfo(const clear_t &) : clear(true) {}
        fontInfo() = default;
        string toString() const
        {
            return std::move(clear ? std::string("\033[0m") : (color_to_string() + font_to_string()));
        }
    private:
        color_t textColor = color_t::color_unset;
        font_t fontType = font_t::font_unset;
        bool clear = false;
    private:
        constexpr static int color_to_int(const color_t &_ct)
        {
            return static_cast<int>(_ct);
        }
        constexpr static int font_to_int(const font_t &_ft)
        {
            return static_cast<int>(_ft);
        }
        constexpr static char color_to_char(const color_t &_ct)
        {
            return _ct == color_t::color_unset ? '\0' : '0' + color_to_int(_ct); //Return '\0' if unset.
        }
        constexpr static char font_to_char(const font_t &_ft)
        {
            return _ft == font_t::font_unset ? '\0' :'0' + font_to_int(_ft);
        }
        string color_to_string() const
        {
            if(textColor == color_t::color_unset)
                return std::move(std::string());
            char toret[] = "\033[3?m";
            toret[3] = color_to_char(textColor);
            return std::move(std::string(toret));
        }
        string font_to_string() const
        {
            if(fontType == font_t::font_unset)
                return std::move(std::string());
            char toret[] = "\033[?m";
            toret[2] = font_to_char(fontType);
            return std::move(std::string(toret)); 
        }
    };
    
    struct _rosi_font {_rosi_font(const fontInfo &_ref_fi) : _ref_fi(_ref_fi) {} const fontInfo &_ref_fi;};
    inline _rosi_font setfont(const fontInfo &__fi) {return _rosi_font(__fi);}

    template<typename _CharT, typename _Traits>
        inline basic_ostream<_CharT, _Traits>& 
        operator<<(basic_ostream<_CharT, _Traits>& __os, const fontInfo &__f)
        {
            __os << __f.toString();
            return __os;
        }

    template<typename _CharT, typename _Traits>
        inline basic_ostream<_CharT, _Traits>& 
        operator<<(basic_ostream<_CharT, _Traits>& __os, _rosi_font __rosi_f)
        {
            const fontInfo &__f = __rosi_f._ref_fi;
            return operator<<<_CharT, _Traits>(__os, __f);
        }
}
#endif