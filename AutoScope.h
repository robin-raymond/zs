
#pragma once

#include <functional>

namespace zs
{

  template <typename TLambda>
  struct AutoScope
  {
    template <typename TLambdaRhs>
    friend struct AutoScope;

    AutoScope() noexcept = delete;
    AutoScope(const AutoScope&) noexcept = delete;

    AutoScope(AutoScope&& rhs) noexcept :
      function_(std::move(rhs.function_))
    {
      rhs.function_ = {};
    }

    AutoScope(TLambda&& lambda) noexcept :
      function_(std::move(lambda))
    {
    }

    ~AutoScope() noexcept {
      if (!function_)
        return;
      std::invoke(function_);
    }

    AutoScope& operator=(const AutoScope&) noexcept = delete;

    AutoScope& operator=(AutoScope&& rhs) noexcept
    {
      if (this == &rhs)
        return *this;
      function_ = std::move(rhs.function_);
      rhs.function_ = {};
      return *this;
    }

    template <typename TLambdaRhs>
    AutoScope& operator=(AutoScope<TLambdaRhs>&& rhs) noexcept
    {
      function_ = std::move(rhs.function_);
      rhs.function_ = {};
      return *this;
    }

    template <typename TLambdaRhs>
    AutoScope& operator=(TLambdaRhs&& lambda) noexcept
    {
      function_ = std::move(lambda);
      return *this;
    }

    void cancel() noexcept
    {
      function_ = {};
    }

  protected:
    std::function<void()> function_;
  };

} // namespace zs

