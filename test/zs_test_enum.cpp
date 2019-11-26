
#include <zs/enum.h>

#include "common.h"

#include <optional>
#include <iostream>

namespace zsTest
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  struct EnumBasics
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
    enum class Something
    {
      C = 13,
      A = 7,
      D = 23,
      B = 5,
    };

    //-------------------------------------------------------------------------
    struct SomethingDeclare final : public zs::EnumDeclare<Something, 4>
    {
      constexpr const Entries operator()() const noexcept {
        return { {
          {Something::C, "c"},
          {Something::A, "a"},
          {Something::D, "d"},
          {Something::B, "b"},
        } };
      }
    };

    using SomethingTraits = zs::EnumTraits<Something, SomethingDeclare>;
    using SomethingTraitsByDefault = zs::EnumTraits<Something, SomethingDeclare, zs::EnumOrder::Default>;
    static_assert(std::is_same_v<SomethingTraits, SomethingTraitsByDefault >);

    using SomethingTraitsByValue = zs::EnumTraits<Something, SomethingDeclare, zs::EnumOrder::Value>;
    using SomethingTraitsByName = zs::EnumTraits<Something, SomethingDeclare, zs::EnumOrder::Name>;

    void basic() noexcept(false)
    {
      constexpr auto f1 = SomethingTraits::firstIndex();
      constexpr auto f2 = SomethingTraitsByValue::firstIndex();
      constexpr auto f3 = SomethingTraitsByName::firstIndex();

      static_assert((f1 == f2) && (f2 == f3));
      static_assert(0 == f1);

      constexpr auto l1 = SomethingTraits::lastIndex();
      constexpr auto l2 = SomethingTraitsByValue::lastIndex();
      constexpr auto l3 = SomethingTraitsByName::lastIndex();

      static_assert((l1 == l2) && (l2 == l3));
      static_assert(3 == l1);

      constexpr auto vf1 = SomethingTraits::first();
      constexpr auto vf2 = SomethingTraitsByValue::first();
      constexpr auto vf3 = SomethingTraitsByName::first();

      static_assert(Something::C == vf1);
      static_assert(Something::C == vf2);
      static_assert(Something::C == vf3);
      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void iterators() noexcept(false)
    {
      const SomethingTraits cDefault;
      const SomethingTraitsByValue cValue;
      const SomethingTraitsByName cName;

      SomethingTraits mutDefault;
      SomethingTraitsByValue mutValue;
      SomethingTraitsByName mutName;

      TEST(std::begin(cDefault) != std::end(cDefault));
      TEST(std::begin(cDefault) < std::end(cDefault));
      TEST(std::begin(cDefault) <= std::end(cDefault));
      TEST(!(std::begin(cDefault) > std::end(cDefault)));
      TEST(!(std::begin(cDefault) >= std::end(cDefault)));

      // check default ordering
      {
        constexpr std::array< std::pair<Something, std::string_view>, 4> values{ {
          {Something::C, "c"},
          {Something::A, "a"},
          {Something::D, "d"},
          {Something::B, "b"},
        } };

        static_assert(values.size() == SomethingTraits::total());
        TEST(values.size() == SomethingTraits::total());

        // test forward const iterator
        {
          int index = 0;
          for (SomethingTraits::const_iterator iter = cDefault.begin(); iter != cDefault.end(); ++iter, ++index) {
            auto [value, name] = *iter;

            TEST(iter == iter);

            TEST(values[index].first == value);
            TEST(values[index].second == name);

            auto asEnum = SomethingTraits::toEnum(name);
            auto asString = SomethingTraits::toString(value);
            TEST(asEnum == value);
            TEST(asString == name);
          }
          TEST(cDefault.begin() == cDefault.begin());
          TEST(cDefault.begin() != cDefault.end());
          TEST(values.size() == cDefault.size());
        }
        // test forward non-const iterator
        {
          int index = 0;
          for (SomethingTraits::iterator iter = mutDefault.begin(); iter != mutDefault.end(); ++iter, ++index) {
            auto [value, name] = *iter;

            TEST(iter == iter);

            TEST(values[index].first == value);
            TEST(values[index].second == name);
            TEST(values[index].first == iter->first);
            TEST(values[index].second == iter->second);
          }
          TEST(mutDefault.begin() == mutDefault.begin());
          TEST(mutDefault.begin() != mutDefault.end());
          TEST(values.size() == mutDefault.size());
        }
        // test reverse const iterator
        {
          int index = static_cast<decltype(index)>(values.size()) - 1;
          for (SomethingTraits::const_reverse_iterator iter = cDefault.rbegin(); iter != cDefault.rend(); ++iter, --index) {
            auto [value, name] = *iter;

            TEST(iter == iter);

            TEST(values[index].first == value);
            TEST(values[index].second == name);
          }
          TEST(cDefault.rbegin() == cDefault.rbegin());
          TEST(cDefault.rbegin() != cDefault.rend());
        }
        // test reverse non-const iterator
        {
          int index = static_cast<decltype(index)>(values.size()) - 1;
          for (SomethingTraits::reverse_iterator iter = mutDefault.rbegin(); iter != mutDefault.rend(); ++iter, --index) {
            auto [value, name] = *iter;

            TEST(iter == iter);

            TEST(values[index].first == value);
            TEST(values[index].second == name);
          }
          TEST(mutDefault.rbegin() == mutDefault.rbegin());
          TEST(mutDefault.rbegin() != mutDefault.rend());
        }
      }

      // check value ordering
      {
        constexpr std::array< std::pair<Something, std::string_view>, 4> values{ {
          {Something::B, "b"},
          {Something::A, "a"},
          {Something::C, "c"},
          {Something::D, "d"},
        } };

        static_assert(values.size() == SomethingTraitsByValue::total());
        TEST(values.size() == SomethingTraitsByValue::total());

        // test forward const iterator
        {
          int index = 0;
          for (SomethingTraitsByValue::const_iterator iter = cValue.begin(); iter != cValue.end(); ++iter, ++index) {
            auto [value, name] = *iter;

            TEST(iter == iter);

            TEST(values[index].first == value);
            TEST(values[index].second == name);

            auto asEnum = SomethingTraitsByValue::toEnum(name);
            auto asString = SomethingTraitsByValue::toString(value);
            TEST(asEnum == value);
            TEST(asString == name);
          }
          TEST(cValue.begin() == cValue.begin());
          TEST(cValue.begin() != cValue.end());
          TEST(values.size() == cValue.size());
        }
        // test forward non-const iterator
        {
          int index = 0;
          for (SomethingTraitsByValue::iterator iter = mutValue.begin(); iter != mutValue.end(); ++iter, ++index) {
            auto [value, name] = *iter;

            TEST(iter == iter);

            TEST(values[index].first == value);
            TEST(values[index].second == name);
          }
          TEST(mutValue.begin() == mutValue.begin());
          TEST(mutValue.begin() != mutValue.end());
          TEST(values.size() == mutValue.size());
        }
        // test reverse const iterator
        {
          int index = static_cast<decltype(index)>(values.size()) - 1;
          for (SomethingTraitsByValue::const_reverse_iterator iter = cValue.rbegin(); iter != cValue.rend(); ++iter, --index) {
            auto [value, name] = *iter;

            TEST(iter == iter);

            TEST(values[index].first == value);
            TEST(values[index].second == name);
          }
          TEST(cValue.rbegin() == cValue.rbegin());
          TEST(cValue.rbegin() != cValue.rend());
        }
        // test reverse non-const iterator
        {
          int index = static_cast<decltype(index)>(values.size()) - 1;
          for (SomethingTraitsByValue::reverse_iterator iter = mutValue.rbegin(); iter != mutValue.rend(); ++iter, --index) {
            auto [value, name] = *iter;

            TEST(iter == iter);

            TEST(values[index].first == value);
            TEST(values[index].second == name);
          }
          TEST(mutValue.rbegin() == mutValue.rbegin());
          TEST(mutValue.rbegin() != mutValue.rend());
        }
      }

      // check name ordering
      {
        constexpr std::array< std::pair<Something, std::string_view>, 4> values{ {
          {Something::A, "a"},
          {Something::B, "b"},
          {Something::C, "c"},
          {Something::D, "d"},
        } };

        static_assert(values.size() == SomethingTraitsByName::total());
        TEST(values.size() == SomethingTraitsByName::total());

        // test forward const iterator
        {
          int index = 0;
          for (SomethingTraitsByName::const_iterator iter = cName.begin(); iter != cName.end(); ++iter, ++index) {
            auto [value, name] = *iter;

            TEST(iter == iter);

            TEST(values[index].first == value);
            TEST(values[index].second == name);

            auto asEnum = SomethingTraitsByName::toEnum(name);
            auto asString = SomethingTraitsByName::toString(value);
            TEST(asEnum == value);
            TEST(asString == name);
          }
          TEST(cName.begin() == cName.begin());
          TEST(cName.begin() != cName.end());
          TEST(values.size() == cName.size());
        }
        // test forward non-const iterator
        {
          int index = 0;
          for (SomethingTraitsByName::iterator iter = mutName.begin(); iter != mutName.end(); ++iter, ++index) {
            auto [value, name] = *iter;

            TEST(iter == iter);

            TEST(values[index].first == value);
            TEST(values[index].second == name);
          }
          TEST(mutName.begin() == mutName.begin());
          TEST(mutName.begin() != mutName.end());
          TEST(values.size() == mutName.size());
        }
        // test reverse const iterator
        {
          int index = static_cast<decltype(index)>(values.size()) - 1;
          for (SomethingTraitsByName::const_reverse_iterator iter = cName.rbegin(); iter != cName.rend(); ++iter, --index) {
            auto [value, name] = *iter;

            TEST(iter == iter);

            TEST(values[index].first == value);
            TEST(values[index].second == name);
          }
          TEST(cName.rbegin() == cName.rbegin());
          TEST(cName.rbegin() != cName.rend());
        }
        // test reverse non-const iterator
        {
          int index = static_cast<decltype(index)>(values.size()) - 1;
          for (SomethingTraitsByName::reverse_iterator iter = mutName.rbegin(); iter != mutName.rend(); ++iter, --index) {
            auto [value, name] = *iter;

            TEST(iter == iter);

            TEST(values[index].first == value);
            TEST(values[index].second == name);
          }
          TEST(mutName.rbegin() == mutName.rbegin());
          TEST(mutName.rbegin() != mutName.rend());
        }
      }

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    template <typename TEnum>
    constexpr static std::optional<Something> valueByIndex(int index) noexcept
    {
      TEnum e{};

      for (auto iter = std::cbegin(e); iter != std::cend(e); ++iter) {
        auto [value, name] = *iter;
        if (0 == index)
          return { value };
        --index;
      }
      return {};
    }

    //-------------------------------------------------------------------------
    template <typename TEnum>
    constexpr static std::string_view nameByIndex(int index) noexcept
    {
      for (auto [name, value] : TEnum{}) {
        if (0 == index)
          return { value };
        --index;
      }
      return {};
    }

    //-------------------------------------------------------------------------
    template <typename TEnum>
    constexpr static int checkStatics() noexcept
    {
      int check{};

      {
        constexpr auto v1 = valueByIndex<TEnum>(0);
        constexpr auto v2 = valueByIndex<TEnum>(1);
        constexpr auto v3 = valueByIndex<TEnum>(2);
        constexpr auto v4 = valueByIndex<TEnum>(3);

        static_assert(v1.has_value());
        static_assert(v2.has_value());
        static_assert(v3.has_value());
        static_assert(v4.has_value());

        if constexpr (zs::EnumOrder::Default == TEnum::Order::value) {
          static_assert(Something::C == v1.value());
          static_assert(Something::A == v2.value());
          static_assert(Something::D == v3.value());
          static_assert(Something::B == v4.value());
          ++check;
        }
        if constexpr (zs::EnumOrder::Value == TEnum::Order::value) {
          static_assert(Something::B == v1.value());
          static_assert(Something::A == v2.value());
          static_assert(Something::C == v3.value());
          static_assert(Something::D == v4.value());
          ++check;
        }
        if constexpr (zs::EnumOrder::Name == TEnum::Order::value) {
          static_assert(Something::A == v1.value());
          static_assert(Something::B == v2.value());
          static_assert(Something::C == v3.value());
          static_assert(Something::D == v4.value());
          ++check;
        }
      }

      {
        constexpr auto v1 = nameByIndex<TEnum>(0);
        constexpr auto v2 = nameByIndex<TEnum>(1);
        constexpr auto v3 = nameByIndex<TEnum>(2);
        constexpr auto v4 = nameByIndex<TEnum>(3);

        if constexpr (zs::EnumOrder::Default == TEnum::Order::value) {
          static_assert("c" == v1);
          static_assert("a" == v2);
          static_assert("d" == v3);
          static_assert("b" == v4);
          ++check;
        }
        if constexpr (zs::EnumOrder::Value == TEnum::Order::value) {
          static_assert("b" == v1);
          static_assert("a" == v2);
          static_assert("c" == v3);
          static_assert("d" == v4);
          ++check;
        }
        if constexpr (zs::EnumOrder::Name == TEnum::Order::value) {
          static_assert("a" == v1);
          static_assert("b" == v2);
          static_assert("c" == v3);
          static_assert("d" == v4);
          ++check;
        }
      }
      return check;
    }

    //-------------------------------------------------------------------------
    void compileType() noexcept(false)
    {
      // check lookup routines when using default sort
      {
        constexpr auto sa = SomethingTraits::toString(Something::A);
        constexpr auto ea = SomethingTraits::toEnum("a");

        constexpr auto sb = SomethingTraits::toString(Something::B);
        constexpr auto eb = SomethingTraits::toEnum("b");

        constexpr auto sc = SomethingTraits::toString(Something::C);
        constexpr auto ec = SomethingTraits::toEnum("c");

        constexpr auto sd = SomethingTraits::toString(Something::D);
        constexpr auto ed = SomethingTraits::toEnum("d");

        static_assert(Something::A == ea);
        static_assert(Something::B == eb);
        static_assert(Something::C == ec);
        static_assert(Something::D == ed);
      }

      // check lookup routines when using value sort
      {
        constexpr auto sa = SomethingTraitsByValue::toString(Something::A);
        constexpr auto ea = SomethingTraitsByValue::toEnum("a");

        constexpr auto sb = SomethingTraitsByValue::toString(Something::B);
        constexpr auto eb = SomethingTraitsByValue::toEnum("b");

        constexpr auto sc = SomethingTraitsByValue::toString(Something::C);
        constexpr auto ec = SomethingTraitsByValue::toEnum("c");

        constexpr auto sd = SomethingTraitsByValue::toString(Something::D);
        constexpr auto ed = SomethingTraitsByValue::toEnum("d");

        static_assert(Something::A == ea);
        static_assert(Something::B == eb);
        static_assert(Something::C == ec);
        static_assert(Something::D == ed);
      }

      // check lookup routines when using name sort
      {
        constexpr auto sa = SomethingTraitsByName::toString(Something::A);
        constexpr auto ea = SomethingTraitsByName::toEnum("a");

        constexpr auto sb = SomethingTraitsByName::toString(Something::B);
        constexpr auto eb = SomethingTraitsByName::toEnum("b");

        constexpr auto sc = SomethingTraitsByName::toString(Something::C);
        constexpr auto ec = SomethingTraitsByName::toEnum("c");

        constexpr auto sd = SomethingTraitsByName::toString(Something::D);
        constexpr auto ed = SomethingTraitsByName::toEnum("d");

        static_assert(Something::A == ea);
        static_assert(Something::B == eb);
        static_assert(Something::C == ec);
        static_assert(Something::D == ed);
      }

      constexpr int check1 = checkStatics<SomethingTraits>();
      constexpr int check2 = checkStatics<SomethingTraitsByValue>();
      constexpr int check3 = checkStatics<SomethingTraitsByName>();

      static_assert(2 == check1);
      static_assert(2 == check2);
      static_assert(2 == check3);

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    enum class None
    {
    };

    //-------------------------------------------------------------------------
    struct NoneDeclare final : public zs::EnumDeclare<None, 0>
    {
      constexpr const Entries operator()() const noexcept
      {
        return {};
      }
    };

    //-------------------------------------------------------------------------
    using NoneTraits = zs::EnumTraits<None, NoneDeclare>;
    using NoneTraitsByValue = zs::EnumTraits<None, NoneDeclare, zs::EnumOrder::Value>;
    using NoneTraitsByName = zs::EnumTraits<None, NoneDeclare, zs::EnumOrder::Name>;

    //-------------------------------------------------------------------------
    template <typename TEnum>
    constexpr static bool checkNone() noexcept
    {
      for (auto [value, name] : TEnum{}) {
        return false;
      }
      return true;
    }

    //-------------------------------------------------------------------------
    void emptyTest() noexcept(false)
    {
      static_assert(0 == NoneTraits::total());

      constexpr auto good1 = checkNone<NoneTraits>();
      constexpr auto good2 = checkNone<NoneTraitsByValue>();
      constexpr auto good3 = checkNone<NoneTraitsByName>();

      static_assert(good1);
      static_assert(good2);
      static_assert(good3);

      constexpr NoneTraits e1{};
      constexpr NoneTraitsByValue e2{};
      constexpr NoneTraitsByName e3{};

      bool found = false;
      for (auto iter = std::begin(e1); iter != std::end(e1); ++iter) {
        found = true;
      }
      for (auto iter = std::begin(e2); iter != std::end(e2); ++iter) {
        found = true;
      }
      for (auto iter = std::begin(e3); iter != std::end(e3); ++iter) {
        found = true;
      }
      for (auto iter = std::rbegin(e1); iter != std::rend(e1); ++iter) {
        found = true;
      }
      for (auto iter = std::rbegin(e2); iter != std::rend(e2); ++iter) {
        found = true;
      }
      for (auto iter = std::rbegin(e3); iter != std::rend(e3); ++iter) {
        found = true;
      }
      TEST(!found);

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    enum class WhoIsOn : int32_t
    {
      Third,
      Second,
      First,
    };

    //-------------------------------------------------------------------------
    struct WhoIsOnDeclare final : public zs::EnumDeclare<WhoIsOn, 3>
    {
      constexpr const Entries operator()() const noexcept
      {
        return { {
          {WhoIsOn::Third, "3rd"},
          {WhoIsOn::Second, "2nd"},
          {WhoIsOn::First, "1st"},
        } };
      }
    };

    //-------------------------------------------------------------------------
    using WhoIsOnTraits = zs::EnumTraits<WhoIsOn, WhoIsOnDeclare>;
    using WhoIsOnTraitsByValue = zs::EnumTraits<WhoIsOn, WhoIsOnDeclare, zs::EnumOrder::Value>;
    using WhoIsOnTraitsByName = zs::EnumTraits<WhoIsOn, WhoIsOnDeclare, zs::EnumOrder::Name>;

    //-------------------------------------------------------------------------
    template <typename TEnum>
    constexpr static int32_t checkWhoIsOn() noexcept
    {
      int index{ 7 };
      int32_t result{ 0 };
      for (auto [value, name] : TEnum{}) {
        ++index;

        auto useValue{ static_cast<std::underlying_type_t<decltype(value)>>(value) };
        useValue <<= 1;

        if (index % 2 == 0) {
          result += (useValue + index);
        } else {
          result *= (useValue * index);
        }
      }
      return result;
    }

    //-------------------------------------------------------------------------
    void whoIsOnTest() noexcept(false)
    {
      static_assert(3 == WhoIsOnTraits::total());

      constexpr auto good1 = checkWhoIsOn<WhoIsOnTraits>();
      constexpr auto good2 = checkWhoIsOn<WhoIsOnTraitsByValue>();
      constexpr auto good3 = checkWhoIsOn<WhoIsOnTraitsByName>();

      static_assert(158 == good1);
      static_assert(158 == good2);
      static_assert(226 == good3);

      constexpr WhoIsOnTraits e1{};
      constexpr WhoIsOnTraitsByValue e2{};
      constexpr WhoIsOnTraitsByName e3{};

      int32_t found = 0;
      for (auto iter = std::begin(e1); iter != std::end(e1); ++iter) {
        ++found;
      }
      for (auto iter = std::begin(e2); iter != std::end(e2); ++iter) {
        ++found;
      }
      for (auto iter = std::begin(e3); iter != std::end(e3); ++iter) {
        ++found;
      }
      for (auto iter = std::rbegin(e1); iter != std::rend(e1); ++iter) {
        ++found;
      }
      for (auto iter = std::rbegin(e2); iter != std::rend(e2); ++iter) {
        ++found;
      }
      for (auto iter = std::rbegin(e3); iter != std::rend(e3); ++iter) {
        ++found;
      }
      TEST((6 * WhoIsOnTraits::total()) == found);

      output(__FILE__ "::" __FUNCTION__);
    }


    //-------------------------------------------------------------------------
    enum class Colors : uint32_t
    {
      Red = 0xFF0000,
      Green = 0x00FF00,
      Blue = 0x0000FF,
      Yellow = 0xFFFF00,
      Purple = 0xCC33FF,
      Violet = 0xCC33FF,
      ToastedWheat = 0xCC6600,
      LightBrown = 0xCC6600,
      WinterMint = 0x66CCFF,
      LightBlue = 0x66CCFF,
      Red2 = 0xFF0000,
    };

    //-------------------------------------------------------------------------
    struct ColorsDeclare final : public zs::EnumDeclare<Colors, 11>
    {
      constexpr const Entries operator()() const noexcept
      {
        return { {
          {Colors::Red, "red"},
          {Colors::Green, "green"},
          {Colors::Blue, "blue"},
          {Colors::Yellow, "yellow"},
          {Colors::Purple, "purple"},
          {Colors::Violet, "violet"},
          {Colors::ToastedWheat, "toasted-wheat"},
          {Colors::LightBrown, "light-brown"},
          {Colors::WinterMint, "winter-mint"},
          {Colors::LightBlue, "light-blue"},
          {Colors::Red2, "red"},
        } };
      }
    };

    //-------------------------------------------------------------------------
    using ColorsTraits = zs::EnumTraits<Colors, ColorsDeclare>;
    using ColorsTraitsByValue = zs::EnumTraits<Colors, ColorsDeclare, zs::EnumOrder::Value>;
    using ColorsTraitsByName = zs::EnumTraits<Colors, ColorsDeclare, zs::EnumOrder::Name>;

    //-------------------------------------------------------------------------
    template <typename TEnum>
    constexpr static int checkColor() noexcept
    {
      constexpr ColorsDeclare::Entries inDefaultOrder{ {
          {Colors::Red, "red"},
          {Colors::Green, "green"},
          {Colors::Blue, "blue"},
          {Colors::Yellow, "yellow"},
          {Colors::Purple, "purple"},
          {Colors::Violet, "violet"},
          {Colors::ToastedWheat, "toasted-wheat"},
          {Colors::LightBrown, "light-brown"},
          {Colors::WinterMint, "winter-mint"},
          {Colors::LightBlue, "light-blue"},
          {Colors::Red2, "red"}
      } };

      constexpr ColorsDeclare::Entries inValueOrder{ {
          {Colors::Blue, "blue"},
          {Colors::Green, "green"},
          {Colors::LightBlue, "light-blue"},
          {Colors::WinterMint, "winter-mint"},
          {Colors::Purple, "purple"},
          {Colors::Violet, "violet"},
          {Colors::ToastedWheat, "toasted-wheat"},
          {Colors::LightBrown, "light-brown"},
          {Colors::Red, "red"},
          {Colors::Red2, "red"},
          {Colors::Yellow, "yellow"}
      } };

      constexpr ColorsDeclare::Entries inNameOrder{ {
          {Colors::Blue, "blue"},
          {Colors::Green, "green"},
          {Colors::LightBlue, "light-blue"},
          {Colors::LightBrown, "light-brown"},
          {Colors::Purple, "purple"},
          {Colors::Red, "red"},
          {Colors::Red2, "red"},
          {Colors::ToastedWheat, "toasted-wheat"},
          {Colors::Violet, "violet"},
          {Colors::WinterMint, "winter-mint"},
          {Colors::Yellow, "yellow"}
      } };

      int result{};
      int index{};

      for (auto& [value, name] : TEnum{}) {
        if (TEnum::toString(value) == name)
          ++result;
        if (TEnum::toEnum(name) == value)
          ++result;
        if constexpr (zs::EnumOrder::Default == TEnum::Order::value) {
          if (inDefaultOrder[index].first == value)
            ++result;
          if (inDefaultOrder[index].second == name)
            ++result;
        }
        else if constexpr (zs::EnumOrder::Value == TEnum::Order::value) {
          if (inValueOrder[index].first == value)
            ++result;
          if (inValueOrder[index].second == name)
            ++result;
        }
        else {
          if (inNameOrder[index].first == value)
            ++result;
          if (inNameOrder[index].second == name)
            ++result;
        }

        ++index;
      }
      return result;
    }

    //-------------------------------------------------------------------------
    template <typename TEnum>
    constexpr static size_t checkIterMath() noexcept
    {
      size_t result{};
      {
        auto iter1 = std::begin(TEnum{});
        auto iter2 = (std::begin(TEnum{}) + 2);

        auto temp1{ iter1 };
        ++temp1;
        ++temp1;

        if (iter1 != iter2)
          ++result;
        if (temp1 == iter2)
          ++result;
        if (iter1 < iter2)
          ++result;
        if (iter1 <= iter2)
          ++result;
        if (iter2 > iter1)
          ++result;
        if (iter2 >= iter1)
          ++result;
      }
      {
        auto iter1 = std::rbegin(TEnum{});
        auto iter2 = (std::rbegin(TEnum{}) + 2);

        auto temp1{ iter1 };
        ++temp1;
        ++temp1;

        if (iter1 != iter2)
          ++result;
        if (temp1 == iter2)
          ++result;
        if (iter1 < iter2)
          ++result;
        if (iter1 <= iter2)
          ++result;
        if (iter2 > iter1)
          ++result;
        if (iter2 >= iter1)
          ++result;
      }
      {
        auto iter1 = std::end(TEnum{});
        auto iter2 = (std::end(TEnum{}) - 2);

        auto temp1{ iter1 };
        --temp1;
        --temp1;

        if (iter1 != iter2)
          ++result;
        if (temp1 == iter2)
          ++result;
        if (iter1 > iter2)
          ++result;
        if (iter1 >= iter2)
          ++result;
        if (iter2 < iter1)
          ++result;
        if (iter2 <= iter1)
          ++result;
      }
      {
        auto iter1 = std::rend(TEnum{});
        auto iter2 = (std::rend(TEnum{}) - 2);

        auto temp1{ iter1 };
        --temp1;
        --temp1;

        if (iter1 != iter2)
          ++result;
        if (temp1 == iter2)
          ++result;
        if (iter1 > iter2)
          ++result;
        if (iter1 >= iter2)
          ++result;
        if (iter2 < iter1)
          ++result;
        if (iter2 <= iter1)
          ++result;
      }
      {
        auto iter1 = std::cbegin(TEnum{});
        auto iter2 = (2 + std::cbegin(TEnum{}));

        auto temp1{ iter1 };
        ++temp1;
        ++temp1;

        if (iter1 != iter2)
          ++result;
        if (temp1 == iter2)
          ++result;
        if (iter1 < iter2)
          ++result;
        if (iter1 <= iter2)
          ++result;
        if (iter2 > iter1)
          ++result;
        if (iter2 >= iter1)
          ++result;
      }
      {
        auto iter1 = std::crbegin(TEnum{});
        auto iter2 = (2 + std::crbegin(TEnum{}));

        auto temp1{ iter1 };
        ++temp1;
        ++temp1;

        if (iter1 != iter2)
          ++result;
        if (temp1 == iter2)
          ++result;
        if (iter1 < iter2)
          ++result;
        if (iter1 <= iter2)
          ++result;
        if (iter2 > iter1)
          ++result;
        if (iter2 >= iter1)
          ++result;
      }
      {
        auto iter1 = std::cend(TEnum{});
        auto iter2 = (std::cend(TEnum{}) - 2);

        auto temp1{ iter1 };
        --temp1;
        --temp1;

        if (iter1 != iter2)
          ++result;
        if (temp1 == iter2)
          ++result;
        if (iter1 >= iter2)
          ++result;
        if (iter1 > iter2)
          ++result;
        if (iter2 <= iter1)
          ++result;
        if (iter2 < iter1)
          ++result;
      }
      return result;
    }

    //-------------------------------------------------------------------------
    void colorsTest() noexcept(false)
    {
      static_assert(11 == ColorsTraits::total());

      constexpr auto good1 = checkColor<ColorsTraits>();
      constexpr auto good2 = checkColor<ColorsTraitsByValue>();
      constexpr auto good3 = checkColor<ColorsTraitsByName>();

      // duplicate color values will cause string/value mismatching at times
      static_assert((ColorsTraits::total()*4) - 3 == good1);
      static_assert((ColorsTraitsByValue::total()*4) - 3 == good2);
      static_assert((ColorsTraitsByName::total()*4) - 3 == good3);

      constexpr auto good4 = checkIterMath<ColorsTraits>();

      static_assert(42 == good4);

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void runAll() noexcept(false)
    {
      auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

      runner([&]() { basic(); });
      runner([&]() { iterators(); });
      runner([&]() { compileType(); });
      runner([&]() { emptyTest(); });
      runner([&]() { whoIsOnTest(); });
      runner([&]() { colorsTest(); });
    }
  };

  //---------------------------------------------------------------------------
  void testEnum() noexcept(false)
  {
    EnumBasics{}.runAll();
  }

}
