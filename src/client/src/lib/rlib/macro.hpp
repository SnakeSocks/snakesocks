#ifndef R_MACRO_HPP
#define R_MACRO_HPP

#ifdef RLIB_EMPTY_MACRO
#undef RLIB_EMPTY_MACRO
#endif
#define RLIB_EMPTY_MACRO

#ifndef RLIB_MACRO_DECAY
#define RLIB_MACRO_DECAY(m) (m)
#endif

#ifndef _RLIB_MACRO_ENSTRING
#define _RLIB_MACRO_ENSTRING(_s) #_s
#endif

#ifndef RLIB_MACRO_TO_CSTR
#define RLIB_MACRO_TO_CSTR(m) _RLIB_MACRO_ENSTRING(m)
#endif

//#ifndef RLIB_MACRO_EQL
//#define RLIB_MACRO_EQL(a, b) (RLIB_MACRO_TO_CSTR(a) == RLIB_MACRO_TO_CSTR(b))
//#endif

#ifndef RLIB_MACRO_CAT
#define RLIB_MACRO_CAT(a, b) _RLIB_MACRO_CAT_I(a, b)
#define _RLIB_MACRO_CAT_I(a, b) _RLIB_MACRO_CAT_II(~, a ## b)
#define _RLIB_MACRO_CAT_II(p, res) res
#endif
#ifndef RLIB_MAKE_UNIQUE_NAME
#define RLIB_MAKE_UNIQUE_NAME(base) RLIB_MACRO_CAT(base, __COUNTER__)
#endif

#endif
