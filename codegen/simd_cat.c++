/* codegen
^f0(
.
.
vinserti128	ymm0, ymm0, xmm1, 1

^f1(
.
.
vinsertf128	ymm0, ymm0, xmm1, 1

^f2(
.
.
vmovlhps	xmm0, xmm0, xmm1
*/

#include "../simd_split.h"

void g(auto);

void f0(std::simd<int, 4> a, std::simd<int, 4> b) {
  g(simd_cat(a, b));
}

void f1(std::simd<float, 4> a, std::simd<float, 4> b) {
  g(simd_cat(a, b));
}

void f2(std::simd<float, 2> a, std::simd<float, 2> b) {
  g(simd_cat(a, b));
}

void f3(std::simd<float, 3> a, std::simd<float, 1> b) {
  g(simd_cat(a, b));
}
