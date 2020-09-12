
#pragma once

#include <memory>

namespace zs
{
  template <typename T>
  class move_only_shared_ptr;

  // The zs::move_shared_ptr<T> code is a drop in replacement for
  // std::share_ptr. The type becomes type zs::move_only_shared_ptr in _DEBUG
  // mode but is a std::shared_ptr in release mode. This class enforces
  // move semantics or pass by reference is done instead of value copies.
  // Sharing a shared_ptr is made explicit.
#ifdef _DEBUG
  template <typename T>
  using move_shared_ptr = move_only_shared_ptr<T>;
#else
  template <typename T>
  using move_shared_ptr = std::shared_ptr<T>;
#endif //_DEBUG

  template <typename T>
  using MoveSharedPtr = move_shared_ptr<T>;

  template <typename T>
  using MoveOnlySharedPtr = move_only_shared_ptr<T>;

  template <typename T>
  class move_only_shared_ptr final
  {
  public:
    template <typename Y>
    friend class move_only_shared_ptr;

    using ptr_type = std::shared_ptr<T>;
    using element_type = typename ptr_type::element_type;
    using weak_type = typename ptr_type::weak_type;

    // default constructor
    constexpr move_only_shared_ptr() noexcept = default;

    // construct from nullptr
    constexpr explicit move_only_shared_ptr(std::nullptr_t) noexcept :
      ptr_{ nullptr } {}

    // copy constructors (DISABLED)
    constexpr move_only_shared_ptr(const move_only_shared_ptr& r) noexcept = delete;
    template <typename Y>
    constexpr move_only_shared_ptr(const move_only_shared_ptr<Y>& r) noexcept = delete;

    // copy constructor from shared_ptr
    template <typename Y>
    constexpr move_only_shared_ptr(const std::shared_ptr<Y>& r) noexcept :
      ptr_{ std::forward<decltype(r)>(r) } {}

    // copy from a weak_ptr
    template <typename Y>
    constexpr explicit move_only_shared_ptr(const std::weak_ptr<Y>& r) :
      ptr_{ std::forward<decltype(r)>(r) } {}

    // move constructors
    constexpr move_only_shared_ptr(move_only_shared_ptr&& r) noexcept = default;
    template <typename Y>
    constexpr move_only_shared_ptr(move_only_shared_ptr<Y>&& r) noexcept :
      ptr_{ std::move(r.ptr_) } {}

    // move constructors from shared_ptr
    template <typename Y>
    constexpr move_only_shared_ptr(std::shared_ptr<Y>&& r) noexcept :
      ptr_{ std::forward<decltype(r)>(r) } {}

    // move constructors from unique_ptr
    template <typename Y, typename Deleter>
    constexpr move_only_shared_ptr(std::unique_ptr<Y, Deleter>&& r) noexcept :
      ptr_{ std::move(r) } {}

    // construct from raw pointer taking ownership
    template <typename Y>
    constexpr explicit move_only_shared_ptr(Y* ptr) noexcept :
      ptr_{ std::forward<decltype(ptr)>(ptr) } {}

    // construct from raw pointer with deleter
    template <typename Y, typename Deleter>
    constexpr move_only_shared_ptr(Y* ptr, Deleter d) noexcept :
      ptr_{ std::forward<decltype(ptr)>(ptr), std::forward<decltype(d)>(d) } {}

    // construct from raw pointer with deleter and allocator
    template <typename Y, typename Deleter, typename Alloc>
    constexpr move_only_shared_ptr(Y* ptr, Deleter d, Alloc alloc) :
      ptr_{ std::forward<decltype(ptr)>(ptr), std::forward<decltype(d)>(d), std::forward<decltype(alloc)>(alloc) } {}

    // construct from nullptr pointer with deleter
    template <typename Deleter>
    constexpr explicit move_only_shared_ptr(nullptr_t, Deleter d) noexcept :
      ptr_{ nullptr, std::forward<decltype(d)>(d) } {}

    // construct from nullptr pointer with deleter and allocator
    template <typename Deleter, typename Alloc>
    constexpr explicit move_only_shared_ptr(nullptr_t, Deleter d, Alloc alloc) :
      ptr_{ nullptr, std::forward<decltype(d)>(d), std::forward<decltype(alloc)>(alloc) } {}

    // copy from an existing ptr pointing to an element (DISABLED)
    template <typename Y>
    constexpr move_only_shared_ptr(const move_only_shared_ptr<Y>& r, element_type* ptr) noexcept = delete;

    // move from an existing ptr pointing to an element
    template <typename Y>
    constexpr move_only_shared_ptr(move_only_shared_ptr<Y>&& r, element_type* ptr) noexcept :
      ptr_{ std::move(r.ptr_), std::forward<decltype(ptr)>(ptr) } {}

    // copy from an existing shared_ptr pointing to an element
    template <typename Y>
    constexpr move_only_shared_ptr(const std::shared_ptr<Y>& r, element_type* ptr) noexcept :
      ptr_{ std::forward<decltype(r)>(r), std::forward<decltype(ptr)>(ptr) } {}

    // move from an existing shared_ptr pointing to an element
    template <typename Y>
    constexpr move_only_shared_ptr(std::shared_ptr<Y>&& r, element_type* ptr) noexcept :
      ptr_{ std::move(r), std::forward<decltype(ptr)>(ptr) } {}

    // assign operator
    constexpr move_only_shared_ptr& operator=(const move_only_shared_ptr& value) noexcept = delete;
    template <typename Y>
    constexpr move_only_shared_ptr& operator=(const move_only_shared_ptr<Y>& value) noexcept = delete;

    // assign operator from nullptr
    constexpr move_only_shared_ptr& operator=(std::nullptr_t) noexcept
    { ptr_ = nullptr; return *this; }

    // assign operator from shared_ptr
    template <typename Y>
    constexpr move_only_shared_ptr &operator=(const std::shared_ptr<Y> &r) noexcept
    { ptr_ = std::forward<decltype(r)>(r); return *this; }

    // assign operator from unique_ptr
    template <typename Y, typename Deleter>
    constexpr move_only_shared_ptr& operator=(const std::unique_ptr<Y, Deleter>& r) noexcept = delete;

    // move operator
    constexpr move_only_shared_ptr& operator=(move_only_shared_ptr&& r) noexcept = default;
    template <typename Y>
    constexpr move_only_shared_ptr& operator=(move_only_shared_ptr<Y> && r) noexcept
    { ptr_ = std::move(r.ptr_); return *this; }
    
    // move operator from shared_ptr
    template <typename Y>
    constexpr move_only_shared_ptr& operator=(std::shared_ptr<Y>&& r) noexcept
    { ptr_ = std::move(r); return *this; }

    // move operator from shared_ptr
    template <typename Y, typename Deleter>
    constexpr move_only_shared_ptr& operator=(std::unique_ptr<Y, Deleter>&& r) noexcept
    { ptr_ = std::move(r); return *this; }

    template <typename U = T, typename TElementType = element_type, std::enable_if_t<std::is_array_v<U>, int> = 0>
    [[nodiscard]] decltype(auto) operator[](ptrdiff_t idx) const noexcept
    { return ptr_[idx]; }

    // other routines
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return nullptr != ptr_.get(); }

    [[nodiscard]] constexpr decltype(auto) use_count() const noexcept { return ptr_.use_count(); }

    constexpr void reset() noexcept
    { ptr_.reset(); }

    template <typename Y>
    constexpr void reset(Y* ptr) noexcept
    { ptr_.reset(ptr); }

    template <typename Y, typename Deleter>
    constexpr void reset(Y* ptr, Deleter d)
    { ptr_.reset(ptr, std::forward<decltype(d)>(d)); }

    template <typename Y, typename Deleter, typename Alloc>
    constexpr void reset(Y* ptr, Deleter d, Alloc alloc)
    { ptr_.reset(ptr, std::forward<decltype(d)>(d), std::forward<decltype(alloc)>(alloc)); }

    template <typename Y>
    constexpr void swap(move_only_shared_ptr<Y>& r) noexcept
    { ptr_.swap(r.ptr_); }
    template <typename Y>
    constexpr void swap(std::shared_ptr<Y>& r) noexcept
    { ptr_.swap(std::forward<decltype(r)>(r)); }

    [[nodiscard]] constexpr decltype(auto) operator*() noexcept { return *ptr_; }
    [[nodiscard]] constexpr decltype(auto) operator*() const noexcept { return *ptr_; }

    [[nodiscard]] constexpr decltype(auto) operator->() noexcept { return ptr_.get(); }
    [[nodiscard]] constexpr decltype(auto) operator->() const noexcept { return ptr_.get(); }

    [[nodiscard]] constexpr decltype(auto) get() noexcept { return ptr_.get(); }
    [[nodiscard]] constexpr decltype(auto) get() const noexcept { return ptr_.get(); }

    [[nodiscard]] constexpr decltype(auto) ptr() noexcept { return ptr_; }
    [[nodiscard]] constexpr decltype(auto) ptr() const noexcept { return ptr_; }

    [[nodiscard]] constexpr decltype(auto) share() const noexcept { return move_only_shared_ptr{ ptr_ }; }
    [[nodiscard]] constexpr decltype(auto) weak() const noexcept { return std::weak_ptr<T>{ ptr_ }; }

    template <typename Y>
    [[nodiscard]] constexpr decltype(auto) owner_before(const move_only_shared_ptr<Y>& other) const noexcept
    { return ptr_.owner_before(other.ptr_); }
    template <typename Y>
    [[nodiscard]] constexpr decltype(auto) owner_before(const std::shared_ptr<Y>& other) const noexcept
    { return ptr_.owner_before(std::forward<decltype(other)>(other)); }
    template <typename Y>
    [[nodiscard]] constexpr decltype(auto) owner_before(const std::weak_ptr<Y>& other) const noexcept
    { return ptr_.owner_before(std::forward<decltype(other)>(other)); }

    template <typename T, typename U>
    friend constexpr bool operator==(const move_only_shared_ptr<T>& lhs, const move_only_shared_ptr<U>& rhs) noexcept;

    template <typename T, typename U>
    friend constexpr bool operator!=(const move_only_shared_ptr<T>& lhs, const move_only_shared_ptr<U>& rhs) noexcept;

    template <typename T, typename U>
    friend constexpr bool operator<(const move_only_shared_ptr<T>& lhs, const move_only_shared_ptr<U>& rhs) noexcept;

    template <typename T, typename U>
    friend constexpr bool operator<=(const move_only_shared_ptr<T>& lhs, const move_only_shared_ptr<U>& rhs) noexcept;

    template <typename T, typename U>
    friend constexpr bool operator>(const move_only_shared_ptr<T>& lhs, const move_only_shared_ptr<U>& rhs) noexcept;

    template <typename T, typename U>
    friend constexpr bool operator>=(const move_only_shared_ptr<T>& lhs, const move_only_shared_ptr<U>& rhs) noexcept;

  protected:
    ptr_type ptr_;
  };

  template <typename T, typename... Args>
  constexpr move_shared_ptr<T> make_move_shared(Args&&... args)
  {
    return { std::make_shared<T>(std::forward<Args>(args)...) };
  }

  template <typename T>
  constexpr move_shared_ptr<T> make_move_shared()
  {
    return { std::make_shared<T>() };
  }

  template <typename T>
  constexpr move_shared_ptr<T> make_move_shared(std::size_t N)
  {
    return { std::make_shared<T>(std::forward<decltype(N)>(N)) };
  }

#if 0

  // needs C++ 20

  template <typename T>
  constexpr move_shared_ptr<T> make_move_shared(std::size_t N, const std::remove_extent_t<T>& u)
  {
    return { std::make_shared<T>(std::forward<decltype(N)>(N), std::forward<decltype(u)>(u)) };
  }

  template <typename T>
  constexpr move_shared_ptr<T> make_move_shared(const std::remove_extent_t<T>& u)
  {
    return { std::make_shared<T>(std::forward<decltype(u)>(u)) };
  }

  template <typename T>
  constexpr move_shared_ptr<T> make_move_shared_default_init()
  {
    return { std::make_shared_default_init<T>() };
  }
  
  template <typename T>
  constexpr move_shared_ptr<T> make_move_shared_default_init(std::size_t N)
  {
    return { std::make_shared_default_init<T>(std::forward<decltype(N)>(N)) };
  }
#endif //0

  template <typename T, typename Alloc, typename ... Args>
  constexpr move_shared_ptr<T> allocate_move_shared(const Alloc& alloc, Args&&... args)
  {
    return { std::allocate_shared<T, Alloc>(std::forward<decltype(alloc)>(alloc), std::forward<Args>(args)...) };
  }

#if 0

  // needs C++ 20

  template <typename T, typename Alloc>
  constexpr move_shared_ptr<T> allocate_move_shared(const Alloc& alloc, std::size_t N)
  {
    return { std::allocate_shared<T, Alloc>(std::forward<decltype(alloc)>(alloc), std::forward<decltype(N)>(N)) };
  }

  template <typename T, typename Alloc>
  constexpr move_shared_ptr<T> allocate_move_shared(const Alloc& alloc)
  {
    return { std::allocate_shared<T, Alloc>(std::forward<decltype(alloc)>(alloc)) };
  }

  template <typename T, typename Alloc>
  constexpr move_shared_ptr<T> allocate_move_shared(
    const Alloc& alloc,
    std::size_t N,
    const std::remove_extent_t<T>& u)
  {
    return { std::allocate_shared<T, Alloc>(std::forward<decltype(alloc)>(alloc), std::forward<decltype(N)>(N), std::forward<decltype(u)>(u)) };
  }

  template <typename T, typename Alloc>
  constexpr move_shared_ptr<T> allocate_move_shared(
    const Alloc& alloc,
    const std::remove_extent_t<T>& u)
  {
    return { std::allocate_shared<T, Alloc>(std::forward<decltype(alloc)>(alloc), std::forward<decltype(u)>(u)) };
  }

  template <typename T, typename Alloc>
  constexpr move_shared_ptr<T> allocate_move_shared_default_init(const Alloc& alloc)
  {
    return { std::allocate_shared_default_init<T, Alloc>(std::forward<decltype(alloc)>(alloc)) };
  }

  template <typename T, typename Alloc>
  constexpr move_shared_ptr<T> allocate_move_shared_default_init(const Alloc& alloc, std::size_t N)
  {
    return { std::allocate_shared_default_init<T, Alloc>(std::forward<decltype(alloc)>(alloc), std::forward<decltype(N)>(N)) };
  }
#endif //0



  template <typename T, typename... Args>
  constexpr move_only_shared_ptr<T> make_move_only_shared(Args&&... args)
  {
    return { std::make_shared<T>(std::forward<Args>(args)...) };
  }

  template <typename T>
  constexpr move_only_shared_ptr<T> make_move_only_shared()
  {
    return { std::make_shared<T>() };
  }

  template <typename T>
  constexpr move_only_shared_ptr<T> make_move_only_shared(std::size_t N)
  {
    return { std::make_shared<T>(std::forward<decltype(N)>(N)) };
  }

#if 0

  // needs C++ 20

  template <typename T>
  constexpr move_only_shared_ptr<T> make_move_only_shared(std::size_t N, const std::remove_extent_t<T>& u)
  {
    return { std::make_shared<T>(std::forward<decltype(N)>(N), std::forward<decltype(u)>(u)) };
  }

  template <typename T>
  constexpr move_only_shared_ptr<T> make_move_only_shared(const std::remove_extent_t<T>& u)
  {
    return { std::make_shared<T>(std::forward<decltype(u)>(u)) };
  }

  template <typename T>
  constexpr move_only_shared_ptr<T> make_move_only_shared_default_init()
  {
    return { std::make_shared_default_init<T>() };
  }

  template <typename T>
  constexpr move_only_shared_ptr<T> make_move_only_shared_default_init(std::size_t N)
  {
    return { std::make_shared_default_init<T>(std::forward<decltype(N)>(N)) };
  }
#endif //0

  template <typename T, typename Alloc, typename ... Args>
  constexpr move_only_shared_ptr<T> allocate_move_only_shared(const Alloc& alloc, Args&&... args)
  {
    return { std::allocate_shared<T, Alloc>(std::forward<decltype(alloc)>(alloc), std::forward<Args>(args)...) };
  }

#if 0

  // needs C++ 20

  template <typename T, typename Alloc>
  constexpr move_only_shared_ptr<T> allocate_move_only_shared(const Alloc& alloc, std::size_t N)
  {
    return { std::allocate_shared<T, Alloc>(std::forward<decltype(alloc)>(alloc), std::forward<decltype(N)>(N)) };
  }

  template <typename T, typename Alloc>
  constexpr move_only_shared_ptr<T> allocate_move_only_shared(const Alloc& alloc)
  {
    return { std::allocate_shared<T, Alloc>(std::forward<decltype(alloc)>(alloc)) };
  }

  template <typename T, typename Alloc>
  constexpr move_only_shared_ptr<T> allocate_move_only_shared(
    const Alloc& alloc,
    std::size_t N,
    const std::remove_extent_t<T>& u)
  {
    return { std::allocate_shared<T, Alloc>(std::forward<decltype(alloc)>(alloc), std::forward<decltype(N)>(N), std::forward<decltype(u)>(u)) };
  }

  template <typename T, typename Alloc>
  constexpr move_only_shared_ptr<T> allocate_move_only_shared(
    const Alloc& alloc,
    const std::remove_extent_t<T>& u)
  {
    return { std::allocate_shared<T, Alloc>(std::forward<decltype(alloc)>(alloc), std::forward<decltype(u)>(u)) };
  }

  template <typename T, typename Alloc>
  constexpr move_only_shared_ptr<T> allocate_move_only_shared_default_init(const Alloc& alloc)
  {
    return { std::allocate_shared_default_init<T, Alloc>(std::forward<decltype(alloc)>(alloc)) };
  }

  template <typename T, typename Alloc>
  constexpr move_only_shared_ptr<T> allocate_move_only_shared_default_init(const Alloc& alloc, std::size_t N)
  {
    return { std::allocate_shared_default_init<T, Alloc>(std::forward<decltype(alloc)>(alloc), std::forward<decltype(N)>(N)) };
  }
#endif //0

  template <typename T, typename U>
  constexpr bool operator==(const move_only_shared_ptr<T>& lhs, const move_only_shared_ptr<U>& rhs) noexcept
  {
    return lhs.ptr_ == rhs.ptr_;
  }
  template <typename T, typename U>
  constexpr bool operator!=(const move_only_shared_ptr<T>& lhs, const move_only_shared_ptr<U>& rhs) noexcept
  {
    return lhs.ptr_ != rhs.ptr_;
  }
  template <typename T, typename U>
  constexpr bool operator<(const move_only_shared_ptr<T>& lhs, const move_only_shared_ptr<U>& rhs) noexcept
  {
    return lhs.ptr_ < rhs.ptr_;
  }
  template <typename T, typename U>
  constexpr bool operator>(const move_only_shared_ptr<T>& lhs, const move_only_shared_ptr<U>& rhs) noexcept
  {
    return lhs.ptr_ > rhs.ptr_;
  }
  template <typename T, typename U>
  constexpr bool operator<=(const move_only_shared_ptr<T>& lhs, const move_only_shared_ptr<U>& rhs) noexcept
  {
    return lhs.ptr_ < rhs.ptr_;
  }
  template <typename T, typename U>
  constexpr bool operator>=(const move_only_shared_ptr<T>& lhs, const move_only_shared_ptr<U>& rhs) noexcept
  {
    return lhs.ptr_ >= rhs.ptr_;
  }

  template <typename T>
  constexpr decltype(auto) share(const move_only_shared_ptr<T>& r) noexcept
  {
    return r.share();
  }

  template <typename T>
  constexpr decltype(auto) weak(const move_only_shared_ptr<T>& r) noexcept
  {
    return r.weak();
  }

  template <typename T>
  constexpr decltype(auto) share(const std::shared_ptr<T>& r) noexcept
  {
    return r;
  }

  template <typename T>
  constexpr decltype(auto) weak(const std::shared_ptr<T>& r) noexcept
  {
    return std::weak_ptr<T>(r);
  }

} // namespace zs

namespace std
{

  template <typename T, typename U>
  constexpr zs::move_only_shared_ptr<T> static_pointer_cast(const zs::move_only_shared_ptr<U>& r) noexcept
  {
    return { std::static_pointer_cast<T>(r.ptr()) };
  }

  template <typename T, typename U>
  constexpr zs::move_only_shared_ptr<T> dynamic_pointer_cast(const zs::move_only_shared_ptr<U>& r) noexcept
  {
    return { std::dynamic_pointer_cast<T>(r.ptr()) };
  }

  template <typename T, typename U>
  constexpr zs::move_only_shared_ptr<T> const_pointer_cast(const zs::move_only_shared_ptr<U>& r) noexcept
  {
    return { std::const_pointer_cast<T>(r.ptr()) };
  }

  template <typename T, typename U>
  constexpr zs::move_only_shared_ptr<T> reinterpret_pointer_cast(const zs::move_only_shared_ptr<U>& r) noexcept
  {
    return { std::reinterpret_pointer_cast<T>(r.ptr()) };
  }

  template <typename T>
  struct hash<zs::move_only_shared_ptr<T>> {
    constexpr decltype(auto) operator()(const zs::move_only_shared_ptr<T>& r) const noexcept
    {
      return hash<std::shared_ptr<T>>()(r.ptr());
    }
  };

  template <typename T>
  constexpr void swap(zs::move_only_shared_ptr<T>& lhs, zs::move_only_shared_ptr<T>& rhs) noexcept
  {
    lhs.swap(rhs);
  }

  template <typename Deleter, typename T>
  constexpr decltype(auto) get_deleter(const zs::move_only_shared_ptr<T>& p) noexcept
  {
    return std::get_deleter<Deleter, T>(p.ptr());
  }

  template <class T, class U, class V>
  decltype(auto) operator<<(std::basic_ostream<U, V>& os, const zs::move_only_shared_ptr<T>& ptr)
  {
    return os << ptr.ptr();
  }

} // namespace zs
