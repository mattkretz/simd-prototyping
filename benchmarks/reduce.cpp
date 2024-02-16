/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2024      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#include "bench.h"
#include <climits>

namespace my
{
  using std::reduce;

  template <typename T>
    requires(std::integral<T> or std::floating_point<T>)
    T
    reduce(T x)
    { return x; }

  template <vec_builtin V>
    auto
    reduce(const V v)
    {
      auto acc = v[0];
      for (int i = 1; i < sizeof(v) / sizeof(acc); ++i)
        acc += v[i];
      return acc;
    }
}

template <>
  struct Benchmark<>
  {
    static constexpr Info<2> info = {"Latency", "Throughput"};

    template <typename T>
      static constexpr bool accept = size_v<T> >= 2;

    template <class T>
      [[gnu::flatten]]
      static Times<2>
      run()
      {
        using TT = value_type_t<T>;
        T one = T() + TT(1);
        T reset = T() + TT(size_v<T>);
        fake_modify(one, reset);

        auto process_one = [&](T& inout) {
          T x = inout + one;
          if constexpr (std::convertible_to<TT, T>)
            x = my::reduce(x);
          else
            x = T() + my::reduce(x);
          inout = x - reset;
        };

        auto fake_one = [&](T& inout) {
          T x = inout + one;
          fake_modify(x);
          inout = x - reset;
        };

        T a[8] = {};
        return { time_latency(a, process_one, fake_one),
                 time_throughput(a, process_one, fake_one) };
      }
  };

int
main()
{
  bench_all<signed char>();
  bench_all<unsigned char>();
  bench_all<signed short>();
  bench_all<unsigned short>();
  bench_all<signed int>();
  bench_all<unsigned int>();
  bench_all<signed long>();
  bench_all<unsigned long>();
  bench_all<float>();
  bench_all<double>();
}
