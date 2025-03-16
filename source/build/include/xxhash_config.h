#pragma once

#ifndef xxhash_config_h__
#define xxhash_config_h__

#ifndef NDEBUG
# define XXH_NO_INLINE_HINTS 1
#endif

#ifndef XXH_STATIC_LINKING_ONLY
# define XXH_STATIC_LINKING_ONLY 1
#endif

#include "xxhash.h"
#endif // xxhash_config_h__
