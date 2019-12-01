
#pragma once

#include "detail/detail_traits.h"

namespace zs
{
  using index_type = gsl::index;
  using size_type = detail::size_type;

  using namespace std::literals::string_view_literals;

  template <typename T>
  class move_only_shared_ptr;

  template <typename ...Args>
  struct ReflectType;

  template <typename TStruct, typename ...Args>
  struct Reflect;

  template <typename TReflect>
  struct ReflectVisitor;

  template <typename TTuple>
  struct TupleReflectType;

  template <typename TTuple>
  struct TupleReflect;

  template <typename TTupleReflect>
  struct TupleReflectVisitor;

  //---------------------------------------------------------------------------
  template <typename T>
  struct remove_member_pointer
  {
    using type = T;
    using member_type = T;
    using struct_type = T;
  };

  template <typename T, typename U>
  struct remove_member_pointer<T U::*>
  {
    using type = T;
    using member_type = T;
    using struct_type = U;
  };

  template< class T >
  using remove_member_pointer_t = typename remove_member_pointer<T>::type;

  template< class T >
  using remove_deduced_member_pointer = remove_member_pointer<std::remove_cvref_t<T>>;

  template< class T >
  using remove_deduced_member_pointer_t = typename remove_deduced_member_pointer<T>::type;


  //---------------------------------------------------------------------------
  template <typename T>
  struct is_member_function_pointer : public std::false_type {};

  template <typename T, typename U>
  struct is_member_function_pointer<T U::*> : public std::true_type {};

  template <class T>
  inline constexpr bool is_member_function_pointer_v = is_member_function_pointer<T>::value;

  template <typename T>
  struct is_deduced_member_function_pointer : is_member_function_pointer<std::remove_cvref_t<T>> {};

  template <class T>
  inline constexpr bool is_deduced_member_function_pointer_v = is_deduced_member_function_pointer<T>::value;

  //---------------------------------------------------------------------------
  template <typename T, typename ...Args>
  struct is_type_in_type_list : public detail::type_in_type_list<T, Args...>::type {};

  template <typename T, typename ...Args>
  inline constexpr bool is_type_in_type_list_v = is_type_in_type_list<T, Args...>::value;

  //---------------------------------------------------------------------------
  template<typename T, typename... Args>  // generic template
  struct are_all;

  template<template<typename...> class TT, typename... Args1, typename... Args2>
  struct are_all<TT<Args1...>, Args2...> : public std::conditional_t<detail::are_all_checker_maybe_empty<TT<Args1...>, Args2...>::is(), std::true_type, std::false_type> {};

  template <typename TT, typename... Args>
  inline constexpr bool are_all_v = are_all<TT, Args...>::value;

  //---------------------------------------------------------------------------
  template<typename T, typename... Args>  // generic template
  struct are_any;

  template<template<typename...> class TT, typename... Args1, typename... Args2>
  struct are_any<TT<Args1...>, Args2...> : public std::conditional_t<detail::are_any_checker_maybe_empty<TT<Args1...>, Args2...>::is(), std::true_type, std::false_type> {};

  template <typename TT, typename... Args>
  inline constexpr bool are_any_v = are_any<TT, Args...>::value;


  //---------------------------------------------------------------------------
  template<typename T1, typename T2>  // generic template
  struct rebind_from_template;

  template<template<typename...> typename TT1, template<typename...> typename TT2, typename... Args1, typename... Args2>
  struct rebind_from_template<TT1<Args1...>, TT2<Args2...>>
  {
    using type = TT1<Args2...>;
  };

  template<typename T1, typename T2>
  using rebind_from_template_t = typename rebind_from_template< T1, T2 >::type;

  //---------------------------------------------------------------------------
  template <typename... Args>
  using count_types = std::integral_constant<size_type, sizeof...(Args)>;
  
  template <typename... Args>
  using is_type_list_empty = std::integral_constant<bool, count_types<Args...> == 0>;

  template <typename... Args>
  inline constexpr bool is_type_list_empty_v = is_type_list_empty<Args...>::value;

  template <typename... Args>
  inline constexpr size_type count_types_v = count_types<Args...>::value;

  //---------------------------------------------------------------------------
  template<typename T1>  // generic template
  struct count_template_types : public std::integral_constant<size_type, 0> {};

  template<template<typename...> typename TT, typename... Args>
  struct count_template_types<TT<Args...>> : public std::integral_constant<size_type, sizeof...(Args)> {};

  template <typename T>
  inline constexpr size_type count_template_types_v = count_template_types<T>::value;

  //---------------------------------------------------------------------------
  template <typename TCurrent, typename... Args>
  struct first_type
  {
    using type = TCurrent;
  };

  template <typename... Args>
  using first_type_t = typename first_type<Args...>::type;


  //---------------------------------------------------------------------------
  template <typename TCurrent, typename... Args>
  struct last_type
  {
    using type = typename last_type<Args...>::type;
  };

  template <typename TCurrent>
  struct last_type<TCurrent>
  {
    using type = TCurrent;
  };

  template <typename... Args>
  using last_type_t = typename last_type<Args...>::type;

  //---------------------------------------------------------------------------
  template <size_type I, typename... T>
  using element = detail::element<I, T...>;

  template <size_type I, typename... T>
  using element_t = typename element<I, T...>::type;


  //---------------------------------------------------------------------------
  template<size_type I, typename T>
  struct template_element
  {
    static_assert(I < 0, "type specified is not a templated type");
    using type = T;
  };

  template<size_type I, template<typename...> typename TT, typename... Args>
  struct template_element<I, TT<Args...>>
  {
    static_assert(I < sizeof...(Args));
    using type = typename detail::element<I, Args...>::type;
  };


  //---------------------------------------------------------------------------
  template <typename ...Args>
  struct TypeList
  {
    inline constexpr static std::integral_constant<size_type, sizeof...(Args)> total{};
    constexpr size_type size() const noexcept { return total(); }

    using type = TypeList<Args...>;

    template<typename T1>  // generic template
    struct rebind;

    template<template<typename...> class TT1, typename... Args1>
    struct rebind<TT1<Args1...>>
    {
      using type = typename rebind_from_template<TT1<Args1...>, TypeList<Args...>>::type;
    };

    template <typename T>
    using rebind_t = typename rebind<T>::type;


    template<typename T>  // generic template
    struct rebind_from
    {
      using type = typename TypeList<T>::type;
    };

    template<template<typename...> class TT1, typename... Args1>
    struct rebind_from<TT1<Args1...>>
    {
      using type = typename rebind_from_template<TypeList<Args...>, TT1<Args1...>>::type;
    };

    template <typename T>
    using rebind_from_t = typename rebind_from<T>::type;


    template<typename T>  // generic template
    struct append_from
    {
      using type = typename TypeList<T>::type;
    };

    template<template<typename...> class TT1, typename... Args1>
    struct append_from<TT1<Args1...>>
    {
      using type = typename TypeList<Args..., Args1...>;
    };

    template <typename T>
    using append_from_t = typename append_from<T>::type;


    template<typename T>  // generic template
    struct prepend_from
    {
      using type = typename TypeList<T>::type;
    };

    template<template<typename...> class TT1, typename... Args1>
    struct prepend_from<TT1<Args1...>>
    {
      using type = typename TypeList<Args1..., Args...>;
    };

    template <typename T>
    using prepend_from_t = typename prepend_from<T>::type;


    template <typename... T>
    struct append_type
    {
      using type = TypeList<Args..., T...>;
    };

    template <>
    struct append_type<>
    {
      using type = TypeList<Args...>;
    };

    template <typename... T>
    struct prepend_type
    {
      using type = TypeList<T..., Args...>;
    };

    template <>
    struct prepend_type<>
    {
      using type = TypeList<Args...>;
    };

    template <typename... T>
    using append_type_t = typename append_type<T...>::type;

    template <typename... T>
    using prepend_type_t = typename prepend_type<T...>::type;

    template <typename... MoreArgs>
    struct append_type_if_unique;

    // inspiration from https://stackoverflow.com/questions/13827319/eliminate-duplicate-entries-from-c11-variadic-template-arguments
    template <typename T, typename... MoreArgs>
    struct append_type_if_unique<T, MoreArgs...>
    {
      using type = std::conditional_t<
        is_type_in_type_list_v<T, Args...>,
        typename TypeList<Args...>:: template append_type_if_unique<MoreArgs...>::type,
        typename TypeList<Args..., T>:: template append_type_if_unique<MoreArgs...>::type
        >;
    };

    template<>
    struct append_type_if_unique<>
    {
      using type = TypeList<Args...>;
    };


    template <typename... MoreArgs>
    struct prepend_type_if_unique;

    template <typename T, typename... MoreArgs>
    struct prepend_type_if_unique<T, MoreArgs...>
    {
      using type = std::conditional_t<
        is_type_in_type_list_v<T, Args...>,
        typename TypeList<Args...>:: template prepend_type_if_unique<MoreArgs...>::type,
        typename TypeList<T, Args...>:: template prepend_type_if_unique<MoreArgs...>::type
      >;
    };

    template<>
    struct prepend_type_if_unique<>
    {
      using type = TypeList<Args...>;
    };

    template <typename... MoreArgs>
    using append_type_if_unique_t = typename append_type_if_unique<MoreArgs...>::type;

    template <typename... MoreArgs>
    using prepend_type_if_unique_t = typename prepend_type_if_unique<MoreArgs...>::type;


    template <typename... MoreArgs>
    struct append_changed_type;

    template <template<typename...> class TT, typename... TTArgs, typename TCurrent, typename... MoreArgs>
    struct append_changed_type<TT<TTArgs...>, TCurrent, MoreArgs...>
    {
      using type = typename TypeList<Args..., typename TT<TCurrent>::type>:: template append_changed_type<TT<TTArgs...>, MoreArgs...>::type;
    };

    template<template<typename...> class TT, typename... TTArgs>
    struct append_changed_type<TT<TTArgs...>>
    {
      using type = TypeList<Args...>;
    };

    template <typename T, typename... MoreArgs>
    using append_changed_type_t = typename append_changed_type<T, MoreArgs...>::type;

    template <typename... MoreArgs>
    struct prepend_changed_type;

    template <template<typename...> class TT, typename... TTArgs, typename... MoreArgs>
    struct prepend_changed_type<TT<TTArgs...>, MoreArgs...>
    {
    protected:
      using reversable_empty_type = detail::holder_type<>;
      using reversable_filled_type = typename detail::holder_type<>:: template append_changed_type<TT<TTArgs...>, MoreArgs...>::type;
      using final_filled_type = typename reversable_filled_type:: template append_type<Args...>::type;

    public:
      using type = rebind_from_template_t<TypeList<>, final_filled_type>;
    };

    template<template<typename...> class TT, typename... TTArgs>
    struct prepend_changed_type<TT<TTArgs...>>
    {
      using type = TypeList<Args...>;
    };

    template <typename T, typename... MoreArgs>
    using prepend_changed_type_t = typename prepend_changed_type<T, MoreArgs...>::type;

    struct reverse_type
    {
    protected:
      using reversable_empty_type = detail::holder_type<>;
      using reversable_filled_type = typename detail::holder_type<Args...>;
      using reversed_filled_type = detail::reversable_types_t<reversable_empty_type, reversable_filled_type>;

    public:
      using type = rebind_from_template_t<TypeList<>, reversed_filled_type>;
    };

    using reverse_type_t = typename reverse_type::type;

    // from https://en.cppreference.com/w/cpp/utility/tuple/tuple_element

    template <size_type N>
    struct element {
      static_assert(N < sizeof...(Args));
      using type = detail::element<N, Args...>;
    };

    template<size_type I>
    using element_t = typename element<I>::type;
  };

  //---------------------------------------------------------------------------
  template <typename... Args>
  struct type_list_with_types
  {
    using type = TypeList<Args...>;
  };

  template <typename... Args>
  using type_list_with_types_t = typename type_list_with_types<Args...>::type;


  //---------------------------------------------------------------------------
  template <typename... Args>
  struct type_list_with_unique_types
  {
    using type = typename TypeList<>::append_type_if_unique<Args...>::type;
  };

  template <typename... Args>
  using type_list_with_unique_types_t = typename type_list_with_unique_types<Args...>::type;


  //---------------------------------------------------------------------------
  template <typename TT, typename... Args>
  struct type_list_with_modified_types
  {
    using type = typename TypeList<>:: template append_changed_type<TT, Args...>::type;
  };

  template <typename TT, typename... Args>
  using type_list_with_modified_types_t = typename type_list_with_modified_types<TT, Args...>::type;


  //---------------------------------------------------------------------------
  template <typename TTypeList, typename TT>
  struct type_list_modify_all_types
  {
    using type = TTypeList;
  };

  template <typename TT, typename... Args>
  struct type_list_modify_all_types<TypeList<Args...>, TT>
  {
    using type = typename TypeList<>:: template append_changed_type<TT, Args...>::type;
  };

  template <typename TTypeList, typename TT>
  using type_list_modify_all_types_t = typename type_list_modify_all_types<TTypeList, TT>::type;

  //---------------------------------------------------------------------------
  // see https://en.cppreference.com/w/cpp/utility/variant/visit
  template<class... Ts> struct overloaded : Ts... { overloaded(Ts&&... args) : Ts{ args }...{} using Ts::operator()...; };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

  //---------------------------------------------------------------------------
  template <typename T, class Traits = std::char_traits<T>>
  struct is_std_basic_string_view : std::false_type {};

  template <typename T, typename Traits>
  struct is_std_basic_string_view<std::basic_string_view<T, Traits>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_basic_string_view_v = is_std_basic_string_view<T>::value;

  template <typename T>
  struct is_std_deduced_basic_string_view : is_std_basic_string_view<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_std_deduced_basic_string_view_v = is_std_deduced_basic_string_view<T>::value;


  //---------------------------------------------------------------------------
  template <typename T, typename Traits = std::char_traits<T>, typename Alloc = std::allocator<T>>
  struct is_std_basic_string : std::false_type {};

  template <typename T, typename Traits, typename Alloc>
  struct is_std_basic_string<std::basic_string<T, Traits, Alloc>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_basic_string_v = is_std_basic_string<T>::value;

  template <typename T>
  struct is_std_deduced_basic_string : is_std_basic_string<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_std_deduced_basic_string_v = is_std_deduced_basic_string<T>::value;


  //---------------------------------------------------------------------------
  template <typename T>
  struct is_std_array : std::false_type {};

  template <typename T, std::size_t N>
  struct is_std_array<std::array<T, N>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_array_v = is_std_array<T>::value;

  template <typename T>
  struct is_deduced_std_array : is_std_array<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_array_v = is_deduced_std_array<T>::value;


  //---------------------------------------------------------------------------
  template <typename T, typename Allocator = std::allocator<T>>
  struct is_std_vector : std::false_type {};

  template <typename T, typename Allocator>
  struct is_std_vector<std::vector<T, Allocator>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_vector_v = is_std_vector<T>::value;

  template <typename T>
  struct is_deduced_std_vector : is_std_vector<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_vector_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template <typename T, typename Allocator = std::allocator<T>>
  struct is_std_deque : std::false_type {};

  template <typename T, typename Allocator>
  struct is_std_deque<std::deque<T, Allocator>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_deque_v = is_std_deque<T>::value;

  template <typename T>
  struct is_deduced_std_deque : is_std_deque<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_deque_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template <typename T, typename Container = std::deque<T>>
  struct is_std_queue : std::false_type {};

  template <typename T, typename Container>
  struct is_std_queue<std::queue<T, Container>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_queue_v = is_std_queue<T>::value;

  template <typename T>
  struct is_deduced_std_queue : is_std_queue<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_queue_v = is_deduced_std_queue<T>::value;

  //---------------------------------------------------------------------------
  template <typename T, typename Allocator = std::allocator<T>>
  struct is_std_list : std::false_type {};

  template <typename T, typename Allocator>
  struct is_std_list<std::list<T, Allocator>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_list_v = is_std_list<T>::value;

  template <typename T>
  struct is_deduced_std_list : is_std_list<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_list_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template <typename T, typename Allocator = std::allocator<T>>
  struct is_std_forward_list : std::false_type {};

  template <typename T, typename Allocator>
  struct is_std_forward_list<std::forward_list<T, Allocator>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_forward_list_v = is_std_forward_list<T>::value;

  template <typename T>
  struct is_deduced_std_forward_list : is_std_forward_list<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_forward_list_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template <typename Key, typename Compare = std::less<Key>, typename Allocator = std::allocator<Key>>
  struct is_std_set : std::false_type {};

  template <typename Key, typename Compare, typename Allocator>
  struct is_std_set<std::set<Key, Compare, Allocator>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_set_v = is_std_set<T>::value;

  template <typename T>
  struct is_deduced_std_set : is_std_set<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_set_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template <typename Key, typename Compare = std::less<Key>, typename Allocator = std::allocator<Key>>
  struct is_std_multiset : std::false_type {};

  template <typename Key, typename Compare, typename Allocator>
  struct is_std_multiset<std::multiset<Key, Compare, Allocator>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_multiset_v = is_std_multiset<T>::value;

  template <typename T>
  struct is_deduced_std_multiset : is_std_multiset<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_multiset_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template<typename Key, typename T = void, typename Compare = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T> >>
  struct is_std_map : std::false_type {};

  template <typename Key, typename T, typename Compare, typename Allocator>
  struct is_std_map<std::map<Key, T, Compare, Allocator>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_map_v = is_std_map<T>::value;

  template <typename T>
  struct is_deduced_std_map : is_std_map<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_map_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template<typename Key, typename T = void, typename Compare = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T> >>
  struct is_std_multimap : std::false_type {};

  template <typename Key, typename T, typename Compare, typename Allocator>
  struct is_std_multimap<std::multimap<Key, T, Compare, Allocator>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_multimap_v = is_std_multimap<T>::value;

  template <typename T>
  struct is_deduced_std_multimap : is_std_multimap<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_multimap_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template<typename Key, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>, typename Allocator = std::allocator<Key>>
  struct is_std_unordered_set : std::false_type {};

  template <typename Key, typename Hash, typename KeyEqual, typename Allocator>
  struct is_std_unordered_set<std::unordered_set<Key, Hash, KeyEqual, Allocator>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_unordered_set_v = is_std_unordered_set<T>::value;

  template <typename T>
  struct is_deduced_std_unordered_set : is_std_unordered_set<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_unordered_set_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template<typename Key, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>, typename Allocator = std::allocator<Key>>
  struct is_std_unordered_multiset : std::false_type {};

  template <typename Key, typename Hash, typename KeyEqual, typename Allocator>
  struct is_std_unordered_multiset<std::unordered_multiset<Key, Hash, KeyEqual, Allocator>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_unordered_multiset_v = is_std_unordered_multiset<T>::value;

  template <typename T>
  struct is_deduced_std_unordered_multiset : is_std_unordered_multiset<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_unordered_multiset_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template<typename Key, typename T = void, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>, typename Allocator = std::allocator< std::pair<const Key, T> >>
  struct is_std_unordered_map : std::false_type {};

  template <typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
  struct is_std_unordered_map<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_unordered_map_v = is_std_unordered_map<T>::value;

  template <typename T>
  struct is_deduced_std_unordered_map : is_std_unordered_map<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_unordered_map_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template<typename Key, typename T = void, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>, typename Allocator = std::allocator< std::pair<const Key, T> >>
  struct is_std_unordered_multimap : std::false_type {};

  template <typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
  struct is_std_unordered_multimap<std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_unordered_multimap_v = is_std_unordered_multimap<T>::value;

  template <typename T>
  struct is_deduced_std_unordered_multimap : is_std_unordered_multimap<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_unordered_multimap_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template<typename T, typename Container = std::deque<T>>
  struct is_std_stack : std::false_type {};

  template <typename T, typename Container>
  struct is_std_stack<std::stack<T, Container>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_stack_v = is_std_stack<T>::value;

  template <typename T>
  struct is_deduced_std_stack : is_std_stack<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_stack_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template<typename T, typename Container = std::vector<T>, typename Compare = std::less<typename Container::value_type>>
  struct is_std_priority_queue : std::false_type {};

  template <typename T, typename Container, typename Compare>
  struct is_std_priority_queue<std::priority_queue<T, Container, Compare>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_priority_queue_v = is_std_priority_queue<T>::value;

  template <typename T>
  struct is_deduced_std_priority_queue : is_std_priority_queue<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_priority_queue_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template<typename T1, typename T2 = void>
  struct is_std_pair : std::false_type {};

  template <typename T1, typename T2>
  struct is_std_pair<std::pair<T1, T2>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_pair_v = is_std_pair<T>::value;

  template <typename T>
  struct is_deduced_std_pair : is_std_pair<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_pair_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template<typename... Types>
  struct is_std_tuple : std::false_type {};

  template<typename... Types>
  struct is_std_tuple<std::tuple<Types...>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_tuple_v = is_std_tuple<T>::value;

  template <typename T>
  struct is_deduced_std_tuple : is_std_tuple<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_tuple_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template<typename T, typename Deleter = std::default_delete<T>>
  struct is_std_unique_ptr : std::false_type {};

  template<typename T, typename Deleter>
  struct is_std_unique_ptr<std::unique_ptr<T, Deleter>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_unique_ptr_v = is_std_unique_ptr<T>::value;

  template <typename T>
  struct is_deduced_std_unique_ptr : is_std_unique_ptr<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_unique_ptr_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template<typename T>
  struct is_std_shared_ptr : std::false_type {};

  template<typename T>
  struct is_std_shared_ptr<std::shared_ptr<T>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_shared_ptr_v = is_std_shared_ptr<T>::value;

  template <typename T>
  struct is_deduced_std_shared_ptr : is_std_shared_ptr<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_shared_ptr_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template<typename T>
  struct is_move_only_shared_ptr : std::false_type {};

  template<typename T>
  struct is_move_only_shared_ptr<zs::move_only_shared_ptr<T>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_move_only_shared_ptr_v = is_move_only_shared_ptr<T>::value;

  template <typename T>
  struct is_deduced_move_only_shared_ptr : is_move_only_shared_ptr<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_move_only_shared_ptr_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template <typename T>
  struct is_reflect_type : std::false_type {};

  template <typename ...Args>
  struct is_reflect_type<ReflectType<Args...>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_reflect_type_v = is_reflect_type<T>::value;

  template <typename T>
  struct is_deduced_reflect_type : is_reflect_type<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_reflect_type_v = is_deduced_reflect_type<T>::value;


  //---------------------------------------------------------------------------
  template <typename T>
  struct is_reflect : std::false_type {};

  template <typename ...Args>
  struct is_reflect<Reflect<Args...>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_reflect_v = is_reflect<T>::value;

  template <typename T>
  struct is_deduced_reflect : is_reflect<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_reflect_v = is_deduced_reflect<T>::value;


  //---------------------------------------------------------------------------
  template <typename T>
  struct is_reflect_visitor : std::false_type {};

  template <typename T>
  struct is_reflect_visitor<ReflectVisitor<T>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_reflect_visitor_v = is_reflect_visitor<T>::value;

  template <typename T>
  struct is_deduced_reflect_visitor : is_reflect_visitor<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_reflect_visitor_v = is_deduced_reflect_visitor<T>::value;


  //---------------------------------------------------------------------------
  template <typename T>
  struct is_tuple_reflect_type : std::false_type {};

  template <typename TTuple>
  struct is_tuple_reflect_type<TupleReflectType<TTuple>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_tuple_reflect_type_v = is_tuple_reflect_type<T>::value;

  template <typename T>
  struct is_deduced_tuple_reflect_type : is_tuple_reflect_type<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_tuple_reflect_type_v = is_deduced_tuple_reflect_type<T>::value;


  //---------------------------------------------------------------------------
  template <typename T>
  struct is_tuple_reflect : std::false_type {};

  template <typename TTuple>
  struct is_tuple_reflect<TupleReflect<TTuple>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_tuple_reflect_v = is_tuple_reflect<T>::value;

  template <typename T>
  struct is_deduced_tuple_reflect : is_tuple_reflect<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_tuple_reflect_v = is_deduced_tuple_reflect<T>::value;


  //---------------------------------------------------------------------------
  template <typename T>
  struct is_tuple_reflect_visitor : std::false_type {};

  template <typename T>
  struct is_tuple_reflect_visitor<ReflectVisitor<T>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_tuple_reflect_visitor_v = is_tuple_reflect_visitor<T>::value;

  template <typename T>
  struct is_deduced_tuple_reflect_visitor : is_tuple_reflect_visitor<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_tuple_reflect_visitor_v = is_deduced_tuple_reflect_visitor<T>::value;


  //---------------------------------------------------------------------------
  template<typename T>
  struct is_std_optional : std::false_type {};

  template<typename T>
  struct is_std_optional<std::optional<T>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_optional_v = is_std_optional<T>::value;

  template <typename T>
  struct is_deduced_std_optional : is_std_optional<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_optional_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template<typename... Types>
  struct is_std_variant : std::false_type {};

  template<typename... Types>
  struct is_std_variant<std::variant<Types...>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_std_variant_v = is_std_variant<T>::value;

  template <typename T>
  struct is_deduced_std_variant : is_std_variant<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_std_variant_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
  template <typename T>
  struct is_gsl_span : std::false_type {};

  template <typename T, std::ptrdiff_t Extent>
  struct is_gsl_span<gsl::span<T, Extent>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_gsl_span_v = is_gsl_span<T>::value;

  template <typename T>
  struct is_deduced_gsl_span : is_gsl_span<std::remove_cvref_t<T>> {};

  template <typename T>
  inline constexpr bool is_deduced_gsl_span_v = is_deduced_std_vector<T>::value;


  //---------------------------------------------------------------------------
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

} // namespace zs

namespace std
{
  //---------------------------------------------------------------------------
  template <typename ...Args>
  struct tuple_size<zs::TypeList<Args...>> : std::integral_constant<size_t, zs::TypeList<Args...>::total()> {};
  
  //---------------------------------------------------------------------------
  template <size_t I, typename ...Args>
  struct tuple_element<I, zs::TypeList<Args...>> {
    using type = typename zs::TypeList<Args...>:: template element<I>::type;
  };

} // namespace std
