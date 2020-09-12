
#pragma once

#include <string>
#include <chrono>

#ifndef ZS_TEST_DOES_NOT_COMPILE

// move_shared_ptr tests
#undef ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_CTOR_COPY
#undef ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_CTOR_Y_COPY
#undef ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_CTOR_COPY_WITH_ELEMENT
#undef ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_ASSIGNMENT
#undef ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_ASSIGNMENT_DERIVED_TO_BASE
#undef ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_UNIQUE_PTR_ASSIGNMENT
#undef ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_INDEX

#endif //ZS_TEST_DOES_NOT_COMPILE



namespace zsTest
{
  void actualCheck(
    bool value,
    const char* str,
    const char* file,
    const char* function,
    size_t line
    ) noexcept;

#define TEST(x) actualCheck(x, #x, __FILE__, __FUNCTION__, __LINE__)

  void testMoveSharedPtr() noexcept(false);
  void testEnum() noexcept(false);
  void testLog() noexcept(false);
  void testTraits() noexcept(false);
  void testReflect() noexcept(false);
  void testTupleReflect() noexcept(false);
  void testAutoScope() noexcept(false);

  void output(std::string_view testName) noexcept;

  inline auto now() noexcept {
    return std::chrono::system_clock::now();
  }

  template<typename T>
  inline auto diff(T start, T end) noexcept {
    return end - start;
  }
}
