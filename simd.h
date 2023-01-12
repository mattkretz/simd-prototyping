/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD2_H_
#define PROTOTYPE_SIMD2_H_

#include "detail.h"
#include "simd_mask.h"
#include <span>

namespace std
{
  template <typename _Tp, typename _Abi>
    class simd : public __detail::simd<_Tp, _Abi>
    {
      using _Base = __detail::simd<_Tp, _Abi>;

    public:
      using value_type = _Base::value_type;
      using mask_type = std::simd_mask<_Tp, _Abi>;

      static inline constexpr __detail::_Cnst<_Base::size()> size = {};

      constexpr
      simd() = default;

      constexpr
      simd(const simd&) = default;

      constexpr
      simd(simd&&) = default;

      constexpr
      simd(const _Base& b)
      : _Base(b)
      {}

      // implicit broadcast constructor
      template <__detail::__value_preserving_or_int<value_type> _Up>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        simd(_Up&& __x) noexcept
        : _Base(std::forward<_Up>(__x))
        {}

      // implicit type conversion constructor
      template <typename _Up, typename _UAbi>
        requires(__detail::simd_size_v<_Up, _UAbi> == size())
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        explicit(not __detail::__value_preserving_convertible_to<_Up, value_type>
                   || __detail::__higher_rank_than<_Up, value_type>)
        simd(const simd<_Up, _UAbi>& __x) noexcept
        : _Base(__detail::static_simd_cast<_Base>(__x))
        {}

      // generator constructor
      template <__detail::__simd_broadcast_invokable<value_type, size()> _Fp>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
        simd(_Fp&& __gen) noexcept
        : _Base(std::forward<_Fp>(__gen))
        {}

      // load constructor
      template <__detail::__vectorizable _Up, typename _Flags = __detail::element_aligned_tag>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        simd(const _Up* __mem, _Flags __f)
        : _Base(__mem, __f)
        {}

      // construction from span is simple
      constexpr
      simd(std::span<_Tp, size()> __mem)
      : _Base(__mem.data(), __detail::element_aligned)
      {}

      // ranges typically don't have a static size() function :(
      // but if one does, this ctor is useful
      template <std::ranges::contiguous_range _Rg>
        requires (std::same_as<std::ranges::range_value_t<_Rg>, _Tp>
                    && __detail::__static_range_size<_Rg> == size())
        constexpr
        simd(const _Rg& __range)
        : _Base(std::ranges::data(__range), __detail::element_aligned)
        {}

      template <std::ranges::random_access_range _Rg>
        requires(std::convertible_to<std::ranges::range_value_t<_Rg>, _Tp>)
        constexpr explicit (not std::same_as<std::ranges::range_value_t<_Rg>, _Tp>)
        simd(const _Rg& __range)
        : _Base([&__range](auto __i) -> _Tp { return __range[__i]; })
        {}

      constexpr std::array<_Tp, size()>
      to_array() const noexcept
      {
        std::array<_Tp, size()> __r = {};
        this->copy_to(__r.data(), __detail::element_aligned);
        return __r;
      }

      explicit
      operator std::array<_Tp, size()>() const noexcept
      { return to_array(); }
    };

  template <typename _Tp, typename _Abi>
    struct is_simd<simd<_Tp, _Abi>> : is_default_constructible<simd<_Tp, _Abi>>
    {};

  template <class _Tp, size_t _Extend>
    simd(std::span<_Tp, _Extend>) -> simd<_Tp, simd_abi::deduce_t<_Tp, _Extend>>;

  template <std::ranges::contiguous_range _Rg>
    simd(const _Rg& x)
    -> simd<std::ranges::range_value_t<_Rg>,
            simd_abi::deduce_t<std::ranges::range_value_t<_Rg>,
                               __detail::__static_range_size<_Rg>>>;

  template <__detail::__vectorizable _Tp, __detail::__simd_type _Simd>
    requires requires { typename simd_abi::deduce_t<_Tp, _Simd::size()>; }
    struct rebind_simd<_Tp, _Simd>
    { using type = simd<_Tp, simd_abi::deduce_t<_Tp, _Simd::size()>>; };

  template <__detail::__vectorizable _Tp, __detail::__mask_type _Mask>
    requires requires { typename simd_abi::deduce_t<_Tp, _Mask::size()>; }
    struct rebind_simd<_Tp, _Mask>
    { using type = simd_mask<_Tp, simd_abi::deduce_t<_Tp, _Mask::size()>>; };

}

#endif  // PROTOTYPE_SIMD2_H_
