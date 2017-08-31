#ifndef _SOCKSEX_DEBUG_H
#define _SOCKSEX_DEBUG_H 1

#include <stdio.h>

extern int _runtime_debugLevel;

#ifdef ___NODEBUG
#define RECORD
#define debug_if(sth) if(false)
#else
#define RECORD debug(4) printf("R> %s:line%d\n", __FILE__, __LINE__);
#define debug_if(sth) if(sth)
#endif

#define debug(level) debug_if(_runtime_debugLevel >= level)


#endif //_SOCKSEX_DEBUG_H
