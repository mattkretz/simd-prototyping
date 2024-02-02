/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_POWER_DETAIL_H_
#define PROTOTYPE_POWER_DETAIL_H_

#include "detail.h"

#if __powerpc__
namespace std::__detail
{
  struct _MachineFlags
  {
    uint64_t _M_arch : 8
#ifdef _ARCH_PWR10
      = 5;
#elif defined __POWER9_VECTOR__
      = 4;
#elif defined __POWER8_VECTOR__
      = 3;
#elif defined __VSX__
      = 2;
#elif defined __VMX__
      = 1;
#else
      = 0;
#endif

    consteval bool _M_vmx()
    { return _M_arch >= 1; }

    consteval bool _M_vsx()
    { return _M_arch >= 2; }

    consteval bool _M_power8()
    { return _M_arch >= 3; }

    consteval bool _M_power9()
    { return _M_arch >= 4; }

    consteval bool _M_power10()
    { return _M_arch >= 5; }

    uint64_t _M_padding = 0;
  };

  static_assert(sizeof(_MachineFlags) == sizeof(uint64_t) * 2);
}
#endif

#endif // PROTOTYPE_POWER_DETAIL_H_
