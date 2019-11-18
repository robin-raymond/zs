
#include <zs/move_shared_ptr.h>

#include "common.h"

#include <iostream>
#include <optional>
#include <functional>

#include <sstream>

namespace zsTest
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  struct MoveSharedPtrTestBasics
  {
    struct Values
    {
      size_t baseConstructed_{};
      size_t baseDestructed_{};
      size_t calledBaseCtorDefault_{};
      size_t calledBaseCtorOne_{};

      size_t derivedConstructed_{};
      size_t derivedDestructed_{};
      size_t calledDerivedCtorDefault_{};
      size_t calledDerivedCtorOne_{};
      size_t calledDerivedDtor_{};
    };
    inline static std::optional<Values> values_{};

    //-------------------------------------------------------------------------
    void reset()
    {
      values_.reset();
      values_.emplace();
    }

    //-------------------------------------------------------------------------
    struct Base
    {
      int b_{};
      Base() noexcept
      {
        ++values_->calledBaseCtorDefault_;
        ++values_->baseConstructed_;
      }

      Base(int value) noexcept
      {
        TEST(value == 42);
        ++values_->calledBaseCtorOne_;
        ++values_->baseConstructed_;
      }

      ~Base() noexcept
      {
        ++values_->baseDestructed_;
      }

      int value() const noexcept { return 42; }
    };

    //-------------------------------------------------------------------------
    struct Derived : public Base
    {
      int d_{};
      Derived() noexcept :
        Base{}
      {
        ++values_->calledDerivedCtorDefault_;
        ++values_->derivedConstructed_;
      }

      Derived(int value) noexcept :
        Base{ value }
      {
        ++values_->calledDerivedCtorOne_;
        ++values_->derivedConstructed_;
      }

      ~Derived() noexcept
      {
        ++values_->derivedDestructed_;
      }
    };

    //-------------------------------------------------------------------------
    void testCtor() noexcept(false)
    {
      {
        zs::move_only_shared_ptr<Base> result{ zs::make_move_shared<Base>() };
        TEST(42 == result->value());

        TEST(1 == values_->calledBaseCtorDefault_);
        TEST(0 == values_->baseDestructed_);
      }
      TEST(1 == values_->baseDestructed_);
      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void testMoreCtor() noexcept(false)
    {
      // check default constructor
      {
        zs::move_only_shared_ptr<Base> r;
        TEST(nullptr == r.ptr());
        TEST(0 == values_->calledBaseCtorDefault_);
      }

      // check nullptr constructor
      {
        zs::move_only_shared_ptr<Base> r{ nullptr };
        TEST(nullptr == r.ptr());
        TEST(0 == values_->calledBaseCtorDefault_);
      }
#ifdef ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_CTOR_COPY
      {
        zs::move_only_shared_ptr<Base> r;
        TEST(nullptr == r.ptr());
        TEST(0 == values_->calledCtorDefault_);

        zs::move_only_shared_ptr<Base> s{ r };
        TEST(nullptr == r.ptr());
        TEST(0 == values_->calledCtorDefault_);
      }
#endif //ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_CTOR_COPY
#ifdef ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_CTOR_Y_COPY
      {
        zs::move_only_shared_ptr<Derived> r;
        TEST(nullptr == r.ptr());
        TEST(0 == values_->calledCtorDefault_);

        zs::move_only_shared_ptr<Base> s{ r };
        TEST(nullptr == r.ptr());
        TEST(0 == values_->calledCtorDefault_);
      }
#endif //ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_CTOR_Y_COPY

      // check move constructor
      {
        {
          reset();
          zs::move_only_shared_ptr<Base> r(new Base);
          TEST(nullptr != r.ptr());
          TEST(1 == values_->baseConstructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(0 == values_->baseDestructed_);

          zs::move_only_shared_ptr<Base> s{ std::move(r) };
          TEST(nullptr == r.ptr());
          TEST(nullptr != s.ptr());
          TEST(0 == values_->baseDestructed_);
          TEST(0 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(0 == r.use_count());
          TEST(1 == s.use_count());
        }
        TEST(1 == values_->baseDestructed_);
      }

      // check weak pointer move from derived into base class
      {
        {
          reset();
          zs::move_only_shared_ptr<Derived> r(new Derived);
          TEST(nullptr != r.ptr());
          TEST(0 == values_->baseDestructed_);
          TEST(1 == values_->baseConstructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == r.use_count());

          std::weak_ptr<Derived> w1(zs::weak(r));
          zs::move_only_shared_ptr<Derived> dw1(w1);
          zs::move_only_shared_ptr<Base> bw1(w1);
          TEST(3 == r.use_count());

          zs::move_only_shared_ptr<Base> s{ std::move(r) };
          TEST(nullptr == r.ptr());
          TEST(nullptr != s.ptr());
          TEST(0 == values_->baseDestructed_);
          TEST(1 == values_->baseConstructed_);
          TEST(1 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(3 == s.use_count());
          TEST(0 == r.use_count());

          std::weak_ptr<Base> w2(zs::weak(s));
          zs::move_only_shared_ptr<Base> bw2(w2);
          TEST(4 == s.use_count());
        }
        TEST(1 == values_->baseDestructed_);
        TEST(1 == values_->derivedDestructed_);
      }

      // check move constructor of derived into base class
      {
        {
          reset();

          zs::move_only_shared_ptr<Derived> r(new Derived);
          TEST(nullptr != r.ptr());
          TEST(0 == values_->baseDestructed_);
          TEST(1 == values_->baseConstructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == r.use_count());

          zs::move_only_shared_ptr<Derived> d(std::move(r));
          TEST(nullptr == r.ptr());
          TEST(nullptr != d.ptr());
          TEST(0 == values_->baseDestructed_);
          TEST(1 == values_->baseConstructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == d.use_count());

          zs::move_only_shared_ptr<Base> b{ std::move(d) };
          TEST(nullptr == r.ptr());
          TEST(nullptr == d.ptr());
          TEST(nullptr != b.ptr());
          TEST(0 == values_->baseDestructed_);
          TEST(1 == values_->baseConstructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(0 == r.use_count());
          TEST(0 == d.use_count());
          TEST(1 == b.use_count());
        }
        TEST(1 == values_->baseDestructed_);
        TEST(1 == values_->derivedDestructed_);
      }

      // check move of shared_ptr into constructor
      {
        {
          reset();

          std::shared_ptr<Derived> r(new Derived);
          TEST(nullptr != r);
          TEST(0 == values_->baseDestructed_);
          TEST(1 == values_->baseConstructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == r.use_count());

          zs::move_only_shared_ptr<Derived> d(std::move(r));
          TEST(nullptr == r);
          TEST(nullptr != d.ptr());
          TEST(0 == values_->baseDestructed_);
          TEST(1 == values_->baseConstructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == d.use_count());
        }
        TEST(1 == values_->baseDestructed_);
        TEST(1 == values_->derivedDestructed_);
      }

      // check move of shared_ptr into constructor which takes base class
      {
        {
          reset();

          std::shared_ptr<Derived> r(new Derived);
          TEST(nullptr != r);
          TEST(0 == values_->baseDestructed_);
          TEST(1 == values_->baseConstructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == r.use_count());

          zs::move_only_shared_ptr<Base> b(std::move(r));
          TEST(nullptr == r);
          TEST(nullptr != b.ptr());
          TEST(0 == values_->baseDestructed_);
          TEST(1 == values_->baseConstructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == b.use_count());
        }
        TEST(1 == values_->baseDestructed_);
        TEST(1 == values_->derivedDestructed_);
      }

      // check move of unique ptr into constructor
      {
        {
          reset();

          std::unique_ptr<Derived> r(new Derived);
          TEST(nullptr != r);
          TEST(0 == values_->baseDestructed_);
          TEST(1 == values_->baseConstructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);

          zs::move_only_shared_ptr<Base> b(std::move(r));
          TEST(nullptr == r);
          TEST(nullptr != b.ptr());
          TEST(0 == values_->baseDestructed_);
          TEST(1 == values_->baseConstructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == b.use_count());
        }
        TEST(1 == values_->baseDestructed_);
        TEST(1 == values_->derivedDestructed_);
      }

      // check move of unique_ptr with custom deleter into constructor
      {
        reset();

        struct DeallocValues
        {
          size_t deallocs_{};
        };
        static std::optional<DeallocValues> deallocValues_{};

        struct MyDeleter {
          void operator()(Derived* ptr) {
            ++(deallocValues_->deallocs_);
            delete ptr;
          }
        };
        deallocValues_.reset();
        deallocValues_.emplace();
        {
          std::unique_ptr<Derived, MyDeleter> r(new Derived);
          TEST(nullptr != r);
          TEST(0 == values_->baseDestructed_);
          TEST(1 == values_->baseConstructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(0 == deallocValues_->deallocs_);

          zs::move_only_shared_ptr<Base> b(std::move(r));
          TEST(nullptr == r);
          TEST(nullptr != b.ptr());
          TEST(0 == values_->baseDestructed_);
          TEST(1 == values_->baseConstructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == b.use_count());
          TEST(0 == deallocValues_->deallocs_);
        }
        TEST(1 == values_->baseDestructed_);
        TEST(1 == values_->derivedDestructed_);
        TEST(1 == deallocValues_->deallocs_);
      }

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    struct MallocatorCommon {
      struct Values {
        size_t totalAllocs_{};
        size_t totalDeallocs_{};
      };
      inline static std::optional<Values> values_{};

      static void reset()
      {
        values_.reset();
        values_.emplace();
      }
    };

    //-------------------------------------------------------------------------
    // https://en.cppreference.com/w/cpp/named_req/Allocator
    template <class T>
    struct Mallocator {
      typedef T value_type;
      Mallocator() = default;
      template <class U> constexpr Mallocator(const Mallocator<U>&) noexcept {}
      [[nodiscard]] T* allocate(std::size_t n) {
        ++(MallocatorCommon::values_->totalAllocs_);
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) throw std::bad_alloc();
        if (auto p = static_cast<T*>(std::malloc(n * sizeof(T)))) return p;
        throw std::bad_alloc();
      }
      void operator()(T* p) noexcept {
        ++(MallocatorCommon::values_->totalDeallocs_);
        std::free(p);
      }
      template <typename U>
      void operator()(U* p) noexcept {
        ++(MallocatorCommon::values_->totalDeallocs_);
        std::free(p);
      }
      void deallocate(T* p, std::size_t) noexcept {
        ++(MallocatorCommon::values_->totalDeallocs_);
        std::free(p);
      }
    };

    //-------------------------------------------------------------------------
    void testPtrCtor() noexcept(false)
    {
      bool didDelete{};

      // check constructor pointer memory adoption
      {
        {
          zs::move_only_shared_ptr<Base> r(new Derived);
          TEST(nullptr != r.ptr());
          TEST(0 == values_->baseDestructed_);
          TEST(1 == values_->baseConstructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
        }
        TEST(1 == values_->baseDestructed_);
        TEST(1 == values_->derivedDestructed_);
      }

      // check constructor pointer memory adoption with custom deleter
      {
        {
          reset();
          zs::move_only_shared_ptr<Base> r(new Derived, [&didDelete](Derived* ptr) noexcept(false) {
            didDelete = true;
            delete ptr;
            });
          TEST(nullptr != r.ptr());
          TEST(0 == values_->baseDestructed_);
          TEST(1 == values_->baseConstructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(!didDelete);
        }
        TEST(1 == values_->baseDestructed_);
        TEST(1 == values_->derivedDestructed_);
        TEST(didDelete);
      }

      // check constructor pointer memory adoption with custom deleter and
      // custom allocator of the shared_ptr control block
      {
        {
          reset();
          MallocatorCommon::reset();
          didDelete = false;

          // The Mallocator object for type Derived is not actually used
          // (directly). Instead a new object of Mallocator with an
          // implementation specific rebinding of the first template parameter
          // to the std::shared_ptr control block is constructed using
          // template rebinding magic.
          zs::move_only_shared_ptr<Base> r(new Derived, [&didDelete](Derived* ptr) noexcept(false) {
            didDelete = true;
            delete ptr;
            }, Mallocator<Derived>{});
          TEST(nullptr != r.ptr());
          TEST(0 == values_->baseDestructed_);
          TEST(1 == values_->baseConstructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == MallocatorCommon::values_->totalAllocs_);
          TEST(0 == MallocatorCommon::values_->totalDeallocs_);
          TEST(!didDelete);
        }
        TEST(1 == values_->baseDestructed_);
        TEST(1 == values_->derivedDestructed_);
        TEST(1 == MallocatorCommon::values_->totalAllocs_);
        TEST(1 == MallocatorCommon::values_->totalDeallocs_);
        TEST(didDelete);
      }

      // check nullptr constructor with custom deleter (shared resource deleter)
      {
        {
          reset();
          didDelete = false;

          zs::move_only_shared_ptr<Base> r(nullptr, [&didDelete](Base* ptr) noexcept(false) {
            didDelete = true;
            TEST(nullptr == ptr);
            delete ptr;
            });
          TEST(nullptr == r.ptr());
          TEST(0 == values_->baseDestructed_);
          TEST(0 == values_->baseConstructed_);
          TEST(0 == values_->calledBaseCtorDefault_);
          TEST(0 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == r.use_count());
          TEST(!didDelete);
        }
        TEST(0 == values_->baseDestructed_);
        TEST(0 == values_->derivedDestructed_);
        TEST(didDelete);
      }

      // custom allocator of the shared_ptr control block for a nullptr
      // resource and custom deleter for that block
      {
        {
          reset();
          MallocatorCommon::reset();

          // The Mallocator object for type Derived is not actually used
          // (directly). Instead a new object of Mallocator with an
          // implementation specific rebinding of the first template parameter
          // to the std::shared_ptr control block is constructed using
          // template rebinding magic.
          zs::move_only_shared_ptr<Base> r(nullptr, Mallocator<Derived>{}, Mallocator<Derived>{});
          TEST(nullptr == r.ptr());
          TEST(1 == r.use_count());
          TEST(0 == values_->baseDestructed_);
          TEST(0 == values_->calledBaseCtorDefault_);
          TEST(0 == values_->derivedConstructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == MallocatorCommon::values_->totalAllocs_);
          TEST(0 == MallocatorCommon::values_->totalDeallocs_);
        }
        TEST(0 == values_->baseDestructed_);
        TEST(0 == values_->derivedDestructed_);
        TEST(1 == MallocatorCommon::values_->totalAllocs_);
        // one attempts to delete nullptr and the other the control block
        TEST(2 == MallocatorCommon::values_->totalDeallocs_);
      }

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void testPtrElementCtor() noexcept(false)
    {
      // move from an existing object element
#ifdef ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_CTOR_COPY_WITH_ELEMENT
      {
        int* ptr = new int[10];
        zs::move_only_shared_ptr<int[]> r(ptr, ptr[1]);
        zs::move_only_shared_ptr<int[]> s(r, &(ptr[2]));
      }
#endif //ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_CTOR_COPY_WITH_ELEMENT
      // copy from an existing object element
      {
        int* ptr = new int[10]{};
        std::shared_ptr<int[]> r(ptr);
        zs::move_only_shared_ptr<int[]> s(r, &(ptr[2]));

        TEST(2 == r.use_count());
        TEST(2 == s.use_count());

        r[2] = 100;
        TEST(100 == s[0]);
      }
      // move from an existing object element
      {
        int* ptr = new int[10]{};
        std::shared_ptr<int[]> r(ptr);
        zs::move_only_shared_ptr<int[]> s(std::move(r), &(ptr[2]));

        TEST(0 == r.use_count());
        TEST(1 == s.use_count());

        ptr[2] = 100;
        TEST(100 == s[0]);
      }
      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void testAssignment() noexcept(false)
    {
#ifdef ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_ASSIGNMENT
      {
        zs::move_only_shared_ptr<int> r;
        zs::move_only_shared_ptr<int> s(r);
      }
#endif //ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_ASSIGNMENT
#ifdef ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_ASSIGNMENT_DERIVED_TO_BASE
      {
        zs::move_only_shared_ptr<Derived> r;
        zs::move_only_shared_ptr<Base> s(r);
      }
#endif //ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_ASSIGNMENT_DERIVED_TO_BASE
      {
        zs::move_only_shared_ptr<Derived> r(new Derived);
        TEST(nullptr != r.ptr());
        TEST(1 == r.use_count());
        TEST(0 == values_->baseDestructed_);
        TEST(0 == values_->derivedDestructed_);
        TEST(1 == values_->calledBaseCtorDefault_);
        TEST(1 == values_->derivedConstructed_);

        r = nullptr;
        TEST(nullptr == r.ptr());
        TEST(0 == r.use_count());
        TEST(1 == values_->baseDestructed_);
        TEST(1 == values_->derivedDestructed_);
        TEST(1 == values_->calledBaseCtorDefault_);
        TEST(1 == values_->derivedConstructed_);
      }

      {
        reset();
        {
          zs::move_only_shared_ptr<Base> r(new Base);
          TEST(nullptr != r.ptr());
          TEST(1 == r.use_count());
          TEST(0 == values_->baseDestructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(0 == values_->derivedConstructed_);

          std::shared_ptr<Derived> s(new Derived);
          r = s;
          TEST(nullptr != r.ptr());
          TEST(nullptr != s.get());
          TEST(2 == r.use_count());
          TEST(1 == values_->baseDestructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(2 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
        }
        TEST(2 == values_->baseDestructed_);
        TEST(1 == values_->derivedDestructed_);
        TEST(2 == values_->calledBaseCtorDefault_);
        TEST(1 == values_->derivedConstructed_);
      }

      // check assign of unique ptr into constructor
#ifdef ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_UNIQUE_PTR_ASSIGNMENT
      {
        std::unique_ptr<Derived> r;
        zs::move_only_shared_ptr<Base> b(new Base);
        b = r;
      }
#endif // ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_UNIQUE_PTR_ASSIGNMENT

      // verify move assignment (simple)
      {
        reset();
        {
          zs::move_only_shared_ptr<Derived> r(new Derived);
          TEST(nullptr != r.ptr());
          TEST(1 == r.use_count());
          TEST(0 == values_->baseDestructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);

          zs::move_only_shared_ptr<Derived> s(new Derived);
          TEST(nullptr != s.ptr());
          TEST(1 == s.use_count());
          TEST(0 == values_->baseDestructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(2 == values_->calledBaseCtorDefault_);
          TEST(2 == values_->derivedConstructed_);

          s = std::move(r);
          TEST(0 == r.use_count());
          TEST(1 == s.use_count());
          TEST(1 == values_->baseDestructed_);
          TEST(1 == values_->derivedDestructed_);
          TEST(2 == values_->calledBaseCtorDefault_);
          TEST(2 == values_->derivedConstructed_);
        }
        TEST(2 == values_->baseDestructed_);
        TEST(2 == values_->derivedDestructed_);
        TEST(2 == values_->calledBaseCtorDefault_);
        TEST(2 == values_->derivedConstructed_);
      }

      // verify move assignment (different types)
      {
        reset();
        {
          zs::move_only_shared_ptr<Derived> r(new Derived);
          TEST(nullptr != r.ptr());
          TEST(1 == r.use_count());
          TEST(0 == values_->baseDestructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);

          zs::move_only_shared_ptr<Base> s(new Base);
          TEST(nullptr != s.ptr());
          TEST(1 == s.use_count());
          TEST(0 == values_->baseDestructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(2 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);

          s = std::move(r);
          TEST(0 == r.use_count());
          TEST(1 == s.use_count());
          TEST(1 == values_->baseDestructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(2 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
        }
        TEST(2 == values_->baseDestructed_);
        TEST(1 == values_->derivedDestructed_);
        TEST(2 == values_->calledBaseCtorDefault_);
        TEST(1 == values_->derivedConstructed_);
      }

      // verify move assignment from shared_ptr (different types)
      {
        reset();
        {
          std::shared_ptr<Derived> r(new Derived);
          TEST(nullptr != r.get());
          TEST(1 == r.use_count());
          TEST(0 == values_->baseDestructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);

          zs::move_only_shared_ptr<Base> s(new Base);
          TEST(nullptr != s.ptr());
          TEST(1 == s.use_count());
          TEST(0 == values_->baseDestructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(2 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);

          s = std::move(r);
          TEST(0 == r.use_count());
          TEST(1 == s.use_count());
          TEST(1 == values_->baseDestructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(2 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
        }
        TEST(2 == values_->baseDestructed_);
        TEST(1 == values_->derivedDestructed_);
        TEST(2 == values_->calledBaseCtorDefault_);
        TEST(1 == values_->derivedConstructed_);
      }

      // verify move assignment from unique_ptr with deleter (different types)
      {
        reset();
        {
          auto myDeleter = [](Derived* ptr) {
            delete ptr;
          };

          std::unique_ptr<Derived, decltype(myDeleter)> r(new Derived);
          TEST(nullptr != r.get());
          TEST(0 == values_->baseDestructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);

          zs::move_only_shared_ptr<Base> s(new Base);
          TEST(nullptr != s.ptr());
          TEST(1 == s.use_count());
          TEST(0 == values_->baseDestructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(2 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);

          s = std::move(r);
          TEST(1 == s.use_count());
          TEST(1 == values_->baseDestructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(2 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
        }
        TEST(2 == values_->baseDestructed_);
        TEST(1 == values_->derivedDestructed_);
        TEST(2 == values_->calledBaseCtorDefault_);
        TEST(1 == values_->derivedConstructed_);
      }

      // verify move assignment from unique_ptr (different types)
      {
        reset();
        {
          std::unique_ptr<Derived> r(new Derived);
          TEST(nullptr != r.get());
          TEST(0 == values_->baseDestructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(1 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);

          zs::move_only_shared_ptr<Base> s(new Base);
          TEST(nullptr != s.ptr());
          TEST(1 == s.use_count());
          TEST(0 == values_->baseDestructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(2 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);

          s = std::move(r);
          TEST(1 == s.use_count());
          TEST(1 == values_->baseDestructed_);
          TEST(0 == values_->derivedDestructed_);
          TEST(2 == values_->calledBaseCtorDefault_);
          TEST(1 == values_->derivedConstructed_);
        }
        TEST(2 == values_->baseDestructed_);
        TEST(1 == values_->derivedDestructed_);
        TEST(2 == values_->calledBaseCtorDefault_);
        TEST(1 == values_->derivedConstructed_);
      }

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void testIndex() noexcept(false)
    {
#ifdef ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_INDEX
      {
        int* ptr = new int{};
        zs::move_only_shared_ptr<int> values(ptr);

        values[5] = 6;
        TEST(6 == ptr[5]);

        ptr[2] = values[5];
        TEST(6 == ptr[2]);
    }
#endif //ZS_VERIFY_DOES_NOT_COMPILE_MOVE_ONLY_SHARED_PTR_INDEX

      {
        int* ptr = new int[10]{};
        zs::move_only_shared_ptr<int[]> values(ptr);

        values[5] = 6;
        TEST(6 == ptr[5]);

        ptr[2] = values[5];
        TEST(6 == ptr[2]);
      }

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void testOther() noexcept(false)
    {
      // check bool operator with no value present
      {
        zs::move_only_shared_ptr<int> r;
        if (r)
          TEST(!"ok");
        else
          TEST("ok");
      }

      // check bool operator with value present
      {
        zs::move_only_shared_ptr<int> r(new int{});
        bool value = static_cast<bool>(r);
        if (r)
          TEST("ok");
        else
          TEST(!"ok");
      }

      // check use count and reset
      {
        zs::move_only_shared_ptr<int> r(new int{});
        TEST(1 == r.use_count());
        zs::move_only_shared_ptr<int> s(r.share());
        TEST(2 == r.use_count());
        TEST(2 == s.use_count());
        zs::move_only_shared_ptr<int> t(std::move(r));
        TEST(0 == r.use_count());
        TEST(2 == s.use_count());
        TEST(2 == t.use_count());

        r.reset();
        TEST(0 == r.use_count());
        TEST(2 == s.use_count());
        TEST(2 == t.use_count());

        s.reset();
        TEST(0 == r.use_count());
        TEST(0 == s.use_count());
        TEST(1 == t.use_count());

        t.reset();
        TEST(0 == r.use_count());
        TEST(0 == s.use_count());
        TEST(0 == t.use_count());
      }

      // check reset and operator* with new values
      {
        int* ptr = new int{};
        zs::move_only_shared_ptr<int> r(ptr);

        *r = 5;
        TEST(5 == *ptr);

        int* ptr2 = new int{ 10 };
        r.reset(ptr2);

        int value2 = *ptr2;
        TEST(10 == value2);
      }

      // check operator->
      {
        struct Foo
        {
          int value_{};
        };

        zs::move_only_shared_ptr<Foo> r(new Foo);
        TEST(0 == r->value_);

        r->value_ = 5;
        TEST(5 == r->value_);

        int value = r->value_;
        TEST(5 == value);
      }

      // check get()
      {
        int* ptr = new int{};
        zs::move_only_shared_ptr<int> r(ptr);

        *(r.get()) = 5;
        TEST(5 == *ptr);

        int* ptr2 = new int{ 10 };
        r.reset(ptr2);

        int *value2 = r.get();
        TEST(10 == *value2);
      }

      // check ptr()
      {
        zs::move_only_shared_ptr<int> r(new int{15});
        auto s = r.ptr();
        static_assert(std::is_same_v<decltype(s), std::shared_ptr<int>>);

        TEST(15 == *s);
      }

      // check share()
      {
        zs::move_only_shared_ptr<int> r(new int{ 15 });
        zs::move_only_shared_ptr<int> s(r.share());
        TEST(r.use_count() == s.use_count());
        TEST(2 == r.use_count());
        TEST(15 == *s);

        *s = 11;
        TEST(11 == *r);
      }

      // check weak
      {
        zs::move_only_shared_ptr<int> r(new int{ 15 });
        auto s = r.weak();
        static_assert(std::is_same_v<decltype(s), std::weak_ptr<int>>);
        TEST(1 == r.use_count());

        std::shared_ptr<int> t(s);
        TEST(2 == r.use_count());

        TEST(15 == *t);
        TEST(15 == *(s.lock()));

        t.reset();
        TEST(1 == r.use_count());

        r.reset();
        TEST(0 == r.use_count());

        std::shared_ptr<int> u(s.lock());
        if (u)
          TEST(!"ok");
        else
          TEST("ok");
      }

      // check owner before
      {
        {
          int* ptr = new int[10]{};
          zs::move_only_shared_ptr<int[]> r(ptr);
          zs::move_only_shared_ptr<int[]> s(r.share(), &(ptr[2]));

          // r and s do not point to the same entry
          TEST(r.get() != s.get());
          TEST(!r.owner_before(s));
          TEST(!s.owner_before(r));
          // r and s are using the same control block so they are considered
          // "equal" thus neither are before or after the other.
          TEST(!((r.owner_before(s)) || (s.owner_before(r))));
        }
        {
          int* ptr1 = new int{};
          int* ptr2 = new int{};
          zs::move_only_shared_ptr<int[]> r(ptr1);
          zs::move_only_shared_ptr<int[]> s(ptr2);

          TEST(r.get() != s.get());
          // r and s are using different control blocks so either r or s will
          // become before the other (which comes first is unspecified).
          TEST((r.owner_before(s)) || (s.owner_before(r)));
        }
      }

      // check owner before with different types specified
      {
        struct Base1
        {
          int bogus1_;
        };

        struct Base2
        {
          int bogus2_;
        };

        struct DerivedBoth : public Base1, public Base2
        {
          int bogus3_;
        };

        {
          zs::move_only_shared_ptr<DerivedBoth> r(new DerivedBoth);
          zs::move_only_shared_ptr<Base1> s(r.ptr());
          zs::move_only_shared_ptr<Base2> t(r.ptr());

          TEST(static_cast<void *>(s.get()) != static_cast<void*>(t.get()));
          // r and s are using the same control block so they are considered
          // "equal" thus neither are before or after the other.
          TEST(!((r.owner_before(s)) || (s.owner_before(r))));
          TEST(!((r.owner_before(t)) || (t.owner_before(r))));
        }
        {
          zs::move_only_shared_ptr<DerivedBoth> r(new DerivedBoth);
          std::shared_ptr<Base1> s(r.ptr());
          std::shared_ptr<Base2> t(r.ptr());

          TEST(static_cast<void*>(s.get()) != static_cast<void*>(t.get()));
          // r and s are using the same control block so they are considered
          // "equal" thus neither are before or after the other.
          TEST(!((r.owner_before(s)) || (s.owner_before(r.ptr()))));
          TEST(!((r.owner_before(t)) || (t.owner_before(r.ptr()))));
        }
        {
          zs::move_only_shared_ptr<DerivedBoth> r(new DerivedBoth);
          std::weak_ptr<Base1> s(r.ptr());
          std::weak_ptr<Base2> t(r.ptr());

          TEST(static_cast<void*>(s.lock().get()) != static_cast<void*>(t.lock().get()));
          // r and s are using the same control block so they are considered
          // "equal" thus neither are before or after the other.
          TEST(!((r.owner_before(s)) || (s.owner_before(r.ptr()))));
          TEST(!((r.owner_before(t)) || (t.owner_before(r.ptr()))));
        }
      }

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void testSpeed() noexcept(false)
    {
      constexpr static size_t lotsOTimes = 1000000000;
      auto passerMove{ [](zs::move_only_shared_ptr<Base> value, volatile int& ignoredCount) noexcept -> auto {
        ++ignoredCount;
        return value;
      } };
      auto passerRef{ [] (zs::move_only_shared_ptr<Base> &value, volatile int& ignoredCount) noexcept -> auto {
        ++ignoredCount;
        return std::move(value);
      } };
      auto passerShared{ [] (std::shared_ptr<Base> value, volatile int& ignoredCount) noexcept -> auto {
        ++ignoredCount;
        return value;
      } };
      auto passerSharedMove{ [] (std::shared_ptr<Base> value, volatile int& ignoredCount) noexcept -> auto {
        ++ignoredCount;
        return value;
      } };

      zs::move_only_shared_ptr<Base> value{ zs::make_move_shared<Base>() };

      auto count1 = diff(now(), now());
      auto count2 = diff(now(), now());
      auto count3 = diff(now(), now());
      auto count4 = diff(now(), now());

      // volatile should prevent the optmizer from completely
      // optimizing the code away
      volatile int ignoredCount = 0;

      {
        auto start = now();

        for (size_t index = 0; index < lotsOTimes; ++index) {
          value = passerRef(value, ignoredCount);
        }

        auto end = now();

        count1 = diff(start, end);
      }

      {
        auto start = now();

        for (size_t index = 0; index < lotsOTimes; ++index) {
          value = passerMove(std::move(value), ignoredCount);
        }

        auto end = now();

        count2 = diff(start, end);
      }

      auto valueShared = value.ptr();

      {
        auto start = now();

        for (size_t index = 0; index < lotsOTimes; ++index) {
          valueShared = passerShared(valueShared, ignoredCount);
        }

        auto end = now();

        count3 = diff(start, end);
      }

      {
        auto start = now();

        for (size_t index = 0; index < lotsOTimes; ++index) {
          value = passerSharedMove(std::move(valueShared), ignoredCount);
        }

        auto end = now();

        count4 = diff(start, end);
      }

      std::cout << "\n";
      std::cout << "TIME DURATION (move by ref):     " << count1.count() << "\n";
      std::cout << "TIME DURATION (move by move):    " << count2.count() << "\n";
      std::cout << "TIME DURATION (shared by value): " << count3.count() << "\n";
      std::cout << "TIME DURATION (shared move):     " << count4.count() << "\n";
      std::cout << "\n";

      TEST(count1 < count2);
      TEST(count2 < count3);

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void testFunctions() noexcept(false)
    {
#if 0
      // needs C++ 20
      {
        struct A
        {
          int a_;
          int b_;
          int c_;
        };
        auto r = zs::make_move_shared<A>({ 1, 2, 3 });
        TEST(1 == r->a_);
        TEST(2 == r->b_);
        TEST(3 == r->c_);
      }
#endif //0
      {
        struct B
        {
          int a_;
          int b_;
          int c_;
          B(int a, int b, int c) noexcept :
            a_(a), b_(b), c_(c) {}
        };
        auto r = zs::make_move_only_shared<B>( 1, 2, 3 );
        TEST(1 == r->a_);
        TEST(2 == r->b_);
        TEST(3 == r->c_);
      }
      {
        struct C
        {
          int a_{ 1 };
          int b_{ 2 };
          int c_{ 3 };
        };
        auto r = zs::make_move_only_shared<C>();
        TEST(1 == r->a_);
        TEST(2 == r->b_);
        TEST(3 == r->c_);
      }
#if 0
      // needs C++ 20
      {
        auto r = zs::make_move_only_shared<int[]>(static_cast<std::size_t>(5));
        r[0] = 1;
        r[1] = 2;
        r[2] = 3;
        r[3] = 4;
        r[4] = 5;

        TEST(1 == r[0]);
        TEST(2 == r[1]);
        TEST(3 == r[2]);
        TEST(4 == r[3]);
        TEST(5 == r[4]);
      }
#endif //0
      {
        struct D
        {
          int a_;
          int b_;
          int c_;
          D(int a, int b, int c) noexcept :
            a_(a), b_(b), c_(c) {}
        };

        MallocatorCommon::reset();

        {
          auto r = zs::allocate_move_only_shared<D>(Mallocator<D>{}, 1, 2, 3);
          TEST(1 == r->a_);
          TEST(2 == r->b_);
          TEST(3 == r->c_);
          TEST(1 == MallocatorCommon::values_->totalAllocs_);
          TEST(0 == MallocatorCommon::values_->totalDeallocs_);
        }
        TEST(1 == MallocatorCommon::values_->totalAllocs_);
        TEST(1 == MallocatorCommon::values_->totalDeallocs_);
      }

      {
        int* ptr1 = new int{};
        int* ptr2 = new int{};
        if (ptr2 < ptr1)
          std::swap(ptr1, ptr2);

        zs::move_only_shared_ptr<int> r(ptr1);
        zs::move_only_shared_ptr<int> s(ptr2);

        TEST(!(r == s));
        TEST(r != s);
        TEST(r < s);
        TEST(!(r > s));
        TEST(r <= s);
        TEST(!(r >= s));
      }

      {
        zs::move_only_shared_ptr<int> r(new int{});
        TEST(1 == r.use_count());
        zs::move_only_shared_ptr<int> s(zs::share(r));
        TEST(2 == r.use_count());
        TEST(2 == s.use_count());
      }
      {
        zs::move_only_shared_ptr<int> r(new int{});
        TEST(1 == r.use_count());
        std::weak_ptr<int> s(zs::weak(r));
        TEST(1 == r.use_count());

        zs::move_only_shared_ptr<int> t(s.lock());
        TEST(2 == r.use_count());
        TEST(2 == t.use_count());
      }
      {
        zs::move_only_shared_ptr<int> r(new int{});
        TEST(1 == r.use_count());
        std::weak_ptr<int> s(zs::weak(r));
        TEST(1 == r.use_count());

        zs::move_only_shared_ptr<int> t(s.lock());
        TEST(2 == r.use_count());
        TEST(2 == t.use_count());
      }

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void testStdFunction() noexcept(false)
    {
      struct B
      {
        int b_{};
        virtual ~B() {};

        B() noexcept {};
        B(int b) noexcept :
          b_(b) {}
      };

      struct D : public B
      {
        int d_{};

        D() noexcept {}
        D(int b, int d) noexcept :
          B(b), d_(d) {};
      };

      // check pointer casting
      {
        zs::move_only_shared_ptr<B> r(new D);
        r->b_ = 17;

        auto s = std::static_pointer_cast<D>(r);
        TEST(17 == s->b_);
        s->d_ = 27;

        zs::move_only_shared_ptr<B> t(std::move(s));

        auto u = std::dynamic_pointer_cast<D>(t);
        TEST(27 == u->d_);

        zs::move_only_shared_ptr<const B> v;
        v = std::const_pointer_cast<const B>(t);

        TEST(17 == v->b_);

        zs::move_only_shared_ptr<void> w(t.share());

        auto x = std::reinterpret_pointer_cast<B>(w);
        TEST(17 == x->b_);
      }

      // check std::swap
      {
        zs::move_only_shared_ptr<B> r(new D{ 19,23 });
        zs::move_only_shared_ptr<B> s(new D{ 13,27 });

        TEST(19 == r->b_);
        TEST(13 == s->b_);

        std::swap(r, s);

        TEST(19 == s->b_);
        TEST(13 == r->b_);
      }

      // check hash routine
      {
        zs::move_only_shared_ptr<B> r(new D{ 19,23 });
        zs::move_only_shared_ptr<B> s(new D{ 13,27 });

        auto value1 = std::hash<decltype(r)>{}(r);
        auto value2 = std::hash<decltype(s)>{}(s);

        static_assert(sizeof(value1) == sizeof(value2));
        static_assert(std::is_same_v<decltype(value1), decltype(value2)>);
        static_assert(std::is_same_v<decltype(value1), size_t>);
      }

      // check iostream operator
      {
        zs::move_only_shared_ptr<B> r(new D{ 19,23 });

        std::stringstream ss;

        TEST(ss.str().length() == 0);
        ss << r;
        TEST(ss.str().length() > 0);
      }

      // std::get_deleter tested later via cpp reference code

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void runAll() noexcept(false)
    {
      auto runner{ [&](auto &&func) noexcept(false) { reset(); func(); } };

      runner([&]() { testCtor(); });
      runner([&]() { testMoreCtor(); });
      runner([&]() { testPtrCtor(); });
      runner([&]() { testPtrElementCtor(); });
      runner([&]() { testAssignment(); });
      runner([&]() { testIndex(); });
      runner([&]() { testOther(); });
      runner([&]() { testFunctions(); });
      runner([&]() { testStdFunction(); });
#ifndef _DEBUG
      runner([&]() { testSpeed(); });
#endif //_DEBUG
    }
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  struct MoveSharedPtrTestCppReference
  {
    //-------------------------------------------------------------------------
    void testCppReferenceConstructor() noexcept
    {
      // https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr

      struct Values
      {
        size_t fooCtor_{};
        size_t fooDtor_{};
        size_t dCall_{};
        size_t dCallLamba_{};
      };

      static std::optional<Values> values_{};
      auto reset{ []() noexcept {
        values_.reset();
        values_.emplace();
      } };
      reset();

      struct Foo {
        Foo() { ++values_->fooCtor_;  }
        ~Foo() { ++values_->fooDtor_; }
      };

      struct D {
        void operator()(Foo* p) const {
          ++values_->dCall_;
          delete p;
        }
      };

      {
        zs::move_only_shared_ptr<Foo> sh1;

        TEST(0 == values_->fooCtor_);
        TEST(0 == values_->fooDtor_);
      }

      {
        zs::move_only_shared_ptr<Foo> sh2(new Foo);
        TEST(1 == values_->fooCtor_);
        TEST(0 == values_->fooDtor_);
        TEST(1 == sh2.use_count());

        zs::move_only_shared_ptr<Foo> sh3(zs::share(sh2));
        TEST(1 == values_->fooCtor_);
        TEST(0 == values_->fooDtor_);
        TEST(2 == sh2.use_count());
        TEST(2 == sh3.use_count());
      }
      TEST(1 == values_->fooCtor_);
      TEST(1 == values_->fooDtor_);

      reset();

      {
        zs::move_only_shared_ptr<Foo> sh4(new Foo, D());

        TEST(1 == values_->fooCtor_);
        TEST(0 == values_->fooDtor_);

        zs::move_only_shared_ptr<Foo> sh5(new Foo, [](Foo *p) {
          TEST(0 == values_->dCall_);
          TEST(0 == values_->dCallLamba_);
          ++values_->dCallLamba_;
          delete p;
          });

        TEST(2 == values_->fooCtor_);
        TEST(0 == values_->fooDtor_);
      }
      TEST(2 == values_->fooCtor_);
      TEST(2 == values_->fooDtor_);
      TEST(1 == values_->dCall_);
      TEST(1 == values_->dCallLamba_);

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void testCppReferenceMakeShared() noexcept(false)
    {
      // https://en.cppreference.com/w/cpp/memory/shared_ptr/make_shared
      struct C
      {
        C(int i) : i(i) {} //< constructor needed (until C++20)
        int i;
      };

      auto sp = zs::make_move_shared<C>(12);

      static_assert(std::is_same_v<decltype(sp), zs::move_shared_ptr<C>>);

      TEST(12 == sp->i);

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void testCppReferenceReset() noexcept(false)
    {
      struct Values {
        int bar_{ -1 };
        size_t ctor_{};
        size_t dtor_{};
      };
      static std::optional<Values> values_;
      auto reset{ []() noexcept {
        values_.reset();
        values_.emplace();
      } };
      reset();

      // https://en.cppreference.com/w/cpp/memory/shared_ptr/reset
      struct Foo {
        Foo(int n = 0) noexcept : bar(n) {
          ++values_->ctor_;
          values_->bar_ = n;
        }
        ~Foo() {
          ++values_->dtor_;
        }
        int getBar() const noexcept { return bar; }
      private:
        int bar;
      };

      {
        zs::move_only_shared_ptr<Foo> sptr = zs::make_move_shared<Foo>(1);

        TEST(1 == values_->bar_);
        TEST(1 == sptr->getBar());
        TEST(1 == values_->ctor_);
        TEST(0 == values_->dtor_);

        // reset the shared_ptr, hand it a fresh instance of Foo
        // (the old instance will be destroyed after this call)
        sptr.reset(new Foo);

        TEST(2 == values_->ctor_);
        TEST(1 == values_->dtor_);

        TEST(0 == values_->bar_);
        TEST(0 == sptr->getBar());
      }
      TEST(2 == values_->ctor_);
      TEST(2 == values_->dtor_);

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void testCppReferenceGet() noexcept(false)
    {
      struct Values {
        int value_{};
      };
      std::optional<Values> values_;

      values_.reset();
      values_.emplace();

      auto checker{ [&](int* value) noexcept {
        values_->value_ = *value;
      } };

      values_->value_ = 0;

      int* pInt = new int(-42);
      checker(pInt);
      TEST(-42 == values_->value_);

      zs::move_only_shared_ptr<int> pShared = zs::make_move_shared<int>(42);
      checker(pShared.get());
      TEST(42 == values_->value_);

      delete pInt;
      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void testCppReferenceOperatorDeference() noexcept(false)
    {
      struct Values {
        size_t printCount_{};
        int count_{};
      };
      static std::optional<Values> values_;

      values_.reset();
      values_.emplace();

      struct Foo
      {
        Foo(int in) : a(in) { values_->count_ += a; }
        void print() const
        {
          ++values_->printCount_;
        }
        int a;
      };

      values_->printCount_ = 0;
      values_->count_ = 0;

      auto ptr = zs::make_move_shared<Foo>(10);
      TEST(10 == values_->count_);
      ptr->print();
      TEST(10 == values_->count_);
      TEST(1 == values_->printCount_);
      (*ptr).print();
      TEST(10 == values_->count_);
      TEST(2 == values_->printCount_);

      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void testCppReferencePointerCast() noexcept(false)
    {
      // https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

      struct Values {
        size_t baseCountF_{};
        size_t baseDtor_{};
        size_t lastA_{};
        size_t derivedCountF_{};
        size_t derivedDtor_{};
      };
      static std::optional<Values> values_;

      values_.reset();
      values_.emplace();

      struct Base
      {
        int a;
        virtual void f() const { ++values_->baseCountF_; }
        virtual ~Base() { ++values_->baseDtor_; }
      };

      struct Derived : Base
      {
        void f() const override { ++values_->derivedCountF_; }
        ~Derived() { ++values_->derivedDtor_; }
      };

      {
        auto basePtr = zs::make_move_shared<Base>();
        basePtr->f();

        TEST(1 == values_->baseCountF_);
        TEST(0 == values_->derivedCountF_);

        auto derivedPtr = zs::make_move_shared<Derived>();
        derivedPtr->f();

        TEST(1 == values_->baseCountF_);
        TEST(1 == values_->derivedCountF_);
        // static_pointer_cast to go up class hierarchy
        basePtr = std::static_pointer_cast<Base>(derivedPtr);
        basePtr->f();
        TEST(1 == values_->baseCountF_);
        TEST(2 == values_->derivedCountF_);

        // dynamic_pointer_cast to go down/across class hierarchy
        auto downcastedPtr = std::dynamic_pointer_cast<Derived>(basePtr);
        if (downcastedPtr)
          downcastedPtr->f();
        TEST(1 == values_->baseCountF_);
        TEST(3 == values_->derivedCountF_);

        // All pointers to derived share ownership
        TEST(3 == basePtr.use_count());
        TEST(3 == derivedPtr.use_count());
      }
      output(__FILE__ "::" __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    void testCppReferenceGetDeleter() noexcept(false)
    {
      // https://en.cppreference.com/w/cpp/memory/shared_ptr/get_deleter

      struct Values {
        size_t fooDeleterCalled_{};
        size_t ownsDeleter_{};
        size_t equalsDeleter_{};
        size_t deleterNull_{};
      };
      static std::optional<Values> values_;

      values_.reset();
      values_.emplace();

      struct Foo { int i; };

      struct FooDeleter {
        static void foo_deleter(Foo* p) noexcept(false)
        {
          ++values_->fooDeleterCalled_;
          delete p;
        }
      };

      {
        zs::move_only_shared_ptr<int> aptr;

        {
          // create a shared_ptr that owns a Foo and a deleter
          auto foo_p = new Foo;
          zs::move_only_shared_ptr<Foo> r(foo_p, FooDeleter::foo_deleter);
          aptr = zs::move_only_shared_ptr<int>(zs::share(r), &r->i); // aliasing ctor
          // aptr is now pointing to an int, but managing the whole Foo
        } // r gets destroyed (deleter not called)

        // obtain pointer to the deleter:
        if (auto del_p = std::get_deleter<void(*)(Foo*)>(aptr))
        {
          ++values_->ownsDeleter_;
          if (*del_p == FooDeleter::foo_deleter)
            ++values_->equalsDeleter_;
        } else
          ++values_->deleterNull_;

        TEST(0 == values_->fooDeleterCalled_);
        TEST(1 == values_->ownsDeleter_);
        TEST(1 == values_->equalsDeleter_);
        TEST(0 == values_->deleterNull_);
      }
      TEST(1 == values_->fooDeleterCalled_);
      TEST(1 == values_->ownsDeleter_);
      TEST(1 == values_->equalsDeleter_);
      TEST(0 == values_->deleterNull_);
    }

    //-------------------------------------------------------------------------
    void runAll() noexcept(false)
    {
      testCppReferenceConstructor();
      testCppReferenceMakeShared();
      testCppReferenceReset();
      testCppReferenceGet();
      testCppReferenceOperatorDeference();
      testCppReferencePointerCast();
      testCppReferenceGetDeleter();
    }
  };

  //---------------------------------------------------------------------------
  void testMoveSharedPtr() noexcept(false)
  {
    MoveSharedPtrTestBasics{}.runAll();
    MoveSharedPtrTestCppReference{}.runAll();
  }

}
