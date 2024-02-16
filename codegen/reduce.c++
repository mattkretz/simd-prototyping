#include "../simd_reductions.h"

/* codegen
^f0(
vmovdqa
vpsrldq	.*, 8
vpaddd
vpshufd	.*, 229
vpaddd
vmovd	eax
ret
 */
auto
f0(std::simd<int, 4> x)
{ return reduce(x); }

/* codegen
^f1(
vmovhlps
vaddps
vpermilps	.*, 229
vaddss	xmm0
ret
 */
auto
f1(std::simd<float, 4> x)
{ return reduce(x); }

/* codegen
^f2(
vpermilpd
vaddsd	xmm0
ret
 */
auto
f2(std::simd<double, 2> x)
{ return reduce(x); }

/* codegen
^f3(
vmovdqa	xmm1, xmm0
vpsrldq	xmm0, xmm0, 8
vpaddw	xmm0, xmm0, xmm1
vpshuflw	xmm1, xmm0, 238
vpaddw	xmm0, xmm0, xmm1
vpshuflw	xmm1, xmm0, 229
vpaddw	xmm0, xmm0, xmm1
vpextrw	eax, xmm0, 0
ret
 */
auto
f3(std::simd<unsigned short, 8> x)
{ return reduce(x); }

/* codegen
^f4(
vmovq	xmm1, xmm0
vpshuflw	xmm0, xmm1, 238
vpaddw	xmm0, xmm0, xmm1
vpshuflw	xmm1, xmm0, 229
vpaddw	xmm0, xmm0, xmm1
vpextrw	eax, xmm0, 0
ret
 */
auto
f4(std::simd<unsigned short, 4> x)
{ return reduce(x); }

/* codegen
^f5(
vmovdqa	xmm1, xmm0
vpsraw	xmm0, xmm0, 8
vpaddw	xmm0, xmm0, xmm1
vpsrldq	xmm1, xmm0, 8
vpaddw	xmm0, xmm0, xmm1
vpshuflw	xmm1, xmm0, 238
vpaddw	xmm0, xmm0, xmm1
vpshuflw	xmm1, xmm0, 229
vpaddw	xmm0, xmm0, xmm1
vpextrw	eax, xmm0, 0
ret
 */
auto
f5(std::simd<signed char, 16> x)
{ return reduce(x); }

/* codegen
^f6(
vmovdqa	xmm1, xmm0
vpsraw	xmm0, xmm0, 8
vpaddw	xmm0, xmm0, xmm1
vpsrldq	xmm1, xmm0, 8
vpaddw	xmm0, xmm0, xmm1
vpshuflw	xmm1, xmm0, 238
vpaddw	xmm0, xmm0, xmm1
vpshuflw	xmm1, xmm0, 229
vpaddw	xmm0, xmm0, xmm1
vpextrw	eax, xmm0, 0
ret
 */
auto
f6(std::simd<unsigned char, 16> x)
{ return reduce(x); }

/* codegen
^f7(
vmovdqa	xmm1, xmm0
vpsrldq	xmm0, xmm0, 8
vpaddq	xmm0, xmm1, xmm0
vmovq	rax, xmm0
ret
 */
auto
f7(std::simd<long long, 2> x)
{ return reduce(x); }
