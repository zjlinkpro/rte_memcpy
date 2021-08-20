/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2015 RehiveTech. All rights reserved.
 */

#ifndef _RTE_MEMCPY_ARM_H_
#define _RTE_MEMCPY_ARM_H_

#ifdef RTE_ARCH_64
#include <rte_memcpy_arch64.h>
#elif RTE_ARCH_32
#include <rte_memcpy_arch32.h>
#else
#include <rte_memcpy_x86.h>
#endif

#endif /* _RTE_MEMCPY_ARM_H_ */
