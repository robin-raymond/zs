
#include <zs/reflect.h>

#include "common.h"

#include <optional>
#include <string>
#include <tuple>

#include <iostream>
#include <iomanip>

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
        std::string_view b;
        double c;
      };

      struct Bar : public Foo
      {
        float a;
      };

      static_assert(zs::is_member_function_pointer_v<std::remove_cvref_t<decltype(&Foo::a)>>);
      static_assert(std::is_same_v<zs::remove_deduced_member_pointer<decltype(&Foo::a)>::struct_type, Foo>);

      constexpr auto reflectType{ zs::make_reflect_type(&Foo::a, &Foo::b, &Foo::c, &Bar::a) };

      static_assert(std::is_same_v<decltype(reflectType)::member_type<3>, float>);

      static_assert(4 == reflectType.size());
      static_assert(4 == decltype(reflectType)::total());

      auto function{ [](auto&& value) noexcept {
        std::cout << "VALUE: " << value << "\n";
      } };

      auto function2{ [](auto&& value1, auto &&value2) noexcept {
        std::cout << "VALUE: " << value1 << " " << value2 << "\n";
      } };


      constexpr Bar bar{ 1, "2", 3.3, 4.4f };

      constexpr auto reflect = zs::make_reflect_from_type(bar, reflectType);

      auto&& value0 = reflect.get<0>();
      function(std::forward<decltype(value0)>(value0));

      auto&& value1 = std::get<1>(reflect);
      function(std::forward<decltype(value1)>(value1));

      reflect.visit(function2, 0, 1);

      // run time index visitation
      reflect.visit(function, 0);
      reflect.visit(function, 1);
      reflect.visit(function, 2);
      reflect.visit(function, 3);

      // compile time visitation
      reflect.visit<0, decltype(function)>(std::forward<decltype(function)>(function));
      reflect.visit<1, decltype(function)>(std::forward<decltype(function)>(function));
      reflect.visit<2, decltype(function)>(std::forward<decltype(function)>(function));
      reflect.visit<3, decltype(function)>(std::forward<decltype(function)>(function));

      auto myCallbacks{ zs::overloaded{
         [](auto arg) { std::cout << arg << ' '; },
         [](float arg) { std::cout << std::fixed << arg << ' '; },
         [](double arg) { std::cout << std::fixed << arg << ' '; },
         [](const std::string_view arg) { std::cout << std::quoted(arg) << ' '; }
      } };

      std::visit([](auto&& value1, auto&& value2, auto&& value3, auto&& value4) noexcept { std::cout << "V1: " << value1 << " V2: " << value2 << " V3: " << value3 << " V4: " << value4 << "\n"; }, reflect, 0, 1, 2, 3);

      // overloaded visitation
      for (const auto &elem : reflect)
      {
        std::visit(myCallbacks, elem);

        constexpr bool value1 = zs::is_deduced_reflect_type_v<void>;
        constexpr bool value2 = zs::is_deduced_reflect_visitor_v<decltype(elem)>;

        static_assert(zs::are_all_v<zs::is_deduced_reflect_visitor<int>, decltype(elem), decltype(elem)>);
      }
      
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
