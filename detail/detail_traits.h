
#pragma once

#include <cstddef>
#include <string_view>
#include <string>
#include <memory>
#include <functional>
#include <utility>

#include "../dependency/gsl.h"

namespace std
{
#ifdef _MSC_VER
#pragma warning(push)
  // see https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-2-c4099
#pragma warning(disable: 4099)
#endif //_MSC_VER

  template <typename T, std::size_t N>
  struct array;

  template <typename T, class Allocator = std::allocator<T>>
  struct vector;

  template<typename T, typename Allocator = std::allocator<T>>
  class deque;

  template<typename T, typename Container = std::deque<T>>
  class queue;

  template<typename T, typename Allocator = std::allocator<T>>
  class list;

  template<typename T, typename Allocator = std::allocator<T>>
  class forward_list;

  template<typename Key, typename Compare = std::less<Key>, typename Allocator = std::allocator<Key>>
  class set;

  template<typename Key, typename Compare = std::less<Key>, typename Allocator = std::allocator<Key>>
  class multiset;

  template<typename Key, typename T, typename Compare = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T> >>
  class map;

  template<typename Key, typename T, typename Compare = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T> >>
  class multimap;

  template<typename Key, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>, typename Allocator = std::allocator<Key>>
  class unordered_set;

  template<typename Key, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>, typename Allocator = std::allocator<Key>>
  class unordered_multiset;

  template<typename Key, typename T, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>, typename Allocator = std::allocator< std::pair<const Key, T> >>
  class unordered_map;

  template<typename Key, typename T, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>, typename Allocator = std::allocator< std::pair<const Key, T> >>
  class unordered_multimap;

  template<typename T, typename Container = std::deque<T>>
  class stack;

  template<typename T, typename Container = std::vector<T>, typename Compare = std::less<typename Container::value_type>>
  class priority_queue;

  template<typename T1, typename T2>
  struct pair;

  template<typename... Types>
  class tuple;

  template <typename T>
  struct optional;

  template <class... Types>
  class variant;

#ifdef _MSC_VER
#pragma warning(push)
#endif //_MSC_VER
} // namespace std

namespace gsl
{

  template <typename ElementType, std::ptrdiff_t Extent>
  class span;

} // namespace gsl

namespace zs
{
  //---------------------------------------------------------------------------
  namespace detail
  {
    using size_type = std::size_t;

    //-------------------------------------------------------------------------
    struct _ImpossibleType
    {
    private:
      _ImpossibleType() = delete;
      _ImpossibleType(const _ImpossibleType&) = delete;
      _ImpossibleType(_ImpossibleType&&) = delete;
      ~_ImpossibleType() = delete;
    };

    //---------------------------------------------------------------------------
    template <typename T, typename TCompareType = detail::_ImpossibleType, typename ...Args>
    constexpr decltype(auto) is_type_in_type_list() noexcept
    {
      if constexpr (std::is_same_v<T, TCompareType>)
        return true;
      else if constexpr (sizeof...(Args) > 0)
        return is_type_in_type_list<T, Args...>();
      else
        return false;
    }

    //-------------------------------------------------------------------------
    template <typename T, typename ...Args>
    struct type_in_type_list
    {
      using type = std::conditional_t<is_type_in_type_list<T, Args...>(), std::true_type, std::false_type>;
    };


    //-------------------------------------------------------------------------
    template <typename T, typename TCurrent, typename... Args>
    struct are_all_checker {};

    template<template<typename...> class TT, typename TCurrent, typename... Args1, typename... Args2>
    struct are_all_checker<TT<Args1...>, TCurrent, Args2...>
    {
      constexpr static bool is() noexcept
      {
        if constexpr (!(TT<TCurrent>::value))
          return false;
        else if constexpr (sizeof...(Args2) > 0)
          return are_all_checker<TT<Args1...>, Args2...>::is();
        else
          return true;
      }
    };

    template <typename T, typename... Args>
    struct are_all_checker_maybe_empty {};

    template<template<typename...> class TT, typename... Args1, typename... Args2>
    struct are_all_checker_maybe_empty<TT<Args1...>, Args2...>
    {
      constexpr static bool is() noexcept
      {
        if constexpr (sizeof...(Args2) > 0)
          return are_all_checker<TT<Args1...>, Args2...>::is();
        else
          return false;
      }
    };

    //-------------------------------------------------------------------------
    template <typename T, typename TCurrent, typename... Args>
    struct are_any_checker {};

    template<template<typename...> class TT, typename TCurrent, typename... Args1, typename... Args2>
    struct are_any_checker<TT<Args1...>, TCurrent, Args2...>
    {
      constexpr static bool is() noexcept
      {
        if constexpr (TT<TCurrent>::value)
          return true;
        else if constexpr (sizeof...(Args2) > 0)
          return are_any_checker<TT<Args1...>, Args2...>::is();
        else
          return false;
      }
    };

    template <typename T, typename... Args>
    struct are_any_checker_maybe_empty {};

    //-------------------------------------------------------------------------
    template<template<typename...> class TT, typename... Args1, typename... Args2>
    struct are_any_checker_maybe_empty<TT<Args1...>, Args2...>
    {
      constexpr static bool is() noexcept
      {
        if constexpr (sizeof...(Args2) > 0)
          return are_any_checker<TT<Args1...>, Args2...>::is();
        else
          return false;
      }
    };

    //-------------------------------------------------------------------------
    // from https://en.cppreference.com/w/cpp/utility/tuple/tuple_element
    template<size_type I, class... T>
    struct element_at_index;

    template<size_type I, typename Head, typename... Tail>
    struct element_at_index<I, Head, Tail...> {
      using type = typename element_at_index<I - 1, Tail...>::type;
    };

    template<class Head, typename... Tail>
    struct element_at_index<0, Head, Tail...> {
      using type = Head;
    };

    template<size_type I, typename...T>
    struct element {
      static_assert(I < sizeof...(T));
      using type = typename element_at_index<I, T...>::type;
    };

    template <typename T1, typename T2>
    struct reversable_types;

    template <template<typename...> typename T1, template<typename...> typename T2, typename... Args1, typename TCurrent, typename... Args2>
    struct reversable_types<T1<Args1...>, T2<TCurrent, Args2...>>
    {
      using type = typename reversable_types<T1<TCurrent, Args1...>, T2<Args2...>>::type;
    };

    template <template<typename...> typename T1, template<typename...> typename T2, typename... Args1>
    struct reversable_types<T1<Args1...>, T2<>>
    {
      using type = T1<Args1...>;
    };

    template <typename T1, typename T2>
    using reversable_types_t = typename reversable_types<T1, T2>::type;

    template <typename... Args>
    struct holder_type
    {
      template <typename... MoreArgs>
      struct append_changed_type;

      template <template<typename...> class TT, typename... TTArgs, typename TCurrent, typename... MoreArgs>
      struct append_changed_type<TT<TTArgs...>, TCurrent, MoreArgs...>
      {
        using type = typename holder_type<Args..., typename TT<TCurrent>::type>:: template append_changed_type<TT<TTArgs...>, MoreArgs...>::type;
      };

      template<template<typename...> class TT, typename... TTArgs>
      struct append_changed_type<TT<TTArgs...>>
      {
        using type = holder_type<Args...>;
      };

      template <typename... T>
      struct append_type
      {
        using type = holder_type<Args..., T...>;
      };

      template <>
      struct append_type<>
      {
        using type = holder_type<Args...>;
      };
    };

  } // namespace detail
} // namespace zs
