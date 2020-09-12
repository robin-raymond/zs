
#include <zs/AutoScope.h>

#include "common.h"

#include <atomic>

#if 0
#include <optional>
#include <iostream>
#include <vector>
#include <variant>

#include <string_view>

using namespace std::string_view_literals;
#endif //0

namespace zsTest
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  struct AutoScopeBasics
  {
    //-------------------------------------------------------------------------
    void reset() noexcept
    {
    }

    //-------------------------------------------------------------------------
    void test() noexcept(false)
    {
      std::atomic_int value{};

      {
        zs::AutoScope s1{ [&]() noexcept { ++value; } };
        TEST(0 == value);
      }
      TEST(1 == value);

      {
        zs::AutoScope s1{ [&]() noexcept { ++value; } };
        TEST(1 == value);
        s1.cancel();
      }
      TEST(1 == value);

      {
        zs::AutoScope s1{ [&]() noexcept { ++value; } };
        zs::AutoScope s2{ std::move(s1) };
        TEST(1 == value);
      }
      TEST(2 == value);

      {
        zs::AutoScope s1{ zs::AutoScope{ [&]() noexcept { ++value; } } };
        TEST(2 == value);
      }
      TEST(3 == value);

      {
        zs::AutoScope s1{ [&]() noexcept { ++value; } };
        zs::AutoScope s2{ [&]() noexcept { --value; } };

        s1 = std::move(s2);
        TEST(3 == value);
      }
      TEST(2 == value);

      {
        zs::AutoScope s1{ [&]() noexcept { ++value; } };

        s1 = [&]() noexcept { --value; };
        TEST(2 == value);
      }
      TEST(1 == value);

      {
        zs::AutoScope s1{ [&]() noexcept { ++value; } };

        s1 = [&]() noexcept { --value; };
        TEST(1 == value);
        s1.cancel();
      }
      TEST(1 == value);

      {
        zs::AutoScope s1{ [&]() noexcept { ++value; } };
        zs::AutoScope s2{ [&]() noexcept { --value; } };

        s1 = std::move(s2);
        TEST(1 == value);
        s2.cancel();
      }
      TEST(0 == value);

      {
        auto temp{ zs::on_scope_exit([&]() noexcept { ++value; }) };
        TEST(0 == value);
      }
      TEST(1 == value);

      {
        auto temp{ zs::on_scope_exit([&]() noexcept(false) { ++value; }) };
        TEST(1 == value);
      }
      TEST(2 == value);

      try {
        auto temp{ zs::on_scope_exit([&]() noexcept(false) { ++value; throw std::exception{}; }) };
        TEST(2 == value);
      }
      catch (const std::exception&) {
        ++value;
      }
      TEST(4 == value);

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void runAll() noexcept(false)
    {
      auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

      runner([&]() { test(); });
    }
  };

  //---------------------------------------------------------------------------
  void testAutoScope() noexcept(false)
  {
    AutoScopeBasics{}.runAll();
  }

}
