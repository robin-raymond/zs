
#include <zs/RandomAccessListIterator.h>

#include "common.h"

#include <list>
#include <optional>
#include <iostream>

namespace zsTest
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  struct RandomAccessListIteratorBasics
  {
    std::optional<std::list<std::string>> values_;

    using ListType = decltype(values_)::value_type;

    //-------------------------------------------------------------------------
    void reset()
    {
      values_.reset();
      values_.emplace();
    }

    auto& get() noexcept { return *values_; }
    const std::list<std::string>& get_const() noexcept { return *values_; }

    void basic() noexcept(false)
    {
      auto checkEmpty{ [&](auto&& value) noexcept(false) {
        TEST(value.empty());
        TEST(!value.has_value());
        TEST(value.isBegin());
        TEST(value.isEnd());
        TEST(value.size() == 0);
        if (value) {
        }
        TEST(!static_cast<bool>(value));
      } };

      auto manipulateEmptySafe{ [&](auto&& value) noexcept(false) {
        ++value;
        --value;
        value += 10;
        value -= 10;
        value += -10;
        value -= -10;
        auto temp1{ value++ };
        auto temp2{ value-- };
        ++temp1;
        ++temp2;
        auto temp3{ value + 5 };
        auto temp4{ value - 5 };
        ++temp3;
        ++temp4;
        auto temp5{ 5 + value };
        auto temp6{ 5 - value };
        ++temp5;
        ++temp6;
        TEST(value.hasAhead(0));
        TEST(value.hasBehind(0));
        TEST(!value.hasAhead(-1));
        TEST(!value.hasBehind(-1));
        TEST(!value.hasAhead(1));
        TEST(!value.hasBehind(1));
        TEST(value == temp1);
        TEST(value == temp2);
        TEST(value == temp3);
        TEST(value == temp4);
        TEST(value == temp5);
        TEST(value == temp6);
        TEST(!(value != temp1));
        TEST(!(value != temp2));
        TEST(!(value != temp3));
        TEST(!(value != temp4));
        TEST(!(value != temp5));
        TEST(!(value != temp6));
      } };

      auto checkData{ [&](auto value, const std::vector<std::string>& data) noexcept(false) {
        TEST(value.hasAhead(data.size() > 0 ? data.size() -1 : 0));
        size_t count{};
        for (auto str : data) {
          TEST(value.has_value());
          TEST(str == *value);
          auto underlyingIter = value.underlying();
          TEST(str == *underlyingIter);
          ++value;
          ++count;
        }
        TEST(!value.empty());
        TEST(value.size() >= data.size());
        TEST(!value.has_value());
        TEST(!value.isBegin());
        TEST(value.isEnd());
        TEST(!static_cast<bool>(value));
      } };


      {
        reset();
        zs::RandomAccessListIterator<ListType> iter{ get() };

        checkEmpty(iter);
        manipulateEmptySafe(iter);
        checkEmpty(iter);
      }

      {
        reset();
        auto iter{ zs::makeRandom(get()) };
        checkEmpty(iter);
        manipulateEmptySafe(iter);
        checkEmpty(iter);
      }

      {
        reset();
        auto iter{ zs::makeRandom(get_const()) };
        checkEmpty(iter);
        manipulateEmptySafe(iter);
        checkEmpty(iter);
      }

      {
        reset();
        auto& list = get();
        list.push_back("apple");
        auto iter{ zs::makeRandom(get()) };
        checkData(iter, std::vector<std::string>{ "apple" });
      }

      {
        reset();
        auto& list = get();
        list.push_back("apple");
        list.push_back("banana");
        auto iter{ zs::makeRandom(get()) };
        checkData(iter, std::vector<std::string>{ "apple", "banana" });
      }

      {
        reset();
        auto& list = get();
        list.push_back("apple");
        list.push_back("banana");
        auto iter{ zs::makeRandom(get()) };
        ++iter;
        checkData(iter, std::vector<std::string>{ "banana" });
      }

      {
        reset();
        auto& list = get();
        list.push_back("apple");
        list.push_back("banana");
        auto iter{ zs::makeRandom(get()) };
        ++iter;
        --iter;
        checkData(iter, std::vector<std::string>{ "apple", "banana" });
      }


      {
        reset();
        auto& list = get();
        list.push_back("apple");
        list.push_back("banana");
        auto iter{ zs::makeRandom(get()) };
        auto temp1{ iter++ };
        auto temp2{ iter-- };
        checkData(iter, std::vector<std::string>{ "apple", "banana" });
        checkData(temp1, std::vector<std::string>{ "apple", "banana" });
        checkData(temp2, std::vector<std::string>{ "banana" });
      }

      {
        reset();
        auto& list = get();
        list.push_back("apple");
        list.push_back("banana");
        list.push_back("carrot");
        list.push_back("dates");
        list.push_back("elderberry");
        list.push_back("fig");
        list.push_back("grapes");
        list.push_back("honeydew");
        list.push_back("imbe");
        list.push_back("jackfruit");
        list.push_back("kiwi");
        list.push_back("lime");
        list.push_back("mango");
        list.push_back("nectarine");
        list.push_back("orange");
        list.push_back("pear");
        auto iter{ zs::makeRandom(get()) };
        auto temp1{ iter + 1 };
        auto temp2{ temp1 - 1 };
        auto temp3{ 5 + iter };
        auto temp4{ iter + 5 };
        auto temp5{ iter + 50000 };
        auto temp6{ iter - 50000 };
        auto temp7{ 1 - temp4 };
        auto temp8{ temp1 };
        TEST(temp8 == temp1);
        TEST(temp8 != temp7);
        checkData(temp8, std::vector<std::string>{
          "banana", "carrot", "dates", "elderberry", "fig", "grapes", "honeydew",
          "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        temp8 = temp7;
        TEST(temp8 == temp7);
        TEST(temp8 != temp1);

        auto temp9{ temp4 };
        temp9 += 2;
        auto temp10{ temp4 };
        temp10 -= 2;

        checkData(iter, std::vector<std::string>{
          "apple", "banana", "carrot", "dates", "elderberry", "fig", "grapes", "honeydew",
          "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        TEST("apple" == *iter);
        TEST(std::string{ "apple" }.length() == iter->length());
        TEST("apple" == iter[0]);
        TEST("banana" == iter[1]);
        TEST("carrot" == iter[2]);
        TEST("pear" == iter[15]);
        TEST("pear" == iter[16]);   // safety protection
        TEST("pear" == iter[200]);  // safety protection
        checkData(temp1, std::vector<std::string>{
          "banana", "carrot", "dates", "elderberry", "fig", "grapes", "honeydew",
          "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        checkData(temp2, std::vector<std::string>{
          "apple", "banana", "carrot", "dates", "elderberry", "fig", "grapes", "honeydew",
          "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        checkData(temp3, std::vector<std::string>{
          "fig", "grapes", "honeydew",
          "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        TEST("fig" == temp3[0]);
        TEST("grapes" == temp3[1]);
        TEST("honeydew" == temp3[2]);
        TEST("elderberry" == temp3[-1]);
        checkData(temp4, std::vector<std::string>{
          "fig", "grapes", "honeydew",
          "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        checkData(temp5, std::vector<std::string>{});
        TEST("pear" == *temp5);                                   // safety protection
        TEST(std::string{ "pear" }.length() == temp5->length());  // safety protection
        checkData(temp6, std::vector<std::string>{
          "apple", "banana", "carrot", "dates", "elderberry", "fig", "grapes", "honeydew",
          "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        checkData(temp7, std::vector<std::string>{
          "elderberry", "fig", "grapes", "honeydew",
          "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        checkData(temp8, std::vector<std::string>{
          "elderberry", "fig", "grapes", "honeydew",
          "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        checkData(temp9, std::vector<std::string>{
          "honeydew",
          "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        checkData(temp10, std::vector<std::string>{
          "dates", "elderberry", "fig", "grapes", "honeydew",
          "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        TEST(iter.hasAhead(15));
        TEST(!iter.hasAhead(16));
        TEST(iter.hasBehind(0));
        TEST(!iter.hasBehind(1));
        TEST(iter.hasAhead(0));
        TEST(!iter.hasAhead(-1));
        TEST(temp10.hasBehind(3));
        TEST(!temp10.hasBehind(4));
        TEST(temp10.hasAhead(-3));
        TEST(!temp10.hasAhead(-4));
        TEST(temp10.hasAhead(12));
        TEST(!temp10.hasAhead(13));
        TEST(temp10.hasBehind(-12));
        TEST(!temp10.hasBehind(-13));

        auto endIter = zs::makeRandom(get(), get().end());
        TEST(endIter.hasAhead(0));
        TEST(!endIter.hasAhead(1));
        TEST(endIter.hasAhead(0));
        TEST(endIter.hasAhead(-1));
        TEST(endIter.hasBehind(0));
        TEST(endIter.hasBehind(1));
        TEST(endIter.hasBehind(0));
        TEST(!endIter.hasBehind(-1));

        TEST(endIter.hasBehind(16));
        TEST(!endIter.hasBehind(17));
        TEST(endIter.hasAhead(-16));
        TEST(!endIter.hasAhead(-17));
      }

      {
        // DO NOT RESET
        auto& list = get_const();
        auto iter{ zs::makeRandom(get()) };
        auto temp1{ iter + 1 };
        auto temp2{ temp1 - 1 };
        auto temp3{ 5 + iter };
        auto temp4{ iter + 5 };
        auto temp5{ iter + 50000 };
        auto temp6{ iter - 50000 };
        auto temp7{ 1 - temp4 };
        auto temp8{ temp1 };
        TEST(temp8 == temp1);
        TEST(temp8 != temp7);
        checkData(temp8, std::vector<std::string>{
          "banana", "carrot", "dates", "elderberry", "fig", "grapes", "honeydew",
            "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        temp8 = temp7;
        TEST(temp8 == temp7);
        TEST(temp8 != temp1);

        auto temp9{ temp4 };
        temp9 += 2;
        auto temp10{ temp4 };
        temp10 -= 2;

        checkData(iter, std::vector<std::string>{
          "apple", "banana", "carrot", "dates", "elderberry", "fig", "grapes", "honeydew",
            "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        TEST("apple" == *iter);
        TEST(std::string{ "apple" }.length() == iter->length());
        TEST("apple" == iter[0]);
        TEST("banana" == iter[1]);
        TEST("carrot" == iter[2]);
        TEST("pear" == iter[15]);
        TEST("pear" == iter[16]);   // safety protection
        TEST("pear" == iter[200]);  // safety protection
        checkData(temp1, std::vector<std::string>{
          "banana", "carrot", "dates", "elderberry", "fig", "grapes", "honeydew",
            "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        checkData(temp2, std::vector<std::string>{
          "apple", "banana", "carrot", "dates", "elderberry", "fig", "grapes", "honeydew",
            "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        checkData(temp3, std::vector<std::string>{
          "fig", "grapes", "honeydew",
            "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        TEST("fig" == temp3[0]);
        TEST("grapes" == temp3[1]);
        TEST("honeydew" == temp3[2]);
        TEST("elderberry" == temp3[-1]);
        checkData(temp4, std::vector<std::string>{
          "fig", "grapes", "honeydew",
            "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        checkData(temp5, std::vector<std::string>{});
        TEST("pear" == *temp5);                                   // safety protection
        TEST(std::string{ "pear" }.length() == temp5->length());  // safety protection
        checkData(temp6, std::vector<std::string>{
          "apple", "banana", "carrot", "dates", "elderberry", "fig", "grapes", "honeydew",
            "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        checkData(temp7, std::vector<std::string>{
          "elderberry", "fig", "grapes", "honeydew",
            "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        checkData(temp8, std::vector<std::string>{
          "elderberry", "fig", "grapes", "honeydew",
            "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        checkData(temp9, std::vector<std::string>{
          "honeydew",
            "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });
        checkData(temp10, std::vector<std::string>{
          "dates", "elderberry", "fig", "grapes", "honeydew",
            "imbe", "jackfruit", "kiwi", "lime", "mango", "nectarine", "orange", "pear" });

        TEST(iter.hasAhead(15));
        TEST(!iter.hasAhead(16));
        TEST(iter.hasBehind(0));
        TEST(!iter.hasBehind(1));
        TEST(iter.hasAhead(0));
        TEST(!iter.hasAhead(-1));
        TEST(temp10.hasBehind(3));
        TEST(!temp10.hasBehind(4));
        TEST(temp10.hasAhead(-3));
        TEST(!temp10.hasAhead(-4));
        TEST(temp10.hasAhead(12));
        TEST(!temp10.hasAhead(13));
        TEST(temp10.hasBehind(-12));
        TEST(!temp10.hasBehind(-13));

        auto endIter = zs::makeRandom(get_const(), get_const().end());
        TEST(endIter.hasAhead(0));
        TEST(!endIter.hasAhead(1));
        TEST(endIter.hasAhead(0));
        TEST(endIter.hasAhead(-1));
        TEST(endIter.hasBehind(0));
        TEST(endIter.hasBehind(1));
        TEST(endIter.hasBehind(0));
        TEST(!endIter.hasBehind(-1));

        TEST(endIter.hasBehind(16));
        TEST(!endIter.hasBehind(17));
        TEST(endIter.hasAhead(-16));
        TEST(!endIter.hasAhead(-17));
      }

      {
        reset();
        auto& list = get();

        list.push_back("apple");

        auto last = std::end(list);
        list.push_back("banana");
        --last;
        TEST(*last == "banana");
      }

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void runAll() noexcept(false)
    {
      auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

      runner([&]() { basic(); });
    }
  };

  //---------------------------------------------------------------------------
  void testRandomAccessListIterator() noexcept(false)
  {
    RandomAccessListIteratorBasics{}.runAll();
  }

}
