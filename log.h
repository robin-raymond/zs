
#pragma once

#include <string_view>
#include <cstring>
#include <cwchar>
#include <cassert>

#include "enum.h"
#include "traits.h"
#include "dependency/safeint.h"
#include "dependency/gsl.h"

namespace zs
{
  namespace log
  {
    inline constexpr std::integral_constant<zs::size_type, static_cast<zs::size_type>(64 * 1024)> maxLogBufferSize;
    inline constexpr std::integral_constant<zs::size_type, static_cast<zs::size_type>(512)> maxLogArrayEntries;
    inline constexpr std::integral_constant<zs::size_type, static_cast<zs::size_type>(1024)> maxLogStringLength;

    class Component;

    //-------------------------------------------------------------------------
    enum class Level
    {
      None,
      Basic,
      Detail,
      Debug,
      Trace,
      Insane,
    };

    //-------------------------------------------------------------------------
    struct LevelDeclare : public EnumDeclare<Level, 6>
    {
      constexpr const Entries operator()() const noexcept {
        return { {
          {Level::None, "none"},
          {Level::Basic, "basic"},
          {Level::Detail, "detail"},
          {Level::Debug, "debug"},
          {Level::Trace, "trace"},
          {Level::Insane, "insane"},
        } };
      }
    };

    using LevelTraits = EnumTraits<Level, LevelDeclare>;

    //-------------------------------------------------------------------------
    enum class Severity
    {
      Info,
      Informational = Info,
      Warning,
      Error,
      Critical,
      Fatal,
    };

    //-------------------------------------------------------------------------
    struct SeverityDeclare : public EnumDeclare<Severity, 5>
    {
      constexpr const Entries operator()() const noexcept {
        return { {
          {Severity::Info, "info"},
          {Severity::Warning, "warning"},
          {Severity::Error, "error"},
          {Severity::Critical, "critical"},
          {Severity::Fatal, "fatal"},
        } };
      }
    };

    using SeverityTraits = EnumTraits<Severity, SeverityDeclare>;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    class Component final
    {
    public:
      struct allow_constexpr {};
      struct all_components;
      friend struct all_components;

      using size_type = zs::size_type;
      using index_type = zs::index_type;
      using id_type = size_type;
      using constexpr_type = allow_constexpr;

      template <typename TComponent>
      struct Iterator;

      using iterator = Iterator<Component>;
      using const_iterator = Iterator<const Component>;

      friend iterator;
      friend const_iterator;

      //-----------------------------------------------------------------------
      Component(
        const std::string_view name,
        Level level = Level::Basic) noexcept :
        id_{ nextId() },
        level_{ level },
        name_{ name },
        next_{ std::exchange(head(), this) }
      {
      }

      //-----------------------------------------------------------------------
      constexpr Component(
        const constexpr_type&,
        const std::string_view name,
        Level level = Level::Basic) noexcept :
        id_{},
        level_{ level },
        name_{ name }
      {
      }

      constexpr Component() noexcept = delete;
      constexpr Component(const Component&) noexcept = delete;
      constexpr Component(Component&&) noexcept = delete;

      constexpr Component& operator=(const Component&) noexcept = delete;
      constexpr Component& operator=(Component&&) noexcept = delete;

      [[nodiscard]] constexpr bool isLogging(Level level) const noexcept { return level_ >= level; }

      constexpr void level(Level level) noexcept { level_ = level; }
      [[nodiscard]] constexpr Level level() const noexcept { return level_; }

      [[nodiscard]] constexpr id_type id() const noexcept { return id_; }
      [[nodiscard]] constexpr const std::string_view name() const noexcept { return name_; }

      //-----------------------------------------------------------------------
      template <typename TComponent>
      struct Iterator
      {
        using forward_iterator = iterator;
        using const_forward_iterator = const_iterator;

        constexpr Iterator() = default;

        template <typename T = const_forward_iterator, typename std::enable_if_t<std::is_const_v<TComponent>, T> * = nullptr>
        constexpr Iterator(const const_forward_iterator& value) noexcept : value_{ const_cast<Component*>(value.value_) } {}
        constexpr Iterator(const forward_iterator& value) noexcept : value_{ value.value_ } {}

        template <typename T = const_forward_iterator, typename std::enable_if_t<std::is_const_v<TComponent>, T> * = nullptr>
        constexpr Iterator(const_forward_iterator&& value) noexcept : value_{ value.value_ } {}
        constexpr Iterator(forward_iterator&& value) noexcept : value_{ value.value_ } {}

        constexpr Iterator(const Component* value) noexcept : value_{ const_cast<Component*>(value) } {}
        constexpr Iterator(TComponent&& value) noexcept : value_{ value } {}

        template <typename T = const_forward_iterator, typename std::enable_if_t<std::is_const_v<TComponent>, T> * = nullptr>
        constexpr auto& operator=(const const_forward_iterator& value) noexcept { value_ = value.value_; return *this; };
        constexpr auto& operator=(forward_iterator& value) noexcept { value_ = value.value_; return *this; };

        template <typename T = const_forward_iterator, typename std::enable_if_t<std::is_const_v<TComponent>, T> * = nullptr>
        constexpr auto& operator=(const_forward_iterator&& value) noexcept { value_ = value.value_; return *this; };
        constexpr auto& operator=(forward_iterator&& value) noexcept { value_ = value.value_; return *this; };

        template <typename T = const_forward_iterator, typename std::enable_if_t<std::is_const_v<TComponent>, T> * = nullptr>
        [[nodiscard]] constexpr const auto& operator*() const noexcept { return *value_; }
        template <typename T = forward_iterator, typename std::enable_if_t<!std::is_const_v<TComponent>, T> * = nullptr>
        [[nodiscard]] constexpr auto& operator*() noexcept { return *value_; }

        constexpr auto& operator++() noexcept {
          value_ = value_->next_;
          return *this;
        }

        [[nodiscard]] constexpr auto operator==(const Iterator& value) const noexcept { return value_ == value.value_; }
        [[nodiscard]] constexpr auto operator!=(const Iterator& value) const noexcept { return value_ != value.value_; }

      private:
        std::remove_const_t<TComponent>* value_{ nullptr };
      };

      struct all_components {
        [[nodiscard]] constexpr iterator begin() noexcept { return iterator{ head() }; }
        [[nodiscard]] constexpr iterator end() noexcept { return iterator{}; }

        [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return const_iterator{ head() }; }
        [[nodiscard]] constexpr const_iterator cend() const noexcept { return const_iterator{}; }
      };

      constexpr static all_components all() noexcept { return {}; }

    protected:
      //-----------------------------------------------------------------------
      [[nodiscard]] static Component*& head() noexcept
      {
        static Component* gHead{ nullptr };
        return gHead;
      }

      //-----------------------------------------------------------------------
      [[nodiscard]] static id_type nextId() noexcept
      {
        static id_type gId{};
        ++gId;
        return gId;
      }

      const id_type id_{};
      volatile Level level_{};
      const std::string_view name_{};

      Component* const next_{ nullptr };
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    struct MetaDataTypeInfo
    {
      using size_type = zs::size_type;
      using index_type = zs::index_type;

      std::string_view typeName_{};
      std::string_view paramName_{};
      bool isIntegral_{};
      bool isSigned_{};
      bool isFloatingPoint_{};
      size_type elementWidth_{};      // 0 is legal (meaning the size is dependent on sub elements)
      size_type totalElements_{};     // 0 is legal (meaning the array size is unknown in advance)
      size_type totalSubEntries_{};   // 0 is legal (meaning no sub-entries exist)

      constexpr bool isArray() const noexcept { return (0 == totalElements_) || (totalElements_ > 1); }
      constexpr bool isArrayVariableSized() const noexcept { return 0 == totalElements_; }
      constexpr bool hasSubEntries() const noexcept { return 0 != totalSubEntries_; }
      constexpr bool subElementsContainSizing() const noexcept { return 0 == elementWidth_; }

      constexpr MetaDataTypeInfo() noexcept = default;
      constexpr MetaDataTypeInfo(const MetaDataTypeInfo&) noexcept = default;
      constexpr MetaDataTypeInfo(MetaDataTypeInfo&&) noexcept = default;

      constexpr MetaDataTypeInfo& operator=(const MetaDataTypeInfo&) noexcept = default;
      constexpr MetaDataTypeInfo& operator=(MetaDataTypeInfo&&) noexcept = default;

      //-----------------------------------------------------------------------
      template <typename T>
      constexpr static MetaDataTypeInfo simple() noexcept {
        MetaDataTypeInfo result{};
        // typeid(...) is sadly runtime only despite the information being known at compile time
        // result.typeName_ = typeid(T).name();
        result.isIntegral_ = std::is_integral_v<T>;
        result.isSigned_ = std::is_signed_v<T>;
        result.isFloatingPoint_ = std::is_floating_point_v<T>;
        result.elementWidth_ = sizeof(T);
        result.totalElements_ = 1;
        return result;
      }

      template <typename T>
      constexpr void fixTypeName() noexcept { typeName_ = typeid(T).name(); }
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    struct MetaDataTypeCommon
    {
      using size_type = zs::size_type;
      using index_type = zs::index_type;
      using array_count_size_type = std::uint32_t;

      //-----------------------------------------------------------------------
      template <typename TMetaDataType>
      constexpr static size_type calculateFixedSize() noexcept
      {
        if constexpr (TMetaDataType::isFixedSize()) {
          constexpr size_type result{ TMetaDataType::size() };
          return result;
        }
        else {
          return 0;
        }
      }

      //-----------------------------------------------------------------------
      template <typename TMetaDataType, typename TValue>
      constexpr static size_type calculateDynamicSize(TMetaDataType&& metaDataTypeInstance, TValue &&value) noexcept
      {
        if constexpr (!TMetaDataType::isFixedSize()) {
          return metaDataTypeInstance.size(std::forward<decltype(value)>(value));
        }
        else {
          return 0;
        }
      }

      //-----------------------------------------------------------------------
      template <typename TMetaDataType, typename TSubType>
      static void fillInfo(MetaDataTypeInfo*& first, MetaDataTypeInfo* last, std::string_view paramName) noexcept
      {
        (*first) = TMetaDataType::info();
        (*first).fixTypeName<TSubType>();
        (*first).paramName_ = paramName;
        ++first;

        if constexpr (TMetaDataType::info().totalSubEntries_ > 0) {
          TMetaDataType::fill(first, last);
          first += TMetaDataType::info().totalSubEntries_;
        }
        assert(first <= last);
      }

      constexpr static size_type sizeCount() noexcept { return sizeof(array_count_size_type); }

      //-----------------------------------------------------------------------
      void packCount(std::byte*& buffer, size_type total, size_type& remaining) const noexcept
      {
        array_count_size_type count = gsl::narrow_cast<decltype(count)>(total);

        if (remaining < sizeof(count)) {
          remaining = 0;
          return;
        }

        memcpy(buffer, &count, sizeof(count));
        buffer += sizeof(count);
        remaining -= sizeof(count);
      }

      //-----------------------------------------------------------------------
      void packData(std::byte*& buffer, const void* source, size_type size, size_type& remaining) const noexcept
      {
        if (remaining < size) {
          remaining = 0;
          return;
        }

        memcpy(buffer, source, size);
        buffer += size;
        remaining -= size;
      }
    };

    template <typename T, typename Enabled = void>
    struct MetaDataType;

    //-------------------------------------------------------------------------
    struct MetaDataTypeVariable : public MetaDataTypeCommon
    {
      constexpr static auto isFixedSize() noexcept { return false; }
    };

    //-------------------------------------------------------------------------
    template <typename T>
    struct MetaDataType<T, std::enable_if_t<std::is_same_v<void, T>>> final : public MetaDataTypeCommon
    {
      using type = void;
    };
  }
}

#include "detail/detail_log.h"

namespace zs
{
  namespace log
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    struct MetaDataLogEntryInfo
    {
      const Component* const component_{ nullptr };
      const std::string_view name_{};
      const std::string_view file_{};
      const std::string_view func_{};
      const int line_{};
    };

    //-------------------------------------------------------------------------
    class MetaDataLogEntry
    {
    public:
      struct allow_constexpr {};
      struct all_types;
      friend struct all_types;

      using size_type = zs::size_type;
      using index_type = zs::index_type;
      using id_type = size_type;
      using constexpr_type = allow_constexpr;

      //-----------------------------------------------------------------------
      MetaDataLogEntry(MetaDataLogEntryInfo &info) noexcept :
        id_{ nextId() },
        component_{ info.component_ },
        name_{ info.name_ },
        file_{ info.file_ },
        func_{ info.func_ },
        line_{ info.line_ },
        next_{ std::exchange(head(), this) }
      {}
      //-----------------------------------------------------------------------
      constexpr MetaDataLogEntry(
        const constexpr_type &,
        MetaDataLogEntryInfo& info) noexcept :
        id_{},
        name_{ info.name_ },
        file_{ info.file_ },
        func_{ info.func_ },
        line_{ info.line_ }
      {}

      constexpr MetaDataLogEntry() = delete;
      constexpr MetaDataLogEntry(const MetaDataLogEntry&) = delete;
      constexpr MetaDataLogEntry(MetaDataLogEntry&&) = delete;

      constexpr MetaDataLogEntry& operator=(const MetaDataLogEntry&) = delete;
      constexpr MetaDataLogEntry& operator=(MetaDataLogEntry&&) = delete;

      [[nodiscard]] constexpr id_type id() const noexcept { return id_; }
      [[nodiscard]] constexpr const std::string_view name() const noexcept { return name_; }

      struct all_types {
        friend class MetaDataLogEntry;

        constexpr const MetaDataTypeInfo* begin() const noexcept { return first_; }
        constexpr const MetaDataTypeInfo* end() const noexcept { return last_; }
        constexpr const MetaDataTypeInfo* cbegin() const noexcept { return first_; }
        constexpr const MetaDataTypeInfo* cend() const noexcept { return last_; }

      protected:
        MetaDataTypeInfo* first_{ nullptr };
        MetaDataTypeInfo* last_{ nullptr };
      };

      constexpr all_types types() noexcept
      {
        all_types result;
        result.first_ = first_;
        result.last_ = last_;
        return result;
      }

    protected:
      //-----------------------------------------------------------------------
      [[nodiscard]] static MetaDataLogEntry*& head() noexcept
      {
        static MetaDataLogEntry* gHead{ nullptr };
        return gHead;
      }

      //-----------------------------------------------------------------------
      [[nodiscard]] static id_type nextId() noexcept
      {
        static id_type gId{};
        ++gId;
        return gId;
      }

   protected:
      const id_type id_{};
      const Component* const component_{ nullptr };
      const std::string_view name_{};
      const std::string_view file_{};
      const std::string_view func_{};
      const int line_{};
      MetaDataTypeInfo* first_{ nullptr };
      MetaDataTypeInfo* last_{ nullptr };

      MetaDataLogEntry* const next_{ nullptr };
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    template <typename TAnon, typename ...Args>
    class MetaDataLogEntryWithArgs : public MetaDataLogEntry
    {
    public:
      using param_array_type = std::array<std::string_view, TAnon::totalParams()>;

      //-----------------------------------------------------------------------
      MetaDataLogEntryWithArgs(
        MetaDataLogEntryInfo& info,
        const param_array_type& params
        ) noexcept :
        MetaDataLogEntry(info)
      {
        static_assert(TAnon::totalParams() == sizeof...(Args));
        fillEntries<Args...>(static_cast<size_type>(0), begin(params));
        first_ = entries_.data();
        last_ = first_ + totalEntries_;
      }

      //-----------------------------------------------------------------------
      constexpr MetaDataLogEntryWithArgs(
        const constexpr_type&,
        MetaDataLogEntryInfo& info) noexcept :
        MetaDataLogEntry(constexpr_type{}, info)
      {}

    protected:
      //-----------------------------------------------------------------------
      template <typename T = void, typename ...Args>
      constexpr static size_type calculateTotalEntries() noexcept
      {
        using type = std::remove_cvref_t<T>;
        using meta_type = MetaDataType<type>;

        if constexpr (std::is_same_v<void, T>) {
          return static_cast<size_type>(0);
        }
        else if constexpr (sizeof...(Args) == 0) {
          return static_cast<size_type>(1) + meta_type::info().totalSubEntries_;
        }
        else {
          return static_cast<size_type>(1) + meta_type::info().totalSubEntries_ + calculateTotalEntries<Args...>();
        }
      }

      //-----------------------------------------------------------------------
      template <typename T = void, typename ...Args>
      constexpr void fillEntries(size_type index, typename param_array_type::const_iterator iter) noexcept
      {
        using type = std::remove_cvref_t<T>;
        using meta_type = MetaDataType<type>;

        if constexpr (!std::is_same_v<void, T>) {
          constexpr size_type totalSubEntries{ meta_type::info().totalSubEntries_ };

          entries_[index] = meta_type::info();
          entries_[index].fixTypeName<type>();
          entries_[index].paramName_ = (*iter);

          if constexpr (totalSubEntries > 0) {
            meta_type::fill(&(entries_[index + static_cast<size_type>(1)]), &(entries_[index + static_cast<size_type>(1) + totalSubEntries]));
          }

          if constexpr (sizeof...(Args) > 0) {
            ++iter;
            fillEntries<Args...>(index + totalSubEntries + static_cast<size_type>(1), iter);
          }
        }
      }

    protected:
      //-----------------------------------------------------------------------
      constexpr static size_type totalEntries_{ calculateTotalEntries<Args...>() };

      std::array<MetaDataTypeInfo, totalEntries_> entries_;
    };

    //-------------------------------------------------------------------------
    template <typename TAnon, typename ...Args>
    inline MetaDataLogEntryWithArgs<TAnon, Args...> logEntryMetaData{ TAnon::info(), TAnon::paramNames() };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    template <typename ...Args>
    class LogEntry
    {
    public:
      using size_type = zs::size_type;
      using index_type = zs::index_type;

    public:
      //-----------------------------------------------------------------------
      void operator()(const MetaDataLogEntry& entry, Args&& ...args) const noexcept
      {
        if constexpr (isFixedSize<Args...>()) {
          constexpr size_type size{ fixedSizeInBytes<Args...>() };

          if constexpr (size > 0) {
            std::array<std::byte, size> buffer;
            std::byte* pos = &(buffer[0]);
            PackerFixedSize pack{ pos, size };

            (pack << ... << args);
          }
        }
        else {
          constexpr size_type bufferMetaDataLargestAlignment{ largestAlignment<Args...>() };
          constexpr size_type bufferMetaDataLargestSize{ largestSize<Args...>() };
          constexpr size_type bufferMetaDataSizePadding{ calculatePadding(bufferMetaDataLargestAlignment + bufferMetaDataLargestSize, bufferMetaDataLargestAlignment) };
          constexpr size_type bufferMetaDataSizeWithPadding{ bufferMetaDataLargestSize + bufferMetaDataSizePadding };
          constexpr size_type maxSize{ maxLogBufferSize() };

          std::array<std::byte, bufferMetaDataLargestAlignment + (bufferMetaDataSizeWithPadding * sizeof...(Args))> bufferMetaData;
          size_type bufferPadding{ calculatePadding(reinterpret_cast<uintptr_t>(bufferMetaData.data()), bufferMetaDataLargestAlignment) };

          std::byte* start{ bufferMetaData.data() + bufferPadding };

          ctorMetaData<Args...>(start, bufferMetaDataSizeWithPadding);

          PackerFlexSizeCalculator sizer{ start, bufferMetaDataSizeWithPadding };
          (sizer << ... << args);

          std::array<std::byte, maxSize> buffer;

          PackerFlexSizePack pack{ start, bufferMetaDataSizeWithPadding, buffer.data(), maxSize };

          (pack << ... << args);

          dtorMetaData<Args...>(start, bufferMetaDataSizeWithPadding);
        }
      }

    protected:
      //-----------------------------------------------------------------------
      template<typename T = void, typename ...Args>
      constexpr static bool isFixedSize() noexcept
      {
        using type = std::remove_cvref_t<T>;
        if constexpr (std::is_same_v<type, void>)
          return true;
        else if constexpr (sizeof...(Args) == 0)
          return MetaDataType<type>::isFixedSize();
        else
          return MetaDataType<type>::isFixedSize() && isFixedSize<Args...>();
      }

      //-----------------------------------------------------------------------
      template<typename T = void, typename ...Args>
      constexpr static size_type fixedSizeInBytes() noexcept
      {
        using type = std::remove_cvref_t<T>;
        if constexpr (std::is_same_v<type, void>)
          return static_cast<size_type>(0);
        else if constexpr (sizeof...(Args) == 0)
          return MetaDataType<type>::size();
        else
          return MetaDataType<type>::size() + size<Args...>();
      }

      //-----------------------------------------------------------------------
      template<typename T = void, typename ...Args>
      constexpr static size_type largestAlignment() noexcept
      {
        using type = std::remove_cvref_t<T>;
        using meta_type = MetaDataType<type>;
        if constexpr (std::is_same_v<type, void>) {
          return static_cast<size_type>(1);
        }
        else if constexpr (sizeof...(Args) == 0) {
          return alignof(meta_type);
        }
        else {
          size_type largestOthers = largestAlignment<Args...>();
          return alignof(meta_type) > largestOthers ? alignof(meta_type) : largestOthers;
        }
      }

      //-----------------------------------------------------------------------
      template<typename T = void, typename ...Args>
      constexpr static size_type largestSize() noexcept
      {
        using type = std::remove_cvref_t<T>;
        using meta_type = MetaDataType<type>;
        if constexpr (std::is_same_v<type, void>) {
          return static_cast<size_type>(0);
        }
        else if constexpr (sizeof...(Args) == 0) {
          return sizeof(meta_type);
        }
        else {
          size_type largestOthers = largestSize<Args...>();
          return sizeof(meta_type) > largestOthers ? sizeof(meta_type) : largestOthers;
        }
      }

      //-----------------------------------------------------------------------
      template <typename T>
      constexpr static size_type calculatePadding(const T offset, const size_type align) noexcept {
        using type = zs::largest_sized_type_t<T, size_type>;
        return static_cast<size_type>((static_cast<type>(align) - (static_cast<type>(offset) % static_cast<type>(align))) % static_cast<type>(align));
      }

      //-----------------------------------------------------------------------
      struct PackerFixedSize final
      {
        std::byte* pos_;
        size_type remaining_;

        template <typename T>
        auto& operator<<(T&& value) noexcept
        {
          using type = std::remove_cvref_t<T>;
          using meta_type = MetaDataType<type>;

          meta_type{}.pack(pos_, std::forward<decltype(value)>(value), remaining_);
          return *this;
        }
      };

      //-----------------------------------------------------------------------
      struct PackerFlexSizeCalculator final
      {
        using size_type = zs::size_type;
        using index_type = zs::index_type;

        std::byte* metaDataPos_{ nullptr };
        const size_type metaDataSizeWithPadding_{};

        size_type size_{};

        constexpr static size_type maxSize_{ maxLogBufferSize() };

        template <typename T>
        auto& operator<<(T&& value) noexcept
        {
          using type = std::remove_cvref_t<T>;
          using meta_type = MetaDataType<type>;

          if constexpr (meta_type::isFixedSize()) {
            size_ += meta_type::size();
          }
          else {
            size_ += ((*reinterpret_cast<meta_type*>(metaDataPos_)).size(std::forward<decltype(value)>(value)));
          }
          metaDataPos_ += metaDataSizeWithPadding_;

          return *this;
        }
      };

      //-----------------------------------------------------------------------
      struct PackerFlexSizePack final
      {
        using size_type = zs::size_type;
        using index_type = zs::index_type;

        std::byte* metaDataPos_{ nullptr };
        const size_type metaDataSizeWithPadding_{};

        std::byte* pos_;
        size_type remaining_{};
        
        template <typename T>
        auto& operator<<(T&& value) noexcept
        {
          using type = std::remove_cvref_t<T>;
          using meta_type = MetaDataType<type>;

          if (remaining_ > 0) {
            (*reinterpret_cast<meta_type*>(metaDataPos_)).pack(pos_, std::forward<decltype(value)>(value), remaining_);
          }
          metaDataPos_ += metaDataSizeWithPadding_;
          return *this;
        }
      };

      //-----------------------------------------------------------------------
      template<typename T = void, typename ...Args>
      constexpr static void ctorMetaData(std::byte* buffer, size_type sizeWithPadding) noexcept
      {
        using type = std::remove_cvref_t<T>;
        using meta_type = MetaDataType<type>;
        if constexpr (!std::is_same_v<type, void>) {

          // in-place ctor the meta data type
          new(buffer) meta_type{};
          if constexpr (sizeof...(Args) > 0) {
            ctorMetaData<Args...>(buffer + sizeWithPadding, sizeWithPadding);
          }
        }
      }

      //-----------------------------------------------------------------------
      template<typename T = void, typename ...Args>
      constexpr static void dtorMetaData(std::byte* buffer, size_type sizeWithPadding) noexcept
      {
        using type = std::remove_cvref_t<T>;
        using meta_type = MetaDataType<type>;
        if constexpr (!std::is_same_v<type, void>) {

          // in-place dtor the meta data type
          reinterpret_cast<meta_type*>(buffer)->~meta_type();
          if constexpr (sizeof...(Args) > 0) {
            ctorMetaData<Args...>(buffer + sizeWithPadding, sizeWithPadding);
          }
        }
      }
    };

    //-------------------------------------------------------------------------
    template <typename TAnon, typename ...Args>
    void output(TAnon&& anon, Args&& ...args) noexcept
    {
      auto& metaData = logEntryMetaData<TAnon, Args...>;
      LogEntry<Args...>{}(metaData, std::forward<Args>(args)...);
    }

    //-------------------------------------------------------------------------
    inline Component component("zs::log", Level::None);

  } // namespace log

    //-------------------------------------------------------------------------
  inline log::Component component("zs");

} // namespace zs

namespace std
{
  auto begin(zs::log::Component::all_components& comp) noexcept
  {
    return comp.begin();
  }

  auto end(zs::log::Component::all_components& comp) noexcept
  {
    return comp.end();
  }

  auto cbegin(const zs::log::Component::all_components& comp) noexcept
  {
    return comp.cbegin();
  }

  auto cend(const zs::log::Component::all_components& comp) noexcept
  {
    return comp.cend();
  }


  auto begin(zs::log::MetaDataLogEntry::all_types& comp) noexcept
  {
    return comp.begin();
  }

  auto end(zs::log::MetaDataLogEntry::all_types& comp) noexcept
  {
    return comp.end();
  }

  auto cbegin(const zs::log::MetaDataLogEntry::all_types& comp) noexcept
  {
    return comp.cbegin();
  }

  auto cend(const zs::log::MetaDataLogEntry::all_types& comp) noexcept
  {
    return comp.cend();
  }

}
