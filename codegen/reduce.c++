#include "../simd_reductions.h"

auto
f0(std::simd<int, 4> x)
{ return reduce(x); }

auto
f1(std::simd<float, 4> x)
{ return reduce(x); }

auto
f2(std::simd<double, 2> x)
{ return reduce(x); }

auto
f3(std::simd<unsigned short, 8> x)
{ return reduce(x); }

auto
f4(std::simd<unsigned short, 4> x)
{ return reduce(x); }

auto
f5(std::simd<signed char, 16> x)
{ return reduce(x); }

auto
f6(std::simd<unsigned char, 16> x)
{ return reduce(x); }

auto
f7(std::simd<long long, 2> x)
{ return reduce(x); }
