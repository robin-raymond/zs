
#pragma once

#include <string_view>
#include <string>

namespace zs
{

  template <typename T, class Traits = std::char_traits<T>>
  struct is_basic_string_view : std::false_type {};

  template <typename T, typename Traits>
  struct is_basic_string_view<std::basic_string_view<T, Traits>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_basic_string_view_v = is_basic_string_view<T>::value;

  template <typename T>
  struct is_deduced_basic_string_view : is_basic_string_view<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_basic_string_view_v = is_deduced_basic_string_view<T>::value;


  template <typename T, typename Traits = std::char_traits<T>, typename Alloc = std::allocator<T>>
  struct is_basic_string : std::false_type {};

  template <typename T, typename Traits, typename Alloc>
  struct is_basic_string<std::basic_string<T, Traits, Alloc>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_basic_string_v = is_basic_string<T>::value;

  template <typename T>
  struct is_deduced_basic_string : is_basic_string<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_basic_string_v = is_deduced_basic_string<T>::value;

  template <typename TFirst, typename TSecond>
  struct largest_sized_type
  {
    using size_type = std::size_t;
    using type_first = std::remove_cvref_t<TFirst>;
    using type_second = std::remove_cvref_t<TSecond>;

    using type = std::conditional_t<sizeof(type_first) >= sizeof(type_second), type_first, type_second>;
  };

  template <typename TFirst, typename TSecond>
  using largest_sized_type_t = typename largest_sized_type<TFirst, TSecond>::type;


}
