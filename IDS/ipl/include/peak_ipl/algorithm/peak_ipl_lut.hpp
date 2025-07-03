/*!
 * \file    peak_ipl_lut.hpp
 *
 * \author  IDS Imaging Development Systems GmbH
 * \date    2025-01-07
 * \since   1.15
 *
 * Copyright (c) 2024 - 2025, IDS Imaging Development Systems GmbH. All rights reserved.
 */

#pragma once

#include <peak_ipl/backend/peak_ipl_backend.h>
#include <peak_ipl/exception/peak_ipl_exception.hpp>
#include <peak_ipl/types/peak_ipl_image.hpp>
#include <peak_ipl/types/peak_ipl_pixel_format.hpp>
#include <peak_ipl/types/peak_ipl_simple_types.hpp>

/*!
 * \brief The "peak::ipl" namespace contains the whole image processing library.
 */
namespace peak
{
namespace ipl
{

class Image;

/*!
 * \brief Applies a LUT to the input image data. 
 * 
 * In image processing, lookup tables (LUTs) are used to map input pixel values to new output values, 
 * enabling efficient transformations of image data. They are commonly used for tasks like color correction, 
 * contrast adjustment, and brightness modification. Instead of recalculating values for each pixel,
 * LUTs allow quick retrieval of precomputed results, speeding up processing. LUTs are particularly effective 
 * in enhancing visual effects or applying filters without altering the image's underlying structure.
 *
 * A custom lookup table (LUT) can be defined or one of several predefined LUTs can be applied. 
 * For all available predefined LUTs see enum class Preset. The LUT can be configured independently for 
 * each bit depth. Depending on the image's bit depth, the corresponding LUT is applied. Ensure that the
 * correct LUT is configured for the bit depth you intend to use.
 *
 * \note When processing monochrome images, only the red channel of the LUT is utilized, which may not produce the expected result.
 *
 * \since 1.15
 */
class Lut final
{
public:
    /*!
     * \brief Channel parameter to select the channel where the operation is applied.
     */
    enum class Channel
    {
        /*! Only operate on the red channel */
        Red = 0,
        /*! Only operate on the green channel */
        Green = 1,
        /*! Only operate on the blue channel */
        Blue = 2,
        /*! Operate on all channels simultaniously */
        All = 3
    };

    /*!
     * \brief Selector parameter to select to which LUT the operation is applied (8/10/12/16 bit LUT). During processing,
     * the LUT is selected based on the pixel format. 
     */
    enum class Selector
    {
        /*! Operate on the 8 bit LUT */
        Lut_8Bit = 0,
        /*! Operate on the 10 bit LUT */
        Lut_10Bit = 1,
        /*! Operate on the 12 bit LUT */
        Lut_12Bit = 2,
        /*! Operate on the 16 bit LUT */
        Lut_16Bit = 3
    };

    /*!
     * \brief Preset Selects a predefined LUT.
     */
    enum class Preset
    {
        /*!
         * This is the default LUT which does not change any pixel values. Each input pixel value will be converted
         * to the exact output pixel value. When choosing this preset no calculations will be performed.
         */
        Identity = 0,
        /*!
         * This LUT inverts all pixel values. The highest pixel value will be converted to the lowest one and
         * vice versa.
         */
        Inverse = 1,
        /*!
         * This LUT creates a custom color mapping where the red, green, and blue channels gradually change across
         * different ranges, producing a smooth transition from dark to light. It adjusts each color channel in a
         * pattern that could be used for unique color grading or effects.
         */
        Jet = 2,
        /*!
         * This LUT creates a color transition where the red channel gradually increases, the green channel
         * increases after the red, and finally, the blue channel increases in the last section. It results
         * in a smooth shift from red to green and then to blue, producing a color gradient effect across the RGB channels.
         */
        Hot = 3,
        /*!
         * This LUT creates a rainbow effect by smoothly transitioning the red, green, and blue channels
         * through specific color ranges. It produces a gradual shift from red to green to blue, and then
         * cycles back through the colors to create a full spectrum effect.
         */
        Rainbow = 4,
        /*!
         * This LUT will only allow red pixels to be shown. This is achieved by converting all pixel values
         * of any other channel to 0.
         */
        OnlyRed = 5,
        /*!
         * This LUT will only allow green pixels to be shown. This is achieved by converting all pixel values
         * of any other channel to 0.
         */
        OnlyGreen = 6,
        /*!
         * This LUT will only allow blue pixels to be shown. This is achieved by converting all pixel values
         * of any other channel to 0.
         */
        OnlyBlue = 7,
        /*!
         * This LUT will multiply each pixel value by 2 resulting in a gain like adjustment.
         */
        DigitalGain2 = 8,
        /*!
         * This LUT will apply a digital black function. See \ref peak::ipl::GammaCorrector::SetDigitalBlack for more information.
         */
        DigitalBlack25Percent = 9,
        /*!
         * The LUT converts the image to black and white by clamping pixel values below the midpoint to 0 (black)
         * and those above to the maximum value (white).
         */
        Binarize = 10
    };

    /*! \brief Constructor for a new LUT instance
     *
     * Creates a new LUT instance.
     *
     * \since 1.15
     */
    Lut();
    
    /*! \brief Destructor for the LUT instance
     *
     * Destroys a LUT instance.
     *
     * \since 1.15
     */
    ~Lut();

    /*! \brief Copy Constructor for a new LUT instance
     *
     * Creates a new LUT instance.
     *
     * \note This only creates a shallow copy. See \ref shallowCopy for a detailed explanation.
     *
     * \param[in] other The other instance to acquire a reference for.
     * 
     * \since 1.15
     */
    Lut(const Lut& other);

    /*! \brief Copy assigment for LUT instance
     *
     * Acquires a reference from \p other to this instance. If this instance already has a reference, it will be destroyed.
     *
     * \note See \ref shallowCopy for a detailed explanation.
     * 
     * \param[in] other The other instance to acquire a reference for.
     *
     * \since 1.15
     */
    Lut& operator=(const Lut& other);

    /*! \brief Move constructor for Lut instance
     *
     * Moves the instance from \p other to this. Afterwards \p other is invalid.
     * 
     * \param[in] other The other instance to move.
     *
     * \since 1.15
     */
    Lut(Lut&& other) noexcept;

    /*! \brief Move assignment for Lut instance
     *
     * Moves the instance from \p other to this. Afterwards \p other is invalid.
     * 
     * \param[in] other The other instance to move.
     *
     * \since 1.15
     */
    Lut& operator=(Lut&& other) noexcept;

    /*!
     * \brief Reads a single LUT value for a given selector and channel at an indexed position.
     *
     * \returns The LUT value at the indexed position.
     *
     * \param[in] selector     The selected LUT (8, 10, 12, 16 bit)
     * \param[in] channel      The channel from where the value is read
     * \param[in] index        The index of the LUT value
     * 
     * \since 1.15
     */
    PEAK_IPL_NO_DISCARD uint32_t LutValue(Selector selector, Channel channel, uint32_t index) const;

    /*!
     * \brief Sets a single LUT value for a given selector and channel at an indexed position.
     *
     * \param[in] selector     The selected LUT (8, 10, 12, 16 bit)
     * \param[in] channel      The channel where the new value is set
     * \param[in] index        The index of the LUT value
     * \param[in] value        The LUT value at indexed position to be set
     *
     * \throws InvalidArgument for values or indexes out of range.
     *
     * \since 1.15
     */
    void SetLutValue(Selector selector, Channel channel, uint32_t index, uint32_t value);

    /*!
     * \brief Reads all LUT values for a given selector and channel at once.
     * 
     * \returns All values in a vector.
     *
     * \param[in] selector     The selected LUT (8, 10, 12, 16 bit)
     * \param[in] channel      The channel where the new values are set
     *
     * \throws InvalidArgument for values out of range or wrong vector size.
     *
     * \since 1.15
     */
    PEAK_IPL_NO_DISCARD std::vector<uint32_t> AllLutValues(Selector selector, Channel channel) const;

    /*!
     * \brief Sets all LUT values for a given selector and channel at once.
     *
     * \param[in] selector     The selected LUT (8, 10, 12, 16 bit)
     * \param[in] channel      The channel where the new values are set
     * \param[in] values       Vector with LUT values
     *
     * \throws InvalidArgument for values out of range or wrong vector size.
     *
     * \since 1.15
     */
    void SetAllLutValues(Selector selector, Channel channel, const std::vector<uint32_t>& values);

    /*!
     * \brief Applies the given preset (identity, inverse, ...) to the specified LUT.
     *
     * \param[in] selector     The selected LUT (8, 10, 12, 16 bit)
     * \param[in] preset       The predefined LUT to be set
     * 
     * \since 1.15
     */
    void SetLutPreset(Selector selector, Preset preset);

    /*!
     * \brief Returns whether the LUT supports the given pixel format.
     *
     * \returns Whether the given pixel format is supported.
     *
     * \param[in] pixelFormatName The pixel format of interest.
     *
     * \since 1.15
     */
    PEAK_IPL_NO_DISCARD bool IsPixelFormatSupported(PixelFormatName pixelFormatName) const;

    /*!
     * \brief Applies the LUT to the given image in place that is, it will change the input image.
     *
     * \param[in] image Image to process.
     *
     * \note Will not work with packed pixel formats. See \ref unpackPixelformat for the correct handling of this.
     *
     * \throws ImageFormatNotSupportedException image has unsupported pixel format.
     *
     * \since 1.15
     */
    void ProcessInPlace(Image& image) const;

    /*! \brief Applies the LUT to the given image.
     *
     * \param[in] inputImage Image to process.
     *
     * \note Will not work with packed pixel formats. See \ref unpackPixelformat for the correct handling of this.
     *
     * \returns A new created image with applied LUT.
     *
     * \throws ImageFormatNotSupportedException image has unsupported pixel format.
     *
     * \since 1.15
     */
    PEAK_IPL_NO_DISCARD Image Process(const Image& inputImage) const;

private:
    PEAK_IPL_LUT_HANDLE m_backendHandle{};
};

inline Lut::Lut()
{
    ExecuteAndMapReturnCodes([&] { return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Lut_Construct(&m_backendHandle); });
}

inline Lut::~Lut()
{
    if (m_backendHandle)
    {
        (void)PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Lut_Destruct(m_backendHandle);
    }
}

inline Lut::Lut(Lut&& other) noexcept
{
    *this = std::move(other);
}

inline Lut::Lut(const Lut& other)
{
    if (other.m_backendHandle)
    {
        ExecuteAndMapReturnCodes([&] { return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Lut_Acquire(other.m_backendHandle); });
    }

    m_backendHandle = other.m_backendHandle;
}

inline Lut& Lut::operator=(const Lut& other)
{
    if (this != &other)
    {
        if (m_backendHandle)
        {
            (void)PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Lut_Destruct(m_backendHandle);
        }

        if (other.m_backendHandle)
        {
            ExecuteAndMapReturnCodes(
                [&] { return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Lut_Acquire(other.m_backendHandle); });
        }
    }

    m_backendHandle = other.m_backendHandle;

    return *this;
}

inline Lut& Lut::operator=(Lut&& other) noexcept
{
    if (this != &other)
    {
        if (m_backendHandle != nullptr)
        {
            (void)PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Lut_Destruct(m_backendHandle);
        }
        m_backendHandle = other.m_backendHandle;
        other.m_backendHandle = nullptr;
    }

    return *this;
}

inline uint32_t Lut::LutValue(Selector selector, Channel channel, uint32_t index) const
{
    uint32_t value = 0;

    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Lut_GetLutValue(
            m_backendHandle, static_cast<PEAK_IPL_LUT_SELECTOR>(selector), static_cast<PEAK_IPL_LUT_CHANNEL>(channel), index, &value);
    });

    return value;
}

inline void Lut::SetLutValue(Selector selector, Channel channel, uint32_t index, uint32_t value)
{
    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Lut_SetLutValue(m_backendHandle,
            static_cast<PEAK_IPL_LUT_SELECTOR>(selector), static_cast<PEAK_IPL_LUT_CHANNEL>(channel), index, value);
    });
}

inline std::vector<uint32_t> Lut::AllLutValues(Selector selector, Channel channel) const
{
    size_t numLutValues = 0;

    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Lut_GetAllLutValues(m_backendHandle,
            static_cast<PEAK_IPL_LUT_SELECTOR>(selector), static_cast<PEAK_IPL_LUT_CHANNEL>(channel), nullptr, &numLutValues);
    });

    std::vector<uint32_t> vector(numLutValues);

    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Lut_GetAllLutValues(m_backendHandle,
            static_cast<PEAK_IPL_LUT_SELECTOR>(selector), static_cast<PEAK_IPL_LUT_CHANNEL>(channel), 
            reinterpret_cast<uint32_t*>(vector.data()), &numLutValues);
    });

    return vector;
}

inline void Lut::SetAllLutValues(Selector selector, Channel channel, const std::vector<uint32_t>& vector)
{
    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Lut_SetAllLutValues(m_backendHandle,
            static_cast<PEAK_IPL_LUT_SELECTOR>(selector), static_cast<PEAK_IPL_LUT_CHANNEL>(channel),
            reinterpret_cast<const uint32_t*>(vector.data()), vector.size());
    });
}

inline void Lut::SetLutPreset(Selector selector, Preset preset)
{
    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Lut_SetLutPreset(
            m_backendHandle, static_cast<PEAK_IPL_LUT_SELECTOR>(selector), static_cast<PEAK_IPL_LUT_PRESET>(preset));
    });
}

inline bool Lut::IsPixelFormatSupported(PixelFormatName pixelFormatName) const
{
    PEAK_IPL_BOOL8 isPixelFormatSupported = 0;

    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Lut_IsPixelFormatSupported(m_backendHandle,
            static_cast<PEAK_IPL_PIXEL_FORMAT>(pixelFormatName),
            reinterpret_cast<PEAK_IPL_BOOL8*>(&isPixelFormatSupported));
    });

    return isPixelFormatSupported > 0;
}

inline void Lut::ProcessInPlace(Image& image) const
{
    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Lut_ProcessInPlace(
            m_backendHandle, ImageBackendAccessor::BackendHandle(image));
    });
}

inline Image Lut::Process(const Image& inputImage) const
{
    PEAK_IPL_IMAGE_HANDLE outputImageHandle = nullptr;
    ExecuteAndMapReturnCodes([&] {
        return PEAK_IPL_C_ABI_PREFIX PEAK_IPL_Lut_Process(
            m_backendHandle, ImageBackendAccessor::BackendHandle(inputImage), &outputImageHandle);
    });

    return ImageBackendAccessor::CreateImage(outputImageHandle);
}

} /* namespace ipl */
} /* namespace peak */
