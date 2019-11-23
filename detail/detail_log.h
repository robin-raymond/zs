
#pragma once

namespace zs
{
  namespace log
  {
    //-------------------------------------------------------------------------
    template <typename T>
    struct MetaDataType<T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>> final : public MetaDataTypeCommon
    {
      using type = std::remove_cvref_t<T>;

      constexpr static auto isFixedSize() noexcept { return true; }
      constexpr static auto size() noexcept { return sizeof(type); }
      constexpr static MetaDataTypeInfo info() noexcept { return MetaDataTypeInfo::simple<type>(); }

      //-----------------------------------------------------------------------
      constexpr void pack(std::byte*& buffer, const type value, size_type &remaining) const noexcept
      {
        packData(buffer, &value, sizeof(value), remaining);
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
        result.totalElements_ = 0;
        return result;
      }

      //-----------------------------------------------------------------------
      constexpr auto size(const type value) const noexcept
      {
        return sizeCount() + (sizeof(element_type) * std::min(value.size(), maxLogStringLength()));
      }

      //-----------------------------------------------------------------------
      constexpr void pack(std::byte*& buffer, const type value, size_type& remaining) const noexcept
      {
        size_type count{ std::min(value.size(), maxLogStringLength()) };
        size_type size = (sizeof(element_type) * count);
        packData(buffer, value.data(), size, remaining);
      }
    };

    //-------------------------------------------------------------------------
    template <typename T>
    struct MetaDataType<T, std::enable_if_t<is_basic_string_v<T>>> final : public MetaDataTypeVariable
    {
      using type = std::remove_cvref_t<T>;
      using element_type = std::remove_cvref_t<typename type::value_type>;

      //-----------------------------------------------------------------------
      constexpr static MetaDataTypeInfo info() noexcept
      {
        auto result{ MetaDataTypeInfo::simple<element_type>() };
        result.totalElements_ = 0;
        return result;
      }

      //-----------------------------------------------------------------------
      template <typename U>
      constexpr auto size(U &&value) const noexcept
      {
        return sizeCount() + (sizeof(element_type) * std::min(value.size(), maxLogStringLength()));
      }

      //-----------------------------------------------------------------------
      template <typename U>
      constexpr void pack(std::byte*& buffer, U &&value, size_type& remaining) const noexcept
      {
        size_type count{ std::min(value.size(), maxLogStringLength()) };
        size_type size = (sizeof(element_type) * count);
        packData(buffer, value.c_str(), size, remaining);
      }
    };

    //-------------------------------------------------------------------------
    template <typename T, zs::size_type N>
    struct MetaDataType<const T [N], std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>>  final : public MetaDataTypeVariable
    {
      using type = const T[N];
      using element_type = T;

      constexpr static auto isFixedSize() noexcept { return true; }

      //-----------------------------------------------------------------------
      constexpr static MetaDataTypeInfo info() noexcept
      {
        auto result{ MetaDataTypeInfo::simple<element_type>() };
        result.totalElements_ = N;
        return result;
      }

      //-----------------------------------------------------------------------
      constexpr static auto size() noexcept
      {
        return (sizeof(element_type) * N);
      }

      //-----------------------------------------------------------------------
      template <typename T>
      void pack(std::byte*& buffer, T &&value, size_type& remaining) const noexcept
      {
        if constexpr(0 != (sizeof(element_type) % alignof(element_type))) {
          for (size_type i{}; i < N; ++i) {
            packData(buffer, &(value[i]), sizeof(element_type), remaining);
          }
        }
        else {
          constexpr size_type length = size();
          packData(buffer, &(value[0]), length, remaining);
        }
      }
    };

    //-------------------------------------------------------------------------
    template <typename T, zs::size_type N>
    struct MetaDataType<const T[N], std::enable_if_t<!(std::is_integral_v<T> || std::is_floating_point_v<T>)>>  final : public MetaDataTypeCommon
    {
      using type = const T[N];
      using value_type = std::remove_cvref_t<T>;
      using sub_meta_type = MetaDataType<value_type>;

      constexpr static auto isFixedSize() noexcept { return sub_meta_type::isFixedSize(); }

      std::array<sub_meta_type, N> subMetaTypes_;

      //-----------------------------------------------------------------------
      constexpr static MetaDataTypeInfo info() noexcept
      {
        auto result{ MetaDataTypeInfo::simple<value_type>() };

        result.totalElements_ = N;
        result.elementWidth_ = 0;
        result.totalSubEntries_ = static_cast<decltype(result.totalSubEntries_)>(1) + sub_meta_type::info().totalSubEntries_;
        return result;
      }

      //-----------------------------------------------------------------------
      constexpr static auto size() noexcept
      {
        constexpr size_type result{ calculateFixedSize<sub_meta_type>() * N };
        return result;
      }

      //-----------------------------------------------------------------------
      constexpr auto size(T&& values) const noexcept
      {
        static_assert(std::is_array_v<std::remove_cvref_t<T>>);
        size_type result{};
        size_type index{};
        for (const auto& value : values) {
          if (index >= N)
            break;
          result += calculateDynamicSize(subMetaTypes_[index].size, std::forward<decltype(value)>(value));
          ++index;
        }
        return result;
      }

      //-----------------------------------------------------------------------
      template <typename T>
      void pack(std::byte*& buffer, T&& values, size_type& remaining) const noexcept
      {
        static_assert(std::is_array_v<std::remove_cvref_t<T>>);

        size_type index{};
        for (const auto& value : values) {
          if (remaining < 1)
            break;
          if (index >= N)
            break;
          subMetaTypes_[index].pack(buffer, std::forward<decltype(value)>(value), remaining);
          ++index;
        }
      }

      //-----------------------------------------------------------------------
      static void fill(MetaDataTypeInfo* first, MetaDataTypeInfo* last) noexcept
      {
        fillInfo<sub_meta_type, value_type>(first, last, "array"sv);
      }
    };

    //-------------------------------------------------------------------------
    template <typename T>
    struct MetaDataType<T, std::enable_if_t<
      is_std_unique_ptr_v<T> ||
      is_std_shared_ptr_v<T> ||
      is_zs_move_only_shared_ptr_v<T>||
      is_std_optional_v<T> ||
      std::is_pointer_v<T>
      >> final : public MetaDataTypeVariable
    {
      template <typename U, typename Enabled = void>
      struct ElementType
      {
      };

      template <typename U>
      struct ElementType<U, std::enable_if_t<is_std_optional_v<U>>>
      {
        using sub_meta_type = MetaDataType<std::remove_cvref_t<typename U::value_type>>;
        using value_type = typename U::value_type;
        using dereference_type = std::add_lvalue_reference_t<value_type>;
      };

      template <typename U>
      struct ElementType<U, std::enable_if_t<!is_std_optional_v<U>>>
      {
        using sub_meta_type = MetaDataType<std::remove_cvref_t<typename U::element_type>>;
        using value_type = typename U::element_type;
        using dereference_type = std::add_lvalue_reference_t<value_type>;
      };

      template <typename U>
      struct ElementType<U, std::enable_if_t<std::is_pointer_v<U>>>
      {
        using sub_meta_type = MetaDataType<std::remove_cvref_t<std::remove_pointer_t<U>>>;
        using value_type = std::remove_pointer_t<U>;
        using dereference_type = std::add_lvalue_reference_t<value_type>;
      };

      using type = std::remove_cvref_t<T>;
      using sub_meta_type = typename ElementType<T>::sub_meta_type;
      using value_type = typename ElementType<T>::value_type;
      using dereference_type = typename ElementType<T>::dereference_type;

      constexpr static auto isFixedSize() noexcept { return false; }

      sub_meta_type subMetaType_;

      //-----------------------------------------------------------------------
      constexpr static MetaDataTypeInfo info() noexcept
      {
        auto result{ MetaDataTypeInfo::simple<type>() };
        result.totalElements_ = 0;  // total array elements might be 0 or 1
        result.elementWidth_ = 0;
        result.totalSubEntries_ = static_cast<decltype(result.totalSubEntries_)>(1) + sub_meta_type::info().totalSubEntries_;
        return result;
      }

      //-----------------------------------------------------------------------
      template <typename U>
      constexpr size_type size(U&& value) const noexcept
      {
        size_type result{ sizeCount() + calculateFixedSize<sub_meta_type>() };
        if constexpr (!sub_meta_type::isFixedSize()) {
          if (value) {
            result += subMetaType_.size(std::forward<dereference_type>(*value));
          }
        }

        return result;
      }

      //-----------------------------------------------------------------------
      template <typename U, std::enable_if_t<!is_std_unique_ptr_v<U>, U> * = nullptr>
      void pack(std::byte*& buffer, U&& value, size_type& remaining) const noexcept
      {
        size_type count{ value ? 1 : 0 };
        packCount(buffer, count, remaining);
        if (count > 0)
          subMetaType_.pack(std::forward<dereference_type>(*value));
      }

      //-----------------------------------------------------------------------
      static void fill(MetaDataTypeInfo* first, MetaDataTypeInfo* last) noexcept
      {
        fillInfo<sub_meta_type, value_type>(first, last, "ptr"sv);
      }
    };

    //-------------------------------------------------------------------------
    template <typename T>
    struct MetaDataType<T, std::enable_if_t<
      is_gsl_span_v<T> ||
      is_std_array_v<T> ||
      is_std_vector_v<T> ||
      is_std_deque_v<T> ||
      is_std_list_v<T> ||
      is_std_forward_list_v<T> ||
      is_std_set_v<T> ||
      is_std_multiset_v<T> ||
      is_std_unordered_set_v<T> ||
      is_std_unordered_multiset_v<T>
      >> final : public MetaDataTypeVariable
    {
      template <typename U, typename Enabled = void>
      struct ElementType
      {
      };

      template <typename U>
      struct ElementType<U, std::enable_if_t<is_gsl_span_v<U>>>
      {
        using value_type = typename U::value_type;
        using sub_meta_type = MetaDataType<std::remove_cvref_t<typename U::value_type>>;

        constexpr static bool isFixedSize() noexcept { return false; }
        constexpr static size_type totalElements() noexcept { return 0; }
      };

      template <typename U>
      struct ElementType<U, std::enable_if_t<is_std_array_v<U>>>
      {
        template <typename W, typename Size = void>
        struct ArraySizer
        {};

        template <typename W, size_type VTotalElements>
        struct ArraySizer<std::array<W, VTotalElements>, void>
        {
          constexpr static size_type size() noexcept { return VTotalElements; }
        };

        using value_type = typename U::value_type;
        using sub_meta_type = MetaDataType<std::remove_cvref_t<typename U::value_type>>;

        constexpr static bool isFixedSize() noexcept { return sub_meta_type::isFixedSize(); }
        constexpr static size_type totalElements() noexcept { return ArraySizer<U>::size(); }
      };

      template <typename U>
      struct ElementType<U, std::enable_if_t< is_std_vector_v<T> || is_std_deque_v<T> || is_std_list_v<T> || is_std_forward_list_v<T> || is_std_set_v<T> || is_std_multiset_v<T> >>
      {
        using value_type = typename U::value_type;
        using sub_meta_type = MetaDataType<std::remove_cvref_t<typename U::value_type>>;

        constexpr static bool isFixedSize() noexcept { return false; }
        constexpr static size_type totalElements() noexcept { return 0; }
      };

      using type = std::remove_cvref_t<T>;
      using value_type = typename ElementType<T>::value_type;
      using sub_meta_type = typename ElementType<T>::sub_meta_type;

      constexpr static size_type totalElements() noexcept { return ElementType<T>::totalElements(); }
      constexpr static bool isFixedSize() noexcept { return (0 != totalElements()) && sub_meta_type::isFixedSize(); }
      constexpr static size_type maxElements() noexcept { return 0 == totalElements() ? maxLogArrayEntries() : totalElements(); }

      std::array<sub_meta_type, maxElements()> subMetaTypes_;

      //-----------------------------------------------------------------------
      constexpr static MetaDataTypeInfo info() noexcept
      {
        auto result{ MetaDataTypeInfo::simple<type>() };
        result.totalElements_ = totalElements();
        result.elementWidth_ = 0;
        result.totalSubEntries_ = static_cast<decltype(result.totalSubEntries_)>(1) + sub_meta_type::info().totalSubEntries_;
        return result;
      }

      //-----------------------------------------------------------------------
      constexpr static size_type size() noexcept
      {
        constexpr size_type size{ totalElements() * calculateFixedSize<sub_meta_type>() };
        return size;
      }

      //-----------------------------------------------------------------------
      template <typename U>
      constexpr size_type size(U&& values) const noexcept
      {
        constexpr size_type countSizeIfDynamicArrayLength{ 0 == totalElements() ? sizeCount() : 0};
        size_type result{ countSizeIfDynamicArrayLength };

        if constexpr (!sub_meta_type::isFixedSize()) {
          size_type index{};
          for (const auto& value : values) {
            if (index >= subMetaTypes_.size())
              break;

            result += subMetaTypes_[index].size(std::forward<decltype(value)>(value));
          }
        }
        else {
          result += (std::min(values.size(), maxElements()) * sub_meta_type::size());
        }
        return result;
      }

      //-----------------------------------------------------------------------
      template <typename U, std::enable_if_t<!is_std_unique_ptr_v<U>, U> * = nullptr>
      void pack(std::byte*& buffer, U&& values, size_type &remaining) const noexcept
      {
        if constexpr (0 == totalElements()) {
          size_type count{ std::min(maxElements(), values.size()) };
          packCount(buffer, count, remaining);
        }

        size_type index{};
        for (auto& value : values) {
          if (remaining < 1)
            break;
          if (index >= subMetaTypes_.size())
            break;
          subMetaTypes_[index].pack(buffer, std::forward<decltype(value)>(value), remaining);
        }
      }

      //-----------------------------------------------------------------------
      static void fill(MetaDataTypeInfo* first, MetaDataTypeInfo* last) noexcept
      {
        fillInfo<sub_meta_type, value_type>(first, last, "value"sv);
      }
    };

    //-------------------------------------------------------------------------
    template <typename T>
    struct MetaDataType<T, std::enable_if_t<
      is_std_map_v<T> ||
      is_std_multimap_v<T> ||
      is_std_unordered_map_v<T> ||
      is_std_unordered_multimap_v<T>
      >> final : public MetaDataTypeVariable
    {
      using type = std::remove_cvref_t<T>;
      using sub_meta_key_type = typename MetaDataType<std::remove_cvref_t<typename T::key_type>>;
      using sub_meta_value_type = typename MetaDataType<std::remove_cvref_t<typename T::value_type>>;

      constexpr static bool isFixedSize() noexcept { return false; }
      constexpr static bool isKeyValueFixedSize() noexcept { return sub_meta_key_type::isFixedSize() && sub_meta_value_type::isFixedSize(); }
      constexpr static size_type maxElements() noexcept { return maxLogArrayEntries(); }

      std::array<sub_meta_key_type, maxElements()> subMetaKeyTypes_;
      std::array<sub_meta_value_type, maxElements()> subMetaValueTypes_;

      //-----------------------------------------------------------------------
      constexpr static MetaDataTypeInfo info() noexcept
      {
        auto result{ MetaDataTypeInfo::simple<type>() };
        result.totalElements_ = 0;
        result.elementWidth_ = 0;
        result.totalSubEntries_ = static_cast<decltype(result.totalSubEntries_)>(2) + sub_meta_key_type::info().totalSubEntries_ + sub_meta_value_type::info().totalSubEntries_;
        return result;
      }

      //-----------------------------------------------------------------------
      template <typename U>
      constexpr auto size(U&& values) const noexcept
      {
        constexpr size_type keyValueFixedSize{ calculateFixedSize<sub_meta_key_type>() + calculateFixedSize<sub_meta_value_type>() };
        size_type result{ sizeCount() };

        if constexpr (!isKeyValueFixedSize()) {
          size_type index{};
          for (const auto& [key, value] : values) {
            if (index >= subMetaKeyTypes_.size())
              break;

            result += keyValueFixedSize;
            if constexpr (!sub_meta_key_type::isFixedSize())
              result += calculateDynamicSize(subMetaKeyTypes_[index], std::forward<decltype(key)>(key));
            if constexpr (!sub_meta_value_type::isFixedSize())
              result += calculateDynamicSize(subMetaKeyTypes_[index], std::forward<decltype(value)>(value));
          }
        }
        else {
          result += (keyValueFixedSize * std::min(values.size(), subMetaKeyTypes_.size()));
        }
        return result;
      }

      //-----------------------------------------------------------------------
      template <typename U, std::enable_if_t<!is_std_unique_ptr_v<U>, U> * = nullptr>
      void pack(std::byte*& buffer, U&& values, size_type& remaining) const noexcept
      {
        packCount(buffer, std::min(values.size(), subMetaKeyTypes_.size()));

        size_type index{};
        for (auto& [key, value] : values) {
          if (remaining < 1)
            break;
          if (index >= subMetaKeyTypes_.size())
            break;
          subMetaKeyTypes_[index].pack(buffer, std::forward<decltype(key)>(key), remaining);
          subMetaValueTypes_[index].pack(buffer, std::forward<decltype(value)>(value), remaining);
        }
      }
      //-----------------------------------------------------------------------
      static void fill(MetaDataTypeInfo* first, MetaDataTypeInfo* last) noexcept
      {
        fillInfo<sub_meta_key_type, typename T::key_type>(first, last, "key"sv);
        fillInfo<sub_meta_value_type, typename T::value_type>(first, last, "value"sv);
      }
    };

    //-------------------------------------------------------------------------
    template <typename T>
    struct MetaDataType<T, std::enable_if_t< is_std_pair_v<T> >> final : public MetaDataTypeVariable
    {
      using type = std::remove_cvref_t<T>;
      using sub_meta_first_type = typename MetaDataType<std::remove_cvref_t<typename T::first_type>>;
      using sub_meta_second_type = typename MetaDataType<std::remove_cvref_t<typename T::second_type>>;

      constexpr static bool isFixedSize() noexcept { return sub_meta_first_type::isFixedSize() && sub_meta_second_type::isFixedSize(); }

      sub_meta_first_type subMetaFirstType_;
      sub_meta_second_type subMetaSecontType_;

      //-----------------------------------------------------------------------
      constexpr static MetaDataTypeInfo info() noexcept
      {
        auto result{ MetaDataTypeInfo::simple<type>() };
        result.totalElements_ = 1;
        result.elementWidth_ = 0;
        result.totalSubEntries_ = static_cast<decltype(result.totalSubEntries_)>(2) + sub_meta_first_type::info().totalSubEntries_ + sub_meta_second_type::info().totalSubEntries_;
        return result;
      }

      //-----------------------------------------------------------------------
      constexpr static size_type size() noexcept
      {
        constexpr size_type result{ calculateFixedSize<sub_meta_first_type>() + calculateFixedSize<sub_meta_second_type>() };
        return result;
      }

      //-----------------------------------------------------------------------
      template <typename U>
      constexpr size_type size(U&& value) const noexcept
      {
        size_type result{ size() };
        if constexpr (!isFixedSize()) {
          if constexpr (!sub_meta_first_type::isFixedSize())
            result += calculateDynamicSize(subMetaFirstType_, value.first);
          if constexpr (!sub_meta_second_type::isFixedSize())
            result += calculateDynamicSize(subMetaSecontType_, value.second);
        }
        return result;
      }

      //-----------------------------------------------------------------------
      template <typename U>
      void pack(std::byte*& buffer, U&& value, size_type& remaining) const noexcept
      {
        subMetaFirstType_.pack(buffer, std::forward<decltype(value.first)>(value.first), remaining);
        subMetaSecontType_.pack(buffer, std::forward<decltype(value.second)>(value.second), remaining);
      }

      //-----------------------------------------------------------------------
      static void fill(MetaDataTypeInfo* first, MetaDataTypeInfo* last) noexcept
      {
        fillInfo<sub_meta_first_type, typename T::first_type>(first, last, "first"sv);
        fillInfo<sub_meta_second_type, typename T::second_type>(first, last, "second"sv);
      }
    };

    //-------------------------------------------------------------------------
    template <>
    struct MetaDataType<const char * const, void>  final : public MetaDataTypeVariable
    {
      using type = const char * const;
      using element_type = char;

      size_type count_;

      constexpr static size_type maxStringLength() noexcept { return  maxLogStringLength(); }

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
          return sizeCount();
        count_ = std::min(maxStringLength(), strlen(value));
        return sizeCount() + (count_ * sizeof(element_type));
      }

      //-----------------------------------------------------------------------
      void pack(type value, std::byte*& buffer, size_type &remaining) const noexcept
      {
        size_type length{ count_ * sizeof(element_type) };
        packCount(buffer, count_, remaining);
        packData(buffer, value, length, remaining);
      }
    };

    //-------------------------------------------------------------------------
    template <>
    struct MetaDataType<const wchar_t* const, void>  final : public MetaDataTypeVariable
    {
      using type = const wchar_t* const;
      using element_type = char;

      size_type count_;

      constexpr static size_type maxStringLength() noexcept { return  maxLogStringLength(); }

      //-----------------------------------------------------------------------
      constexpr auto size(type value) noexcept
      {
        if (!value) {
          count_ = 0;
          return sizeCount();
        }
        count_ = std::min(maxStringLength(), wcslen(value));
        return sizeCount() + (count_ * sizeof(element_type));
      }

      //-----------------------------------------------------------------------
      void pack(std::byte*& buffer, type value, size_type& remaining) const noexcept
      {
        size_type length{ count_ * sizeof(element_type) };
        packCount(buffer, count_, remaining);
        packData(buffer, value, length, remaining);
      }
    };
    
  } // namespace log

} // namespace zs
