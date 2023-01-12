/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD2_H_
#define PROTOTYPE_SIMD2_H_

#include "stdx.h"
#include "detail.h"
#include <span>

template <class T, class A = stdx::simd_abi::native<T>>
  class simd2 : public stdx::simd<T, A>
  {
    using Base = stdx::simd<T, A>;

  public:
    using value_type = Base::value_type;
    using mask_type = Base::mask_type;

    static inline constexpr std::integral_constant<std::size_t, Base::size()>
      size = {};

    constexpr
    simd2() = default;

    constexpr
    simd2(const simd&) = default;

    constexpr
    simd2(simd&&) = default;

    constexpr
    simd2(const Base& b)
    : Base(b)
    {}

    // implicit broadcast constructor
    template <stdx::value_preserving_or_int<value_type> _Up>
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
      simd2(_Up&& __x)
      : Base(std::forward<_Up>(__x)) noexcept
      {}

    // implicit type conversion constructor
    template <typename _Up, typename _UAbi>
      requires(stdx::simd_size_v<_Up, _UAbi> == size())
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
      explicit(not stdx::value_preserving_conversion<_Up, value_type>
                 || stdx::higher_rank<_Up, value_type>)
      simd2(const simd<_Up, simd_abi::fixed_size<size()>>& __x) noexcept
      : Base(stdx::static_simd_cast<Base>(__x))
      {}

    // generator constructor
    template <stdx::simd_broadcast_invokable<value_type, size()> _Fp>
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
      simd2(_Fp&& __gen) noexcept
      : Base(std::forward<_Fp>(__gen))
      {}

    // load constructor
    template <stdx::vectorizable _Up, typename _Flags = stdx::element_aligned_tag>
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
      simd2(const _Up* __mem, _Flags __f)
      : Base(__mem, __f)
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

#endif  // PROTOTYPE_SIMD2_H_
