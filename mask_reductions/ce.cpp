#include "../simd.h"
#include "../mask_reductions.h"

int unspecified_value(std::vector<int> x)
{
  const int N = x.size();
  for (int i = 0; i < N; i += std::simd<int>::size)
  {
    std::simd<int> values(x.begin() + i);
    if (any_of(values < 0))
      return i + reduce_min_index(values < 0);
  }
  return -1;
}

int unspecified_value_nocheck(std::simd<int> x)
{
  x[x.size - 1] = -1;
  return reduce_min_index(x < 0);
}
