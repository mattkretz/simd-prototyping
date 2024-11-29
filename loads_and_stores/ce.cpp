#include "../simd"

namespace simd = std;

auto f(std::vector<float>& data) {
  using V = simd::simd<float>;
  const auto end = data.end();
  for (auto it = data.begin(); it + V::size() <= end; it += V::size()) {
    simd::simd<float> x = simd::load(it, V::size(), simd::flag_default_init);
    x += 1.f;
    simd::store(x, it, V::size(), x > 0.f);
  }
}

auto runtime_sized(std::vector<float>& data) {
  simd::simd<float> x = simd::load(data, simd::flag_throw);
  return x + 1.f;
}

auto compiletime_sized(std::array<float, 8>& data) {
  simd::simd<float> x = simd::load(data, simd::flag_throw);
  return x + 1.f;
}

  /*void naive_range_iteration(std::vector<float> &data) {
  using V = simd::simd<float>;
  for (auto it = data.begin(); it < data.end(); it += V::size())
    {
      auto chunk = std::ranges::subrange(it, data.end());
      static_assert(simd::__detail::__static_range_size<decltype(chunk)> == std::dynamic_extent);
      simd::simd<float> x = simd::load(chunk, simd::flag_default_init);
      x += 1.f;
      simd::store(x, chunk);
    }
}

void smart_range_iteration(std::vector<float> &data) {
  using V = simd::simd<float>;
  auto it = data.begin();
  auto simd_end = data.end() - V::size();
  for (; it <= simd_end; it += V::size())
    {
      auto chunk = std::ranges::subrange(it, it + V::size());
      simd::simd<float> x = simd::load(chunk, simd::flag_default_init);
      x += 1.f;
      simd::store(x, chunk);
    }
  auto chunk = std::ranges::subrange(it, data.end());
  simd::simd<float> x = simd::load(chunk, simd::flag_default_init);
  x += 1.f;
  simd::store(x, chunk);
}*/
