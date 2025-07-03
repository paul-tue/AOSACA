/*!
* \file    peak_bufferpart_converter_ipl.hpp
*
* \author  IDS Imaging Development Systems GmbH
* \date    2024-04-08
* \since   1.8.0
*
* Copyright (c) 2024 - 2025, IDS Imaging Development Systems GmbH. All rights reserved.
*/

#pragma once


#include <peak_ipl/peak_ipl.hpp>
#include <peak/data_stream/peak_data_stream.hpp>
#include <peak/device/peak_device.hpp>
#include <peak/peak_buffer_converter.hpp>

#include <cassert>


namespace peak
{

/*!
* \brief Converts a core::BufferPart into a peak::ipl::Image.
*
* This creates a peak::ipl::Image as a shallow copy of the buffer (i.e. using the same memory).
*
* \param[in] part The buffer part to convert
* \returns The buffer part converted to an Image.
* \remark Remember that the buffer part's memory is only under your control until you re-queue the buffer.
*
* \since 1.8
*
* \note To use this method, this file needs to be included explicitly:
* \code
* #include <peak/converters/peak_bufferpart_converter_ipl.hpp>
* \endcode
*/
template <>
inline peak::ipl::Image BufferPartTo(const std::shared_ptr<peak::core::BufferPart>& part)
{
   if (part == nullptr)
   {
       throw core::InvalidArgumentException("The given buffer is a nullptr!");
   }

   const auto partType = part->Type();

   if (partType == core::BufferPartType::Image2D || partType == core::BufferPartType::ConfidenceMap || partType == core::BufferPartType::Image3D)
   {
       assert(part->FormatNamespace() != static_cast<uint64_t>(core::PixelFormatNamespace::IIDC)
           && part->FormatNamespace() != static_cast<uint64_t>(core::PixelFormatNamespace::Custom));

       const size_t height = part->Height();
       const size_t width = part->Width();
       const size_t byteCount = part->Size();
       uint8_t* firstPixelPtr = static_cast<uint8_t*>(part->BasePtr());

       return peak::ipl::Image(static_cast<peak::ipl::PixelFormatName>(part->Format()),
           firstPixelPtr, byteCount, width, height, part->ParentBuffer()->Timestamp_ns());
   }
   else
   {
       throw core::InvalidCastException("BufferPart has no image data.");
   }
}

} /* namespace peak */
