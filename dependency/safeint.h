
#pragma once

#if defined(__has_include) && !defined(ZS_INCLUDE_IGNORE_RELATIVE_PATHS)
# if __has_include("../../SafeInt/SafeInt.hpp")
#   include "../../SafeInt/SafeInt.hpp"
# else //__has_include("../../SafeInt/SafeInt.hpp")
#   include <SafeInt/SafeInt.hpp>
# endif //__has_include("../SafeInt/SafeInt.hpp")
#else //__has_include
#   include <SafeInt/SafeInt.hpp>
#endif //defined(__has_include) && !defined(ZS_INCLUDE_IGNORE_RELATIVE_PATHS)
