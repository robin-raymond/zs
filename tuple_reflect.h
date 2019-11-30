
#pragma once

#include "traits.h"

#include <tuple>
#include <variant>

namespace zs
{
  template <typename TTuple>
  struct TupleReflectType
  {
    friend struct TupleReflect<TTuple>;

    using size_type = zs::size_type;

    using related_tuple_type = std::remove_reference_t<TTuple>;
    using related_tuple_type_list = zs::rebind_from_template_t<TypeList<void>, related_tuple_type>;

    template <size_type N>
    using member_type = typename std::tuple_element_t<N, related_tuple_type>;

    inline constexpr static count_template_types<related_tuple_type> total{};
    [[nodiscard]] constexpr size_type size() const noexcept { return total(); }

  protected:
    template <size_type tuple_index>
    struct TupleExtractor
    {
      inline constexpr static auto extractorLambda_{
        [](related_tuple_type& tup) noexcept -> decltype(auto) { return std::get<tuple_index>(tup); }
      };

      using function_extractor_type = decltype(extractorLambda_);
    };

    template <typename... Args>
    struct MakeTuppleExtractor;

    template <size_type... Is>
    struct MakeTuppleExtractor<std::integer_sequence<size_type, Is...>>
    {
      template <size_type N>
      using extractor_type_t = typename TupleExtractor<N>::function_extractor_type;

      using type_list = type_list_with_types_t<extractor_type_t<Is>...>;
      using variant_type_list = type_list_with_unique_types_t<extractor_type_t<Is>...>;

      using variant_type = rebind_from_template_t<std::variant<void>, variant_type_list>;
    };

    using make_tuple_extractor = MakeTuppleExtractor<std::make_integer_sequence<size_type, count_template_types_v<related_tuple_type>>>;
    using extractor_type_list = typename make_tuple_extractor::type_list;
    using extractor_variant_type = typename make_tuple_extractor::variant_type;

    using extractor_variant_array_type = std::array<extractor_variant_type, count_template_types_v<related_tuple_type>>;

    template <size_type N>
    constexpr static decltype(auto) extractor() noexcept { return TupleExtractor<N>::extractorLambda_; }

    template <typename TIndex>
    [[nodiscard]] constexpr const extractor_variant_type& variantAt(TIndex index) const noexcept
    {
      return holder_.extractors_[index];
    }

    template <typename... Args>
    struct ArrayHolder;

    template <size_type... Is>
    struct ArrayHolder<std::integer_sequence<size_type, Is...>>
    {
      template <size_type N>
      constexpr static size_type get() { return N; }

      extractor_variant_array_type extractors_{ extractor<Is>()... };
    };
    using array_holder_type = ArrayHolder<std::make_integer_sequence<size_type, count_template_types_v<related_tuple_type>>>;

    array_holder_type holder_;
  };

  //---------------------------------------------------------------------------
  template <typename TTuple>
  struct TupleReflect
  {
    using index_type = zs::index_type;
    using type = TupleReflect;

    using tuple_reflect_type = std::remove_cvref_t<TupleReflectType<TTuple>>;

    using related_tuple_type = typename TupleReflectType<TTuple>::related_tuple_type;

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

    constexpr TupleReflect() noexcept = delete;
    constexpr TupleReflect(const TupleReflect&) noexcept = default;
    constexpr TupleReflect(TupleReflect&&) noexcept = default;

    constexpr TupleReflect& operator=(const TupleReflect&) noexcept = default;
    constexpr TupleReflect& operator=(TupleReflect&&) noexcept = default;

    //-------------------------------------------------------------------------
    constexpr TupleReflect(related_tuple_type& structure) noexcept :
      structure_{ std::forward<decltype(structure)>(structure) }
    {
    }

    inline constexpr static count_template_types<related_tuple_type> total{};
    [[nodiscard]] constexpr size_type size() const noexcept { return total(); }

    [[nodiscard]] decltype(auto) tuple() const noexcept { return reflectType_; }

    //-------------------------------------------------------------------------
    template <size_type N>
    [[nodiscard]] constexpr decltype(auto) get() const noexcept
    {
      return std::get<N>(structure_);
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
        [&](auto &&... getterFunc) noexcept(std::is_nothrow_invocable_v<decltype(function)>) -> decltype(auto) { return function(std::invoke(std::forward<decltype(getterFunc)>(getterFunc), std::forward<decltype(structure_)>(structure_))...); },
        (reflectType_.variantAt(index))...
      );
    }

    //-------------------------------------------------------------------------
    template <typename TFunction>
    constexpr void visit(TFunction&& function) const noexcept(std::is_nothrow_invocable_v<decltype(function)>)
    {
      for (size_type index{}; index < TupleReflectType<TTuple>::total(); ++index) {
        static_cast<void>(std::visit(
          [&](auto&& getterFunc) noexcept(std::is_nothrow_invocable_v<decltype(function)>) -> decltype(auto) { return function(std::invoke(std::forward<decltype(getterFunc)>(getterFunc), std::forward<decltype(structure_)>(structure_))); },
          reflectType_.variantAt(index)
        ));
      }
    }

    //-------------------------------------------------------------------------
    constexpr auto operator[](index_type index) const noexcept
    {
      return TupleReflectVisitor<TupleReflect>{ *this, index };
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

      constexpr Iterator(const TupleReflect& outer, const index_type &value) noexcept : outer_{ outer }, value_ { value } {}
      constexpr Iterator(const TupleReflect& outer, index_type &&value) noexcept : outer_{ outer }, value_{ value } {}

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
      const TupleReflect& outer_;
      index_type value_{};
    };

    [[nodiscard]] constexpr auto begin() noexcept { return iterator{ *this, 0 }; }
    [[nodiscard]] constexpr auto begin() const noexcept { return const_iterator{ *this, 0 }; }

    [[nodiscard]] constexpr auto end() noexcept { return iterator{ *this, TupleReflect::total() }; }
    [[nodiscard]] constexpr auto end() const noexcept { return const_iterator{ { *this, Reflect::total() } }; }

    [[nodiscard]] constexpr auto rbegin() noexcept { return reverse_iterator{ { *this, TupleReflect::total() - 1 } }; }
    [[nodiscard]] constexpr auto rbegin() const noexcept { return const_reverse_iterator{ { *this, TupleReflect::total() - 1 }  }; }

    [[nodiscard]] constexpr auto rend() noexcept { return reverse_iterator{ { *this, -1 } }; }
    [[nodiscard]] constexpr auto rend() const noexcept { return const_reverse_iterator{ { *this, -1 } }; }

  protected:
    inline constexpr static TupleReflectType<TTuple> reflectType_{};
    related_tuple_type& structure_;
  };

  template <typename TTupleReflect>
  struct TupleReflectVisitor
  {
    using index_type = zs::index_type;
    using tuple_reflect_type = std::remove_cvref_t<TTupleReflect>;

    friend tuple_reflect_type;

    constexpr TupleReflectVisitor(const tuple_reflect_type& reflect, index_type index) noexcept : reflect_(reflect), index_(index) {}

    constexpr TupleReflectVisitor() = default;
    constexpr TupleReflectVisitor(const TupleReflectVisitor&) = default;
    constexpr TupleReflectVisitor(TupleReflectVisitor&&) = default;

    [[nodiscard]] constexpr index_type index() noexcept { return index_; }

    template <typename TVisitorFunction>
    constexpr decltype(auto) visit(TVisitorFunction&& vis) const noexcept(std::is_nothrow_invocable_v<decltype(vis)>)
    {
      return reflect_.visit(std::forward<decltype(vis)>(vis), index_);
    }

    [[nodiscard]] constexpr const tuple_reflect_type& reflect() const noexcept { return reflect_; }

  protected:
    const tuple_reflect_type& reflect_;
    index_type index_;
  };

  template <typename TTuple>
  [[nodiscard]] constexpr TupleReflectType<TTuple> make_tuple_reflect_type() noexcept
  {
    return TupleReflectType<TTuple>{};
  }

    template <typename TTuple>
  [[nodiscard]] constexpr TupleReflect<TTuple> make_reflect(TTuple&& structure) noexcept
  {
    return TupleReflect<TTuple>{ std::forward<TTuple>(structure) };
  }

  template <typename TStruct, typename ...Args>
  [[nodiscard]] constexpr auto make_reflect_from_type(TStruct&& structure, const TupleReflectType<Args...> &TupleReflectType) noexcept
  {
     Reflect<TStruct, Args...> result { std::forward<TStruct>(structure), TupleReflectType };
     return result;
  }

  template <typename TStruct, typename ...Args>
  [[nodiscard]] constexpr auto make_reflect_from_type(TStruct&& structure, TupleReflectType<Args...>&& TupleReflectType) noexcept
  {
    Reflect<TStruct, Args...> result = { std::forward<TStruct>(structure), std::forward<decltype(TupleReflectType)>(TupleReflectType) };
    return result;
  }

#if 0
  //---------------------------------------------------------------------------
  namespace detail
  {
    template <typename TFunction, typename TTupleReflect, size_type... N>
    constexpr decltype(auto ) apply(TFunction&& function, TTupleReflect&& tupleReflect, std::index_sequence<N...>&&) noexcept(std::is_nothrow_invocable_v<decltype(function)>)
    {
      return std::invoke(std::forward<TFunction>(function), (tupleReflect.get<N>())...);
    }
  }
#endif //0

} // namespace zs

namespace std
{
  //---------------------------------------------------------------------------
  template <zs::size_type N, typename T, std::enable_if_t<zs::is_deduced_tuple_reflect_v<T>, int> * = nullptr>
  [[nodiscard]] constexpr static decltype(auto) get(T& reflect) noexcept { return reflect.get<N>(); }

  template <zs::size_type N, typename T, std::enable_if_t<zs::is_deduced_tuple_reflect_v<T>, int> * = nullptr>
  [[nodiscard]] constexpr static decltype(auto) get(const T &reflect) noexcept { return reflect.get<N>(); }

  template <zs::size_type N, typename T, std::enable_if_t<zs::is_deduced_tuple_reflect_v<T>, int> * = nullptr>
  [[nodiscard]] constexpr static decltype(auto) get(T&& reflect) noexcept { return reflect.get<N>(); }

  template <zs::size_type N, typename T, std::enable_if_t<zs::is_deduced_tuple_reflect_v<T>, int> * = nullptr>
  [[nodiscard]] constexpr static decltype(auto) get(const T&& reflect) noexcept { return reflect.get<N>(); }

  //---------------------------------------------------------------------------
  template <typename TTuple>
  struct tuple_size<zs::TupleReflectType<TTuple>> : std::integral_constant<size_t, zs::TupleReflectType<TTuple>::total()> {};

  template <typename TTuple>
  struct tuple_size<zs::TupleReflect<TTuple>> : std::integral_constant<size_t, zs::TupleReflect<TTuple>::total()> {};

  //---------------------------------------------------------------------------
  template <size_t I, typename TTuple>
  struct tuple_element<I, zs::TupleReflectType<TTuple>> { // recursive tuple_element definition
    using type = typename zs::TupleReflectType<TTuple>:: template member_type<I>;
  };

  template <size_t I, typename TTuple>
  struct tuple_element<I, zs::TupleReflect<TTuple>> { // recursive tuple_element definition
    using type = typename zs::TupleReflect<TTuple>:: template member_type<I>;
  };

  //---------------------------------------------------------------------------
  template <typename TFunction, typename TTuple>
  constexpr decltype(auto) apply(TFunction&& function, const zs::TupleReflect<TTuple>& reflect) noexcept(std::is_nothrow_invocable_v<decltype(function)>)
  {
    return std::apply(std::forward<TFunction>(function), reflect.tuple());
  }

  //---------------------------------------------------------------------------
  template <typename TFunction, typename TTuple>
  constexpr decltype(auto) apply(TFunction&& function, zs::TupleReflect<TTuple>& reflect) noexcept(std::is_nothrow_invocable_v<decltype(function)>)
  {
    return std::apply(std::forward<TFunction>(function), reflect.tuple());
  }

  //---------------------------------------------------------------------------
  template <typename TFunction, typename TTuple>
  constexpr decltype(auto) apply(TFunction&& function, zs::TupleReflect<TTuple>&& reflect) noexcept(std::is_nothrow_invocable_v<decltype(function)>)
  {
    return std::apply(std::forward<TFunction>(function), reflect.tuple());
  }

  //---------------------------------------------------------------------------
  template <class TVisitor, typename T, std::enable_if_t<zs::is_deduced_tuple_reflect_visitor_v<T>> * = nullptr>
  constexpr decltype(auto) visit(TVisitor&& vis, T&& reflectVisitor) noexcept(std::is_nothrow_invocable_v<decltype(vis)>)
  {
    return reflectVisitor.visit(std::forward<decltype(vis)>(vis));
  }

  //---------------------------------------------------------------------------
  template <class TVisitor, typename TTupleReflect, typename... Args, std::enable_if_t<zs::is_deduced_tuple_reflect_v<TTupleReflect>> * = nullptr>
  constexpr decltype(auto) visit(TVisitor&& vis, TTupleReflect&& reflect, Args... args) noexcept(std::is_nothrow_invocable_v<decltype(vis)>)
  {
    return reflect.visit(std::forward<decltype(vis)>(vis), std::forward<Args>(args)...);
  }

  //---------------------------------------------------------------------------
  template <class TVisitor, typename... TTupleReflect, std::enable_if_t<zs::are_all_v<zs::is_deduced_tuple_reflect<void>, TTupleReflect...>> * = nullptr>
  constexpr void visit(TVisitor&& vis, TTupleReflect&&... reflects) noexcept(std::is_nothrow_invocable_v<decltype(vis)>)
  {
    auto function{ [&](auto&& reflect) noexcept(std::is_nothrow_invocable_v<decltype(vis)>) { reflect.visit(std::forward<decltype(vis)>(vis)); } };
    [&](auto&& ...args) noexcept(std::is_nothrow_invocable_v<decltype(vis)>) { (static_cast<void>(function(std::forward<decltype(args)>(args))), ...); }(std::forward<TTupleReflect>(reflects)...);
  }

} // namespace std
