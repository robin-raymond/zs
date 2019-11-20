
#pragma once

#include "enum.h"
#include "traits.h"

#include <string_view>
#include <cstring>
#include <cwchar>

namespace zs
{
  namespace log
  {
    inline constexpr std::integral_constant<std::size_t, static_cast<std::size_t>(10 * 1024)> maxLogBufferSize;

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

      using id_type = std::size_t;
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
        const constexpr_type &,
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
        constexpr Iterator(const const_forward_iterator& value) noexcept : value_{ const_cast<Component *>(value.value_) } {}
        constexpr Iterator(const forward_iterator& value) noexcept : value_{ value.value_ } {}

        template <typename T = const_forward_iterator, typename std::enable_if_t<std::is_const_v<TComponent>, T> * = nullptr>
        constexpr Iterator(const_forward_iterator&& value) noexcept : value_{ value.value_ } {}
        constexpr Iterator(forward_iterator&& value) noexcept : value_{ value.value_ } {}

        constexpr Iterator(const Component* value) noexcept : value_{ const_cast<Component *>(value) } {}
        constexpr Iterator(TComponent&& value) noexcept : value_{ value } {}

        template <typename T = const_forward_iterator, typename std::enable_if_t<std::is_const_v<TComponent>, T> * = nullptr>
        constexpr auto& operator=(const const_forward_iterator& value) noexcept { value_ = value.value_; return *this; };
        constexpr auto& operator=(forward_iterator& value) noexcept { value_ = value.value_; return *this; };

        template <typename T = const_forward_iterator, typename std::enable_if_t<std::is_const_v<TComponent>, T> * = nullptr>
        constexpr auto& operator=(const_forward_iterator&& value) noexcept { value_ = value.value_; return *this; };
        constexpr auto& operator=(forward_iterator&& value) noexcept { value_ = value.value_; return *this; };

        template <typename T = const_forward_iterator, typename std::enable_if_t<std::is_const_v<TComponent>, T> * = nullptr>
        [[nodiscard]] constexpr const auto &operator*() const noexcept { return *value_; }
        template <typename T = forward_iterator, typename std::enable_if_t<!std::is_const_v<TComponent>, T> * = nullptr>
        [[nodiscard]] constexpr auto& operator*() noexcept { return *value_; }

        constexpr auto& operator++() noexcept {
          value_ = value_->next_;
          return *this;
        }

        [[nodiscard]] constexpr auto operator==(const Iterator& value) const noexcept { return value_ == value.value_; }
        [[nodiscard]] constexpr auto operator!=(const Iterator& value) const noexcept { return value_ != value.value_; }

      private:
        std::remove_const_t<TComponent>* value_{nullptr};
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
      using size_type = std::size_t;
      using array_count_size_type = std::uint32_t;

      std::string_view typeName_{};
      std::string_view paramName_{};
      bool isIntegral_{};
      bool isSigned_{};
      bool isFloatingPoint_{};
      size_type elementWidth_{};
      size_type totalElements_{};
      size_type totalSubEntries_{};

      constexpr bool isVariableArraySize() const noexcept { return 0 == totalElements_; }

      constexpr MetaDataTypeInfo() noexcept = default;
      constexpr MetaDataTypeInfo(const MetaDataTypeInfo &) noexcept = default;
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
        result.totalElements_= 1;
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
      using size_type = MetaDataTypeInfo::size_type;
      using array_count_size_type = MetaDataTypeInfo::array_count_size_type;
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

    //-------------------------------------------------------------------------
    template <typename T>
    struct MetaDataType<T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>> final : public MetaDataTypeCommon
    {
      using type = std::remove_cvref_t<T>;

      constexpr static auto isFixedSize() noexcept { return true; }
      constexpr static auto size() noexcept { return sizeof(type); }
      constexpr static MetaDataTypeInfo info() noexcept { return MetaDataTypeInfo::simple<type>(); }

      //-----------------------------------------------------------------------
      constexpr void pack(const type value, std::byte*& buffer) const noexcept
      {
        memcpy(buffer, &value, size());
        buffer += size();
      }
    };

    //-------------------------------------------------------------------------
    template <typename T>
    struct MetaDataType<T, std::enable_if_t<is_basic_string_view_v<T>>> final : public MetaDataTypeVariable
    {
      using type = std::remove_cvref_t<T>;
      using element_type = std::remove_cvref_t<typename type::value_type>;

      //-----------------------------------------------------------------------
      constexpr static MetaDataTypeInfo info() noexcept
      {
        auto result{ MetaDataTypeInfo::simple<element_type>() };
        result.totalElements_ = {};
        return result;
      }

      //-----------------------------------------------------------------------
      constexpr auto size(const type value) const noexcept
      {
        return sizeof(array_count_size_type) + (sizeof(type::value_type) * value.size());
      }
      //-----------------------------------------------------------------------
      constexpr void pack(const type value, std::byte*& buffer) const noexcept
      {
        array_count_size_type count = static_cast<array_count_size_type>(value.size());
        size_type length = (sizeof(type::value_type) * value.size());
        memcpy(buffer, &count, sizeof(count));
        buffer += sizeof(count);
        memcpy(buffer, value.data(), length);
        buffer += length;
      }
    };

    //-------------------------------------------------------------------------
    template <typename T>
    struct MetaDataType<T, std::enable_if_t<is_basic_string_v<T>>> final : public MetaDataTypeVariable
    {
      using type = std::remove_cvref_t<T>;
      using element_type = std::remove_cvref_t<typename type::value_type>;

      constexpr static MetaDataTypeInfo info() noexcept
      {
        auto result{ MetaDataTypeInfo::simple<element_type>() };
        result.totalElements_ = 0;
        return result;
      }
      constexpr auto size(const type &value) const noexcept
      {
        return sizeof(array_count_size_type) + (sizeof(type::value_type) * value.size());
      }
      constexpr void pack(const type &value, std::byte*& buffer) const noexcept
      {
        array_count_size_type count = static_cast<array_count_size_type>(value.size());
        size_t length = (sizeof(type::value_type) * value.size());
        memcpy(buffer, &count, sizeof(count));
        buffer += sizeof(count);
        memcpy(buffer, value.c_str(), length);
        buffer += length;
      }
    };

    //-------------------------------------------------------------------------
    template <typename T, size_t N>
    struct MetaDataType<const T [N], std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>>  final : public MetaDataTypeVariable
    {
      using type = const T[N];
      using element_type = T;

      constexpr static auto isFixedSize() noexcept { return true; }

      //-----------------------------------------------------------------------
      constexpr static MetaDataTypeInfo info() noexcept
      {
        auto result{ MetaDataTypeInfo::simple<element_type>() };
        result.totalElements_ = static_cast<decltype(result.totalElements_)>(N);
        return result;
      }
      //-----------------------------------------------------------------------
      constexpr static auto size() noexcept
      {
        return (sizeof(element_type) * static_cast<size_type>(N));
      }
      //-----------------------------------------------------------------------
      void pack(type &value, std::byte*& buffer) const noexcept
      {
        if constexpr(0 != (sizeof(element_type) % alignof(element_type))) {
          for (size_type i{}; i < N; ++i) {
            memcpy(buffer, &(value[i]), sizeof(element_type));
            buffer += sizeof(element_type);
          }
        }
        else {
          constexpr size_type length = size();
          memcpy(buffer, &(value[0]), length);
          buffer += length;
        }
      }
    };

    //-------------------------------------------------------------------------
    template <>
    struct MetaDataType<const char * const, void>  final : public MetaDataTypeVariable
    {
      using type = const char * const;
      using element_type = char;

      array_count_size_type count_{};

      //-----------------------------------------------------------------------
      constexpr static MetaDataTypeInfo info() noexcept
      {
        auto result{ MetaDataTypeInfo::simple<element_type>() };
        result.totalElements_ = {};
        return result;
      }

      //-----------------------------------------------------------------------
      constexpr auto size(type value) noexcept
      {
        if (!value)
          return sizeof(array_count_size_type);
        count_ = static_cast<array_count_size_type>(strlen(value));
        return sizeof(array_count_size_type) + (static_cast<size_type>(count_)*sizeof(element_type));
      }
      //-----------------------------------------------------------------------
      void pack(type value, std::byte*& buffer) const noexcept
      {
        size_type length = (static_cast<size_type>(count_) * sizeof(element_type));
        memcpy(buffer, &count_, sizeof(count_));
        buffer += sizeof(count_);
        memcpy(buffer, value, length);
        buffer += length;
      }
    };

    //-------------------------------------------------------------------------
    template <>
    struct MetaDataType<const wchar_t* const, void>  final : public MetaDataTypeVariable
    {
      using type = const wchar_t* const;
      using element_type = char;

      array_count_size_type count_{};

      //-----------------------------------------------------------------------
      constexpr auto size(type value) noexcept
      {
        if (!value)
          return sizeof(array_count_size_type);
        count_ = static_cast<array_count_size_type>(wcslen(value));
        return sizeof(array_count_size_type) + (static_cast<size_type>(count_) * sizeof(element_type));
      }
      //-----------------------------------------------------------------------
      void pack(type value, std::byte*& buffer) const noexcept
      {
        size_type length = (static_cast<size_type>(count_) * sizeof(element_type));
        memcpy(buffer, &count_, sizeof(count_));
        buffer += sizeof(count_);
        memcpy(buffer, value, length);
        buffer += length;
      }
    };

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

      using size_type = std::size_t;
      using id_type = std::size_t;
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
      //-----------------------------------------------------------------------
      MetaDataLogEntryWithArgs(MetaDataLogEntryInfo& info) noexcept :
        MetaDataLogEntry(info)
      {
        fillEntries<Args...>(static_cast<size_type>(0));
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
      constexpr void fillEntries(size_type index) noexcept
      {
        using type = std::remove_cvref_t<T>;
        using meta_type = MetaDataType<type>;

        if constexpr (std::is_same_v<void, T>) {
          return;
        }
        else {
          entries_[index] = meta_type::info();
          entries_[index].fixTypeName<type>();

          if constexpr (sizeof...(Args) > 0) {
            fillEntries<Args...>(index + 1);
          }
        }
      }

    protected:
      //-----------------------------------------------------------------------
      constexpr static size_type totalEntries_{ calculateTotalEntries<int, int>() };

      std::array<MetaDataTypeInfo, totalEntries_> entries_;
    };

    //-------------------------------------------------------------------------
    template <typename TAnon, typename ...Args>
    inline MetaDataLogEntryWithArgs<TAnon, Args...> logEntryMetaData{ TAnon::info() };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    template <typename ...Args>
    class LogEntry
    {
    public:
      using size_type = std::size_t;

    public:
      //-----------------------------------------------------------------------
      void operator()(const MetaDataLogEntry& entry, Args&& ...args) const noexcept
      {
        if constexpr (isFixedSize<Args...>()) {
          constexpr size_type size{ fixedSizeInBytes<Args...>() };

          if constexpr (size > 0) {
            std::array<std::byte, size> buffer;
            std::byte* pos = &(buffer[0]);
            PackerFixedSize pack{ pos };

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

          PackerFlexSizePack pack{ start, bufferMetaDataSizeWithPadding, nullptr, sizer.totalEntriesSafelyPacked_ };

          auto done{ [&]() {
          } };

          if (sizer.totalSize_ < 1024) {
            std::array<std::byte, 1024> buffer;
            pack.pos_ = buffer.data();
            (pack << ... << args);
            done();
          }
          else {
            std::array<std::byte, maxSize> buffer;
            pack.pos_ = buffer.data();
            (pack << ... << args);
            done();
          }

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

        template <typename T>
        auto& operator<<(T&& value) noexcept
        {
          using type = std::remove_cvref_t<T>;
          using meta_type = MetaDataType<type>;

          meta_type{}.pack(std::forward<decltype(value)>(value), pos_);
          return *this;
        }
      };

      //-----------------------------------------------------------------------
      struct PackerFlexSizeCalculator final
      {
        using size_type = std::size_t;

        std::byte* metaDataPos_{ nullptr };
        const size_type metaDataSizeWithPadding_{};

        size_type totalSize_{};
        size_type totalEntriesSafelyPacked_{};

        constexpr static size_type maxSize_{ maxLogBufferSize() };

        template <typename T>
        auto& operator<<(T&& value) noexcept
        {
          using type = std::remove_cvref_t<T>;
          using meta_type = MetaDataType<type>;

          if constexpr (meta_type::isFixedSize()) {
            totalSize_ += meta_type::size();
          }
          else {
            totalSize_ += ((*reinterpret_cast<meta_type*>(metaDataPos_)).size(std::forward<decltype(value)>(value)));
          }
          metaDataPos_ += metaDataSizeWithPadding_;

          if (totalSize_ <= maxSize_)
            ++totalEntriesSafelyPacked_;

          return *this;
        }
      };

      //-----------------------------------------------------------------------
      struct PackerFlexSizePack final
      {
        using size_type = std::size_t;

        std::byte* metaDataPos_{ nullptr };
        const size_type metaDataSizeWithPadding_{};

        std::byte* pos_;
        size_type totalEntriesSafelyPacked_{};
        
        template <typename T>
        auto& operator<<(T&& value) noexcept
        {
          using type = std::remove_cvref_t<T>;
          using meta_type = MetaDataType<type>;

          if (totalEntriesSafelyPacked_) {
            ((*reinterpret_cast<meta_type*>(metaDataPos_)).pack(std::forward<decltype(value)>(value), pos_));
            --totalEntriesSafelyPacked_;
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
      LogEntry<Args...>{}(logEntryMetaData<TAnon, Args...>, std::forward<Args>(args)...);
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
