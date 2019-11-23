
#include <zs/traits.h>

#include "common.h"

#include <optional>
#include <iostream>
#include <vector>

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
    void testUniquePtrByRef(std::unique_ptr<int> &&value) noexcept(false)
    {
      static_assert(zs::is_std_unique_ptr_v<std::remove_cvref_t<decltype(value)>>);
    }

    //-------------------------------------------------------------------------
    void testMemberPointer() noexcept(false)
    {
      struct Fooish
      {
        int bar;
        std::string something;
      };

      constexpr int Fooish::* ptr1{ &Fooish::bar };
      constexpr std::string Fooish::* ptr2{ &Fooish::something };

      zs::remove_deduced_member_pointer_t<decltype(ptr1)> myValue1{ 5 };
      zs::remove_deduced_member_pointer_t<decltype(ptr2)> myValue2{ "hello" };

      static_assert(std::is_same_v<std::remove_cvref_t<decltype(myValue1)>, int>);
      static_assert(std::is_same_v<std::remove_cvref_t<decltype(myValue2)>, std::string>);
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

      std::array<int, 5> arrayOfInts{};
      static_assert(zs::is_std_array_v<decltype(arrayOfInts)>);

      std::vector<float> floatVector;
      static_assert(zs::is_std_vector_v<decltype(floatVector)>);

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void runAll() noexcept(false)
    {
      auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

      runner([&]() { test(); });
      runner([&]() { std::unique_ptr<int> ignored; testUniquePtrByRef(std::move(ignored)); });
    }
  };

  //---------------------------------------------------------------------------
  void testTraits() noexcept(false)
  {
    TraitBasics{}.runAll();
  }

}
