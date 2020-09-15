#pragma once

#include "traits.h"
#include <assert.h>

namespace zs
{

// std::list is very slow for random access but had some significant features,
// e.g. iterator stability when removing unrelated elements. For tokenizers,
// the ability to splice out contents and keep stability is important and they
// might require limited look ahead capabilities; the random access iterator
// creates a wrapper around the list's iterator to treat a non-random access
// list iterator as a random access iterator.

struct RandomAccessListTypes
{
  template <typename TList, bool TSafe>
  struct Iterator;

  template <typename TList, bool TSafe>
  using random_access_list_iterator = Iterator<std::remove_cvref_t<TList>, TSafe>;

  template <typename TList, bool TSafe>
  using const_random_access_list_iterator = Iterator<std::add_const_t<std::remove_cvref_t<TList>>, TSafe>;

  template <typename TList, bool TSafe>
  struct Iterator
  {
    friend struct Iterator;

    using IsSafe = std::integral_constant<bool, TSafe>;

    using List = std::remove_reference_t<std::remove_volatile_t<TList>>;
    using MutableList = std::remove_const_t<List>;
    using ImmutableList = std::add_const_t<List>;
    using UseList = List;

    using list_iterator = typename MutableList::iterator;
    using const_list_iterator = typename ImmutableList::const_iterator;
    using use_list_iterator = std::conditional_t<std::is_const_v<List>, const_list_iterator, list_iterator>;

    using iterator = Iterator<MutableList, TSafe>;
    using const_iterator = Iterator<ImmutableList, TSafe>;
    using use_iterator = Iterator<List, TSafe>;

    using index_type = zs::index_type;
    using size_type = zs::size_type;

    Iterator() noexcept = default;
    Iterator(const Iterator&) noexcept = default;
    Iterator(Iterator&&) noexcept = default;

    template <typename T = ImmutableList, typename std::enable_if_t<std::is_const_v<UseList>, T>* = nullptr>
    Iterator(const random_access_list_iterator<TList, TSafe> & rhs) noexcept : list_(rhs.list_), iterator_(rhs.iterator_) {}

    template <typename T = ImmutableList, typename std::enable_if_t<std::is_const_v<UseList>, T>* = nullptr>
    Iterator(const ImmutableList& rhs) noexcept : list_{ &rhs }, iterator_{ rhs.begin() } {}
    template <typename T = MutableList, typename std::enable_if_t<!std::is_const_v<UseList>, T>* = nullptr>
    Iterator(MutableList& rhs) noexcept : list_{ &rhs }, iterator_{ rhs.begin() } {}

    template <typename T = ImmutableList, typename std::enable_if_t<std::is_const_v<UseList>, T>* = nullptr>
    Iterator(const ImmutableList& rhs, typename ImmutableList::const_iterator iter) noexcept : list_{ &rhs }, iterator_{ iter } {}
    template <typename T = MutableList, typename std::enable_if_t<!std::is_const_v<UseList>, T>* = nullptr>
    Iterator(MutableList& rhs, typename MutableList::iterator iter) noexcept : list_{ &rhs }, iterator_{ iter } {}

    Iterator& operator=(const Iterator&) noexcept = default;
    Iterator& operator=(Iterator&&) noexcept = default;

    template <typename T = ImmutableList, typename std::enable_if_t<std::is_const_v<UseList>, T>* = nullptr>
    auto& operator=(const ImmutableList& value) noexcept { list_ = &value; iterator_ = value.begin(); return *this; }

    template <typename T = MutableList, typename std::enable_if_t<!std::is_const_v<UseList>, T>* = nullptr>
    auto& operator=(MutableList& value) noexcept { list_ = &value; iterator_ = value.begin(); return *this; }

    auto& operator++() noexcept
    {
      if constexpr (IsSafe::value) {
        if (list_) {
          if (iterator_ != list_->end())
            ++iterator_;
        }
      }
      else {
        ++iterator_;
      }
      return *this;
    }

    auto& operator--() noexcept
    {
      if constexpr (IsSafe::value) {
        if (list_) {
          if (iterator_ != list_->begin())
            --iterator_;
        }
      }
      else
        --iterator_;
      return *this;
    }

    [[nodiscard]] use_list_iterator underlying() noexcept
    {
      return iterator_;
    }

    [[nodiscard]] auto& list() noexcept
    {
      return *list_;
    }

    [[nodiscard]] bool empty() noexcept
    {
      if constexpr (IsSafe::value) {
        return list_ ? list_->empty() : true;
      }
      else
        return list_->empty();
    }

    [[nodiscard]] bool has_value() noexcept
    {
      if constexpr (IsSafe::value) {
        return list_ ? list_->end() != iterator_ : false;
      }
      else
        return list_->end() == iterator_;
    }

    [[nodiscard]] bool valid() noexcept
    {
      return list_;
    }

    [[nodiscard]] explicit operator bool() noexcept
    {
      return has_value();
    }

    [[nodiscard]] bool isBegin() noexcept
    {
      if constexpr (IsSafe::value) {
        return list_ ? list_->begin() == iterator_ : true;
      }
      else
        return list_->begin() == iterator_;
    }

    [[nodiscard]] bool isEnd() noexcept
    {
      if constexpr (IsSafe::value) {
        return list_ ? list_->end() == iterator_ : true;
      }
      else
        return list_->end() == iterator_;
    }

    [[nodiscard]] auto size() noexcept
    {
      if constexpr (IsSafe::value) {
        return list_ ? list_->size() : static_cast<typename std::remove_pointer_t<decltype(list_)>::size_type>(0);
      }
      else
        return list_->size();
    }

    Iterator& operator+=(index_type distance) noexcept
    {
      if (distance < 0)
        return *this -= (distance * static_cast<index_type>(-1));

      if constexpr (IsSafe::value) {
        if (list_) {
          while ((iterator_ != list_->end()) && (distance > 0)) {
            ++iterator_;
            --distance;
          }
        }
      }
      else {
        while (distance > 0) {
          ++iterator_;
          --distance;
        }
      }
      return *this;
    }

    Iterator& operator-=(index_type distance) noexcept
    {
      if (distance < 0)
        return *this += (distance * static_cast<index_type>(-1));

      if constexpr (IsSafe::value) {
        if (list_) {
          while ((iterator_ != list_->begin()) && (distance > 0)) {
            --iterator_;
            --distance;
          }
        }
      }
      else {
        while (distance > 0) {
          --iterator_;
          --distance;
        }
      }
      return *this;
    }

    [[nodiscard]] decltype(auto) operator*() const noexcept {
      if constexpr (IsSafe::value) {
        assert(list_);
        assert(list_->size() > 0);
        if (iterator_ == list_->end()) {
          auto temp{ *this };
          --temp;
          return *(temp.iterator_);
        }
      }
      return *iterator_;
    }
    [[nodiscard]] decltype(auto) operator->() const noexcept { return &(*(*this)); }

    [[nodiscard]] decltype(auto) operator[](index_type distance) const noexcept {
      auto temp{ *this };
      temp += distance;
      return *temp;
    }

    auto operator++(int) noexcept { auto temp{ *this }; ++(*this); return temp; }
    auto operator--(int) noexcept { auto temp{ *this }; --(*this); return temp; }

    [[nodiscard]] constexpr decltype(auto) operator+(index_type distance) const noexcept { auto temp{ *this }; temp += distance; return temp; }
    [[nodiscard]] constexpr decltype(auto) operator-(index_type distance) const noexcept { auto temp{ *this }; temp -= distance; return temp; }

    [[nodiscard]] friend constexpr decltype(auto) operator+(index_type distance, const Iterator& rhs) noexcept { auto temp{ rhs }; return temp + distance; }
    [[nodiscard]] friend constexpr decltype(auto) operator-(index_type distance, const Iterator& rhs) noexcept { auto temp{ rhs }; return temp + (static_cast<index_type>(-1) * distance); }

    [[nodiscard]] constexpr auto operator==(const random_access_list_iterator<TList, TSafe>& rhs) const noexcept { return (list_ == rhs.list_) && (iterator_ == rhs.iterator_); }
    [[nodiscard]] constexpr auto operator!=(const random_access_list_iterator<TList, TSafe>& rhs) const noexcept { return !((*this) == rhs); }

    [[nodiscard]] constexpr auto operator==(const const_random_access_list_iterator<TList, TSafe>& rhs) const noexcept { return (list_ == rhs.list_) && (iterator_ == rhs.iterator_); }
    [[nodiscard]] constexpr auto operator!=(const const_random_access_list_iterator<TList, TSafe>& rhs) const noexcept { return !((*this) == rhs); }

  protected:
      UseList* list_{};
      use_list_iterator iterator_;
  };
};

template <typename TList, bool TSafe = true>
using RandomAccessListIterator = RandomAccessListTypes::Iterator<TList, TSafe>;

template <typename TList, bool TSafe = true>
using random_access_list_iterator = RandomAccessListTypes::random_access_list_iterator<TList, TSafe>;

template <typename TList, bool TSafe = true>
using const_random_access_list_iterator = RandomAccessListTypes::const_random_access_list_iterator<TList, TSafe>;

template <typename TList, bool TSafe = true>
RandomAccessListTypes::Iterator<TList, TSafe> makeRandom(TList& rhs) { return RandomAccessListTypes::Iterator<TList, TSafe>{ rhs }; }

template <typename TList, bool TSafe = true>
RandomAccessListTypes::Iterator<TList, TSafe> makeRandom(TList& rhs, typename TList::iterator iter) { return RandomAccessListTypes::Iterator<TList, TSafe>{ rhs, iter }; }

template <typename TList, bool TSafe = true>
RandomAccessListTypes::Iterator<TList, TSafe> makeRandom(TList& rhs, typename TList::const_iterator iter) { return RandomAccessListTypes::Iterator<TList, TSafe>{ rhs, iter }; }

template <typename TList, bool TSafe = true>
RandomAccessListTypes::Iterator<TList, TSafe> makeRandomSafe(TList& rhs) { return RandomAccessListTypes::Iterator<TList, TSafe>{ rhs }; }

template <typename TList, bool TSafe = true>
RandomAccessListTypes::Iterator<TList, TSafe> makeRandomSafe(TList& rhs, typename TList::iterator iter) { return RandomAccessListTypes::Iterator<TList, TSafe>{ rhs, iter }; }

template <typename TList, bool TSafe = true>
RandomAccessListTypes::Iterator<TList, TSafe> makeRandomSafe(TList& rhs, typename TList::const_iterator iter) { return RandomAccessListTypes::Iterator<TList, TSafe>{ rhs, iter }; }


template <typename TList, bool TSafe = false>
RandomAccessListTypes::Iterator<TList, TSafe> makeRandomUnsafe(TList& rhs) { return RandomAccessListTypes::Iterator<TList, TSafe>{ rhs }; }

template <typename TList, bool TSafe = false>
RandomAccessListTypes::Iterator<TList, TSafe> makeRandomUnsafe(TList& rhs, typename TList::iterator iter) { return RandomAccessListTypes::Iterator<TList, TSafe>{ rhs, iter }; }

template <typename TList, bool TSafe = false>
RandomAccessListTypes::Iterator<TList, TSafe> makeRandomUnsafe(TList& rhs, typename TList::const_iterator iter) { return RandomAccessListTypes::Iterator<TList, TSafe>{ rhs, iter }; }

} // namespace zs
