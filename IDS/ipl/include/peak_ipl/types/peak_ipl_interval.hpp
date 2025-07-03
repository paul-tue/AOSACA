/*!
 * \file    peak_ipl_interval.hpp
 *
 * \author  IDS Imaging Development Systems GmbH
 * \date    2025-02-01
 * \since   1.15
 *
 * Copyright (c) 2025, IDS Imaging Development Systems GmbH. All rights reserved.
 */

#pragma once

#include <peak_ipl/backend/peak_ipl_attribute_defines.h>
#include <peak_ipl/exception/peak_ipl_exception.hpp>
#include <type_traits>
#include <cmath>
#include <limits>
#include <algorithm>

/*!
 * \brief The "peak::ipl" namespace contains the whole image processing library.
 */
namespace peak
{
namespace ipl
{

/*!
 * \brief Interval support and convenience class
 *
 * \since 1.15
 */
template <typename T>
class Interval
{
public:
    /*!
     * \brief Constructor for the interval
     *
     * \param start  The start value (inclusive) for the interval.
     * \param end  The end value (inclusive) for the interval.
     *
     * \throws InvalidArgumentException The end must be greater than start
     */
    constexpr Interval(T start, T end)
        : m_start{ start }
        , m_end{ end }
    {
        if (start > end)
        {
            throw InvalidArgumentException("start > end is not allowed!");
        }
        if (almostEqual(start, end))
        {
            throw InvalidArgumentException("start == end is not allowed!");
        }
    }

    /*!
     * \brief Get the start value (inclusive) for the interval
     *
     * \returns the start value.
     *
     * \since 1.15
     */
    PEAK_IPL_NO_DISCARD constexpr T Start() const
    {
        return m_start;
    }

    /*!
     * \brief Get the end value (inclusive) for the interval
     *
     * \returns the end value.
     *
     * \since 1.15
     */
    PEAK_IPL_NO_DISCARD constexpr T End() const
    {
        return m_end;
    }

    /*!
     * \brief Check if the value is in the interval
     *
     * \param value The value to check.
     *
     * \returns true if in the interval, false otherwise.
     *
     * \since 1.15
     */
    PEAK_IPL_NO_DISCARD constexpr bool InInterval(T value) const
    {
        return value >= m_start && value <= m_end;
    }

private:
    T m_start{}; //!< The start value for the interval
    T m_end{}; //!< The end value for the interval

    template <typename Q = T, std::enable_if_t<std::is_integral<Q>::value, int> = 0>
    static constexpr bool almostEqual(Q v1, Q v2)
    {
        return v1 == v2;
    }

    template <typename Q = T, std::enable_if_t<std::is_floating_point<Q>::value, int> = 0>
    static constexpr Q almostEqualMaxHelper(Q v1, Q v2)
    {
        return std::max<Q>({ 1.0, std::fabs(v1), std::fabs(v2) });
    }

    template <typename Q = T, std::enable_if_t<std::is_floating_point<Q>::value, int> = 0>
    static constexpr bool almostEqual(Q v1, Q v2)
    {
        return std::fabs(v1 - v2) <= std::numeric_limits<Q>::epsilon() * almostEqualMaxHelper(v1, v2);
    }
};

} /* namespace ipl */
} /* namespace peak */
