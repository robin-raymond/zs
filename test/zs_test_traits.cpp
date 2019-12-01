
#include <zs/traits.h>

#include "common.h"

#include <optional>
#include <iostream>
#include <vector>
#include <variant>

#include <string_view>

using namespace std::string_view_literals;


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
      static_assert(!zs::is_std_basic_string_view_v<decltype(name)>);
      static_assert(zs::is_std_deduced_basic_string_view_v<decltype(name)>);

      std::string value{};
      static_assert(zs::is_std_basic_string_v<decltype(value)>);

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

    void testTypeList() noexcept
    {
      using CheckVoidTypeList = typename zs::TypeList<void>::type;
      static_assert(std::is_same_v<CheckVoidTypeList, zs::TypeList<void>>);

      using MyTypeList1 = zs::TypeList<int>::append_type_if_unique_t<float>;
      using MyTypeList2 = zs::TypeList<int>::prepend_type_t<int>;
      using MyTypeList3 = zs::TypeList<int>::prepend_type_if_unique_t<int>;
      using MyTypeList4 = typename zs::TypeList<>::append_type_if_unique<int, float, int>::type;
      using MyTypeList5 = zs::TypeList<int>::prepend_type_t<float>;
      using MyTypeList6 = zs::TypeList<int>::append_type_t<float>;
      using MyVariant1 = typename zs::rebind_from_template<std::variant<void>, MyTypeList6>::type;
      using MyVariant2 = typename MyTypeList6::rebind<std::variant<void>>::type;
      using MyTypeList7 = zs::TypeList<>::rebind_from_t<std::variant<int, float>>;
      using MyTypeList8 = zs::type_list_with_modified_types_t< std::add_lvalue_reference<void>, int, float>;

      using MyTypeList9 = zs::type_list_modify_all_types_t< zs::TypeList<int, float>, std::add_lvalue_reference<void>>;

      using MyTypeList10 = zs::TypeList<>::rebind_from_t<int>;
      using MyTypeList11 = zs::TypeList<>::rebind_from_t<std::tuple<int, float>>;
      using MyTypeList12 = typename zs::TypeList<std::string, std::tuple<int, float>>::rebind_from<std::tuple<int, float>>::type;

      using MyTypeList13 = typename MyTypeList12::append_type<std::variant<int, float>>::type;
      using MyTypeList14 = typename MyTypeList12::prepend_type<std::variant<int, float>>::type;

      using MyTypeList15 = typename MyTypeList12::append_type_t<std::variant<int, float>>;
      using MyTypeList16 = typename MyTypeList12::prepend_type_t<std::variant<int, float>>;

      using MyTypeList17 = typename zs::TypeList<std::variant<int, float>>::prepend_type_if_unique<std::variant<int, float>>::type;
      using MyTypeList18 = typename zs::TypeList<std::variant<int, float>>::prepend_type_if_unique<std::variant<float, int>>::type;
      using MyTypeList19 = zs::TypeList<std::variant<int, float>>::prepend_type_if_unique_t<std::variant<float, int>>;

      using MyTypeList20 = typename zs::TypeList<int, float>::prepend_type<>::type;
      using MyTypeList21 = zs::TypeList<int, float>::prepend_type_t<>;

      using MyTypeList22 = typename zs::TypeList<int, float>::append_type<>::type;
      using MyTypeList23 = zs::TypeList<int, float>::append_type_t<>;

      using MyTypeList24 = zs::TypeList<int, float>::append_changed_type_t<std::remove_cvref<void>, short &, const char, const double &&>;
      using MyTypeList25 = zs::TypeList<int, float>::prepend_changed_type_t<std::remove_cvref<void>, short&, const char, const double&&>;
      using MyTypeList26 = typename MyTypeList25::reverse_type_t;

      static_assert(std::is_same_v<MyTypeList1, zs::TypeList<int, float>>);
      static_assert(std::is_same_v<MyTypeList2, zs::TypeList<int, int>>);
      static_assert(std::is_same_v<MyTypeList3, zs::TypeList<int>>);
      static_assert(zs::is_type_in_type_list_v<int, int>);
      static_assert(std::is_same_v<MyTypeList4, zs::TypeList<int, float>>);
      static_assert(std::is_same_v<MyTypeList5, zs::TypeList<float, int>>);
      static_assert(std::is_same_v<MyTypeList6, zs::TypeList<int, float>>);
      static_assert(std::is_same_v<MyVariant1, std::variant<int, float>>);
      static_assert(std::is_same_v<MyVariant2, std::variant<int, float>>);
      static_assert(std::is_same_v<MyVariant1, MyVariant2>);
      static_assert(std::is_same_v<MyTypeList7, zs::TypeList<int, float>>);
      static_assert(std::is_same_v<MyTypeList8, zs::TypeList<int&, float&>>);
      static_assert(std::is_same_v<MyTypeList9, zs::TypeList<int&, float&>>);

      static_assert(std::is_same_v<MyTypeList10, zs::TypeList<int>>);
      static_assert(std::is_same_v<MyTypeList11, zs::TypeList<int, float>>);
      static_assert(std::is_same_v<MyTypeList12, zs::TypeList<int, float>>);
      static_assert(std::is_same_v<MyTypeList13, zs::TypeList<int, float, std::variant<int, float>>>);
      static_assert(std::is_same_v<MyTypeList14, zs::TypeList<std::variant<int, float>, int, float>>);
      static_assert(std::is_same_v<MyTypeList15, zs::TypeList<int, float, std::variant<int, float>>>);
      static_assert(std::is_same_v<MyTypeList16, zs::TypeList<std::variant<int, float>, int, float>>);

      static_assert(std::is_same_v<MyTypeList17, zs::TypeList<std::variant<int, float>>>);
      static_assert(std::is_same_v<MyTypeList18, zs::TypeList<std::variant<float, int>, std::variant<int, float>>>);
      static_assert(std::is_same_v<MyTypeList19, zs::TypeList<std::variant<float, int>, std::variant<int, float>>>);

      static_assert(std::is_same_v<MyTypeList20, zs::TypeList<int, float>>);
      static_assert(std::is_same_v<MyTypeList21, zs::TypeList<int, float>>);

      static_assert(std::is_same_v<MyTypeList22, zs::TypeList<int, float>>);
      static_assert(std::is_same_v<MyTypeList23, zs::TypeList<int, float>>);
      static_assert(std::is_same_v<MyTypeList24, zs::TypeList<int, float, short, char, double>>);
      static_assert(std::is_same_v<MyTypeList25, zs::TypeList<short, char, double, int, float>>);
      static_assert(std::is_same_v<MyTypeList26, zs::TypeList<float, int, double, char, short>>);

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    constexpr static bool shouldFind(bool isTrue, std::string_view value, std::string_view search) noexcept
    {
      bool found = std::string_view::npos != search.find(value);
      return ((isTrue) && (found)) || ((!isTrue) && (!found));
    }

    //-------------------------------------------------------------------------
    template <typename T>
    constexpr static bool checkTraits(const std::string_view search) noexcept
    {
      bool okay = true;
      okay = okay && shouldFind(zs::is_member_function_pointer_v<T>, "|member_ptr|", search);
      okay = okay && shouldFind(zs::is_deduced_member_function_pointer_v<T>, "|deduced_member_ptr|", search);
      okay = okay && shouldFind(zs::is_member_function_pointer<T>::value, "|member_ptr|", search);
      okay = okay && shouldFind(zs::is_deduced_member_function_pointer<T>::value, "|deduced_member_ptr|", search);

      okay = okay && shouldFind(zs::is_type_in_type_list_v<T, int, float&, std::tuple<int,float>>, "|type_in_list|", search);
      okay = okay && shouldFind(zs::is_type_in_type_list<T, int, float&, std::tuple<int, float>>::value, "|type_in_list|", search);

      okay = okay && shouldFind(zs::are_all_v<std::is_integral<void>, int, T, short>, "|are_all|", search);
      okay = okay && shouldFind(zs::are_all<std::is_integral<void>, int, T, short>::value, "|are_all|", search);
      okay = okay && shouldFind(zs::are_any_v<std::is_integral<void>, float, T, std::tuple<int, char>>, "|are_any|", search);
      okay = okay && shouldFind(zs::are_any<std::is_integral<void>, float, T, std::tuple<int, char>>::value, "|are_any|", search);
      return okay;
    }

    //-------------------------------------------------------------------------
    void testCheckTraitAsserts()
    {
      struct A
      {
        int a;
        float b;
        std::string c;

        char d[25];
      };
      constexpr int A::* memberAPtr = &A::a;
      constexpr float A::* memberBPtr = &A::b;
      constexpr std::string A::* memberCPtr = &A::c;
      constexpr auto A::* memberDPtr = &A::d;

      static_assert(checkTraits<std::remove_cvref_t<decltype(memberAPtr)>>("|member_ptr|deduced_member_ptr|"));
      static_assert(checkTraits<decltype(memberAPtr)>("|deduced_member_ptr|"));
      static_assert(checkTraits<std::remove_cvref_t<decltype(memberBPtr)>>("|member_ptr|deduced_member_ptr|"));
      static_assert(checkTraits<decltype(memberBPtr)>("|deduced_member_ptr|"));
      static_assert(checkTraits<std::remove_cvref_t<decltype(memberCPtr)>>("|member_ptr|deduced_member_ptr|"));
      static_assert(checkTraits<decltype(memberCPtr)>("|deduced_member_ptr|"));
      static_assert(checkTraits<std::remove_cvref_t<decltype(memberDPtr)>>("|member_ptr|deduced_member_ptr|"));
      static_assert(checkTraits<decltype(memberDPtr)>("|deduced_member_ptr|"));

      static_assert(checkTraits<const int>("|are_all|are_any|"));
      static_assert(checkTraits<int>("|type_in_list|are_all|are_any|"));
      static_assert(checkTraits<float>("||"));
      static_assert(checkTraits<float&>("|type_in_list|"));

      float myFloat{};
      float& myFloatRef{ myFloat };
      static_assert(checkTraits<decltype(myFloatRef)>("|type_in_list|"));

      static_assert(checkTraits<std::tuple<int, float>>("|type_in_list|"));
      static_assert(checkTraits<std::tuple<float, int>>("||"));

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void testRebind()
    {
      std::tuple<float> myValue1;

      using VectorRebindType = zs::rebind_from_template_t<std::vector<int>, decltype(myValue1)>;

      static_assert(std::is_same_v<std::vector<float>, VectorRebindType>);

      std::tuple<int, float, std::string> myValue2;

      using VariantRebindType = zs::rebind_from_template<std::variant<void>, decltype(myValue2)>::type;

      static_assert(std::is_same_v<std::variant<int, float, std::string>, VariantRebindType>);

      float originalValue = 4.3f;
      VariantRebindType myVariant{ originalValue };

      float value = std::get<1>(myVariant);
      TEST(value == originalValue);

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void testOther()
    {
      static_assert(3 == zs::count_types_v<int, float, std::tuple<int, float>>);
      static_assert(0 == zs::count_types_v<>);
      static_assert(1 == zs::count_types_v<int>);
      static_assert(1 == zs::count_types_v<std::variant<int, float>>);

      static_assert(3 == zs::count_types<int, float, std::tuple<int, float>>::value);
      static_assert(0 == zs::count_types<>::value);
      static_assert(1 == zs::count_types<int>::value);
      static_assert(1 == zs::count_types<std::variant<int, float>>::value);

      static_assert(0 == zs::count_template_types_v<int>);
      static_assert(2 == zs::count_template_types_v<std::variant<int, float>>);
      static_assert(4 == zs::count_template_types_v<std::tuple<int, float, char[5], std::string>>);

      static_assert(0 == zs::count_template_types<int>::value);
      static_assert(2 == zs::count_template_types<std::variant<int, float>>::value);
      static_assert(4 == zs::count_template_types<std::tuple<int, float, char[5], std::string>>::value);

      static_assert(std::is_same_v<int, zs::first_type_t<int, float, short>>);
      static_assert(std::is_same_v<float, zs::first_type_t<float>>);

      static_assert(std::is_same_v<short, zs::last_type_t<int, float, short>>);
      static_assert(std::is_same_v<float, zs::last_type_t<float>>);

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void runAll() noexcept(false)
    {
      auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

      runner([&]() { test(); });
      runner([&]() { std::unique_ptr<int> ignored; testUniquePtrByRef(std::move(ignored)); });
      runner([&]() { testTypeList(); });
      runner([&]() { testCheckTraitAsserts(); });
      runner([&]() { testRebind(); });
      runner([&]() { testOther(); });
    }
  };

  //---------------------------------------------------------------------------
  void testTraits() noexcept(false)
  {
    TraitBasics{}.runAll();
  }

}
