#include "../simd.h"

auto f(std::vector<float>& data) {
  using V = std::simd<float>;
  const auto end = data.end();
  for (auto it = data.begin(); it + V::size() <= end; it += V::size()) {
    std::simd<float> x(it);
    x += 1;
    x.copy_to_if(it, x > 0);
  }
}
