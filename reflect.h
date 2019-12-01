
#pragma once

#include "traits.h"

#include <tuple>
#include <variant>

namespace zs
{
  namespace detail
  {
    template <typename TReflectType>
    [[nodiscard]] const typename TReflectType::variant_array_type& reflectTypeVariantAccessor(const TReflectType &reflectType) noexcept {return reflectType.membersAsArray_; }
  }

  template <typename ...Args>
  struct ReflectType
  {
    template <typename TReflectType>
    friend const typename TReflectType::variant_array_type& detail::reflectTypeVariantAccessor(const TReflectType&) noexcept;

    using size_type = zs::size_type;
    using tuple_type = std::tuple<Args...>;
    using type_list = TypeList<Args...>;
    using unique_types_type_list = typename TypeList<>::append_type_if_unique_t<Args...>;
    using unique_types_variant = typename unique_types_type_list:: template rebind_t<std::variant<void>>;
    using variant_array_type = std::array<unique_types_variant, sizeof...(Args)>;

    using first_type = typename std::tuple_element<0, tuple_type>::type;
    using first_struct_type = typename remove_deduced_member_pointer<first_type>::struct_type;

    template <size_type N>
    using struct_type = typename remove_deduced_member_pointer<typename std::tuple_element_t<N, tuple_type>>::struct_type;

    template <size_type N>
    using member_type = remove_deduced_member_pointer_t<typename std::tuple_element_t<N, tuple_type>>;

    constexpr ReflectType() noexcept = delete;
    constexpr ReflectType(const ReflectType&) noexcept = default;
    constexpr ReflectType(ReflectType&&) noexcept = default;

    constexpr ReflectType& operator=(const ReflectType&) noexcept = default;
    constexpr ReflectType& operator=(ReflectType&&) noexcept = default;

    //-------------------------------------------------------------------------
    constexpr ReflectType(Args ...args) noexcept :
      members_{ std::forward<Args>(args)... },
      membersAsArray_{ std::forward<Args>(args)... }
    {
      static_assert(!std::is_same_v<void, std::remove_cvref_t<first_type>>);
      static_assert(Assertion<Args...>::check());
    }

    inline constexpr static std::integral_constant<size_type, sizeof...(Args)> total{};
    [[nodiscard]] constexpr size_type size() const noexcept { return total(); }

    [[nodiscard]] constexpr const tuple_type &members() const noexcept { return members_; }

  protected:
    tuple_type members_;
    variant_array_type membersAsArray_;

    //-------------------------------------------------------------------------
    template <typename T = void, typename ...Args>
    struct Assertion
    {
      //-----------------------------------------------------------------------
      [[nodiscard]] constexpr static bool check() noexcept
      {
        using recurse_type = Assertion<Args...>;

        if constexpr (!std::is_same_v<void, T>) {
          if constexpr (zs::is_member_function_pointer_v<std::remove_cv_t<T>>) {
            return (((std::is_same_v<zs::remove_deduced_member_pointer<T>::struct_type, first_struct_type>) ||
                     (std::is_base_of_v<zs::remove_deduced_member_pointer<T>::struct_type, first_struct_type>) ||
                     (std::is_base_of_v<first_struct_type, zs::remove_deduced_member_pointer<T>::struct_type>)) &&
                    (recurse_type::check()));
          }
          else {
            return false;
          }
        }
        else {
          return true;
        }
      }
    };
  };

  //---------------------------------------------------------------------------
  template <typename TStruct, typename ...Args>
  struct Reflect
  {
    using tuple_type = typename ReflectType<Args...>::tuple_type;
    using reflect_type = ReflectType<Args...>;

    using related_struct = std::remove_reference_t<TStruct>;

    using index_type = zs::index_type;
    using type = Reflect;

    template <typename TIndexType, index_type VDirection = static_cast<index_type>(1)>
    class Iterator;

    using iterator = Iterator<index_type, static_cast<index_type>(1)>;
    using const_iterator = Iterator<const index_type, static_cast<index_type>(1)>;

    using reverse_iterator = Iterator<index_type, static_cast<index_type>(-1)>;
    using const_reverse_iterator = Iterator<const index_type, static_cast<index_type>(-1)>;

    friend iterator;
    friend const_iterator;

    friend reverse_iterator;
    friend const_reverse_iterator;

    constexpr Reflect() noexcept = delete;
    constexpr Reflect(const Reflect&) noexcept = default;
    constexpr Reflect(Reflect&&) noexcept = default;

    constexpr Reflect& operator=(const Reflect&) noexcept = default;
    constexpr Reflect& operator=(Reflect&&) noexcept = default;

    inline constexpr static std::integral_constant<size_type, sizeof...(Args)> total{};
    [[nodiscard]] constexpr size_type size() const noexcept { return total(); }

    //-------------------------------------------------------------------------
    constexpr Reflect(
      related_struct& structure,
      const reflect_type& reflectType) noexcept :
      reflectType_{ std::forward<decltype(reflectType)>(reflectType) },
      structure_{ std::forward<decltype(structure)>(structure) }
    {
    }

    //-------------------------------------------------------------------------
    template <size_type N>
    [[nodiscard]] constexpr decltype(auto) get() const noexcept
    {
      return structure_.*std::get<N>(reflectType_.members());
    }

    //-------------------------------------------------------------------------
    template <size_type N, typename TFunction>
    constexpr decltype(auto) visit(TFunction&& function) const noexcept(std::is_nothrow_invocable_v<decltype(function)>)
    {
      return function(get<N>());
    }

    //-------------------------------------------------------------------------
    template <typename TFunction, typename... TIndex>
    constexpr decltype(auto) visit(TFunction&& function, TIndex... index) const noexcept(std::is_nothrow_invocable_v<decltype(function)>)
    {
      return std::visit(
        [&]<typename... MemberPtrs>(auto&& ...memberPtr) noexcept(std::is_nothrow_invocable_v<decltype(function)>) -> decltype(auto) { return function((structure_.*memberPtr)...); },
        (std::forward<const typename ReflectType<Args...>::unique_types_variant&>(detail::reflectTypeVariantAccessor(reflectType_)[index]))...
      );
    }

    //-------------------------------------------------------------------------
    template <typename TFunction>
    constexpr void visit(TFunction&& function) const noexcept(std::is_nothrow_invocable_v<decltype(function)>)
    {
      for (index_type index{}; index < sizeof...(Args); ++index) {
        static_cast<void>(std::visit(
          [&]<typename... MemberPtrs>(auto&& ...memberPtr) noexcept(std::is_nothrow_invocable_v<decltype(function)>) -> decltype(auto) { return function((structure_.*memberPtr)...); },
          std::forward<const typename ReflectType<Args...>::unique_types_variant&>(detail::reflectTypeVariantAccessor(reflectType_)[index])
        ));
      }
    }

    //-------------------------------------------------------------------------
    constexpr auto operator[](index_type index) const noexcept
    {
      return ReflectVisitor<Reflect>{ *this, index };
    }

    //-------------------------------------------------------------------------
    template <typename TIndexType, index_type VDirection>
    class Iterator
    {
      friend class iterator;
      friend class const_iterator;

    public:
      using index_type = zs::index_type;

      using index_iterator = Iterator<index_type, VDirection>;
      using const_index_iterator = Iterator<const index_type, VDirection>;

      constexpr Iterator() = default;

      template <typename T = const_index_iterator, typename std::enable_if_t<std::is_const_v<TIndexType>, T> * = nullptr>
      constexpr Iterator(const const_index_iterator& value) noexcept : outer_{ value.outer_ }, value_ { value.value_ } {}
      constexpr Iterator(const index_iterator& value) noexcept : outer_{ value.outer_ }, value_{ value.value_ } {}

      template <typename T = const_index_iterator, typename std::enable_if_t<std::is_const_v<TIndexType>, T> * = nullptr>
      constexpr Iterator(const_index_iterator&& value) noexcept : outer_{ value.outer_ }, value_{ value.value_ } {}
      constexpr Iterator(index_iterator&& value) noexcept : outer_{ value.outer_ }, value_{ value.value_ } {}

      constexpr Iterator(const Reflect& outer, const index_type &value) noexcept : outer_{ outer }, value_ { value } {}
      constexpr Iterator(const Reflect& outer, index_type &&value) noexcept : outer_{ outer }, value_{ value } {}

      template <typename T = const_index_iterator, typename std::enable_if_t<std::is_const_v<TIndexType>, T> * = nullptr>
      constexpr auto& operator=(const const_index_iterator& value) noexcept { value_ = value.value_; return *this; };
      constexpr auto& operator=(index_iterator& value) noexcept { value_ = value.value_; return *this; };

      template <typename T = const_index_iterator, typename std::enable_if_t<std::is_const_v<TIndexType>, T> * = nullptr>
      constexpr auto& operator=(const_index_iterator&& value) noexcept { value_ = value.value_; return *this; };
      constexpr auto& operator=(index_iterator&& value) noexcept { value_ = value.value_; return *this; };

      [[nodiscard]] constexpr index_type index() const noexcept { return value_; }

      [[nodiscard]] constexpr decltype(auto) operator*() const noexcept
      {
        return outer_[value_];
      }
      [[nodiscard]] constexpr decltype(auto) operator->() const noexcept { return &(*(*this)); }

      template <class Visitor>
      constexpr decltype(auto) visit(Visitor&& vis) noexcept(std::is_nothrow_invocable_v<decltype(vis)>)
      {
        return outer_.visit(value_, std::forward<decltype(vis)>(vis));
      }

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

      [[nodiscard]] constexpr decltype(auto) operator+(index_type distance) const noexcept {
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
        return temp + (static_cast<decltype(distance)>(-1)* distance);
      }

      [[nodiscard]] constexpr decltype(auto) operator+=(index_type distance) const noexcept {
        value_ = (distance * VDirection);
        return *this;
      }
      [[nodiscard]] constexpr decltype(auto) operator-=(index_type distance) const noexcept {
        value_ = (distance * VDirection);
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
      const Reflect& outer_;
      index_type value_{};
    };

    [[nodiscard]] constexpr auto begin() noexcept { return iterator{ *this, 0 }; }
    [[nodiscard]] constexpr auto begin() const noexcept { return const_iterator{ *this, 0 }; }

    [[nodiscard]] constexpr auto end() noexcept { return iterator{ *this, Reflect::total() }; }
    [[nodiscard]] constexpr auto end() const noexcept { return const_iterator{ { *this, Reflect::total() } }; }

    [[nodiscard]] constexpr auto rbegin() noexcept { return reverse_iterator{ { *this, Reflect::total() - 1 } }; }
    [[nodiscard]] constexpr auto rbegin() const noexcept { return const_reverse_iterator{ { *this, Reflect::total() - 1 }  }; }

    [[nodiscard]] constexpr auto rend() noexcept { return reverse_iterator{ { *this, -1 } }; }
    [[nodiscard]] constexpr auto rend() const noexcept { return const_reverse_iterator{ { *this, -1 } }; }

  protected:
    const reflect_type& reflectType_;
    related_struct& structure_;
  };

  template <typename TReflect>
  struct ReflectVisitor
  {
    using index_type = zs::index_type;
    using reflect_type = std::remove_cvref_t<TReflect>;

    friend reflect_type;

    constexpr ReflectVisitor(const reflect_type& reflect, index_type index) noexcept : reflect_(reflect), index_(index) {}

    constexpr ReflectVisitor() = default;
    constexpr ReflectVisitor(const ReflectVisitor&) = default;
    constexpr ReflectVisitor(ReflectVisitor&&) = default;

    [[nodiscard]] constexpr index_type index() noexcept { return index_; }

    template <typename TVisitorFunction>
    constexpr decltype(auto) visit(TVisitorFunction&& vis) const noexcept(std::is_nothrow_invocable_v<decltype(vis)>)
    {
      return reflect_.visit(std::forward<decltype(vis)>(vis), index_);
    }

    [[nodiscard]] constexpr const reflect_type& reflect() const noexcept { return reflect_; }

  protected:
    const reflect_type& reflect_;
    index_type index_;
  };

  template <typename ...Args>
  [[nodiscard]] constexpr ReflectType<Args...> make_reflect_type(Args ...args) noexcept
  {
    return ReflectType<Args...>{ std::forward<Args>(args)... };
  }

  template <typename TStruct, typename ...Args>
  [[nodiscard]] constexpr auto make_reflect(TStruct&& structure, const ReflectType<Args...> &reflectType) noexcept
  {
     Reflect<TStruct, Args...> result { std::forward<TStruct>(structure), reflectType };
     return result;
  }

  //---------------------------------------------------------------------------
  namespace detail
  {
    template <typename TFunction, typename TReflect, size_type... N>
    constexpr decltype(auto ) apply(TFunction&& function, TReflect&& reflect, std::index_sequence<N...>&&) noexcept(std::is_nothrow_invocable_v<decltype(function)>)
    {
      return std::invoke(std::forward<TFunction>(function), (reflect.get<N>())...);
    }
  }

} // namespace zs

namespace std
{
  //---------------------------------------------------------------------------
  template <zs::size_type N, typename T, std::enable_if_t<zs::is_deduced_reflect_v<T>, int> * = nullptr>
  [[nodiscard]] constexpr static decltype(auto) get(T& reflect) noexcept { return reflect.get<N>(); }

  template <zs::size_type N, typename T, std::enable_if_t<zs::is_deduced_reflect_v<T>, int> * = nullptr>
  [[nodiscard]] constexpr static decltype(auto) get(const T &reflect) noexcept { return reflect.get<N>(); }

  template <zs::size_type N, typename T, std::enable_if_t<zs::is_deduced_reflect_v<T>, int> * = nullptr>
  [[nodiscard]] constexpr static decltype(auto) get(T&& reflect) noexcept { return reflect.get<N>(); }

  template <zs::size_type N, typename T, std::enable_if_t<zs::is_deduced_reflect_v<T>, int> * = nullptr>
  [[nodiscard]] constexpr static decltype(auto) get(const T&& reflect) noexcept { return reflect.get<N>(); }

  //---------------------------------------------------------------------------
  template <typename ...Args>
  struct tuple_size<zs::ReflectType<Args...>> : std::integral_constant<size_t, zs::ReflectType<Args...>::total()> {};

  template <typename ...Args>
  struct tuple_size<zs::Reflect<Args...>> : std::integral_constant<size_t, zs::Reflect<Args...>::total()> {};

  //---------------------------------------------------------------------------
  template <size_t I, typename ...Args>
  struct tuple_element<I, zs::ReflectType<Args...>> {
    using type = typename zs::ReflectType<Args...>:: template member_type<I>;
  };

  template <size_t I, typename TStruct, typename ...Args>
  struct tuple_element<I, zs::Reflect<TStruct, Args...>> {
    using type = typename zs::Reflect<TStruct, Args...>:: template member_type<I>;
  };

  //---------------------------------------------------------------------------
  template <typename TFunction, typename... Args>
  constexpr decltype(auto) apply(TFunction&& function, const zs::Reflect<Args...>& reflect) noexcept(std::is_nothrow_invocable_v<decltype(function)>)
  {
    return zs::detail::apply(std::forward<TFunction>(function), std::forward<decltype(reflect)>(reflect), std::make_index_sequence<tuple_size_v<std::remove_cvref_t<decltype(reflect)>>>{});
  }

  //---------------------------------------------------------------------------
  template <typename TFunction, typename... Args>
  constexpr decltype(auto) apply(TFunction&& function, zs::Reflect<Args...>& reflect) noexcept(std::is_nothrow_invocable_v<decltype(function)>)
  {
    return zs::detail::apply(std::forward<TFunction>(function), std::forward<decltype(reflect)>(reflect), std::make_index_sequence<tuple_size_v<std::remove_cvref_t<decltype(reflect)>>>{});
  }

  //---------------------------------------------------------------------------
  template <typename TFunction, typename... Args>
  constexpr decltype(auto) apply(TFunction&& function, zs::Reflect<Args...>&& reflect) noexcept(std::is_nothrow_invocable_v<decltype(function)>)
  {
    return zs::detail::apply(std::forward<TFunction>(function), std::forward<decltype(reflect)>(reflect), std::make_index_sequence<tuple_size_v<std::remove_cvref_t<decltype(reflect)>>>{});
  }

  //---------------------------------------------------------------------------
  template <class TVisitor, typename T, std::enable_if_t<zs::is_deduced_reflect_visitor_v<T>> * = nullptr>
  constexpr decltype(auto) visit(TVisitor&& vis, T&& reflectVisitor) noexcept(std::is_nothrow_invocable_v<decltype(vis)>)
  {
    return reflectVisitor.visit(std::forward<decltype(vis)>(vis));
  }

  //---------------------------------------------------------------------------
  template <class TVisitor, typename TReflect, typename... Args, std::enable_if_t<zs::is_deduced_reflect_v<TReflect>> * = nullptr>
  constexpr decltype(auto) visit(TVisitor&& vis, TReflect&& reflect, Args... args) noexcept(std::is_nothrow_invocable_v<decltype(vis)>)
  {
    return reflect.visit(std::forward<decltype(vis)>(vis), std::forward<Args>(args)...);
  }

  //---------------------------------------------------------------------------
  template <class TVisitor, typename... TReflect, std::enable_if_t<zs::are_all_v<zs::is_deduced_reflect<void>, TReflect...>> * = nullptr>
  constexpr void visit(TVisitor&& vis, TReflect&&... reflects) noexcept(std::is_nothrow_invocable_v<decltype(vis)>)
  {
    auto function{ [&](auto&& reflect) noexcept(std::is_nothrow_invocable_v<decltype(vis)>) { reflect.visit(std::forward<decltype(vis)>(vis)); } };
    [&](auto&& ...args) noexcept(std::is_nothrow_invocable_v<decltype(vis)>) { (static_cast<void>(function(std::forward<decltype(args)>(args))), ...); }(std::forward<TReflect>(reflects)...);
  }

} // namespace std
