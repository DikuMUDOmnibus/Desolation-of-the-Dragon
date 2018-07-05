/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer and Heath Leach
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(START_DB) || defined (USE_DB)
#include "mysql.h"
#endif

#ifdef MEMDEBUG
#include "memdebug.h"
#endif

/*#include "leak_detector.h"*/
