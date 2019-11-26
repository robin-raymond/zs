
#pragma once

#include <array>
#include <string_view>
#include <utility>
#include <type_traits>
#include <algorithm>
#include <optional>
#include <cstddef>

#include "traits.h"

namespace zs
{
  enum class EnumOrder
  {
    Default,
    Name,
    Value,
  };

  template <typename TEnumType, zs::size_type TotalEntries>
  struct EnumDeclare
  {
    using UnderlyingType = typename std::remove_const<typename std::underlying_type<TEnumType>::type>::type;
    using EnumType = typename std::remove_const<TEnumType>::type;
    using EntryType = std::pair<EnumType, std::string_view>;
    using Entries = std::array<EntryType, TotalEntries>;
  };

  template <typename TEnumType, typename TEnumDeclare, EnumOrder VOrder = EnumOrder::Default>
  class EnumTraits final
  {
    static_assert(std::is_enum_v<TEnumType>, "class only designed for simple enum types");

  private:
    inline static constexpr auto entries_ = TEnumDeclare{}();

  public:
    using size_type = zs::size_type;
    using index_type = zs::index_type;

    using Total = std::integral_constant<size_type, entries_.size()>;

    using FirstIndex = std::integral_constant<index_type, 0>;
    using LastIndex = std::integral_constant<index_type, static_cast<index_type>(Total::value)>;
    using Order = std::integral_constant<EnumOrder, VOrder>;

  private:
    // When the std::sort algorithm is marked constexpr, this logic should be
    // should be removed in favor of the standard version.
    // See quicksort algorithm: https://en.wikipedia.org/wiki/Quicksort
    struct Sorter
    {
      template <typename TElementType>
      constexpr static void doSwap(TElementType& l, TElementType& r)
      {
        TElementType temp{ l };
        l.first = r.first;
        r.first = temp.first;

        l.second = r.second;
        r.second = temp.second;
      }

      template <typename TArray, typename TSizeType, typename TLess>
      constexpr static auto partition(TArray& a, TSizeType lo, TSizeType hi) noexcept
      {
        auto pivot = a[hi];
        auto i = lo;
        for (auto j = lo; j <= hi; ++j) {
          if (TLess{}(a[j], pivot)) {
            doSwap(a[i], a[j]);
            ++i;
          }
        }
        doSwap(a[i], a[hi]);
        return i;
      }

      template <typename TArray, typename TSizeType, typename TLess>
      constexpr static void quickSort(TArray& a, TSizeType lo, TSizeType hi) noexcept
      {
        if (lo < hi) {
          auto p = partition<TArray, TSizeType, TLess>(a, lo, hi);
          quickSort<TArray, TSizeType, TLess>(a, lo, p - 1);
          quickSort<TArray, TSizeType, TLess>(a, p + 1, hi);
        }
      }

      template <typename TElementType>
      struct isLessFirst
      {
        constexpr bool operator()(TElementType& l, TElementType& r)
        {
          return static_cast<std::underlying_type_t<decltype(l.first )>>(l.first) < static_cast<std::underlying_type_t<decltype(r.first)>>(r.first);
        }
      };

      template <typename TElementType>
      struct isLessSecond
      {
        constexpr bool operator()(TElementType& l, TElementType& r)
        {
          return l.second < r.second;
        }
      };

      constexpr static auto sortedByEnumValue() noexcept
      {
        auto result = entries_;
        quickSort<decltype(result), int, isLessFirst<decltype(result[0])>>(result, 0, static_cast<int>(LastIndex::value) - 1);
        return result;
      }
      constexpr static auto sortedByName() noexcept
      {
        auto result = entries_;
        quickSort<decltype(result), int, isLessSecond<decltype(result[0])>>(result, 0, static_cast<int>(LastIndex::value) - 1);
        return result;
      }
    };

    inline static constexpr auto sortedEntriesByEnumValue_ = Sorter::sortedByEnumValue();
    inline static constexpr auto sortedEntriesByName_ = Sorter::sortedByName();

    // When std::lower_bound is marked as constexpr, this function should be
    // removed in favor of the standard.
    // http://www.cplusplus.com/reference/algorithm/lower_bound/
    template <typename ForwardIterator, typename T, typename TLess>
    constexpr static ForwardIterator quick_lower_bound(ForwardIterator first, ForwardIterator last, const T& val, TLess)
    {
      ForwardIterator it{};
      typename std::iterator_traits<ForwardIterator>::difference_type count{}, step{};
      count = std::distance(first, last);
      while (count > 0)
      {
        it = first;
        step = count / 2;
        advance(it, step);
        if (TLess{}(*it, val)) {                 // or: if (comp(*it,val)), for version (2)
          first = ++it;
          count -= step + 1;
        }
        else {
          count = step;
        }
      }
      return first;
    }

  public:
    template <typename TIteratorEnumType, index_type VDirection = static_cast<index_type>(1)>
    class Iterator;

    using Type = EnumTraits<TEnumType, TEnumDeclare, VOrder>;
    using UnderlyingType = typename std::remove_const<typename std::underlying_type<TEnumType>::type>::type;
    using EnumType = typename std::remove_const<TEnumType>::type;

    using iterator = Iterator<index_type, static_cast<index_type>(1)>;
    using const_iterator = Iterator<const index_type, static_cast<index_type>(1)>;

    using reverse_iterator = Iterator<index_type, static_cast<index_type>(-1)>;
    using reverse_const_iterator = Iterator<const index_type, static_cast<index_type>(-1)>;

    template <typename TIteratorEnumType, index_type VDirection>
    class Iterator
    {
      friend class iterator;
      friend class const_iterator;
      friend class EnumTraits;

    public:
      using index_type = zs::index_type;

      using index_iterator = Iterator<index_type, VDirection>;
      using const_index_iterator = Iterator<const index_type, VDirection>;

      constexpr Iterator() = default;

      template <typename T = const_index_iterator, typename std::enable_if_t<std::is_const_v<TIteratorEnumType>, T> * = nullptr>
      constexpr Iterator(const const_index_iterator& value) noexcept : value_{ value.value_ } {}
      constexpr Iterator(const index_iterator& value) noexcept : value_{ value.value_ } {}

      template <typename T = const_index_iterator, typename std::enable_if_t<std::is_const_v<TIteratorEnumType>, T> * = nullptr>
      constexpr Iterator(const_index_iterator&& value) noexcept : value_{ value.value_ } {}
      constexpr Iterator(index_iterator&& value) noexcept : value_{ value.value_ } {}

      constexpr Iterator(const index_type& value) noexcept : value_{ value } {}
      constexpr Iterator(index_type&& value) noexcept : value_{ value } {}

      template <typename T = const_index_iterator, typename std::enable_if_t<std::is_const_v<TIteratorEnumType>, T> * = nullptr>
      constexpr auto& operator=(const const_index_iterator& value) noexcept { value_ = value.value_; return *this; };
      constexpr auto& operator=(index_iterator& value) noexcept { value_ = value.value_; return *this; };

      template <typename T = const_index_iterator, typename std::enable_if_t<std::is_const_v<TIteratorEnumType>, T> * = nullptr>
      constexpr auto& operator=(const_index_iterator&& value) noexcept { value_ = value.value_; return *this; };
      constexpr auto& operator=(index_iterator&& value) noexcept { value_ = value.value_; return *this; };

      [[nodiscard]] constexpr decltype(auto) operator*() const noexcept {
        if constexpr (EnumOrder::Default == Order::value) {
          return entries_[value_];
        }
        else if constexpr (EnumOrder::Value == Order::value) {
          return sortedEntriesByEnumValue_[value_];
        }
        else {
          return sortedEntriesByName_[value_];
        }
      }
      [[nodiscard]] constexpr decltype(auto) operator->() const noexcept { return &(*(*this)); }

      constexpr auto& operator++() noexcept {
        value_ += VDirection;
        return *this;
      }
      constexpr auto& operator--() noexcept {
        value_ -= VDirection;
        return *this;
      }

      [[nodiscard]] constexpr decltype(auto) operator[](index_type distance) const noexcept {
        auto temp{ *this };
        temp += distance;
        return *temp;
      }

      constexpr auto operator++(int) noexcept { auto temp{ *this }; ++(*this); return temp; }
      constexpr auto operator--(int) noexcept { auto temp{ *this }; --(*this); return temp; }

      [[nodiscard]]  constexpr decltype(auto) operator+(index_type distance) const noexcept {
        auto temp{ *this };
        temp.value_ += (distance * VDirection);
        return temp;
      }
      [[nodiscard]] constexpr decltype(auto) operator-(index_type distance) const noexcept {
        auto temp{ *this };
        temp.value_ -= (distance * VDirection);
        return temp;
      }

      [[nodiscard]] friend constexpr decltype(auto) operator+(index_type distance, const Iterator& value) noexcept {
        auto temp{ value };
        return temp + distance;
      }
      [[nodiscard]] friend constexpr decltype(auto) operator-(index_type distance, const Iterator& value) noexcept {
        auto temp{ value };
        return temp + (static_cast<decltype(distance)>(-1) * distance);
      }

      [[nodiscard]] constexpr decltype(auto) operator+=(index_type distance) const noexcept {
        value_ += (distance * VDirection);
        return *this;
      }
      [[nodiscard]] constexpr decltype(auto) operator-=(index_type distance) const noexcept {
        value_ -= (distance * VDirection);
        return *this;
      }

      [[nodiscard]] constexpr auto operator==(const const_index_iterator& value) const noexcept { return value_ == value.value_; }
      [[nodiscard]] constexpr auto operator!=(const const_index_iterator& value) const noexcept { return value_ != value.value_; }

      [[nodiscard]] constexpr auto operator<(const const_index_iterator& value) const noexcept {
        if constexpr (VDirection > 0)
          return static_cast<index_type>(value_) < static_cast<index_type>(value.value_);
        else
          return static_cast<index_type>(value_) >= static_cast<index_type>(value.value_);
      }
      [[nodiscard]] constexpr auto operator>(const const_index_iterator& value) const noexcept {
        if constexpr (VDirection > 0)
          return static_cast<index_type>(value_) > static_cast<index_type>(value.value_);
        else
          return static_cast<index_type>(value_) <= static_cast<index_type>(value.value_);
      }

      [[nodiscard]] constexpr auto operator<=(const const_index_iterator& value) const noexcept {
        if constexpr (VDirection > 0)
          return static_cast<index_type>(value_) <= static_cast<index_type>(value.value_);
        else
          return static_cast<index_type>(value_) > static_cast<index_type>(value.value_);
      }
      [[nodiscard]] constexpr auto operator>=(const const_index_iterator& value) const noexcept {
        if constexpr (VDirection > 0)
          return static_cast<index_type>(value_) >= static_cast<index_type>(value.value_);
        else
          return static_cast<index_type>(value_) < static_cast<index_type>(value.value_);
      }

    private:
      index_type value_{ FirstIndex::value };
    };

    constexpr EnumTraits() = default;
    constexpr EnumTraits(const EnumTraits&) noexcept = default;
    constexpr EnumTraits(EnumTraits&&) noexcept = default;
    constexpr EnumTraits& operator=(const EnumTraits&) noexcept = default;
    constexpr EnumTraits& operator=(EnumTraits&&) noexcept = default;

    [[nodiscard]] constexpr static auto firstIndex() noexcept { return FirstIndex::value; }
    [[nodiscard]] constexpr static auto lastIndex() noexcept { return LastIndex::value - 1; }

    [[nodiscard]] constexpr static auto first() noexcept { return entries_[firstIndex()].first; }
    [[nodiscard]] constexpr static auto last() noexcept { return entries_[lastIndex()].first; }

    [[nodiscard]] constexpr auto begin() noexcept { return iterator{ firstIndex() }; }
    [[nodiscard]] constexpr auto begin() const noexcept { return const_iterator{ firstIndex() }; }

    [[nodiscard]] constexpr auto end() noexcept { iterator temp{ lastIndex() }; ++temp; return temp; }
    [[nodiscard]] constexpr auto end() const noexcept { const_iterator temp{ lastIndex() }; ++temp; return temp; }

    [[nodiscard]] constexpr auto rbegin() noexcept { return reverse_iterator{ lastIndex() }; }
    [[nodiscard]] constexpr auto rbegin() const noexcept { return reverse_const_iterator{ lastIndex() }; }

    [[nodiscard]] constexpr auto rend() noexcept { reverse_iterator temp{ firstIndex() }; ++temp; return temp; }
    [[nodiscard]] constexpr auto rend() const noexcept { reverse_const_iterator temp{ firstIndex() }; ++temp; return temp; }

    [[nodiscard]] constexpr static auto total() noexcept { return Total::value; }
    [[nodiscard]] constexpr auto size() const noexcept { return Total::value; }

    [[nodiscard]] constexpr static std::string_view toString(TEnumType value) noexcept
    {
      auto found = quick_lower_bound(cbegin(sortedEntriesByEnumValue_), cend(sortedEntriesByEnumValue_), value, [](const auto& l, const auto& r) noexcept -> bool {
        return l.first < r;
        });
      if (cend(sortedEntriesByEnumValue_) == found)
        return {};
      return found->second;
    }

    [[nodiscard]] constexpr static std::optional<TEnumType> toEnum(std::string_view value) noexcept
    {
      auto found = quick_lower_bound(cbegin(sortedEntriesByName_), cend(sortedEntriesByName_), value, [](const auto& l, const auto& r) noexcept -> bool {
        return l.second < r;
        });
      if (cend(sortedEntriesByName_) == found)
        return {};
      if (found->second != value)
        return {};
      return found->first;
    }
  };

} // namespace zs

namespace std
{
  template <typename TEnumType, typename TEnumDeclare>
  [[nodiscard]] constexpr auto begin(zs::EnumTraits<TEnumType, TEnumDeclare> e)
  {
    return typename zs::EnumTraits<TEnumType, TEnumDeclare>::iterator(e.begin());
  }
  template <typename TEnumType, typename TEnumDeclare>
  [[nodiscard]] constexpr auto end(zs::EnumTraits<TEnumType, TEnumDeclare> e)
  {
    return typename zs::EnumTraits<TEnumType, TEnumDeclare>::iterator(e.end());
  }
  template <typename TEnumType, typename TEnumDeclare>
  [[nodiscard]] constexpr auto cbegin(zs::EnumTraits<TEnumType, TEnumDeclare> e)
  {
    return typename zs::EnumTraits<TEnumType, TEnumDeclare>::const_iterator(e.begin());
  }
  template <typename TEnumType, typename TEnumDeclare>
  [[nodiscard]] constexpr auto cend(zs::EnumTraits<TEnumType, TEnumDeclare> e)
  {
    return typename zs::EnumTraits<TEnumType, TEnumDeclare>::const_iterator(e.end());
  }

  template <typename TEnumType, typename TEnumDeclare>
  [[nodiscard]] constexpr auto rbegin(zs::EnumTraits<TEnumType, TEnumDeclare> e)
  {
    return typename zs::EnumTraits<TEnumType, TEnumDeclare>::reverse_iterator(e.rbegin());
  }
  template <typename TEnumType, typename TEnumDeclare>
  [[nodiscard]] constexpr auto rend(zs::EnumTraits<TEnumType, TEnumDeclare> e)
  {
    return typename zs::EnumTraits<TEnumType, TEnumDeclare>::reverse_iterator(e.rend());
  }
  template <typename TEnumType, typename TEnumDeclare>
  [[nodiscard]] constexpr auto crbegin(zs::EnumTraits<TEnumType, TEnumDeclare> e)
  {
    return typename zs::EnumTraits<TEnumType, TEnumDeclare>::reverse_const_iterator(e.rbegin());
  }
  template <typename TEnumType, typename TEnumDeclare>
  [[nodiscard]] constexpr auto crend(zs::EnumTraits<TEnumType, TEnumDeclare> e)
  {
    return typename zs::EnumTraits<TEnumType, TEnumDeclare>::reverse_const_iterator(e.rend());
  }

} // namespace zs
