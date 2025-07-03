/*!
* \file    peak_ipl_attribute_defines.h
 *
 * \author  IDS Imaging Development Systems GmbH
 * \date    2025-02-01
 * \since   1.15
 *
 * Copyright (c) 2025, IDS Imaging Development Systems GmbH. All rights reserved.
 */

#pragma once


#if __cplusplus >= 201703L || _MSVC_LANG >= 201703L
#    define PEAK_IPL_NO_DISCARD [[nodiscard]]
#    define PEAK_IPL_MAYBE_UNUSED [[maybe_unused]]
#else
#    ifdef _MSC_VER
#        define PEAK_IPL_NO_DISCARD _Check_return_
#        define PEAK_IPL_MAYBE_UNUSED
#    elif defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#        define PEAK_IPL_NO_DISCARD __attribute__((warn_unused_result))
#        define PEAK_IPL_MAYBE_UNUSED __attribute__((unused))
#    else
#        define PEAK_IPL_NO_DISCARD
#        define PEAK_IPL_MAYBE_UNUSED
#    endif
#endif

