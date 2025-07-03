/*!
* \file    peak_ipl_library.hpp
*
* \author  IDS Imaging Development Systems GmbH
* \date    2023-06-20
* \since   1.9
*
* Copyright (c) 2019 - 2025, IDS Imaging Development Systems GmbH. All rights reserved.
*/

#pragma once

#include <peak_ipl/backend/peak_ipl_backend.h>
#include <peak_ipl/exception/peak_ipl_exception.hpp>
#include <peak_ipl/types/peak_ipl_simple_types.hpp>

#include <cstddef>
#include <cstdint>

/*!
* \brief The "peak::ipl" namespace contains the whole image processing library.
*/
namespace peak
{
namespace ipl
{

/*!
 * Version information for ids_peak_ipl
 */
struct Version_t
{
    std::uint32_t major;    //! Major
    std::uint32_t minor;    //! Minor
    std::uint32_t subminor; //! Subminor

    /*!
     * The String representation for the ids_peak_ipl version
     * \returns the string representation
     */
    PEAK_IPL_NO_DISCARD std::string ToString() const
    {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(subminor);
    }
};

/*!
* \brief Library related functions
*/
class Library final
{
public:
    Library() = delete;
    ~Library() = delete;

    Library(const Library&) = delete;
    Library(Library&&) = delete;

    Library& operator=(const Library&) = delete;
    Library& operator=(Library&&) = delete;

    /*!
     * \brief Queries the library version.
     *
     * \return The library version as Version_t
     *
     * \throws Exception An internal error has occurred.
     *
     * \since 1.9.2
     */
    PEAK_IPL_NO_DISCARD static Version_t Version();

    /*!
     * \brief Queries the maximum number of concurrent threads used by the library.
     *
     * \returns The number of concurrent threads used by the library. Or 0 to let the library decide.
     *
     * \throws Exception An internal error has occurred.
     *
     * \since 1.9.2
     */
    PEAK_IPL_NO_DISCARD static size_t MaxConcurrentThreads();

    /*!
     * \brief Sets the maximum number of concurrent threads used by the library.
     *
     * Set the number of threads to 0, to let the library decide the amount. Select any other number to set the amount
     * manually.
     *
     * \param[in] concurrentThreads The number of concurrent threads used by the library.
     *
     * \throws Exception An internal error has occurred.
     *
     * \since 1.9.2
     */
    static void SetMaxConcurrentThreads(size_t concurrentThreads);

    /*!
     * \brief Get if multithreading is enabled in the library
     *
     * \returns true if multithreading is enabled, false otherwise.
     *
     * \throws Exception An internal error has occurred.
     *
     * \since 1.9.2
     */
    PEAK_IPL_NO_DISCARD static bool MultiThreadingEnabled();

    /*!
     * \brief Disables/Enables multithreading in the library
     *
     * \param[in] enabled set to true to enable multithreading, false to disable it
     *
     * \throws Exception An internal error has occurred.
     *
     * \since 1.9.2
     */
    static void SetMultiThreadingEnabled(bool enabled);

    /*!
     * \brief Get if SIMD is enabled in the library
     *
     * \return True, if SIMD optimized versions are used, otherwise they're not used.
     *
     * \throws Exception An internal error has occurred.
     *
     * \since 1.9.2
     */
    PEAK_IPL_NO_DISCARD static bool SIMDEnabled();

    /*!
     * \brief Disables/Enables SIMD in the library
     *
     * \param[in] enabled set to true to use the SIMD optimized functions, false to disable them
     *
     * \note This is normally true
     *
     * \throws Exception An internal error has occurred.
     *
     * \since 1.9.2
     */
    static void SetSIMDEnabled(bool enabled);
};

inline Version_t Library::Version()
{
   uint32_t majorVersion{};
   uint32_t minorVersion{};
   uint32_t subminorVersion{};

   ExecuteAndMapReturnCodes([&] {
       return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Library_GetVersionMajor(&majorVersion);
   });

   ExecuteAndMapReturnCodes([&] {
       return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Library_GetVersionMinor(&minorVersion);
   });

   ExecuteAndMapReturnCodes([&] {
       return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Library_GetVersionSubminor(&subminorVersion);
   });

   return Version_t{majorVersion, minorVersion, subminorVersion};
}

inline size_t Library::MaxConcurrentThreads()
{
   size_t maxConcurrentThreads = 0;

   ExecuteAndMapReturnCodes([&] {
       return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Library_GetMaxConcurrentThreads(
           &maxConcurrentThreads);
   });

   return maxConcurrentThreads;
}

inline void Library::SetMaxConcurrentThreads(size_t concurrentThreads)
{
   ExecuteAndMapReturnCodes([&] {
       return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Library_SetMaxConcurrentThreads(concurrentThreads);
   });
}

inline bool Library::MultiThreadingEnabled()
{
   PEAK_IPL_BOOL8 multiThreadingEnabled{};

   ExecuteAndMapReturnCodes([&] {
       return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Library_GetMultiThreadingEnabled(&multiThreadingEnabled);
   });

   return multiThreadingEnabled != 0;
}

inline void Library::SetMultiThreadingEnabled(bool enabled)
{
   ExecuteAndMapReturnCodes([&] {
       return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Library_SetMultiThreadingEnabled(enabled ? 1 : 0);
   });
}

inline bool Library::SIMDEnabled()
{
   PEAK_IPL_BOOL8 simdEnabled{};

   ExecuteAndMapReturnCodes([&] {
       return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Library_GetSIMDEnabled(&simdEnabled);
   });

   return simdEnabled != 0;
}

inline void Library::SetSIMDEnabled(bool enabled)
{
   ExecuteAndMapReturnCodes([&] {
       return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Library_SetSIMDEnabled(enabled ? 1 : 0);
   });
}

} /* namespace ipl */
} /* namespace peak */
