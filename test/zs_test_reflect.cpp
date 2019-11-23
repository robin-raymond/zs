
#include <zs/reflect.h>

#include "common.h"

#include <optional>
#include <iostream>
#include <string>

namespace zsTest
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  struct ReflectBasics
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
      struct Foo
      {
        int a;
        std::string b;
        double c;
      };

      struct Bar : public Foo
      {
        float a;
      };

      static_assert(zs::is_member_function_pointer_v<std::remove_cvref_t<decltype(&Foo::a)>>);
      static_assert(std::is_same_v<zs::remove_deduced_member_pointer<decltype(&Foo::a)>::struct_type, Foo>);

      constexpr auto reflect{ zs::make_reflection(&Foo::a, &Foo::b, &Foo::c, &Bar::a) };

      std::cout << "TOTAL: " << reflect.size() << "\n";
      std::cout << "TOTAL: " << reflect.size() << "\n";

      static_assert(4 == reflect.size());

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
  void testReflect() noexcept(false)
  {
    ReflectBasics{}.runAll();
  }

}
