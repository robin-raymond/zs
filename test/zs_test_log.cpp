
#include <zs/log.h>

#include "common.h"

#include <optional>
#include <iostream>

namespace zsTest
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  struct LogBasics
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
      TEST("zs" == zs::component.name());
      TEST("zs::log" == zs::log::component.name());
      TEST(zs::component.id() != zs::log::component.id());

      for (auto& comp : zs::log::Component::all()) {
        std::cout << "name: " << comp.name() << " log-level: " << zs::log::LevelTraits::toString(comp.level()) << "\n";
      }

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void testEntry() noexcept(false)
    {
      {
        struct _AnonEntry {

          static auto& info() {
            static zs::log::MetaDataLogEntryInfo info{ &zs::log::component, "test1", __FILE__, __FUNCTION__, __LINE__};
            return info;
          }

        };

        std::string_view hello{ "hello" };

        zs::log::output(_AnonEntry{}, hello, 2);
        //auto value = _AnonEntry<>{};

        //std::cout << "value: " << value << "\n";
      }
      {
        struct _AnonEntry {
          static zs::log::MetaDataLogEntry::id_type link(const zs::log::MetaDataLogEntry& g) noexcept {
            return g.id();
          }
          static auto& info() {
            static zs::log::MetaDataLogEntryInfo info{ &zs::log::component, "test2", __FILE__, __FUNCTION__, __LINE__ };
            return info;
          }
        };
        auto value = _AnonEntry::link(zs::log::logEntryMetaData<_AnonEntry>);

        std::cout << "value: " << value << "\n";
      }

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void runAll() noexcept(false)
    {
      auto runner{ [&](auto&& func) noexcept(false) { reset(); func(); } };

      runner([&]() { test(); });
      runner([&]() { testEntry(); });
      runner([&]() { testEntry(); });
    }
  };

  //---------------------------------------------------------------------------
  void testLog() noexcept(false)
  {
    LogBasics{}.runAll();
  }

}
