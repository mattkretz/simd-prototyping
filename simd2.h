#include "../prototype_simd_index/stdx.h"
#include <span>
#include <ranges>

namespace detail
{
  template <std::ranges::sized_range T>
    constexpr inline std::size_t
    static_range_size = []() {
      if constexpr (requires { {T::size()} -> std::integral; })
        return T::size();
      else if constexpr (requires { {T::extent} -> std::integral; })
        return T::extent;
      else if constexpr (requires { {std::tuple_size_v<T>} -> std::integral; })
        return std::tuple_size_v<T>;
      else
        return std::dynamic_extent;
    }();
}

template <class T, class A>
class simd2 : public stdx::simd<T, A>
{
  using Base = stdx::simd<T, A>;
  public:
    using stdx::simd<T, A>::simd;

    static inline constexpr std::integral_constant<std::size_t, Base::size()>
      size = {};

    constexpr
    simd2(const Base& b)
    : Base(b)
    {}

    // construction from span is simple
    constexpr
    simd2(std::span<T, size()> mem)
    : Base(mem.data(), stdx::element_aligned)
    {}

    // ranges typically don't have a static size() function :(
    // but if one does, this ctor is useful
    template <std::ranges::contiguous_range R>
      requires (std::same_as<std::ranges::range_value_t<R>, T>
                  && detail::static_range_size<R> == size())
      constexpr
      simd2(const R& range)
      : Base(std::ranges::data(range), stdx::element_aligned)
      {}

    template <std::ranges::random_access_range R>
      requires(std::convertible_to<std::ranges::range_value_t<R>, T>)
      constexpr explicit (not std::same_as<std::ranges::range_value_t<R>, T>)
      simd2(const R& range)
      : Base([&range](auto i) -> T { return range[i]; })
      {}

    constexpr std::array<T, size()>
    to_array() const noexcept
    {
      std::array<T, size()> r = {};
      this->copy_to(r.data(), stdx::element_aligned);
      return r;
    }

    explicit
    operator std::array<T, size()>() const noexcept
    { return to_array(); }
};

template <class T, std::size_t Extend>
simd2(std::span<T, Extend>) -> simd2<T, stdx::simd_abi::deduce_t<T, Extend>>;

template <std::ranges::contiguous_range R>
simd2(const R& x)
  -> simd2<std::ranges::range_value_t<R>,
           stdx::simd_abi::deduce_t<std::ranges::range_value_t<R>, R::size()>>;

template <typename T>
  using native_simd2 = simd2<T, stdx::simd_abi::native<T>>;
