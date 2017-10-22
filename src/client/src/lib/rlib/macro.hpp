#ifndef R_MACRO_HPP
#define R_MACRO_HPP

#ifndef MACRO_DECAY
#define MACRO_DECAY(m) (m)
#endif

#ifndef _R_MACRO_ENSTRING
#define _R_MACRO_ENSTRING(_s) #_s
#endif

#ifndef MACRO_TO_CSTR
#define MACRO_TO_CSTR(m) _R_MACRO_ENSTRING(m)
#endif

#ifndef MACRO_EQL
#define MACRO_EQL(a, b) (MACRO_TO_CSTR(a) == MACRO_TO_CSTR(b))
#endif

#ifndef MACRO_CAT
#define MACRO_CAT(a, b) _MACRO_CAT_I(a, b)
#define _MACRO_CAT_I(a, b) _MACRO_CAT_II(~, a ## b)
#define _MACRO_CAT_II(p, res) res
#endif
#ifndef MAKE_UNIQUE_NAME
#define MAKE_UNIQUE_NAME(base) MACRO_CAT(base, __COUNTER__)
#endif

#endif
