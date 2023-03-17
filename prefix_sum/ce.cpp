#include "../simd.h"
#include "../iota.h"

namespace std
{
  namespace __detail
  {
    template <int __n>
      struct __prefix_sum_permutation
      {
        constexpr int
        operator()(int __i) const
        { return __i & __n ? (__i ^ __n) | (__n - 1) : __i; }
      };

    constexpr void
    __for_template(auto __begin, auto __end, auto&& __f)
    {
      if constexpr (__end > __begin)
        {
          __f(__begin);
          __for_template(std::__detail::__cnst<__begin + 1>, __end, __f);
        }
    }

    constexpr void
    __for_template(auto __end, auto&& __f)
    {
      if constexpr (__end > 0)
        {
          __f(std::__detail::__cnst<0>);
          __for_template(std::__detail::__cnst<1>, __end, __f);
        }
    }
  }

  template <typename _Tp, typename _Abi>
    constexpr simd<_Tp, _Abi>
    scan(simd<_Tp, _Abi> __v, auto&& __binary_op)
    {
      using _V = simd<_Tp, _Abi>;
      using _M = typename _V::mask_type;
      using _Ip = __detail::__int_for_sizeof_t<_Tp>;
      using _IV = rebind_simd_t<_Ip, _V>;
      __detail::__for_template(
        __detail::__cnst<std::bit_width(__v.size()) - 1>, [&](auto __i) {
          constexpr int __n = 1 << __i;
          const _M __k = std::bit_cast<_M>((iota_v<_IV> & _IV(__n)) != 0);
          __v = __k ? __binary_op(__v,
                            permute(__v, __detail::__prefix_sum_permutation<(1 << __i)>{}))
                    : __v;
        });
      return __v;
    }

  template <typename _Tp, typename _Abi>
    constexpr simd<_Tp, _Abi>
    prefix_sum(simd<_Tp, _Abi> const& __v)
    { return scan(__v, std::plus<>()); }

  template <typename _Tp, typename _Abi>
    constexpr simd<_Tp, _Abi>
    prefix_sum(simd<_Tp, _Abi> __v, _Tp __a)
    {
      using _V = simd<_Tp, _Abi>;
      using _M = typename _V::mask_type;
      using _Ip = __detail::__int_for_sizeof_t<_Tp>;
      using _IV = rebind_simd_t<_Ip, _V>;
      _V __factor = __a;
      __detail::__for_template(
        __detail::__cnst<std::bit_width(__v.size()) - 1>, [&](auto __i) {
          constexpr int __n = 1 << __i;
          const _M __k = std::bit_cast<_M>((iota_v<_IV> & _IV(__n)) != 0);
          __v = __k ? __v + __factor * permute(__v, __detail::__prefix_sum_permutation<__n>{})
                    : __v;
          __factor = __k ? __a * __factor : __factor;
          __a *= __a;
        });
      return __v;
    }

} // namespace std

auto f(float last, std::span<float> data)
{
  using V = std::simd<float>;
  constexpr float a = 0.125f;
  constexpr float b = 1.f - a;
  constexpr auto aa = std::scan(V(a), std::multiplies<>());
  for (std::size_t i = 0; i < 1024; i += V::size)
    {
      __asm volatile("# LLVM-MCA-BEGIN simd":::"memory");
      V x(data.begin() + i);
      V y = aa * last + prefix_sum(b * x, a);
      last = y[V::size - 1];
      y.copy_to(data.begin() + i);
      __asm volatile("# LLVM-MCA-END":::"memory");
    }
}

auto f_scalar(float last, std::span<float> data)
{
  constexpr float a = 0.125f;
  constexpr float b = 1.f - a;
  for (std::size_t i = 0; i < 1024; ++i)
    {
      __asm volatile("# LLVM-MCA-BEGIN scalar":::"memory");
      const float y = a * last + b * data[i];
      last = y;
      data[i] = y;
      __asm volatile("# LLVM-MCA-END":::"memory");
    }
}

constexpr auto x = prefix_sum(std::simd<int>(1), 2);
auto xx = x;
