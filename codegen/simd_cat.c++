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

#include "../simd"
namespace simd = std;

void g(auto);

void f0(simd::simd<int, 4> a, simd::simd<int, 4> b) {
  g(cat(a, b));
}

void f1(simd::simd<float, 4> a, simd::simd<float, 4> b) {
  g(cat(a, b));
}

void f2(simd::simd<float, 2> a, simd::simd<float, 2> b) {
  g(cat(a, b));
}

void f3(simd::simd<float, 3> a, simd::simd<float, 1> b) {
  g(cat(a, b));
}
