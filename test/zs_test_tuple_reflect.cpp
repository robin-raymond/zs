
#include <zs/tuple_reflect.h>

#include "common.h"

#include <optional>
#include <string>
#include <tuple>

#include <iostream>
#include <iomanip>

namespace std
{
 
}

namespace zsTest
{

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  struct TupleReflectBasics
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
      std::tuple<int, float> myTuple1{ 42, 17.93f };

      zs::TupleReflect<decltype(myTuple1)> reflector1{ myTuple1 };

      std::cout << "SIZE: " << sizeof(reflector1) << "\n";


      auto function{ [](auto&& value) noexcept {
        std::cout << "VALUE: " << value << "\n";
      } };


      reflector1.visit([&](auto&&... args) noexcept {(static_cast<void>(function(std::forward<decltype(args)>(args))), ...); }, 0, 1);

      std::visit([&](auto&& arg) {std::cout << "ARG: " << arg << "\n"; }, reflector1);

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
  void testTupleReflect() noexcept(false)
  {
    TupleReflectBasics{}.runAll();
  }

}
