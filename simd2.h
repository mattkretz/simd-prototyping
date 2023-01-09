#include "../prototype_simd_index/stdx.h"
#include <span>
#include <ranges>

template <class T, class A>
class simd2 : public stdx::simd<T, A>
{
  using Base = stdx::simd<T, A>;
  public:
    using stdx::simd<T, A>::simd;

    constexpr simd2(const Base& b)
    : Base(b)
    {}

    // construction from span is simple
    constexpr simd2(std::span<T, Base::size()> mem)
    : Base(mem.data(), stdx::element_aligned)
    {}

    // ranges typically don't have a static size() function :(
    // but if one does, this ctor is useful
    template <std::ranges::contiguous_range R>
      requires (std::same_as<std::ranges::range_value_t<R>, T> && R::size() == Base::size())
      constexpr simd2(const R& range)
      : Base(std::ranges::data(range), stdx::element_aligned)
      {}

    template <std::ranges::input_range R>
      explicit (std::same_as<std::ranges::range_value_t<R>, T>)
      constexpr simd2(const R& range)
      : Base(std::ranges::data(range), stdx::element_aligned)
      {}


    std::array<T, Base::size()> to_array() const noexcept
    {
      std::array<T, Base::size()> r;
      this->copy_to(r.data(), stdx::element_aligned);
      return r;
    }

    explicit operator std::array<T, Base::size()>() const noexcept
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
