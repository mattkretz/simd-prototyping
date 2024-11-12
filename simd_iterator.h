/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_RANGES_INTEGRATION_ITERATOR_H_
#define PROTOTYPE_RANGES_INTEGRATION_ITERATOR_H_

#include "fwddecl.h"

#if SIMD_IS_A_RANGE
namespace SIMD_NSPC
{
  class __simd_iterator_sentinel
  {};

  template <typename _Vp>
    class __simd_iterator
    {
      const _Vp* _M_data = nullptr;
      int _M_offset = 0;

    public:
      using value_type = typename _Vp::value_type;
      using iterator_category = std::random_access_iterator_tag;
      using difference_type = int;

      constexpr __simd_iterator() = default;

      constexpr
      __simd_iterator(const _Vp& __d, int __x)
      : _M_data(&__d), _M_offset(__x)
      {}

      constexpr
      __simd_iterator(const __simd_iterator &) = default;

      constexpr __simd_iterator&
      operator=(const __simd_iterator &) = default;

      constexpr value_type
      operator*() const
      { return (*_M_data)[_M_offset]; }

      constexpr __simd_iterator&
      operator++()
      {
        ++_M_offset;
        return *this;
      }

      constexpr __simd_iterator
      operator++(int)
      {
        __simd_iterator r = *this;
        ++_M_offset;
        return r;
      }

      constexpr __simd_iterator&
      operator--()
      {
        --_M_offset;
        return *this;
      }

      constexpr __simd_iterator
      operator--(int)
      {
        __simd_iterator r = *this;
        --_M_offset;
        return r;
      }

      constexpr difference_type
      operator-(__simd_iterator __rhs) const
      { return _M_offset - __rhs._M_offset; }

      constexpr friend difference_type
      operator-(__simd_iterator __it, __simd_iterator_sentinel)
      { return __it._M_offset - difference_type(_Vp::size.value); }

      constexpr friend difference_type
      operator-(__simd_iterator_sentinel, __simd_iterator __it)
      { return difference_type(_Vp::size.value) - __it._M_offset; }

      constexpr friend __simd_iterator
      operator+(difference_type __x, const __simd_iterator& __it)
      { return __simd_iterator(*__it._M_data, __it._M_offset + __x); }

      constexpr friend __simd_iterator
      operator+(const __simd_iterator& __it, difference_type __x)
      { return __simd_iterator(*__it._M_data, __it._M_offset + __x); }

      constexpr friend __simd_iterator
      operator-(const __simd_iterator& __it, difference_type __x)
      { return __simd_iterator(*__it._M_data, __it._M_offset - __x); }

      constexpr __simd_iterator&
      operator+=(difference_type __x)
      {
        _M_offset += __x;
        return *this;
      }

      constexpr __simd_iterator&
      operator-=(difference_type __x)
      {
        _M_offset -= __x;
        return *this;
      }

      constexpr value_type
      operator[](difference_type __i) const
      { return (*_M_data)[_M_offset + __i]; }

      constexpr friend auto operator<=>(__simd_iterator __a, __simd_iterator __b)
      { return __a._M_offset <=> __b._M_offset; }

      constexpr friend bool operator==(__simd_iterator __a, __simd_iterator __b) = default;

      constexpr friend bool operator==(__simd_iterator __a, __simd_iterator_sentinel)
      { return __a._M_offset == difference_type(_Vp::size.value); }
    };
}
#endif

#endif  // PROTOTYPE_RANGES_INTEGRATION_ITERATOR_H_
