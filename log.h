
#pragma once

namespace zs
{
  namespace log
  {
    enum class Level
    {
      Basic,
      Detail,
      Debug,
      Trace,
      Insane,
    };

    enum class Severity
    {
      Info,
      Informational = Info,
      Warning,
      Error,
      Critical,
      Fatal,
    };

  } // namespace log
} // namespace zs
