/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2024      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef TESTS_FIXED_POINT_H_
#define TESTS_FIXED_POINT_H_

#include "../constexpr_wrapper.h"
#include "../udt_simd.h"

#include <concepts>
#include <cmath>
#include <iostream>

namespace vir
{
  template <std::integral Rep, int Exponent>
    class fixed_point;

  // highest_bit_v<uint32_t> == 31
  // highest_bit_v<int32_t> == 30
  // i.e. counting from zero
  template <typename T>
    struct highest_bit;

  template <typename T>
    inline constexpr int highest_bit_v = highest_bit<T>::value;

  template <typename T>
    struct highest_bit<const T>
    : highest_bit<T>
    {};

  template <std::integral T>
    struct highest_bit<T>
    : std::integral_constant<int, int(sizeof(T) * __CHAR_BIT__) - std::is_signed_v<T> - 1>
    {};

  template <std::integral T, int E>
    struct highest_bit<fixed_point<T, E>>
    : std::integral_constant<int, int(sizeof(T) * __CHAR_BIT__) - std::is_signed_v<T> - 1 + E>
    {};

  template <vir::constexpr_value X>
    requires std::integral<decltype(X::value)> and (X::value >= 0)
    struct highest_bit<X>
    : std::integral_constant<int, std::bit_width(static_cast<unsigned long long>(X::value)) - 1>
    {};

  template <vir::constexpr_value X>
    requires std::signed_integral<decltype(X::value)> and (X::value < 0)
    struct highest_bit<X>
    // -1 requires 0 bits + sign bit (like 0)
    // -2 requires 1 bit  + sign bit (like 1)
    // -3 requires 2 bits + sign bit (like 2)
    // -4 requires 2 bits + sign bit (like 3)
    // -5 requires 3 bits + sign bit (like 4)
    : highest_bit<vir::constexpr_wrapper< -X::value - 1>>
    {};


  // The value of fixed_point<T, N> is the value of T * pow(2, Exponent)
  template <std::integral Rep, int Exponent>
    class fixed_point
    {
      Rep data;

    public:
      using representation_type = Rep;

      static constexpr std::integral_constant<int, Exponent> exponent = {};

/*      template <std::same_as<fixed_point> T>
        friend constexpr T
        from_representation(T*, Rep x)
        {
          T r;
          r.data = x;
          return r;
        }

      friend constexpr Rep
      to_representation(const fixed_point& x)
      { return x.data; }*/

      static constexpr int _s_frac_bits = -Exponent;

      static constexpr bool _s_signed = std::is_signed_v<Rep>;

      static constexpr bool _s_can_shift
        = _s_frac_bits > -int(sizeof(Rep) * __CHAR_BIT__ - _s_signed)
            and _s_frac_bits < int(sizeof(Rep) * __CHAR_BIT__ - _s_signed);

      // <short, 4> has 15 value bits implicitly shifted left by 4, so the highest non-sign bit
      // corresponds to bit 19 in an int.
      // <short, -4> uses the low 4 bits for fractional values, so the highest non-sign bit
      // corresponds to bit 11 in an int.
      // => finite_max_v<fixed_point> = pow(2, _s_highest_bit + 1) - 1
      static constexpr int _s_highest_bit
        = int(sizeof(Rep) * __CHAR_BIT__) - _s_signed - 1 - _s_frac_bits;

      static constexpr int _s_lowest_bit = -_s_frac_bits;

      constexpr
      fixed_point() = default;

      constexpr
      fixed_point(const fixed_point&) = default;

      /////////////////////////////////////////////////////////////////////////////
      // conversion from arithmetic type (or integral constant expression)
      template <std::integral T>
        requires _s_can_shift
        constexpr explicit(highest_bit_v<T> > _s_highest_bit
                             or _s_lowest_bit > 0
                             or (std::is_signed_v<T> and std::is_unsigned_v<Rep>))
        fixed_point(T x)
          : data(_s_frac_bits >= 0 ? Rep(Rep(x) << _s_frac_bits) : Rep(x >> -_s_frac_bits))
        {}

      constexpr explicit
      fixed_point(std::floating_point auto x)
      : data(std::ldexp(x, -Exponent))
      {}

      template <vir::constexpr_value X>
        requires std::integral<decltype(X::value)> and _s_can_shift
        // refuse to assign negative constant to unsigned Rep
          and (X::value >= 0 or _s_signed)
        // refuse dropping bits at the low end
          and (_s_frac_bits >= 0 or X::value == 0
                 or -_s_frac_bits <= std::countr_zero(static_cast<unsigned long long>(X::value)))
        // refuse dropping bits at the high end
          and (highest_bit_v<X> <= _s_highest_bit)
        constexpr
        fixed_point(X)
        : data([] {
            if constexpr (X::value == 0)
              return Rep();
            else if constexpr (_s_frac_bits >= 0)
              return Rep(Rep(X::value) << _s_frac_bits);
            else
              return Rep(X::value >> -_s_frac_bits);
          }())
        {}

      template <vir::constexpr_value X>
        requires (std::integral<decltype(X::value)> and not _s_can_shift and X::value == 0)
        constexpr
        fixed_point(X)
        : data()
        {}

      /////////////////////////////////////////////////////////////////////////////
      // fixed_point <-> fixed_point conversions
      template <typename U, int E>
        constexpr
        explicit(fixed_point<U, E>::_s_frac_bits > _s_frac_bits // if dropping fractional bits
                   or (std::is_signed_v<U> and not _s_signed) // if dropping negative values
                   or fixed_point<U, E>::_s_highest_bit > _s_highest_bit) // if dropping high bits
        fixed_point(fixed_point<U, E> x)
        : data([](std::common_type_t<Rep, U> bits) {
            constexpr int bits_to_drop = fixed_point<U, E>::_s_frac_bits - _s_frac_bits;
            if constexpr (bits_to_drop > 0)
              return static_cast<Rep>(bits >> bits_to_drop);
            else
              return static_cast<Rep>(bits << -bits_to_drop);
          }(std::to_representation(x)))
        {}

      /////////////////////////////////////////////////////////////////////////////
      // conversion to arithmetic types
      // static_cast to floating_point
      template <std::floating_point T>
        constexpr explicit
        operator T() const
        { return std::ldexp(static_cast<T>(data), Exponent); }

      // truncates to Rep (implicit if no shift is needed)
      // Constraints: at least one bit of fixed_point must overlap with the bits of T
      constexpr explicit(Exponent != 0)
      operator Rep() const requires(_s_can_shift)
      { return _s_frac_bits > 0 ? data >> _s_frac_bits : data << -_s_frac_bits; }

      // truncates to T (implicit if no bits can get lost)
      // Constraints: at least one bit of fixed_point must overlap with the bits of T
      template <std::integral T>
        constexpr explicit(Exponent < 0 or highest_bit_v<T> < _s_highest_bit)
        operator T() const requires (_s_highest_bit >= 0 and _s_lowest_bit <= highest_bit_v<T>)
        {
          if constexpr (_s_frac_bits > 0)
            return T(data >> _s_frac_bits);
          else if constexpr (highest_bit_v<T> > highest_bit_v<Rep>)
            return T(data) << -_s_frac_bits;
          else
            return T(data << -_s_frac_bits);
        }

      // rounds up to T
      // Constraints: at least one bit of fixed_point must overlap with the bits of T
      template <std::integral T = Rep>
        constexpr T
        round_up() const requires (_s_highest_bit >= 0 and _s_lowest_bit <= highest_bit_v<T>)
        {
          if constexpr (Exponent >= 0)
            return static_cast<T>(*this);
          else if constexpr (highest_bit_v<T> > highest_bit_v<Rep>)
            return (data + ((T(1) << _s_frac_bits) >> 1)) >> _s_frac_bits;
          else
            return (data + ((Rep(1) << _s_frac_bits) >> 1)) >> _s_frac_bits;
        }

      /////////////////////////////////////////////////////////////////////////////
      // unary operators
      constexpr fixed_point
      operator+() const
      { return *this; }

      constexpr fixed_point
      operator-() const
      { return std::from_representation<fixed_point>(-data); }

      constexpr fixed_point
      operator~() const
      { return std::from_representation<fixed_point>(~data); }

      constexpr fixed_point&
      operator++() requires (Exponent <= 0 and _s_frac_bits <= _s_highest_bit)
      {
        data += Rep(1) << _s_frac_bits;
        return *this;
      }

      constexpr fixed_point
      operator++(int) requires (Exponent <= 0 and _s_frac_bits <= _s_highest_bit)
      {
        fixed_point r = *this;
        data += Rep(1) << _s_frac_bits;
        return r;
      }

      constexpr fixed_point&
      operator--() requires (Exponent <= 0 and _s_frac_bits <= _s_highest_bit)
      {
        data -= Rep(1) << _s_frac_bits;
        return *this;
      }

      constexpr fixed_point
      operator--(int) requires (Exponent <= 0 and _s_frac_bits <= _s_highest_bit)
      {
        fixed_point r = *this;
        data -= Rep(1) << _s_frac_bits;
        return r;
      }

      /////////////////////////////////////////////////////////////////////////////
      // comparisons
      friend constexpr bool
      operator==(fixed_point, fixed_point) = default;

      template <int B>
        requires (-Exponent > -B)
        friend constexpr bool
        operator==(fixed_point a, fixed_point<Rep, B> b)
        {
          constexpr int extra_bits = -Exponent + B;
          if (a.data & ((Rep(1) << extra_bits) - 1))
            return false;
          return (a.data >> extra_bits) == std::to_representation(b);
        }

      friend constexpr auto
      operator<=>(fixed_point, fixed_point) = default;

#if FIXED_POINT_SPACESHIP

      template <int B>
        requires (Exponent > B)
        friend constexpr std::strong_ordering
        operator<=>(fixed_point a, fixed_point<Rep, B> b)
        {
          constexpr int extra_bits = Exponent - B;
          constexpr bool disjunct = extra_bits >= sizeof(Rep) * __CHAR_BIT__ - _s_signed;
          const Rep b_high = disjunct ? Rep() : Rep(std::to_representation(b) >> extra_bits);
          const Rep b_low = disjunct ? std::to_representation(b)
                                     : std::to_representation(b) & ((Rep(1) << extra_bits) - 1);

          if (a.data == b_high)
            return Rep() <=> b_low;
          else
            return a.data <=> b_high;
        }

#else

      template <int B>
        requires (Exponent > B)
        friend constexpr bool
        operator<(fixed_point a, fixed_point<Rep, B> b)
        {
          constexpr int extra_bits = Exponent - B;
          constexpr bool disjunct = extra_bits >= sizeof(Rep) * __CHAR_BIT__ - _s_signed;
          const Rep b_high = disjunct ? Rep() : Rep(std::to_representation(b) >> extra_bits);
          const Rep b_low = disjunct ? std::to_representation(b)
                                     : std::to_representation(b) & ((Rep(1) << extra_bits) - 1);

          if (a.data == b_high)
            return Rep() < b_low;
          else
            return a.data < b_high;
        }

      template <int B>
        requires (Exponent < B)
        friend constexpr bool
        operator>(fixed_point a, fixed_point<Rep, B> b)
        { return b < a; }

      template <int B>
        requires (Exponent > B)
        friend constexpr bool
        operator<=(fixed_point a, fixed_point<Rep, B> b)
        {
          constexpr int extra_bits = Exponent - B;
          if constexpr (extra_bits >= sizeof(Rep) * __CHAR_BIT__ - _s_signed)
            return a.data == Rep() ? Rep() <= std::to_representation(b) : a.data < Rep();
          else
            return a.data <= (std::to_representation(b) >> extra_bits);
        }

      template <int B>
        requires (Exponent < B)
        friend constexpr bool
        operator>=(fixed_point a, fixed_point<Rep, B> b)
        { return b <= a; }

#endif

      /////////////////////////////////////////////////////////////////////////////
      // shift via the type system
      friend constexpr auto
      operator<<(fixed_point a, vir::constexpr_value auto n)
      { return std::from_representation<fixed_point<Rep, Exponent + n>>(a.data); }

      friend constexpr auto
      operator>>(fixed_point a, vir::constexpr_value auto n)
      { return std::from_representation<fixed_point<Rep, Exponent - n>>(a.data); }

      /////////////////////////////////////////////////////////////////////////////
      // bit-shift on the underlying representation
      friend constexpr fixed_point&
      operator<<=(fixed_point& a, int n)
      {
        a.data <<= n;
        return a;
      }

      friend constexpr fixed_point
      operator<<(fixed_point a, int n)
      { return a <<= n; }

      friend constexpr fixed_point&
      operator>>=(fixed_point& a, int n)
      {
        a.data >>= n;
        return a;
      }

      friend constexpr fixed_point
      operator>>(fixed_point a, int n)
      { return a >>= n; }

      /////////////////////////////////////////////////////////////////////////////
      // arithmetic
      friend constexpr fixed_point&
      operator+=(fixed_point& a, fixed_point b)
      {
        a.data += b.data;
        return a;
      }

      friend constexpr fixed_point
      operator+(fixed_point a, fixed_point b)
      { return a += b; }

      friend constexpr fixed_point&
      operator-=(fixed_point& a, fixed_point b)
      {
        a.data -= b.data;
        return a;
      }

      friend constexpr fixed_point
      operator-(fixed_point a, fixed_point b)
      { return a -= b; }

      template <int B>
        friend constexpr fixed_point<Rep, Exponent + B>
        operator*(fixed_point a, fixed_point<Rep, B> b)
        {
          return std::from_representation<fixed_point<Rep, Exponent + B>>(
                   a.data * std::to_representation(b));
        }

      friend constexpr fixed_point&
      operator*=(fixed_point& a, fixed_point<Rep, 0> b)
      {
        a.data *= std::to_representation(b);
        return a;
      }

      // adjust Exponent as far as possible without losing bits at the low end
      friend constexpr auto
      operator*(fixed_point a, vir::constexpr_value auto n)
      {
        constexpr int zero_bits = std::countr_zero(static_cast<unsigned long long>(n));
        return std::from_representation<fixed_point<Rep, Exponent + zero_bits>>(
                 a.data * (n >> zero_bits));
      }

      template <int B>
        requires (Exponent >= B and B <= 0)
      friend constexpr fixed_point&
      operator/=(fixed_point& a, fixed_point<Rep, B> b)
      {
        a.data /= std::to_representation(b);
        return a;
      }

      // (a 2^E) / (b 2^B)
      // = a / b * 2^(E-B)
      // = a / b * 2^N * 2^(E-B-N)
      // = ((a << Nᵃ / (b >> Nᵇ) << Nᶜ) * 2^(E-B-N), with Nᵃ + Nᵇ + Nᶜ == N (positive N)
      // = ((a >> Nᵃ / (b << Nᵇ) >> Nᶜ) * 2^(E-B-N), with Nᵃ + Nᵇ + Nᶜ == -N (negative N)
      template <int B>
        friend constexpr auto
        operator/(fixed_point a, fixed_point<Rep, B> b)
        {
          Rep avalue = a.data;
          Rep bvalue = std::to_representation(b);
          using U = std::make_unsigned_t<Rep>;
          const U abits = avalue;
          const U bbits = bvalue;
          // if N = min(E - B, max(E, B)) then
          //   if E >= B >= 0 => E - B >= 0 and max(E, B) = E
          //                  => N = min(E - B, E) = E - B
          //   if  E < B >= 0 => E - B < 0 and max(E, B) = B
          //                  => N = min(E - B, B) = E - B
          //   => B >= 0 implies N = E - B
          //
          // However B = 0 implies N = E is suboptimal: this is only necessary for b = 1,
          //   otherwise N = E - 1 were better; but x / 1 not yielding x is even more surprising
          // => B = 0 implies N = E
          //
          // if E >= B => N = min(E - B, E)
          //   if B <= 0 => N = E
          //   if B >= 0 => N = E - B
          constexpr int new_exponent = std::min(Exponent - B, std::max(Exponent, B));
          constexpr int shift = Exponent - B - new_exponent;
          if constexpr (shift == 0)
            return std::from_representation<fixed_point<Rep, new_exponent>>(avalue / bvalue);
          else if constexpr (shift > 0)
            {
              const int ashift = std::min(shift, avalue < 0 ? std::countl_one(abits) - 1
                                                            : std::countl_zero(abits) - _s_signed);
              avalue <<= ashift;
              int shift_remainder = shift - ashift;
              if (shift_remainder != 0)
                {
                  const int bshift = std::min(shift_remainder, std::countr_zero(U(bvalue)));
                  bvalue >>= bshift;
                  shift_remainder -= bshift;
                }
              return std::from_representation<fixed_point<Rep, new_exponent>>(
                       (avalue / bvalue) << shift_remainder);
            }
          else if constexpr (shift < 0)
            {
              const int ashift = std::min(-shift, std::countr_zero(U(avalue)));
              avalue >>= ashift;
              int shift_remainder = -shift - ashift;
              if (shift_remainder != 0)
                {
                  const int bshift = std::min(shift_remainder,
                                              bvalue < 0 ? std::countl_one(bbits) - 1
                                                         : std::countl_zero(bbits) - _s_signed);
                  bvalue <<= bshift;
                  shift_remainder -= bshift;
                }
              return std::from_representation<fixed_point<Rep, new_exponent>>(
                       (avalue / bvalue) >> shift_remainder);
            }
        }

      /////////////////////////////////////////////////////////////////////////////
      // ostream output
      friend std::ostream&
      operator<<(std::ostream& s, fixed_point v)
      {
        // 2 _s_frac_bits (Exponent = -2):
        // 0 - 1.00 => 0b0000'0000 - 0b0000'0100 = 0b1111'1100 => -4 / 4 == 1 correct
        // 0 - 0.50 => 0b0000'0000 - 0b0000'0010 = 0b1111'1110 => -2 / 4 == 0.5 correct
        // 0 - 1.50 => 0b0000'0000 - 0b0000'0110 = 0b1111'1010 => -6 / 4 == 1.5 correct
        // 0 - 1.25 => 0b0000'0000 - 0b0000'0101 = 0b1111'1011 => -5 / 4 == 1.25 correct
        // <signed char, -2>(-1.25) == 0b1111'1011
        // 0b1111'1011 >> 2 == 0b1111'1110
        if (v.data < Rep())
          return s << '-' << -v;

        if constexpr (_s_frac_bits >= 0)
          {
            s << (v.data >> _s_frac_bits);
            if (_s_frac_bits > 0)
              {
                s << '.';
                const auto w = s.width(_s_frac_bits);
                const auto f = s.fill('0');
                if constexpr (_s_frac_bits < 20)
                  {
                    constexpr auto pow5 = [] {
                      unsigned long long pow5 = 5;
                      for (int i = 1; i < _s_frac_bits; ++i)
                        pow5 *= 5;
                      return pow5;
                    }();
                    s << (v.data & ((Rep(1) << _s_frac_bits) - 1)) * pow5;
                  }
                else
                  {
                    const auto p = s.precision(_s_frac_bits);
                    double tmp = (v.data & ((Rep(1) << _s_frac_bits) - 1)) * 95367431640625.;
                    for (int i = 20; i < _s_frac_bits; ++i)
                      tmp *= 5;
                    s << tmp;
                    s.precision(p);
                  }
                s.width(w);
                s.fill(f);
              }
            return s;
          }
        else
          {
            const auto p = s.precision(_s_highest_bit / 3);
            s << std::ldexp(double(v.data), Exponent);
            s.precision(p);
            return s;
          }
      }
    };

  template <typename Rep>
    fixed_point(Rep) -> fixed_point<Rep, 0>;

  template <vir::constexpr_value X>
    fixed_point(X) -> fixed_point<std::remove_const_t<decltype(X::value)>,
                                  std::countr_zero(static_cast<unsigned long long>(X::value))>;
}

#endif  // TESTS_FIXED_POINT_H_
