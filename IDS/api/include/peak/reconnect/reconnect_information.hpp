#pragma once

#include <peak/backend/peak_backend.h>

#include <cstdint>
#include <utility>

namespace peak
{
namespace core
{

/*! Reconnect information signaling the state of the Device after it has been reconnected. */
class DeviceReconnectInformation
{
public:
    DeviceReconnectInformation() = default;

    explicit DeviceReconnectInformation(const PEAK_DEVICE_RECONNECT_INFORMATION& cStruct)
        : m_remoteDeviceAcquisitionRunning(cStruct.IsRemoteDeviceAcquisitionRunning != 0)
        , m_remoteDeviceConfigurationRestored(cStruct.IsRemoteDeviceConfigurationRestored != 0)
        , m_errorFlags(cStruct.ErrorFlags)
    {}

    ~DeviceReconnectInformation() = default;

    DeviceReconnectInformation(const DeviceReconnectInformation&) = default;
    DeviceReconnectInformation(DeviceReconnectInformation&&) = default;
    DeviceReconnectInformation& operator=(const DeviceReconnectInformation&) = default;
    DeviceReconnectInformation& operator=(DeviceReconnectInformation&&) = default;

    /*! Returns true if no error occured during the reconnect */
    bool IsSuccessful() const 
    {
        return m_errorFlags == PEAK_DEVICE_RECONNECT_OPERATION_NO_ERROR;
    }

    /*! Returns true if the remote device acquisition is running
     *  when the device got reconnected.
     */
    bool IsRemoteDeviceAcquisitionRunning() const
    {
        return m_remoteDeviceAcquisitionRunning;
    }

    /*! Returns true if the configuration of the remote device has been
     *  restored.
     * 
     */
    bool IsRemoteDeviceConfigurationRestored() const
    {
        return m_remoteDeviceConfigurationRestored;
    }

    /*! Returns error flags indicating the failed reconnect operation and the corresponding error.
     *
     * If an error occurs during a reconnect this flag field indicates the failed reconnect operation as well as a more
     * detailed error flag. Based on the error the users application can complete the missing reconnect operations,
     * e.g. reallocating the buffers if the
     * \ref PEAK_DEVICE_RECONNECT_OPERATION_REMOTE_DEVICE_ACQUISITION_RESTART_PAYLOAD_SIZE_LARGER_THAN_BUFFERS flag is set.
     *
     */
    PEAK_DEVICE_RECONNECT_ERROR_FLAGS GetErrorFlags() const
    {
        return m_errorFlags;
    }

private:
    /*! True if the remote device acquisition has been restarted */
    bool m_remoteDeviceAcquisitionRunning{};
    /*! True if the configuration of the remote device has been recovered */
    bool m_remoteDeviceConfigurationRestored{};


    PEAK_DEVICE_RECONNECT_ERROR_FLAGS m_errorFlags{};
};

} // namespace core
} // namespace peak
