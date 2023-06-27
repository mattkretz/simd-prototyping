#include "../simd.h"

using V = std::simd<float, std::simd<float>::size() * 8>;

auto peak_ret(V x)
{
  for (int i = 0; i < 1'000'000; ++i)
    x = x * 2.f - 1.f;
  return x;
}
