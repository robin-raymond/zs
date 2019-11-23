
#pragma once

#include "traits.h"

#include <tuple>

namespace zs
{
  template <typename ...Args>
  struct ReflectedType
  {
    using size_type = zs::size_type;
    using tuple_type = std::tuple<Args...>;

#if 0
    template <typename...>
    struct ByIndex
    {
    };

    template <typename T, typename U...>
    struct ByIndex<T, T, U...> : std::integral_constant<size_type, 0>
    {
      typename type = T;
    };

    template <typename T, typename U, typename ...Args>
    struct ByIndex<T, U, Args...> : std::integral_constant<size_type, 1 + ByIndex<T, Args...>::value>
    {
      typename type = T;
    };

    template <typename T, typename ...Args>
    struct StructType
    {
      using member_type = remove_deduced_member_pointer<T>::member_type;
      using struct_type = remove_deduced_member_pointer<T>::struct_type;
    };
#endif //0

    using TFirst = typename std::tuple_element<0, tuple_type>::type;

    tuple_type values_;

    //-------------------------------------------------------------------------
    constexpr ReflectedType(Args ...args) noexcept :
      values_{ std::forward<Args>(args)... }
    {
      static_assert(Assertion<Args...>::check());
    }

    constexpr static size_type total() noexcept { return count_(); }
    constexpr size_type size() const noexcept { return count_(); }

  protected:

    template <typename T = void, typename ...Args>
    struct Assertion
    {
      constexpr static bool check() noexcept
      {
        using recurse_type = Assertion<Args...>;

        if constexpr (!std::is_same_v<void, T>) {
          return (zs::is_member_function_pointer_v<std::remove_cv_t<T>>) &&
                 ((std::is_same_v<zs::remove_deduced_member_pointer<T>::struct_type, zs::remove_deduced_member_pointer<TFirst>::struct_type>) ||
                  (std::is_base_of_v<zs::remove_deduced_member_pointer<T>::struct_type, zs::remove_deduced_member_pointer<TFirst>::struct_type>) ||
                  (std::is_base_of_v<zs::remove_deduced_member_pointer<TFirst>::struct_type, zs::remove_deduced_member_pointer<T>::struct_type>)) &&
                 (recurse_type::check());
        }
        else {
          return true;
        }
      }
    };

    inline constexpr static std::integral_constant<size_type, sizeof...(Args)> count_{};
  };

  template <typename ...Args>
  constexpr ReflectedType<Args...> make_reflection(Args ...args)
  {
    return ReflectedType<Args...>{ std::forward<Args>(args)... };
  }

} // namespace zs
