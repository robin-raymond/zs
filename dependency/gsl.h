
#pragma once

#if defined(__has_include) && !defined(ZS_INCLUDE_IGNORE_RELATIVE_PATHS)
# if __has_include("../../GSL/include/gsl/gsl")
#   include "../../GSL/include/gsl/gsl"
# else //__has_include("../../GSL/include/gsl/gsl")
#   include <gsl/gsl>
# endif //__has_include("../../GSL/include/gsl/gsl")
#else //__has_include
#   include <gsl/gsl>
#endif //defined(__has_include) && !defined(ZS_INCLUDE_IGNORE_RELATIVE_PATHS)
