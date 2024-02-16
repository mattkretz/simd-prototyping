/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2019-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef BENCH_H_
#define BENCH_H_

#include "../simd.h"
#include "../simd_reductions.h"
#include "../mask_reductions.h"

#include <array>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>

template <class T>
  auto
  value_type_t_impl()
  {
    if constexpr (requires { typename T::value_type; })
      return typename T::value_type();
    else if constexpr (requires(T x) { x[0]; })
      return T()[0];
    else
      return T();
  }

template <class T>
  using value_type_t = decltype(value_type_t_impl<T>());

template <typename T>
  concept vec_builtin = std::__detail::__vec_builtin<T>;

template <class T>
  struct size
  {
    static inline constexpr int
    value_impl()
    {
      if constexpr (requires {T::size;})
        if constexpr (vir::constexpr_value<decltype(T::size)>)
          return T::size;

      if constexpr (vec_builtin<T>)
        return std::__detail::__width_of<T>;
      else
        {
          static_assert (sizeof(T) < 16 or requires {T::size;});
          return 1;
        }
    }

    static inline constexpr int value = value_impl();
  };

template <class T>
  constexpr inline int size_v = size<T>::value;

template <class T>
  struct speedup_size : size<T> {};

/* TODO
template <class T, int N>
  struct speedup_size<stdx::simd<T, stdx::simd_abi::fixed_size<N>>>
  { static inline constexpr int value = stdx::__fixed_size_storage_t<T, N>::_S_first_size; };
*/

template <class T>
  constexpr inline int speedup_size_v = speedup_size<T>::value;

template <int N>
  using Info = std::array<const char*, N>;

template <int N>
  using Times = std::array<double, N>;

struct DefaultBenchmark
{
  static constexpr Info<2> info = {"Latency", "Throughput"};

  template <typename T>
    static constexpr bool accept = true;
};

template <class...>
  struct Benchmark
  { static_assert("The benchmark must specialize this type."); };

struct NoRef {};

template <class T, class... ExtraFlags, class Ref = NoRef>
  auto
  bench_lat_thr(const char* id, const Ref& ref = {})
  {
    using B = Benchmark<ExtraFlags...>;
    if constexpr (not B::template accept<T> or not std::default_initializable<T>)
      return ref;
    else
      {
        constexpr int N = B::info.size();
        static constexpr char red[] = "\033[1;40;31m";
        static constexpr char green[] = "\033[1;40;32m";
        static constexpr char dgreen[] = "\033[0;40;32m";
        static constexpr char normal[] = "\033[0m";

        const Times<N> results = B::template run<T>();
        std::cout << id;
        for (int i = 0; i < N; ++i)
          {
            double speedup = 1;
            if constexpr (!std::is_same_v<Ref, NoRef>)
              speedup = ref[i] * size_v<T> / results[i];

            std::cout << std::setprecision(3) << std::setw(15) << results[i];
            if (speedup >= speedup_size_v<T> * 0.90 && speedup >= 1.5)
              std::cout << green;
            else if (speedup > 1.1)
              std::cout << dgreen;
            else if (speedup < 0.95)
              std::cout << red;
            std::cout << std::setw(12) << speedup << normal;
          }
        std::cout << std::endl;
        if constexpr (std::same_as<Ref, NoRef>)
          return results;
        else
          return ref;
      }
  }

template <std::size_t N>
  using cstr = char[N];

template <class B, std::size_t N>
  void
  print_header(const cstr<N> &id_name)
  {
    std::cout << id_name;
    for (int i = 0; i < B::info.size(); ++i)
      std::cout << ' ' << std::setw(14) << B::info[i] << std::setw(12) << "Speedup";
    std::cout << '\n';

    char pad[N] = {};
    std::memset(pad, ' ', N - 1);
    pad[N - 1] = '\0';
    std::cout << pad;
    for (int i = 0; i < B::info.size(); ++i)
      std::cout << std::setw(15) << "[cycles/call]" << std::setw(12) << "[per value]";
    std::cout << '\n';
  }

template <class T, class... ExtraFlags>
  void
  bench_all()
  {
    constexpr std::size_t value_type_field = 6;
    constexpr std::size_t abi_field = 24;
    constexpr std::size_t type_field = value_type_field + 2 + abi_field;
    constexpr std::size_t id_size = type_field + (1 + ... + (1 + sizeof(ExtraFlags::name)));
    char id[id_size];
    std::memset(id, ' ', id_size - 1);
    id[id_size - 1] = '\0';
    std::strncpy(id + id_size/2 - 2, "TYPE", 4);
    print_header<Benchmark<ExtraFlags...>>(id);
    std::strncpy(id + id_size/2 - 2, "    ", 4);
    char* const typestr = id;
    char* const abistr  = id + value_type_field + 2;
    id[value_type_field] = ',';
    char* extraflags = id + type_field + 1;

    if constexpr (std::is_same_v<T, float>)
      std::strncpy(typestr, " float", value_type_field);
    else if constexpr (std::is_same_v<T, double>)
      std::strncpy(typestr, "double", value_type_field);
#ifdef __STDCPP_FLOAT16_T__
    else if constexpr (std::is_same_v<T, std::float16_t>)
      std::strncpy(typestr, " flt16", value_type_field);
#endif
#ifdef __STDCPP_FLOAT32_T__
    else if constexpr (std::is_same_v<T, std::float32_t>)
      std::strncpy(typestr, " flt32", value_type_field);
#endif
#ifdef __STDCPP_FLOAT64_T__
    else if constexpr (std::is_same_v<T, std::float64_t>)
      std::strncpy(typestr, " flt64", value_type_field);
#endif
    else if constexpr (std::is_same_v<T, long long>)
      std::strncpy(typestr, " llong", value_type_field);
    else if constexpr (std::is_same_v<T, unsigned long long>)
      std::strncpy(typestr, "ullong", value_type_field);
    else if constexpr (std::is_same_v<T, long>)
      std::strncpy(typestr, "  long", value_type_field);
    else if constexpr (std::is_same_v<T, unsigned long>)
      std::strncpy(typestr, " ulong", value_type_field);
    else if constexpr (std::is_same_v<T, int>)
      std::strncpy(typestr, "   int", value_type_field);
    else if constexpr (std::is_same_v<T, unsigned>)
      std::strncpy(typestr, "  uint", value_type_field);
    else if constexpr (std::is_same_v<T, short>)
      std::strncpy(typestr, " short", value_type_field);
    else if constexpr (std::is_same_v<T, unsigned short>)
      std::strncpy(typestr, "ushort", value_type_field);
    else if constexpr (std::is_same_v<T, char>)
      std::strncpy(typestr, "  char", value_type_field);
    else if constexpr (std::is_same_v<T, signed char>)
      std::strncpy(typestr, " schar", value_type_field);
    else if constexpr (std::is_same_v<T, unsigned char>)
      std::strncpy(typestr, " uchar", value_type_field);
    else if constexpr (std::is_same_v<T, char8_t>)
      std::strncpy(typestr, " char8", value_type_field);
    else if constexpr (std::is_same_v<T, char16_t>)
      std::strncpy(typestr, "char16", value_type_field);
    else if constexpr (std::is_same_v<T, char32_t>)
      std::strncpy(typestr, "char32", value_type_field);
    else if constexpr (std::is_same_v<T, wchar_t>)
      std::strncpy(typestr, " wchar", value_type_field);
    else
      std::strncpy(typestr, "??????", value_type_field);

    ([&] {
      std::strncpy(extraflags, ExtraFlags::name, sizeof(ExtraFlags::name) - 1);
      extraflags += sizeof(ExtraFlags::name) + 1;
    }(), ...);

    auto set_abistr = [&](const char* str) {
      std::size_t len = std::strlen(str);
      if (len > abi_field)
        {
          str += len - abi_field;
          len = abi_field;
        }
      const std::size_t ncpy = std::min(abi_field, len);
      std::memcpy(abistr, str, ncpy);
      if (len < abi_field)
        std::memset(abistr + ncpy, ' ', abi_field - ncpy);
    };

    set_abistr("");
    auto ref0 = bench_lat_thr<T, ExtraFlags...>(id);
    using V16 [[gnu::vector_size(16)]] = T;
    set_abistr("[[gnu::vector_size(16)]]");
    auto ref_vec = bench_lat_thr<V16, ExtraFlags...>(id, ref0);
    set_abistr("1");
    auto ref1 = bench_lat_thr<std::simd<T, 1>, ExtraFlags...>(id, ref_vec);
    set_abistr("2");
    auto ref2 = bench_lat_thr<std::simd<T, 2>, ExtraFlags...>(id, ref1);
    set_abistr("4");
    const auto ref = bench_lat_thr<std::simd<T, 4>, ExtraFlags...>(id, ref2);
    set_abistr("8");
    bench_lat_thr<std::simd<T, 8>, ExtraFlags...>(id, ref);
    set_abistr("16");
    bench_lat_thr<std::simd<T, 16>, ExtraFlags...>(id, ref);
    set_abistr("32");
    bench_lat_thr<std::simd<T, 32>, ExtraFlags...>(id, ref);
    set_abistr("64");
    bench_lat_thr<std::simd<T, 64>, ExtraFlags...>(id, ref);
    //set_abistr("fixed_size<12>");
    //bench_lat_thr<std::simd<T, fixed_size<12>>, ExtraFlags...>(id, ref);
    //set_abistr("fixed_size<16>");
    //bench_lat_thr<std::simd<T, fixed_size<16>>, ExtraFlags...>(id, ref);

    using V32 [[gnu::vector_size(32)]] = T;
    if constexpr (alignof(V32) == sizeof(V32))
      {
        set_abistr("[[gnu::vector_size(32)]]");
        bench_lat_thr<V32, ExtraFlags...>(id, ref);
      }
    using V64 [[gnu::vector_size(64)]] = T;
    if constexpr (alignof(V64) == sizeof(V64))
      {
        set_abistr("[[gnu::vector_size(64)]]");
        bench_lat_thr<V64, ExtraFlags...>(id, ref);
      }

    char sep[id_size + 2 * 15 + 2 * 12];
    std::memset(sep, '-', sizeof(sep) - 1);
    sep[sizeof(sep) - 1] = '\0';
    std::cout << sep << std::endl;
  }

template <long Iterations, int Retries = 10, class F>
  [[gnu::noinline]]
  double
  time_mean2(F&& fun)
  {
    struct {
      double mean = 0;
      double minimum = std::numeric_limits<double>::max();
      unsigned long tsc_start = 0;
      int todo = Retries;
      long it = 1;

      [[gnu::always_inline]]
      operator bool() &
      {
        if (--it > 0) [[likely]]
          return true;

        unsigned int tmp = 0;
        const auto tsc_end = __rdtscp(&tmp);
        if (todo < Retries)
          {
            const double elapsed = tsc_end - tsc_start;
            const double one_mean = elapsed / Iterations;
            mean += one_mean / Retries;
            minimum = std::min(minimum, one_mean);
          }
        if (todo) [[likely]]
          {
            --todo;
            it = Iterations + 1;
            tsc_start = __rdtscp(&tmp);
            return true;
          }
        return false;
      }
    } collector;
    fun(collector);
    return collector.minimum;
  }

template <long Iterations, int Retries = 10, class F, class... Args>
  double
  time_mean(F&& fun, Args&&... args)
  {
    double minimum = std::numeric_limits<double>::max();
    for (int tries = 0; tries < Retries; ++tries)
      {
        unsigned int tmp;
        long i = Iterations;
        const auto start = __rdtscp(&tmp);
        for (; i; --i)
          fun(std::forward<Args>(args)...);
        const auto end = __rdtscp(&tmp);
        const double elapsed = end - start;
        minimum = std::min(minimum, elapsed);
      }
    return minimum / Iterations;
  }

template <typename T>
  [[gnu::always_inline]] inline void
  fake_modify_one(T& x)
  {
    if constexpr (std::is_floating_point_v<T>)
      asm volatile("" : "+v,x"(x));
    else if constexpr (requires { {auto(__data(x))} -> vec_builtin; })
      fake_modify_one(__data(x));
    else if constexpr (requires { {auto(__data(x)[0])} -> vec_builtin; })
      {
        for (auto& d : __data(x))
          fake_modify_one(d);
      }
    else if constexpr (sizeof(x) >= 16)
      asm volatile("" : "+v,x"(x));
    else
      asm volatile("" : "+v,x,g"(x));
  }

template <typename... Ts>
  [[gnu::always_inline]] inline void
  fake_modify(Ts&... more)
  { (fake_modify_one(more), ...); }

template <typename T>
  [[gnu::always_inline]] inline void
  fake_read_one(const T& x)
  {
    if constexpr (std::is_floating_point_v<T>)
      asm volatile("" ::"v,x"(x));
    else if constexpr (requires { {auto(__data(x))} -> vec_builtin; })
      fake_read_one(__data(x));
    else if constexpr (requires { {auto(__data(x)[0])} -> vec_builtin; })
      {
        for (const auto& d : __data(x))
          fake_read_one(d);
      }
    else if constexpr (sizeof(x) >= 16)
      {
        static_assert(vec_builtin<T>);
        asm volatile("" ::"v,x"(x));
      }
    else
      asm volatile("" ::"v,x,g"(x));
  }

template <typename... Ts>
  [[gnu::always_inline]] inline void
  fake_read(const Ts&... more)
  { (fake_read_one(more), ...); }

template <typename T, std::size_t N>
  using carray = T[N];

template <long Iterations = 200'000, int Retries = 10, typename T, std::size_t N>
  double
  time_latency(carray<T, N>& data, auto&& process_one, auto&& fake_one)
  {
    for (auto& x : data)
      fake_modify_one(x);

    const double dt = time_mean<Iterations, Retries>([&] { process_one(data[0]); })
                        - time_mean<Iterations, Retries>([&] { fake_one(data[0]); });

    if (dt >= 0.98)
      return dt;

    std::cerr << "impossible latency value: " << dt << '\n';
    std::abort();
  }

template <long Iterations = 200'000, int Retries = 10, typename T, std::size_t N>
  double
  time_throughput(carray<T, N>& data, auto&& process_one, auto&& fake_one)
  {
    for (auto& x : data)
      fake_modify_one(x);

    const double dt
      = (time_mean<Iterations, Retries>([&]<std::size_t... Is>(std::index_sequence<Is...>) {
           (process_one(data[Is]), ...);
         }, std::make_index_sequence<N>())
           - time_mean<Iterations, Retries>([&]<std::size_t... Is>(std::index_sequence<Is...>) {
               (fake_one(data[Is]), ...);
             }, std::make_index_sequence<N>()))
          / N;

    return dt;
  }

template <typename T>
  [[gnu::always_inline]] inline auto
  subscript_or_value(T x, int i)
  {
    if constexpr(std::is_arithmetic_v<T>)
      return x;
    else
      return x[i];
  }

#define MAKE_VECTORMATH_OVERLOAD(name)                                         \
template <class T, class... More,                                              \
	  class VT = std::experimental::_VectorTraits<T>>                      \
  T name(T a, More... more)                                                    \
  {                                                                            \
    T r;                                                                       \
    for (int i = 0; i < VT::_S_full_size; ++i)                                 \
      r[i] = std::name(a[i], subscript_or_value(more, i)...);                  \
    return r;                                                                  \
  }

#define FUN1(fun)                                                   \
MAKE_VECTORMATH_OVERLOAD(fun)                                       \
typedef struct F_##fun                                              \
{                                                                   \
  static constexpr char name[] = #fun "(x)";                        \
                                                                    \
  template <class T>                                                \
    [[gnu::always_inline]] static auto                              \
    apply(const T& x)                                               \
    {                                                               \
      using ::fun;                                                  \
      using std::fun;                                               \
      return fun(x);                                                \
    }                                                               \
}

#define FUN2(fun)                                                   \
MAKE_VECTORMATH_OVERLOAD(fun)                                       \
typedef struct F_##fun                                              \
{                                                                   \
  static constexpr char name[] = #fun "(x, y)";                     \
                                                                    \
  template <class T, class U>                                       \
    [[gnu::always_inline]] static auto                              \
    apply(const T& x, const U& y)                                   \
    {                                                               \
      using ::fun;                                                  \
      using std::fun;                                               \
      return fun(x, y);                                             \
    }                                                               \
}

#endif  // BENCH_H_
