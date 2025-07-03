/*!
 * \file    peak_interface.hpp
 *
 * \author  IDS Imaging Development Systems GmbH
 * \date    2019-05-01
 * \since   1.0
 *
 * Copyright (c) 2019 - 2025, IDS Imaging Development Systems GmbH. All rights reserved.
 */

#pragma once


#include <peak/backend/peak_backend.h>
#include <peak/common/peak_common_structs.hpp>
#include <peak/common/peak_event_supporting_module.hpp>
#include <peak/device/peak_device_descriptor.hpp>
#include <peak/dll_interface/peak_dll_interface_util.hpp>
#include <peak/generic/peak_t_callback_manager.hpp>
#include <peak/reconnect/reconnect_information.hpp>

#include <unordered_map>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>


namespace peak
{
namespace core
{

class System;

struct DeviceFoundCallbackContainer;

/*!
 * \brief Represents a GenTL Interface module.
 *
 * This class allows to query information about a GenTL Interface module and to enumerate its
 * \link DeviceDescriptor DeviceDescriptors\endlink, which allow you to open the corresponding Device.
 */
class Interface
    : public EventSupportingModule
    , public std::enable_shared_from_this<Interface>
{
public:
    /*! The type of device found callbacks. */
    using DeviceFoundCallback = std::function<void(const std::shared_ptr<DeviceDescriptor>& foundDevice)>;
    /*! The type of device found callback handles. */
    using DeviceFoundCallbackHandle = DeviceFoundCallback*;
    /*! The type of device disconnected callbacks. */
    using DeviceDisconnectedCallback = std::function<void(const std::shared_ptr<DeviceDescriptor>& disconnectedDevice)>;
    /*! The type of device disconnect callback handles. */
    using DeviceDisconnectedCallbackHandle = DeviceDisconnectedCallback*;
    /*! The type of device reconnected callbacks. */
    using DeviceReconnectedCallback = std::function<void(
        const std::shared_ptr<DeviceDescriptor>& reconnectedDevice, DeviceReconnectInformation reconnectInformation)>;
    /*! The type of device reconnected callback handles. */
    using DeviceReconnectedCallbackHandle = DeviceReconnectedCallback*;
    /*! The type of device lost callbacks. */
    using DeviceLostCallback = std::function<void(const std::string& lostDeviceId)>;
    /*! The type of device lost callback handles. */
    using DeviceLostCallbackHandle = DeviceLostCallback*;
    /*! The type of device list changed callbacks. */
    using DeviceListChangedCallback = std::function<void()>;
    /*! The type of device list changed callback handles. */
    using DeviceListChangedCallbackHandle = DeviceListChangedCallback*;

    Interface() = delete;
    ~Interface() override;
    Interface(const Interface& other) = delete;
    Interface& operator=(const Interface& other) = delete;
    Interface(Interface&& other) = delete;
    Interface& operator=(Interface&& other) = delete;

    /*! @copydoc ProducerLibrary::Key() */
    std::string Key() const;

    /*! @copydoc SystemDescriptor::Info() */
    RawInformation Info(int32_t infoCommand) const;
    /*! @copydoc System::ID() */
    std::string ID() const;
    /*! @copydoc InterfaceDescriptor::DisplayName() */
    std::string DisplayName() const;
    /*! @copydoc SystemDescriptor::TLType() */
    std::string TLType() const;

    /*!
     * \brief Searches for devices.
     *
     * This function triggers an update of the internal device list. The callbacks registered on the interface
     * will be triggered if an device is found or lost.
     *
     * \param[in] timeout_ms The time to wait for new devices in milliseconds. In any case the
     *                       GenTL Producer must make sure that this operation is completed in a
     *                       reasonable amount of time depending on the underlying technology.
     *
     * \since 1.0
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    void UpdateDevices(Timeout timeout_ms);

    /*!
     * \brief Returns the device list.
     *
     * \return Device list
     *
     * \since 1.0
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    std::vector<std::shared_ptr<DeviceDescriptor>> Devices() const;

    /*! @copydoc InterfaceDescriptor::ParentSystem() */
    std::shared_ptr<System> ParentSystem() const;

    /*!
     * \brief Registers a callback for signaling a found device.
     *
     * This function registers a callback which gets called every time a new device is found. Pass the callback
     * handle returned by this function to UnregisterDeviceFoundCallback() to unregister the callback.
     *
     * \param[in] callback The callback to call if a new device is found.
     *
     * \return Callback handle
     *
     * \since 1.0
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    DeviceFoundCallbackHandle RegisterDeviceFoundCallback(const DeviceFoundCallback& callback);

    /*!
     * \brief Unregisters a device found callback.
     *
     * This function unregisters a device found callback by taking its handle.
     *
     * \param[in] callbackHandle The handle of the callback to unregister.
     *
     * \since 1.0
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    void UnregisterDeviceFoundCallback(DeviceFoundCallbackHandle callbackHandle);

    /*!
     * \brief Registers a callback for signaling a lost device.
     *
     * This function registers a callback which gets called every time a device is lost. Pass the callback
     * handle returned by this function to UnregisterDeviceLostCallback() to unregister the callback.
     *
     * \param[in] callback The callback to call if a device is lost.
     *
     * \return Callback handle
     *
     * \since 1.0
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    DeviceLostCallbackHandle RegisterDeviceLostCallback(const DeviceLostCallback& callback);

    /*!
     * \brief Unregisters a device lost callback.
     *
     * This function unregisters a device lost callback by taking its handle.
     *
     * \param[in] callbackHandle The handle of the callback to unregister.
     *
     * \since 1.0
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    void UnregisterDeviceLostCallback(DeviceLostCallbackHandle callbackHandle);

    /*!
     * \brief Registers a callback for signaling a reconnected device.
     *
     * This function registers a callback which gets called every time a device has beeen reconnected.
     * Pass the callback handle returned by this function to UnregisterDeviceReconnectedCallback() to unregister
     * the callback.
     *
     * \param[in] callback The callback to call if a device has been reconnected.
     *
     * \return Callback handle
     *
     * \since 1.7
     *
     * \throws NotAvailableException The reconnect callback is not available in the parent system
     * \throws InternalErrorException An internal error has occurred.
     */
    DeviceReconnectedCallbackHandle RegisterDeviceReconnectedCallback(const DeviceReconnectedCallback& callback);

    /*!
     * \brief Unregisters a device reconnected callback.
     *
     * This function unregisters a device reconnected callback by taking its handle.
     *
     * \param[in] callbackHandle The handle of the callback to unregister.
     *
     * \since 1.7
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    void UnregisterDeviceReconnectedCallback(DeviceReconnectedCallbackHandle callbackHandle);

        /*!
     * \brief Registers a callback for signaling a disconnected device.
     *
     * This function registers a callback which gets called every time a device has beeen disconnected.
     * Pass the callback handle returned by this function to UnregisterDeviceDisconnectedCallback() to unregister
     * the callback.
     *
     * \param[in] callback The callback to call if a device has been disconnected.
     *
     * \return Callback handle
     *
     * \since 1.7
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    DeviceDisconnectedCallbackHandle RegisterDeviceDisconnectedCallback(const DeviceDisconnectedCallback& callback);

    /*!
     * \brief Unregisters a device disconnected callback.
     *
     * This function unregisters a device disconnected callback by taking its handle.
     *
     * \param[in] callbackHandle The handle of the callback to unregister.
     *
     * \since 1.7
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    void UnregisterDeviceDisconnectedCallback(DeviceDisconnectedCallbackHandle callbackHandle);

    /*!
     * \brief Registers a callback for signaling a device list changed.
     *
     * This function registers a callback which gets called every time something changes within the device list.
     * Pass the callback handle returned by this function to UnregisterDeviceDisconnectedCallback() to unregister
     * the callback.
     *
     * \param[in] callback The callback to call if a device has been disconnected.
     *
     * \return Callback handle
     *
     * \since 1.7
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    DeviceListChangedCallbackHandle RegisterDeviceListChangedCallback(const DeviceListChangedCallback& callback);

    /*!
     * \brief Unregisters a device list changed callback.
     *
     * This function unregisters a device list changed callback by taking its handle.
     *
     * \param[in] callbackHandle The handle of the callback to unregister.
     *
     * \since 1.7
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    void UnregisterDeviceListChangedCallback(DeviceListChangedCallbackHandle callbackHandle);

private:
    struct DeviceFoundCallbackContainer
    {
        std::weak_ptr<Interface> _Interface;
        Interface::DeviceFoundCallback Callback;
    };
    struct DeviceDisconnectedCallbackContainer
    {
        std::weak_ptr<Interface> _Interface;
        Interface::DeviceDisconnectedCallback Callback;
    };
    struct DeviceReconnectedCallbackContainer
    {
        std::weak_ptr<Interface> _Interface;
        Interface::DeviceReconnectedCallback Callback;
    };
    static void PEAK_CALL_CONV DeviceFoundCallbackCWrapper(
        PEAK_DEVICE_DESCRIPTOR_HANDLE foundDevice, void* context);
    static void PEAK_CALL_CONV DeviceLostCallbackCWrapper(
        const char* lostDeviceId, size_t lostDeviceIdSize, void* context);
    static void PEAK_CALL_CONV DeviceDisconnectedCallbackCWrapper(
        PEAK_DEVICE_DESCRIPTOR_HANDLE foundDevice, void* context);
    static void PEAK_CALL_CONV DeviceReconnectedCallbackCWrapper(PEAK_DEVICE_DESCRIPTOR_HANDLE reconnectedDevice,
        PEAK_DEVICE_RECONNECT_INFORMATION reconnectInformation, void* context);
    static void PEAK_CALL_CONV DeviceListChangedCallbackCWrapper(void* context);

    std::shared_ptr<DeviceDescriptor> GetOrAddFoundDevice(PEAK_DEVICE_DESCRIPTOR_HANDLE foundDevice);

    PEAK_MODULE_HANDLE ModuleHandle() const override;
    PEAK_EVENT_SUPPORTING_MODULE_HANDLE EventSupportingModuleHandle() const override;

    void InitializeUpdateMechanismIfNecessary();

    friend ClassCreator<Interface>;
    Interface(PEAK_INTERFACE_HANDLE interfaceHandle, const std::weak_ptr<System>& parentSystem);
    PEAK_INTERFACE_HANDLE m_backendHandle;

    std::weak_ptr<System> m_parentSystem;

    std::unique_ptr<TCallbackManager<PEAK_DEVICE_FOUND_CALLBACK_HANDLE, DeviceFoundCallbackContainer>>
        m_deviceFoundCallbackManager;
    std::unique_ptr<TCallbackManager<PEAK_DEVICE_LOST_CALLBACK_HANDLE, DeviceLostCallback>>
        m_deviceLostCallbackManager;

    std::unique_ptr<TCallbackManager<PEAK_DEVICE_DISCONNECTED_CALLBACK_HANDLE, DeviceDisconnectedCallbackContainer>>
        m_deviceDisconnectedCallbackManager;
    std::unique_ptr<TCallbackManager<PEAK_DEVICE_RECONNECTED_CALLBACK_HANDLE, DeviceReconnectedCallbackContainer>>
        m_deviceReconnectedCallbackManager;

    std::unique_ptr<TCallbackManager<PEAK_DEVICE_LIST_CHANGED_CALLBACK_HANDLE, DeviceListChangedCallback>>
        m_deviceListChangedCallbackManager;

    std::vector<std::shared_ptr<DeviceDescriptor>> m_devices;
    std::unordered_map<std::string, std::shared_ptr<DeviceDescriptor>> m_devicesByKey;
    std::unordered_map<std::string, std::string> m_devicesKeyById;
    mutable std::mutex m_devicesMutex;
    std::once_flag m_updateMechanismInitializedFlag;

    std::string m_key;
};

} /* namespace core */
} /* namespace peak */

/* Implementation */
namespace peak
{
namespace core
{

inline Interface::Interface(PEAK_INTERFACE_HANDLE interfaceHandle, const std::weak_ptr<System>& parentSystem)
    : m_backendHandle(interfaceHandle)
    , m_parentSystem(parentSystem)
    , m_key(QueryStringFromCInterfaceFunction([&](char* key, size_t* keySize) {
        return PEAK_C_ABI_PREFIX PEAK_Interface_GetKey(interfaceHandle, key, keySize);
    }))
{
    m_deviceFoundCallbackManager =
        std::make_unique<TCallbackManager<PEAK_DEVICE_FOUND_CALLBACK_HANDLE, DeviceFoundCallbackContainer>>(
            [&](void* callbackContext) {
                return QueryNumericFromCInterfaceFunction<PEAK_DEVICE_FOUND_CALLBACK_HANDLE>(
                    [&](PEAK_DEVICE_FOUND_CALLBACK_HANDLE* deviceFoundCallbackHandle) {
                        return PEAK_C_ABI_PREFIX PEAK_Interface_RegisterDeviceFoundCallback(
                            m_backendHandle, DeviceFoundCallbackCWrapper, callbackContext, deviceFoundCallbackHandle);
                    });
            },
            [&](PEAK_DEVICE_FOUND_CALLBACK_HANDLE callbackHandle) {
                CallAndCheckCInterfaceFunction([&] {
                    return PEAK_C_ABI_PREFIX PEAK_Interface_UnregisterDeviceFoundCallback(
                        m_backendHandle, callbackHandle);
                });
            });

    m_deviceLostCallbackManager =
        std::make_unique<TCallbackManager<PEAK_DEVICE_LOST_CALLBACK_HANDLE, DeviceLostCallback>>(
            [&](void* callbackContext) {
                return QueryNumericFromCInterfaceFunction<PEAK_DEVICE_LOST_CALLBACK_HANDLE>(
                    [&](PEAK_DEVICE_LOST_CALLBACK_HANDLE* deviceLostCallbackHandle) {
                        return PEAK_C_ABI_PREFIX PEAK_Interface_RegisterDeviceLostCallback(
                            m_backendHandle, DeviceLostCallbackCWrapper, callbackContext, deviceLostCallbackHandle);
                    });
            },
            [&](PEAK_DEVICE_LOST_CALLBACK_HANDLE callbackHandle) {
                CallAndCheckCInterfaceFunction([&] {
                    return PEAK_C_ABI_PREFIX PEAK_Interface_UnregisterDeviceLostCallback(
                        m_backendHandle, callbackHandle);
                });
            });

    m_deviceDisconnectedCallbackManager = std::make_unique<
        TCallbackManager<PEAK_DEVICE_DISCONNECTED_CALLBACK_HANDLE, DeviceDisconnectedCallbackContainer>>(
        [&](void* callbackContext) {
            return QueryNumericFromCInterfaceFunction<PEAK_DEVICE_DISCONNECTED_CALLBACK_HANDLE>(
                [&](PEAK_DEVICE_DISCONNECTED_CALLBACK_HANDLE* deviceDisconnectedCallbackHandle) {
                    return PEAK_C_ABI_PREFIX PEAK_Interface_RegisterDeviceDisconnectedCallback(m_backendHandle,
                        DeviceDisconnectedCallbackCWrapper, callbackContext, deviceDisconnectedCallbackHandle);
                });
        },
        [&](PEAK_DEVICE_DISCONNECTED_CALLBACK_HANDLE callbackHandle) {
            CallAndCheckCInterfaceFunction([&] {
                return PEAK_C_ABI_PREFIX PEAK_Interface_UnregisterDeviceDisconnectedCallback(
                    m_backendHandle, callbackHandle);
            });
        });

    m_deviceReconnectedCallbackManager = std::make_unique<
        TCallbackManager<PEAK_DEVICE_RECONNECTED_CALLBACK_HANDLE, DeviceReconnectedCallbackContainer>>(
        [&](void* callbackContext) {
            return QueryNumericFromCInterfaceFunction<PEAK_DEVICE_RECONNECTED_CALLBACK_HANDLE>(
                [&](PEAK_DEVICE_RECONNECTED_CALLBACK_HANDLE* deviceReconnectedCallbackHandle) {
                    return PEAK_C_ABI_PREFIX PEAK_Interface_RegisterDeviceReconnectedCallback(m_backendHandle,
                        DeviceReconnectedCallbackCWrapper, callbackContext, deviceReconnectedCallbackHandle);
                });
        },
        [&](PEAK_DEVICE_RECONNECTED_CALLBACK_HANDLE callbackHandle) {
            CallAndCheckCInterfaceFunction([&] {
                return PEAK_C_ABI_PREFIX PEAK_Interface_UnregisterDeviceReconnectedCallback(
                    m_backendHandle, callbackHandle);
            });
        });

    m_deviceListChangedCallbackManager =
        std::make_unique<TCallbackManager<PEAK_DEVICE_LIST_CHANGED_CALLBACK_HANDLE, DeviceListChangedCallback>>(
            [&](void* callbackContext) {
                return QueryNumericFromCInterfaceFunction<PEAK_DEVICE_LIST_CHANGED_CALLBACK_HANDLE>(
                    [&](PEAK_DEVICE_LIST_CHANGED_CALLBACK_HANDLE* deviceListChangedCallbackHandle) {
                        return PEAK_C_ABI_PREFIX PEAK_Interface_RegisterDeviceListChangedCallback(m_backendHandle,
                            DeviceListChangedCallbackCWrapper, callbackContext, deviceListChangedCallbackHandle);
                    });
            },
            [&](PEAK_DEVICE_LIST_CHANGED_CALLBACK_HANDLE callbackHandle) {
                CallAndCheckCInterfaceFunction([&] {
                    return PEAK_C_ABI_PREFIX PEAK_Interface_UnregisterDeviceListChangedCallback(
                        m_backendHandle, callbackHandle);
                });
            });
}

inline Interface::~Interface()
{
    try
    {
        m_deviceFoundCallbackManager->UnregisterAllCallbacks();
        m_deviceLostCallbackManager->UnregisterAllCallbacks();

        m_deviceDisconnectedCallbackManager->UnregisterAllCallbacks();
        m_deviceReconnectedCallbackManager->UnregisterAllCallbacks();

        m_deviceListChangedCallbackManager->UnregisterAllCallbacks();
    }
    catch (const Exception&)
    {}

    (void)PEAK_C_ABI_PREFIX PEAK_Interface_Destruct(m_backendHandle);
}

inline std::string Interface::Key() const
{
    return m_key;
}

inline RawInformation Interface::Info(int32_t infoCommand) const
{
    return QueryRawInformationFromCInterfaceFunction([&](int32_t* dataType, uint8_t* buffer, size_t* bufferSize) {
        return PEAK_C_ABI_PREFIX PEAK_Interface_GetInfo(
            m_backendHandle, infoCommand, dataType, buffer, bufferSize);
    });
}

inline std::string Interface::ID() const
{
    return QueryStringFromCInterfaceFunction([&](char* id, size_t* idSize) {
        return PEAK_C_ABI_PREFIX PEAK_Interface_GetID(m_backendHandle, id, idSize);
    });
}

inline std::string Interface::DisplayName() const
{
    return QueryStringFromCInterfaceFunction([&](char* displayName, size_t* displayNameSize) {
        return PEAK_C_ABI_PREFIX PEAK_Interface_GetDisplayName(m_backendHandle, displayName, displayNameSize);
    });
}

inline std::string Interface::TLType() const
{
    return QueryStringFromCInterfaceFunction([&](char* tlType, size_t* tlTypeSize) {
        return PEAK_C_ABI_PREFIX PEAK_Interface_GetTLType(m_backendHandle, tlType, tlTypeSize);
    });
}

inline void Interface::UpdateDevices(Timeout timeout_ms)
{
    InitializeUpdateMechanismIfNecessary();

    CallAndCheckCInterfaceFunction(
        [&] { return PEAK_C_ABI_PREFIX PEAK_Interface_UpdateDevices(m_backendHandle, timeout_ms); });
}

inline std::vector<std::shared_ptr<DeviceDescriptor>> Interface::Devices() const
{
    std::lock_guard<std::mutex> lock(m_devicesMutex);
    return m_devices;
}

inline std::shared_ptr<System> Interface::ParentSystem() const
{
    return LockOrThrow(m_parentSystem);
}

inline Interface::DeviceFoundCallbackHandle Interface::RegisterDeviceFoundCallback(
    const Interface::DeviceFoundCallback& callback)
{
    return reinterpret_cast<DeviceFoundCallbackHandle>(
        m_deviceFoundCallbackManager->RegisterCallback(DeviceFoundCallbackContainer{ shared_from_this(), callback }));
}

inline void Interface::UnregisterDeviceFoundCallback(Interface::DeviceFoundCallbackHandle callbackHandle)
{
    m_deviceFoundCallbackManager->UnregisterCallback(
        reinterpret_cast<PEAK_DEVICE_FOUND_CALLBACK_HANDLE>(callbackHandle));
}

inline Interface::DeviceReconnectedCallbackHandle Interface::RegisterDeviceReconnectedCallback(
    const DeviceReconnectedCallback& callback)
{
    return reinterpret_cast<DeviceReconnectedCallbackHandle>(m_deviceReconnectedCallbackManager->RegisterCallback(
        DeviceReconnectedCallbackContainer{ shared_from_this(), callback }));
}

inline void Interface::UnregisterDeviceReconnectedCallback(DeviceReconnectedCallbackHandle callbackHandle)
{
    m_deviceReconnectedCallbackManager->UnregisterCallback(
        reinterpret_cast<PEAK_DEVICE_RECONNECTED_CALLBACK_HANDLE>(callbackHandle));
}

inline Interface::DeviceDisconnectedCallbackHandle Interface::RegisterDeviceDisconnectedCallback(
    const DeviceDisconnectedCallback& callback)
{
    return reinterpret_cast<DeviceDisconnectedCallbackHandle>(m_deviceDisconnectedCallbackManager->RegisterCallback(
        DeviceDisconnectedCallbackContainer{ shared_from_this(), callback }));
}

inline void Interface::UnregisterDeviceDisconnectedCallback(DeviceDisconnectedCallbackHandle callbackHandle)
{
    m_deviceDisconnectedCallbackManager->UnregisterCallback(
        reinterpret_cast<PEAK_DEVICE_DISCONNECTED_CALLBACK_HANDLE>(callbackHandle));
}

inline Interface::DeviceLostCallbackHandle Interface::RegisterDeviceLostCallback(
    const Interface::DeviceLostCallback& callback)
{
    return reinterpret_cast<DeviceLostCallbackHandle>(m_deviceLostCallbackManager->RegisterCallback(callback));
}

inline void Interface::UnregisterDeviceLostCallback(Interface::DeviceLostCallbackHandle callbackHandle)
{
    m_deviceLostCallbackManager->UnregisterCallback(
        reinterpret_cast<PEAK_DEVICE_LOST_CALLBACK_HANDLE>(callbackHandle));
}

inline Interface::DeviceListChangedCallbackHandle Interface::RegisterDeviceListChangedCallback(
    const Interface::DeviceListChangedCallback& callback)
{
    return reinterpret_cast<DeviceListChangedCallbackHandle>(m_deviceListChangedCallbackManager->RegisterCallback(callback));
}

inline void Interface::UnregisterDeviceListChangedCallback(Interface::DeviceListChangedCallbackHandle callbackHandle)
{
    m_deviceListChangedCallbackManager->UnregisterCallback(
        reinterpret_cast<PEAK_DEVICE_LIST_CHANGED_CALLBACK_HANDLE>(callbackHandle));
}

inline void PEAK_CALL_CONV Interface::DeviceFoundCallbackCWrapper(
    PEAK_DEVICE_DESCRIPTOR_HANDLE foundDevice, void* context)
{
    auto callbackContainer = static_cast<DeviceFoundCallbackContainer*>(context);

    auto interfaceDescriptor = callbackContainer->_Interface.lock()->GetOrAddFoundDevice(foundDevice);

    callbackContainer->Callback(interfaceDescriptor);
}

inline void PEAK_CALL_CONV Interface::DeviceLostCallbackCWrapper(
    const char* lostDeviceId, size_t lostDeviceIdSize, void* context)
{
    auto callback = static_cast<Interface::DeviceLostCallback*>(context);

    callback->operator()(std::string(lostDeviceId, lostDeviceIdSize - 1));
}

inline void PEAK_CALL_CONV Interface::DeviceDisconnectedCallbackCWrapper(
    PEAK_DEVICE_DESCRIPTOR_HANDLE foundDevice, void* context)
{
    auto callbackContainer = static_cast<DeviceDisconnectedCallbackContainer*>(context);

    auto deviceDescriptor = callbackContainer->_Interface.lock()->GetOrAddFoundDevice(foundDevice);

    callbackContainer->Callback(deviceDescriptor);
}

inline void PEAK_CALL_CONV Interface::DeviceReconnectedCallbackCWrapper(PEAK_DEVICE_DESCRIPTOR_HANDLE foundDevice,
    PEAK_DEVICE_RECONNECT_INFORMATION reconnectInformation, void* context)
{
    auto callbackContainer = static_cast<DeviceReconnectedCallbackContainer*>(context);

    auto deviceDescriptor = callbackContainer->_Interface.lock()->GetOrAddFoundDevice(foundDevice);

    callbackContainer->Callback(deviceDescriptor, DeviceReconnectInformation(std::move(reconnectInformation)));
}

inline void PEAK_CALL_CONV Interface::DeviceListChangedCallbackCWrapper(void* context)
{
    auto callback = static_cast<Interface::DeviceListChangedCallback*>(context);
    callback->operator()();
}

inline std::shared_ptr<DeviceDescriptor> Interface::GetOrAddFoundDevice(PEAK_DEVICE_DESCRIPTOR_HANDLE foundDevice)
{
    std::lock_guard<std::mutex> lock(m_devicesMutex);

    const auto deviceKey = QueryStringFromCInterfaceFunction([&](char* key, size_t* keySize) {
        return PEAK_C_ABI_PREFIX PEAK_DeviceDescriptor_GetKey(foundDevice, key, keySize);
    });
    auto keyDevicePairIterator = m_devicesByKey.find(deviceKey);
    if (keyDevicePairIterator == m_devicesByKey.end())
    {
        auto deviceDescriptor = std::static_pointer_cast<DeviceDescriptor>(
            std::make_shared<ClassCreator<DeviceDescriptor>>(
                foundDevice, std::weak_ptr<Interface>{ shared_from_this() }));
        m_devices.emplace_back(deviceDescriptor);
        m_devicesByKey.emplace(deviceKey, deviceDescriptor);
        m_devicesKeyById.emplace(deviceDescriptor->ID(), deviceKey);

        return deviceDescriptor;
    }

    return keyDevicePairIterator->second;
}

inline PEAK_MODULE_HANDLE Interface::ModuleHandle() const
{
    auto moduleHandle = QueryNumericFromCInterfaceFunction<PEAK_MODULE_HANDLE>(
        [&](PEAK_MODULE_HANDLE* _moduleHandle) {
            return PEAK_C_ABI_PREFIX PEAK_Interface_ToModule(m_backendHandle, _moduleHandle);
        });

    return moduleHandle;
}

inline PEAK_EVENT_SUPPORTING_MODULE_HANDLE Interface::EventSupportingModuleHandle() const
{
    auto eventSupportingModuleHandle = QueryNumericFromCInterfaceFunction<PEAK_EVENT_SUPPORTING_MODULE_HANDLE>(
        [&](PEAK_EVENT_SUPPORTING_MODULE_HANDLE* _eventSupportingModuleHandle) {
            return PEAK_C_ABI_PREFIX PEAK_Interface_ToEventSupportingModule(
                m_backendHandle, _eventSupportingModuleHandle);
        });

    return eventSupportingModuleHandle;
}

inline void Interface::InitializeUpdateMechanismIfNecessary()
{
    std::call_once(m_updateMechanismInitializedFlag, [&] {
        (void)RegisterDeviceFoundCallback([](const std::shared_ptr<DeviceDescriptor>&) {
            // Registering an empty callback is enough since DeviceFoundCallbackCWrapper() handles everything else.
        });

        (void)RegisterDeviceLostCallback([&](const std::string& lostDeviceId) {
            {
                std::lock_guard<std::mutex> lock(m_devicesMutex);
                const auto lostDeviceKey = m_devicesKeyById.at(lostDeviceId);
                m_devices.erase(std::remove_if(std::begin(m_devices), std::end(m_devices),
                                    [lostDeviceKey](const std::shared_ptr<DeviceDescriptor>& device) {
                                        return device->Key() == lostDeviceKey;
                                    }),
                    std::end(m_devices));
                m_devicesByKey.erase(lostDeviceKey);
                m_devicesKeyById.erase(lostDeviceId);
            }
        });
    });
}

} /* namespace core */
} /* namespace peak */
