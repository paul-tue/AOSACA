/*!
 * \file    peak_ipl_image_converter.hpp
 *
 * \author  IDS Imaging Development Systems GmbH
 * \date    2019-05-01
 * \since   1.0
 *
 * Copyright (c) 2019 - 2025, IDS Imaging Development Systems GmbH. All rights reserved.
 */

#pragma once

#include <peak_ipl/backend/peak_ipl_backend.h>
#include <peak_ipl/exception/peak_ipl_exception.hpp>
#include <peak_ipl/types/peak_ipl_pixel_format.hpp>
#include <peak_ipl/types/peak_ipl_simple_types.hpp>

#include <peak_ipl/types/peak_ipl_range.hpp>
#include <peak_ipl/types/peak_ipl_interval.hpp>
#include <cstdint>
#include <utility>
#include <vector>

/*!
 * \brief The "peak::ipl" namespace contains the whole image processing library.
 */
namespace peak
{
namespace ipl
{

class Image;

/*!
 * \brief Converts images from one PixelFormat to another.
 * 
 * \note In order to speed up processing, instances of this class maintain internal memory pools,
 * which allows reuse of memory instead of allocating new memory on each conversion.\n
 * The memory is freed when the instance is destroyed. There is one pool for every needed byte count, therefore, a new
 * pool will be allocated if an image with a new size or format needs to be processed.\n
 * One may select to create a new instance of this class when changing image settings in order to free old pools.
 * If a new pool or buffer is needed, they will get allocated automatically during the conversion, which
 * results in the first conversion taking more time.\n
 * This can be prevented by using the function #ImageConverter::PreAllocateConversion before the conversion.
 * Buffers can be reused once a converted image gets freed.
 * 
 */
class ImageConverter final
{
public:
    /*! \brief Constructor for a new ImageConverter instance
     *
     * Creates a new ImageConverter instance.
     *
     * \since 1.0
     */
    ImageConverter();
    
    /*! \brief Destructor for the ImageConverter instance
     *
     * Destroys an ImageConverter instance.
     *
     * \since 1.0
     */
    ~ImageConverter();

    /*! \brief Copy Constructor for a new ImageConverter instance
     *
     * Creates a new ImageConverter instance.
     *
     * \note This only creates a shallow copy. See \ref shallowCopy for a detailed explanation.
     *
     * \param[in] other The other instance to acquire a reference for.
     * 
     * \since 1.0 Deleted
     * \since 1.8 Implemented as shallow copy
     */
    ImageConverter(const ImageConverter& other);

    /*! \brief Copy assigment for ImageConverter instance
     *
     * Acquires a reference from \p other to this instance. If this instance already has a reference, it will be destroyed.
     *
     * \note See \ref shallowCopy for a detailed explanation.
     * 
     * \param[in] other The other instance to acquire a reference for.
     *
     * \since 1.0 Deleted
     * \since 1.8 Implemented as shallow copy
     */
    ImageConverter& operator=(const ImageConverter& other);

    /*! \brief Move constructor for ImageConverter instance
     *
     * Moves the instance from \p other to this. Afterwards \p other is invalid.
     * 
     * \param[in] other The other instance to move.
     *
     * \since 1.0
     */
    ImageConverter(ImageConverter&& other) noexcept;

    /*! \brief Move assignment for ImageConverter instance
     *
     * Moves the instance from \p other to this. Afterwards \p other is invalid.
     * 
     * \param[in] other The other instance to move.
     *
     * \since 1.0
     */
    ImageConverter& operator=(ImageConverter&& other) noexcept;

    /*!
     * \brief Convenience constructor to directly pre-allocate an internal buffer pool.
     * This calls [PreAllocateConversion](\ref ImageConverter::PreAllocateConversion) internally.
     * 
     * \param[in] inputPixelFormat The pixelformat of the source image
     * \param[in] outputPixelFormat The pixelformat of the converted image
     * \param[in] width Width of the source image
     * \param[in] height Height of the source image
     * \param[in] imageCount Maximum number of converted images held at one time
     * 
     * \throws Exception An internal error has occurred.
     *
     * \since 1.4
     */
    ImageConverter(const peak::ipl::PixelFormat& inputPixelFormat,
        const peak::ipl::PixelFormat& outputPixelFormat, size_t width, size_t height, size_t imageCount = 5);

    /*!
     * \brief Pre-allocate buffer pools needed for the specified conversion.
     * 
     * \param[in] inputPixelFormat The pixelformat of the source image
     * \param[in] outputPixelFormat The pixelformat of the converted image
     * \param[in] width Width of the source image
     * \param[in] height Height of the source image
     * \param[in] imageCount Maximum number of converted images held at one time
     *
     * \throws Exception An internal error has occurred.
     * 
     * \since 1.4
     */
    void PreAllocateConversion(const peak::ipl::PixelFormat& inputPixelFormat,
        const peak::ipl::PixelFormat& outputPixelFormat, size_t width, size_t height, size_t imageCount = 5);

    /*!
     * \brief Returns the current conversion mode.
     *
     * \returns The current conversion mode
     *
     * \throws Exception An internal error has occurred.
     *
     * \since 1.0
     */
    PEAK_IPL_NO_DISCARD peak::ipl::ConversionMode ConversionMode() const;

    /*!
     * \brief Set conversion mode.
     *
     * \param[in] conversionMode The conversion mode to set.
     *
     * \throws Exception An internal error has occurred.
     *
     * \since 1.0
     */
    void SetConversionMode(peak::ipl::ConversionMode conversionMode);

    /*!
     * \brief Returns the supported output pixel formats for a given input pixel format.
     *
     * \param[in] inputPixelFormat The input pixel format.
     *
     * \returns A vector of supported pixel formats for the given input pixel format
     *
     * \throws Exception An internal error has occurred.
     *
     * \since 1.0
     */
    PEAK_IPL_NO_DISCARD std::vector<peak::ipl::PixelFormatName> SupportedOutputPixelFormatNames(
        const peak::ipl::PixelFormat& inputPixelFormat) const;

    /*!
     * \brief Converts the given \p inputImage to the specified \p outputPixelFormat.
     *
     * \param[in] inputImage        The source image to be converted.
     * \param[in] outputPixelFormat The desired pixel format for the output image.
     *
     * \returns A new peak::ipl::Image containing the content of \p inputImage in the specified
     *          \p outputPixelFormat.
     *
     * \throws Exception An internal error has occurred.
     *
     * \since 1.0
     */
    PEAK_IPL_NO_DISCARD Image Convert(const Image& inputImage, const peak::ipl::PixelFormat& outputPixelFormat) const;

    /*! \brief Converts the given \p inputImage to the specified \p outputPixelFormat into an existing
     *         \p outputImageBuffer.
     *
     * \param[in]  inputImage            The source image to be converted.
     * \param[in]  outputPixelFormat     The desired pixel format for the output image.
     * \param[in]  outputImageBuffer     Pointer to the output buffer.
     * \param[in]  outputImageBufferSize Size of the output buffer.
     *
     * \returns A peak::ipl::Image object representing the converted image. The returned Image object encapsulates
     *          the \p outputImageBuffer. Note that the memory is not owned by the image object. Ensure the buffer is
     *          not deleted until you have finished working with the image.
     *
     * \throws BufferTooSmallException  The provided output buffer is too small for the output image.
     * \throws InvalidArgumentException The provided output buffer pointer is invalid.
     * \throws Exception                An internal error has occurred.
     *
     * \since 1.1
     */
    Image Convert(const Image& inputImage, const peak::ipl::PixelFormat& outputPixelFormat,
        uint8_t* outputImageBuffer, size_t outputImageBufferSize) const;

    /*!
     * \brief Converts the given \p inputImage to the specified \p outputPixelFormat and scales the pixel values
     *        by the \p factor.
     *
     * \note The \p outputPixelFormat must be a float pixel format.
     *
     * \param[in] inputImage        The source image to be converted.
     * \param[in] outputPixelFormat The desired pixel format for the output image.
     * \param[in] factor            Scale factor that is applied to each pixel value.
     * \param[in] offset            The offset to apply to all output pixels.
     *
     * \returns A new peak::ipl::Image containing the content of \p inputImage in the specified
     *          \p outputPixelFormat with pixel values scaled by \p factor (\a m) and \p offset (\a c) added
     *          according to <em>y = m · x + c</em>.
     *
     * \throws ImageFormatNotSupportedException The given input or output pixel format is not supported.
     * \throws Exception An internal error has occurred.
     *
     * \since 1.15
     */
    PEAK_IPL_NO_DISCARD Image Convert(const Image& inputImage, const peak::ipl::PixelFormat& outputPixelFormat,
        double factor, double offset = 0.0) const;

    /*!
     * \brief Converts the given \p inputImage to the specified \p outputPixelFormat into an existing
     *        \p outputImageBuffer and scales the pixel values by the \p factor.
     *
     * \param[in] inputImage             The source image to be converted.
     * \param[in] outputPixelFormat      The desired pixel format for the output image.
     * \param[in] outputImageBuffer      Pointer to the output buffer.
     * \param[in] outputImageBufferSize  Size of the output buffer.
     * \param[in] factor                 Scale factor that is applied to each pixel value.
     * \param[in] offset                 The offset to apply to all output pixels.
     *
     * \returns A peak::ipl::Image object representing the converted image. The returned Image object encapsulates
     *          the \p outputImageBuffer with pixel values scaled by \p factor (\a m) and \p offset (\a c) added
     *          according to <em>y = m · x + c</em>. Note that the memory is not owned by the image object. Ensure the
     *          buffer is not deleted until you have finished working with the image.
     *
     * \throws ImageFormatNotSupportedException The given input or output pixel format is not supported.
     * \throws BufferTooSmallException The provided buffer is too small for the output image.
     * \throws InvalidArgumentException The given output buffer pointer is invalid.
     * \throws Exception An internal error has occurred.
     *
     * \since 1.15
     */
    PEAK_IPL_NO_DISCARD Image Convert(const Image& inputImage, const peak::ipl::PixelFormat& outputPixelFormat,
        uint8_t* outputImageBuffer, size_t outputImageBufferSize, double factor, double offset = 0.0) const;

    /*!
    * \brief Converts the given \p inputImage to the specified \p outputPixelFormat. The pixel values are mapped from
     *        \p interval to the value range of the \p outputPixelFormat. All pixel values outside the interval will be
     *        clamped.
     *
     * \note The pixel format of the \p inputImage must be a float pixel format.
     *
     * \param[in] inputImage        The source image to be converted.
     * \param[in] outputPixelFormat The desired pixel format for the output image.
     * \param[in] interval          The interval of the input range.
     *
     * \returns A new peak::ipl::Image containing the content of \p inputImage in the specified
     *          \p outputPixelFormat with mapped pixel values.
     *
     * \throws ImageFormatNotSupportedException The given input or output pixel format is not supported.
     * \throws Exception An internal error has occurred.
     *
     * \since 1.15
     */
    PEAK_IPL_NO_DISCARD Image Convert(const Image& inputImage, const peak::ipl::PixelFormat& outputPixelFormat,
        peak::ipl::Interval<double> interval) const;

    /*!
     * \brief Converts the given \p inputImage to the specified \p outputPixelFormat into an existing
     *        \p outputImageBuffer. The pixel values are mapped from \p interval to the value range of the
     *        \p outputPixelFormat. All pixel values outside the interval will be clamped.
     *
     * \note The pixel format of the \p inputImage must be a float pixel format.
     *
     * \param[in] inputImage             The source image to be converted.
     * \param[in] outputPixelFormat      The desired pixel format for the output image.
     * \param[in] outputImageBuffer      Pointer to the output buffer.
     * \param[in] outputImageBufferSize  Size of the output buffer.
     * \param[in] interval               The interval of the input range.
     *
     * \returns A peak::ipl::Image object representing the converted image. The returned Image object encapsulates
     *          the \p outputImageBuffer with mapped pixel values. Note that the memory is not owned by the
     *          image object. Ensure the buffer is not deleted until you have finished working with the image.
     *
     * \throws ImageFormatNotSupportedException The given input or output pixel format is not supported.
     * \throws BufferTooSmallException The provided buffer is too small for the output image.
     * \throws InvalidArgumentException The given output buffer pointer is invalid.
     * \throws Exception An internal error has occurred.
     *
     * \since 1.15
     */
    PEAK_IPL_NO_DISCARD Image Convert(const Image& inputImage, const peak::ipl::PixelFormat& outputPixelFormat,
        uint8_t* outputImageBuffer, size_t outputImageBufferSize, peak::ipl::Interval<double> interval) const;

private:
    PEAK_IPL_IMAGE_CONVERTER_HANDLE m_backendHandle{};
};

} /* namespace ipl */
} /* namespace peak */

#include <peak_ipl/types/peak_ipl_image.hpp>

namespace peak
{
namespace ipl
{

inline ImageConverter::ImageConverter()
{
    ExecuteAndMapReturnCodes([&] { return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_Construct(&m_backendHandle); });
}

inline ImageConverter::ImageConverter(const peak::ipl::PixelFormat& inputPixelFormat,
    const peak::ipl::PixelFormat& outputPixelFormat, const size_t width, const size_t height, const size_t imageCount)
{
    ExecuteAndMapReturnCodes([&] { return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_Construct(&m_backendHandle); });
    PreAllocateConversion(inputPixelFormat, outputPixelFormat, width, height, imageCount);
}

inline ImageConverter::~ImageConverter()
{
    if (m_backendHandle)
    {
        (void)PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_Destruct(m_backendHandle);
    }
}

inline ImageConverter::ImageConverter(const ImageConverter& other)
{
    if (other.m_backendHandle)
    {
        ExecuteAndMapReturnCodes([&] { return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_Acquire(other.m_backendHandle); });
    }

    m_backendHandle = other.m_backendHandle;
}

inline ImageConverter& ImageConverter::operator=(const ImageConverter& other)
{
    if (this != &other)
    {
        if (m_backendHandle)
        {
            (void)PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_Destruct(m_backendHandle);
        }

        if (other.m_backendHandle)
        {
            ExecuteAndMapReturnCodes(
                [&] { return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_Acquire(other.m_backendHandle); });
        }
    }

    m_backendHandle = other.m_backendHandle;

    return *this;
}

inline ImageConverter::ImageConverter(ImageConverter&& other) noexcept
{
    *this = std::move(other);
}

inline ImageConverter& ImageConverter::operator=(ImageConverter&& other) noexcept
{
    if (this != &other)
    {
        if (m_backendHandle != nullptr)
        {
            (void)PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_Destruct(m_backendHandle);
        }

        m_backendHandle = other.m_backendHandle;
        other.m_backendHandle = nullptr;
    }

    return *this;
}

inline void ImageConverter::PreAllocateConversion(const peak::ipl::PixelFormat& inputPixelFormat,
    const peak::ipl::PixelFormat& outputPixelFormat, const size_t width, const size_t height,
    const size_t imageCount)
{
    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_PreAllocateConversion(m_backendHandle,
            static_cast<PEAK_IPL_PIXEL_FORMAT>(inputPixelFormat.PixelFormatName()),
            static_cast<PEAK_IPL_PIXEL_FORMAT>(outputPixelFormat.PixelFormatName()), width, height, imageCount);
    });
}

inline peak::ipl::ConversionMode ImageConverter::ConversionMode() const
{
    peak::ipl::ConversionMode conversionMode = peak::ipl::ConversionMode::Fast;

    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_GetConversionMode(
            m_backendHandle, reinterpret_cast<PEAK_IPL_CONVERSION_MODE*>(&conversionMode));
    });

    return conversionMode;
}

inline void ImageConverter::SetConversionMode(peak::ipl::ConversionMode conversionMode)
{
    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_SetConversionMode(
            m_backendHandle, static_cast<PEAK_IPL_CONVERSION_MODE>(conversionMode));
    });
}

inline std::vector<PixelFormatName> ImageConverter::SupportedOutputPixelFormatNames(
    const PixelFormat& inputPixelFormat) const
{
    size_t size = 0;
    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_GetSupportedOutputPixelFormats(
            m_backendHandle, static_cast<PEAK_IPL_PIXEL_FORMAT>(inputPixelFormat.PixelFormatName()), nullptr, &size);
    });
    std::vector<PixelFormatName> supportedOutputPixelFormats(size);
    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_GetSupportedOutputPixelFormats(m_backendHandle,
            static_cast<PEAK_IPL_PIXEL_FORMAT>(inputPixelFormat.PixelFormatName()),
            reinterpret_cast<PEAK_IPL_PIXEL_FORMAT*>(supportedOutputPixelFormats.data()), &size);
    });

    return supportedOutputPixelFormats;
}

inline Image ImageConverter::Convert(const Image& inputImage, const PixelFormat& outputPixelFormat) const
{
    PEAK_IPL_IMAGE_HANDLE outputImageHandle = nullptr;
    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_Convert(m_backendHandle,
            ImageBackendAccessor::BackendHandle(inputImage),
            static_cast<PEAK_IPL_PIXEL_FORMAT>(outputPixelFormat.PixelFormatName()), &outputImageHandle);
    });

    return ImageBackendAccessor::CreateImage(outputImageHandle);
}

inline Image ImageConverter::Convert(const Image& inputImage, const PixelFormat& outputPixelFormat,
    uint8_t* outputImageBuffer, size_t outputImageBufferSize) const
{
    PEAK_IPL_IMAGE_HANDLE outputImageHandle = nullptr;
    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_ConvertToBuffer(m_backendHandle,
            ImageBackendAccessor::BackendHandle(inputImage),
            static_cast<PEAK_IPL_PIXEL_FORMAT>(outputPixelFormat.PixelFormatName()), outputImageBuffer,
            outputImageBufferSize, &outputImageHandle);
    });

    return ImageBackendAccessor::CreateImage(outputImageHandle);
}

inline Image ImageConverter::Convert(const Image& inputImage, const PixelFormat& outputPixelFormat, peak::ipl::Interval<double> interval) const
{
    PEAK_IPL_IMAGE_HANDLE outputImageHandle = nullptr;
    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_ConvertFromInterval(m_backendHandle,
            ImageBackendAccessor::BackendHandle(inputImage),
            static_cast<PEAK_IPL_PIXEL_FORMAT>(outputPixelFormat.PixelFormatName()), interval.Start(),
            interval.End(), &outputImageHandle);
    });

    return ImageBackendAccessor::CreateImage(outputImageHandle);
}

inline Image ImageConverter::Convert(const Image& inputImage, const PixelFormat& outputPixelFormat,
    uint8_t* outputImageBuffer, size_t outputImageBufferSize, peak::ipl::Interval<double> interval) const
{
    PEAK_IPL_IMAGE_HANDLE outputImageHandle = nullptr;
    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_ConvertToBufferFromInterval(m_backendHandle,
            ImageBackendAccessor::BackendHandle(inputImage), interval.Start(), interval.End(),
            static_cast<PEAK_IPL_PIXEL_FORMAT>(outputPixelFormat.PixelFormatName()), outputImageBuffer,
            outputImageBufferSize, &outputImageHandle);
    });

    return ImageBackendAccessor::CreateImage(outputImageHandle);
}

inline Image ImageConverter::Convert(
    const Image& inputImage, const PixelFormat& outputPixelFormat, double factor, double offset) const
{
    PEAK_IPL_IMAGE_HANDLE outputImageHandle = nullptr;
    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_ConvertWithFactorAndOffset(m_backendHandle,
            ImageBackendAccessor::BackendHandle(inputImage),
            static_cast<PEAK_IPL_PIXEL_FORMAT>(outputPixelFormat.PixelFormatName()), factor, offset, &outputImageHandle);
    });

    return ImageBackendAccessor::CreateImage(outputImageHandle);
}

inline Image ImageConverter::Convert(const Image& inputImage, const PixelFormat& outputPixelFormat,
    uint8_t* outputImageBuffer, size_t outputImageBufferSize, double factor, double offset) const
{
    PEAK_IPL_IMAGE_HANDLE outputImageHandle = nullptr;
    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_ImageConverter_ConvertToBufferWithFactorAndOffset(m_backendHandle,
            ImageBackendAccessor::BackendHandle(inputImage), factor, offset,
            static_cast<PEAK_IPL_PIXEL_FORMAT>(outputPixelFormat.PixelFormatName()), outputImageBuffer,
            outputImageBufferSize, &outputImageHandle);
    });

    return ImageBackendAccessor::CreateImage(outputImageHandle);
}

} /* namespace ipl */
} /* namespace peak */
