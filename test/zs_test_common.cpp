
#include <iostream>
#include <atomic>
#include <sstream>

#include <assert.h>

#include "common.h"

using namespace zsTest;

//-----------------------------------------------------------------------------
class Testing
{
public:

  static Testing& singleton() noexcept
  {
    static Testing singleton;
    return singleton;
  }

  static void check(
    bool value,
    const char* str,
    const char* file,
    const char* function,
    size_t line
  ) noexcept
  {
    if (value) {
      ++(singleton().passed_);
    }  else {
      std::stringstream ss;
      ss << "CHECK FAILED:  " << str << "\n";
      ss << "FILE:          " << file << "\n";
      ss << "FUNCTION:      " << function << "\n";
      ss << "LINE:          " << line << "\n\n";

      std::cout << ss.str();
      assert(!"CHECK FAILED");
      ++(singleton().failed_);
    }
  }

  static unsigned int passed() noexcept
  {
    return singleton().passed_;
  }

  static unsigned int failed() noexcept
  {
    return singleton().failed_;
  }

  static unsigned int total() noexcept
  {
    return passed() + failed();
  }

private:
  std::atomic_uint passed_{};
  std::atomic_uint failed_{};
};

//-----------------------------------------------------------------------------
void zsTest::actualCheck(
  bool value,
  const char* str,
  const char* file,
  const char* function,
  size_t line
) noexcept
{
  Testing::singleton().check(value, str, file, function, line);
}

//-----------------------------------------------------------------------------
void zsTest::output(std::string_view testName) noexcept
{
  auto passed = Testing::passed();
  auto failed = Testing::failed();

  std::cout << "TEST: " << testName << "\n";
  std::cout << "\n";
  std::cout << "TOTAL PASSED: " << Testing::passed() << "\n";
  std::cout << "TOTAL FAILED: " << Testing::failed() << "\n";
  std::cout << "\n";
}

//-----------------------------------------------------------------------------
int runAllTests() noexcept
{
  try {
    testMoveSharedPtr();
    testEnum();
    testLog();
    testTraits();
    testReflect();
    testTupleReflect();
    testAutoScope();
    testRandomAccessListIterator();
  } catch (...) {
    std::cout << "ERROR: uncaught exception thrown!\n";
    TEST(!"uncaught exception");
  }

  std::cout << "TOTAL TESTS RUN: " << Testing::total() << "\n\n";
  std::cout << "TOTAL PASSED: " << Testing::passed() << "\n";
  std::cout << "TOTAL FAILED: " << Testing::failed() << "\n";

  return Testing::failed() > 0 ? -1 : 0;
}
