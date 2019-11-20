
#include <zs/traits.h>

#include "common.h"

#include <optional>
#include <iostream>

namespace zsTest
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  struct TraitBasics
  {
    struct Values
    {
    };

    std::optional<Values> values_;

    //-------------------------------------------------------------------------
    void reset()
    {
      values_.reset();
      values_.emplace();
    }

    //-------------------------------------------------------------------------
    void test() noexcept(false)
    {
      constexpr std::string_view name{ "john smith" };
      static_assert(!zs::is_basic_string_view_v<decltype(name)>);
      static_assert(zs::is_deduced_basic_string_view_v<decltype(name)>);

      std::string value{};
      static_assert(zs::is_basic_string_v<decltype(value)>);

      int i = 0;
      int& ref = i;

      static_assert(std::is_integral_v<std::remove_cvref_t<decltype(ref)>>);

      zs::largest_sized_type_t<uint16_t, uint32_t> larger1{};
      (void)larger1;
      static_assert(sizeof(larger1) == sizeof(uint32_t));

      zs::largest_sized_type_t<uint32_t, uint16_t> larger2{};
      (void)larger2;
      static_assert(sizeof(larger2) == sizeof(uint32_t));

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
  void testTraits() noexcept(false)
  {
    TraitBasics{}.runAll();
  }

}
