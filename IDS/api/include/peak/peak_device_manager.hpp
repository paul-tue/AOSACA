/*!
 * \file    peak_device_manager.hpp
 *
 * \author  IDS Imaging Development Systems GmbH
 * \date    2019-05-01
 * \since   1.0
 *
 * Copyright (c) 2019 - 2025, IDS Imaging Development Systems GmbH. All rights reserved.
 */

#pragma once

#include <peak/common/peak_timeout.hpp>
#include <peak/device/peak_device.hpp>
#include <peak/device/peak_device_descriptor.hpp>
#include <peak/environment/peak_environment_inspector.hpp>
#include <peak/exception/peak_exception.hpp>
#include <peak/generic/peak_t_callback_manager.hpp>
#include <peak/interface/peak_interface.hpp>
#include <peak/interface/peak_interface_descriptor.hpp>
#include <peak/producer_library/peak_producer_library.hpp>
#include <peak/reconnect/reconnect_information.hpp>
#include <peak/system/peak_system.hpp>
#include <peak/system/peak_system_descriptor.hpp>
#include <utility>


#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <exception>
#include <functional>
#include <future>
#include <iterator>
#include <memory>
#include <mutex>
#include <new>
#include <utility>
#include <vector>


namespace peak
{

/*!
 * \brief The global DeviceManager (singleton) searches all installed producer libraries (*.cti) and enumerates the
 * modules contained in them.
 *
 * Retrieve the global DeviceManager with DeviceManager::Instance().
 *
 * When Update() is called, it searches for all producer libraries contained in the directories found in the official
 * GenICam GenTL environment variable GENICAM_GENTL{32/64}_PATH. It then openes all found
 * \link core::ProducerLibrary ProducerLibraries\endlink, their \link core::System Systems\endlink, their
 * \link core::Interface Interfaces\endlink, and lists all available \link core::DeviceDescriptor DeviceDescriptors\endlink.
 *
 * \code
 * auto& deviceManager = peak::DeviceManager::Instance();
 * deviceManager.Update();
 * deviceDescriptors = deviceManager.Devices();
 * \endcode
 *
 * \note The found producer libraries depend on the architecture your application is compiled for. This means you are
 *       getting 32-bit producer libraries if your application is compiled for a 32-bit system and 64-bit producer
 *       libraries if your application is compiled for a 64-bit system.
 *
 * The DeviceManager is a singleton. Therefore, after it was used, it stays active until program termination. That
 * means the opened \link core::ProducerLibrary ProducerLibraries\endlink, \link core::System Systems\endlink, and
 * \link core::Interface Interfaces \endlink stay open. To close them before and start from scratch during runtime, call
 * Reset().
 */

class DeviceManager final
{
public:
    /*! \brief Enum holding the possible update policies. */
    enum class UpdatePolicy
    {
        /*! Scan env variable and add newly found producer libraries on #DeviceManager::Update() call */
        ScanEnvironmentForProducerLibraries,

        /*! Don't scan env variable and add newly found producer libraries on #DeviceManager::Update() call */
        DontScanEnvironmentForProducerLibraries
    };

    /*! \brief Enum holding the possible reset policies. */
    enum class ResetPolicy
    {
        /*! Check and throw an exception if an opened device is found */
        ErrorOnOpenDevices,

        /*! Ignore opened device and close them */
        IgnoreOpenDevices
    };

    /*! The type of system found callbacks. */
    using SystemFoundCallback = std::function<void(const std::shared_ptr<const core::System>& foundSystem)>;

    /*! The type of system found callback handles. */
    using SystemFoundCallbackHandle = SystemFoundCallback*;

    /*! The type of interface found callbacks. */
    using InterfaceFoundCallback = std::function<void(const std::shared_ptr<const core::Interface>& foundInterface)>;

    /*! The type of interface found callback handles. */
    using InterfaceFoundCallbackHandle = InterfaceFoundCallback*;

    /*! The type of interface lost callbacks. */
    using InterfaceLostCallback = std::function<void(const std::string& lostInterfaceKey)>;

    /*! The type of interface lost callback handles. */
    using InterfaceLostCallbackHandle = InterfaceLostCallback*;

    /*! The type of device found callbacks. */
    using DeviceFoundCallback = std::function<void(const std::shared_ptr<core::DeviceDescriptor>& foundDevice)>;

    /*! The type of device found callback handles. */
    using DeviceFoundCallbackHandle = DeviceFoundCallback*;

    /*! The type of device lost callbacks. */
    using DeviceLostCallback = std::function<void(const std::string& lostDeviceKey)>;

    /*! The type of device lost callback handles. */
    using DeviceLostCallbackHandle = DeviceLostCallback*;

    /*! The type of device disconnect callbacks. */
    using DeviceDisconnectedCallback =
        std::function<void(const std::shared_ptr<core::DeviceDescriptor>& disconnectedDevice)>;

    /*! The type of device disconnect callback handles. */
    using DeviceDisconnectedCallbackHandle = DeviceDisconnectedCallback*;

    /*! The type of device reconnect callbacks. */
    using DeviceReconnectedCallback =
        std::function<void(const std::shared_ptr<core::DeviceDescriptor>& reconnectedDevice,
            core::DeviceReconnectInformation reconnectInformation)>;

    /*! The type of device reconnect callback handles. */
    using DeviceReconnectedCallbackHandle = DeviceReconnectedCallback*;

    /*! The type of device list changed callbacks. */
    using DeviceListChangedCallback = std::function<void()>;

    /*! The type of device list changed callback handles. */
    using DeviceListChangedCallbackHandle = DeviceListChangedCallback*;

    /*! The type of update error callbacks. */
    using UpdateErrorCallback = std::function<void(const std::string& errorDescription)>;

    /*!
     * \brief Returns the global DeviceManager.
     *
     * \since 1.0
     */
    static DeviceManager& Instance();

    /*!
     * \brief Adds the given producer library (CTI).
     *
     * This function can be used to add producer libraries manually. This is useful when the desired producer
     * library is not registered at the GenTL environment variable.
     *
     * \param[in] ctiPath The path to the producer library (CTI) to add
     *
     * \since 1.0
     *
     * \throws core::InternalErrorException An internal error has occurred.
     *
     * \note Changes only take effect when they are applied before calling Update().
     *       Changes applied during a running update only take effect with the next call to Update().
     */
    void AddProducerLibrary(const std::string& ctiPath);

    /*!
     * \brief Adds the given producer library (CTI).
     *
     * This function can be used to add already opened producer libraries manually. This is useful when the desired
     * producer library is not registered at the GenTL environment variable. If the producer library is already present,
     * this call does nothing.
     *
     * \param[in] library The library instance
     *
     * \since 1.7
     *
     * \throws core::BadAccessException Access denied
     * \throws core::InternalErrorException An internal error has occurred.
     *
     * \note Changes only take effect when they are applied before calling Update().
     *       Calling the function while an update is running will block the call.
     * \note If the system of the producer library is not open, it will be opened immediately.
     * \note Will trigger the SystemFound callback if the system is added to the internal list.
     */
    void AddProducerLibrary(const std::shared_ptr<core::ProducerLibrary>& library);

    /*!
     * \brief Starts an update.
     *
     * If there is already an update in progress, it waits for the previous update to finish before starting the new
     * update.
     *
     * \param[in] updatePolicy The update policy to use.
     * \param[in] errorCallback A thread-safe callback getting called for each error occurring during the update.
     *
     * \since 1.0
     *
     * \throws core::NotInitializedException The library was not initialized before use.
     * \throws core::InternalErrorException An internal error has occurred.
     * \throws core::NotFoundException The environment variable GENICAM_GENTL32_PATH / GENICAM_GENTL64_PATH was not
     *                                 found or was empty when scanning for environment ProducerLibraries.
     * \throws core::CTILoadingException A error has occurred when loading a CTI from the environment variable
     *                                   GENICAM_GENTL32_PATH / GENICAM_GENTL64_PATH.
     */
    void Update(UpdatePolicy updatePolicy = UpdatePolicy::ScanEnvironmentForProducerLibraries,
        const UpdateErrorCallback& errorCallback = UpdateErrorCallback());

    /*!
     * \brief Resets the DeviceManager.
     *
     * All \link core::Interface Interfaces \endlink and \link core::System Systems \endlink are closed, additional
     * ProducerLibraries (added via AddProducerLibrary()) are cleared. Any registered callbacks stay active and the
     * DeviceLostCallback and InterfaceLostCallback are called for all devices / interfaces.
     *
     * \note Can't reset the DeviceManager while there are open \link core::Device Devices\endlink. Close all devices before
     *       calling this method. Otherwise, a core::InternalErrorException is thrown.
     *
     * \param[in] resetPolicy With the default ResetPolicy::ErrorOnOpenDevices, an exception is thrown if there are
     *            any open devices managed by the DeviceManager. With ResetPolicy::IgnoreOpenDevices, no such exception
     *            is thrown.
     *
     * \since 1.0
     *
     * \throws core::InternalErrorException If there are open devices (with ResetPolicy::ErrorOnOpenDevices).
     */
    void Reset(ResetPolicy resetPolicy = ResetPolicy::ErrorOnOpenDevices);

    /*!
     * \brief Returns the interface update timeout.
     *
     * \return Interface update timeout in milliseconds
     *
     * \since 1.0
     *
     * \throws core::InternalErrorException An internal error has occurred.
     */
    core::Timeout InterfaceUpdateTimeout() const;

    /*!
     * \brief Sets the interface update timeout.
     *
     * \param[in] timeout_ms The time to wait for new interfaces in milliseconds.
     *
     * \since 1.0
     *
     * \throws core::InternalErrorException An internal error has occurred.
     *
     * \note Changes only take effect when they are
     *       applied before calling Update(). Changes applied during a running update only take effect with the next
     *       call to Update().
     */
    void SetInterfaceUpdateTimeout(core::Timeout timeout_ms);

    /*!
     * \brief Returns the device update timeout.
     *
     * \return Device update timeout in milliseconds
     *
     * \since 1.0
     *
     * \throws core::InternalErrorException An internal error has occurred.
     */
    core::Timeout DeviceUpdateTimeout() const;

    /*!
     * \brief Sets the device update timeout.
     *
     * \param[in] timeout_ms The time to wait for new devices in milliseconds.
     *
     * \since 1.0
     *
     * \throws core::InternalErrorException An internal error has occurred.
     *
     * \note Changes only take effect when they are
     *       applied before calling Update(). Changes applied during a running update only take effect with the next
     *       call to Update().
     */
    void SetDeviceUpdateTimeout(core::Timeout timeout_ms);

    /*!
     * \brief Returns the found and opened systems.
     *
     * \return Found and opened systems
     *
     * \since 1.0
     *
     * \throws core::InternalErrorException An internal error has occurred.
     */
    std::vector<std::shared_ptr<const core::System>> Systems() const;

    /*!
     * \brief Returns the found and opened interfaces.
     *
     * \return Found and opened interfaces
     *
     * \since 1.0
     *
     * \throws core::InternalErrorException An internal error has occurred.
     */
    std::vector<std::shared_ptr<const core::Interface>> Interfaces() const;

    /*!
     * \brief Returns the found devices.
     *
     * \return Found devices
     *
     * \since 1.0
     *
     * \throws core::InternalErrorException An internal error has occurred.
     */
    std::vector<std::shared_ptr<core::DeviceDescriptor>> Devices() const;

    /*!
     * \brief Registers a callback for signaling a found system.
     *
     * This function registers a callback which gets called every time a new system is found. Pass the callback
     * handle returned by this function to UnregisterSystemFoundCallback() to unregister the callback.
     *
     * \param[in] callback The callback to call if a new system is found.
     *
     * \return Callback handle
     *
     * \since 1.0
     *
     * \throws core::InternalErrorException An internal error has occurred.
     *
     * \note The registered callback is executed in the thread calling Update().
     */
    SystemFoundCallbackHandle RegisterSystemFoundCallback(const SystemFoundCallback& callback);

    /*!
     * \brief Unregisters a system found callback.
     *
     * This function unregisters a system found callback by taking its handle.
     *
     * \param[in] callbackHandle The handle of the callback to unregister.
     *
     * \since 1.0
     *
     * \throws core::InternalErrorException An internal error has occurred.
     */
    void UnregisterSystemFoundCallback(SystemFoundCallbackHandle callbackHandle);

    /*!
     * \brief Registers a callback for signaling a found interface.
     *
     * This function registers a callback which gets called every time a new interface is found. Pass the callback
     * handle returned by this function to UnregisterInterfaceFoundCallback() to unregister the callback.
     *
     * \param[in] callback The callback to call if a new interface is found.
     *
     * \return Callback handle
     *
     * \since 1.0
     *
     * \throws core::InternalErrorException An internal error has occurred.
     *
     * \note The registered callback is executed in the thread calling Update().
     */
    InterfaceFoundCallbackHandle RegisterInterfaceFoundCallback(const InterfaceFoundCallback& callback);

    /*!
     * \brief Unregisters a interface found callback.
     *
     * This function unregisters a interface found callback by taking its handle.
     *
     * \param[in] callbackHandle The handle of the callback to unregister.
     *
     * \since 1.0
     *
     * \throws core::InternalErrorException An internal error has occurred.
     */
    void UnregisterInterfaceFoundCallback(InterfaceFoundCallbackHandle callbackHandle);

    /*!
     * \brief Registers a callback for signaling a lost interface.
     *
     * This function registers a callback which gets called every time an interface is lost. Pass the callback
     * handle returned by this function to UnregisterInterfaceLostCallback() to unregister the callback.
     *
     * \param[in] callback The callback to call if an interface is lost.
     *
     * \return Callback handle
     *
     * \since 1.0
     *
     * \throws core::InternalErrorException An internal error has occurred.
     *
     * \note The registered callback is executed in the thread calling Update().
     */
    InterfaceLostCallbackHandle RegisterInterfaceLostCallback(const InterfaceLostCallback& callback);

    /*!
     * \brief Unregisters an interface lost callback.
     *
     * This function unregisters an interface lost callback by taking its handle.
     *
     * \param[in] callbackHandle The handle of the callback to unregister.
     *
     * \since 1.0
     *
     * \throws core::InternalErrorException An internal error has occurred.
     */
    void UnregisterInterfaceLostCallback(InterfaceLostCallbackHandle callbackHandle);

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
     * \throws core::InternalErrorException An internal error has occurred.
     *
     * \note The registered callback is executed in the thread calling Update().
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
     * \throws core::InternalErrorException An internal error has occurred.
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
     * \throws core::InternalErrorException An internal error has occurred.
     *
     * \note The registered callback is executed in the thread calling Update().
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
     * \throws core::InternalErrorException An internal error has occurred.
     */
    void UnregisterDeviceLostCallback(DeviceLostCallbackHandle callbackHandle);

    /*!
     * \brief Registers a callback for signaling a disconnected device.
     *
     * This function registers a callback which gets called every time a device is disconnected. Pass the callback
     * handle returned by this function to UnregisterDeviceDisconnectCallback() to unregister the callback.
     *
     * \param[in] callback The callback to call if a device is disconnected.
     *
     * \return Callback handle
     *
     * \since 1.7
     *
     * \throws core::InternalErrorException An internal error has occurred.
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
     * \throws core::InternalErrorException An internal error has occurred.
     */
    void UnregisterDeviceDisconnectedCallback(DeviceDisconnectedCallbackHandle callbackHandle);

    /*!
     * \brief Registers a callback for signaling a reconnected device.
     *
     * This function registers a callback which gets called every time a device is reconnected. Pass the callback
     * handle returned by this function to UnregisterDeviceReconnectedCallback() to unregister the callback.
     *
     * \param[in] callback The callback to call if a device is reconnected.
     *
     * \return Callback handle
     *
     * \since 1.7
     *
     * \throws core::InternalErrorException An internal error has occurred.
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
     * \throws core::InternalErrorException An internal error has occurred.
     */
    void UnregisterDeviceReconnectedCallback(DeviceReconnectedCallbackHandle callbackHandle);

    /*!
     * \brief Registers a callback for signaling a change in the device list.
     *
     * This function registers a callback which gets called every time the device list changes. Pass the callback
     * handle returned by this function to UnregisterDeviceListChangedCallback() to unregister the callback.
     *
     * \param[in] callback The callback to call if the device list has changed.
     *
     * \return Callback handle
     *
     * \since 1.7
     *
     * \throws core::InternalErrorException An internal error has occurred.
     */
    DeviceListChangedCallbackHandle RegisterDeviceListChangedCallback(const DeviceListChangedCallback& callback);

    /*!
     * \brief Unregisters a device list change callback.
     *
     * This function unregisters a device lost callback by taking its handle.
     *
     * \param[in] callbackHandle The handle of the callback to unregister.
     *
     * \since 1.7
     *
     * \throws core::InternalErrorException An internal error has occurred.
     */
    void UnregisterDeviceListChangedCallback(DeviceListChangedCallbackHandle callbackHandle);

    DeviceManager(const DeviceManager& other) = delete;
    DeviceManager& operator=(const DeviceManager& other) = delete;

private:
    friend class Library;

    DeviceManager() = default;
    ~DeviceManager();

    void GetLostDevicesFromLostInterfaces(
        const std::vector<std::string>& lostInterfaces, std::vector<std::string>& lostDevices);

    void TriggerSystemFoundCallbacks(const std::shared_ptr<const core::System>& foundSystem) const;
    void TriggerInterfaceFoundCallbacks(const std::shared_ptr<const core::Interface>& foundInterface) const;
    void TriggerInterfaceLostCallbacks(const std::string& lostInterfaceKey) const;
    void TriggerDeviceFoundCallbacks(const std::shared_ptr<core::DeviceDescriptor>& foundDevice) const;
    void TriggerDeviceLostCallbacks(const std::string& lostDeviceKey) const;

    void AddSystemCallbacks(const std::shared_ptr<core::System>& addSystem);
    void RemoveSystemCallbacks(const std::shared_ptr<core::System>& removeSystem);
    void AddInterfaceCallbacks(const std::shared_ptr<core::Interface>& addInterface);
    void RemoveInterfaceCallbacks(const std::shared_ptr<core::Interface>& removeInterface);

    void CheckDevicesOpened() const;

    std::mutex m_updateMutex;
    std::atomic<core::Timeout> m_interfaceUpdateTimeout_ms{ 100 };
    std::atomic<core::Timeout> m_deviceUpdateTimeout_ms{ 300 };

    std::vector<std::string> m_producerLibrariesToAdd;

    std::unordered_map<std::string, std::shared_ptr<core::ProducerLibrary>> m_producerLibraries;
    std::vector<std::shared_ptr<const core::System>> m_systems;
    std::vector<std::shared_ptr<const core::Interface>> m_interfaces;
    std::vector<std::shared_ptr<core::DeviceDescriptor>> m_devices;

    struct SystemCallbackContainer
    {
        SystemCallbackContainer(
            core::System::InterfaceFoundCallbackHandle foundHandle, InterfaceLostCallbackHandle lostHandle)
            : foundCallbackHandle(foundHandle)
            , lostCallbackHandle(lostHandle)
        {}
        ~SystemCallbackContainer() = default;
        SystemCallbackContainer(const SystemCallbackContainer&) = delete;
        SystemCallbackContainer& operator=(const SystemCallbackContainer&) = delete;

        SystemCallbackContainer(SystemCallbackContainer&& a) noexcept
        {
            std::lock_guard<std::mutex> lock(a.mutex);

            std::swap(foundCallbackHandle, a.foundCallbackHandle);
            std::swap(lostCallbackHandle, a.lostCallbackHandle);
        }

        SystemCallbackContainer& operator=(SystemCallbackContainer&& a) noexcept
        {
            std::lock(a.mutex, mutex);

            std::lock_guard<std::mutex> lock1(a.mutex, std::adopt_lock);
            std::lock_guard<std::mutex> lock2(mutex, std::adopt_lock);

            std::swap(foundCallbackHandle, a.foundCallbackHandle);
            std::swap(lostCallbackHandle, a.lostCallbackHandle);

            return *this;
        }

        mutable std::mutex mutex;
        core::System::InterfaceFoundCallbackHandle foundCallbackHandle{};
        InterfaceLostCallbackHandle lostCallbackHandle{};

        std::vector<std::shared_ptr<core::Interface>> foundInterfaces;
        std::vector<std::string> lostInterfaces;

        // TL reports interface lost only with interface id part
        // in order to get the full interface key we need to map it
        std::unordered_map<std::string, std::string> interfaceIdKeyMap;
    };

    struct InterfaceCallbackContainer
    {
        InterfaceCallbackContainer(DeviceFoundCallbackHandle foundHandle, DeviceLostCallbackHandle lostHandle,
            DeviceDisconnectedCallbackHandle disconnectedHandle, DeviceReconnectedCallbackHandle reconnectedHandle,
            DeviceListChangedCallbackHandle listChangedHandle)
            : foundCallbackHandle(foundHandle)
            , lostCallbackHandle(lostHandle)
            , disconnectedCallbackHandle(disconnectedHandle)
            , reconnectedCallbackHandle(reconnectedHandle)
            , deviceListChangedCallbackHandle(listChangedHandle)
        {}

        ~InterfaceCallbackContainer() = default;
        InterfaceCallbackContainer(const InterfaceCallbackContainer&) = delete;
        InterfaceCallbackContainer& operator=(const InterfaceCallbackContainer&) = delete;

        InterfaceCallbackContainer(InterfaceCallbackContainer&& a) noexcept
        {
            std::lock_guard<std::mutex> lock(a.mutex);

            std::swap(foundCallbackHandle, a.foundCallbackHandle);
            std::swap(lostCallbackHandle, a.lostCallbackHandle);
            std::swap(disconnectedCallbackHandle, a.disconnectedCallbackHandle);
            std::swap(reconnectedCallbackHandle, a.reconnectedCallbackHandle);
            std::swap(deviceListChangedCallbackHandle, a.deviceListChangedCallbackHandle);
        }

        InterfaceCallbackContainer& operator=(InterfaceCallbackContainer&& a) noexcept
        {
            std::lock(a.mutex, mutex);

            std::lock_guard<std::mutex> lock1(a.mutex, std::adopt_lock);
            std::lock_guard<std::mutex> lock2(mutex, std::adopt_lock);

            std::swap(foundCallbackHandle, a.foundCallbackHandle);
            std::swap(lostCallbackHandle, a.lostCallbackHandle);
            std::swap(disconnectedCallbackHandle, a.disconnectedCallbackHandle);
            std::swap(reconnectedCallbackHandle, a.reconnectedCallbackHandle);
            std::swap(deviceListChangedCallbackHandle, a.deviceListChangedCallbackHandle);

            return *this;
        }

        mutable std::mutex mutex;
        DeviceFoundCallbackHandle foundCallbackHandle{};
        DeviceLostCallbackHandle lostCallbackHandle{};
        DeviceDisconnectedCallbackHandle disconnectedCallbackHandle{};
        DeviceReconnectedCallbackHandle reconnectedCallbackHandle{};
        DeviceListChangedCallbackHandle deviceListChangedCallbackHandle{};

        std::vector<std::shared_ptr<core::DeviceDescriptor>> foundDevices;
        std::vector<std::string> lostDevices;

        // TL reports device lost only with device id part
        // in order to get the full device key we need to map it
        std::unordered_map<std::string, std::string> deviceIdKeyMap;
    };

    mutable std::mutex m_callbackContainerMutex;
    std::unordered_map<std::string, SystemCallbackContainer> m_systemCallbackContainers;
    std::unordered_map<std::string, InterfaceCallbackContainer> m_interfaceCallbackContainers;

    struct OpenedDevice
    {
        std::weak_ptr<core::Device> device{};
        std::string displayName;
    };
    std::vector<OpenedDevice> m_openedDevices;

    core::TTriggerCallbackManager<SystemFoundCallbackHandle, SystemFoundCallback> m_systemFoundCallbackManager;
    core::TTriggerCallbackManager<InterfaceFoundCallbackHandle, InterfaceFoundCallback> m_interfaceFoundCallbackManager;
    core::TTriggerCallbackManager<InterfaceLostCallbackHandle, InterfaceLostCallback> m_interfaceLostCallbackManager;
    core::TTriggerCallbackManager<DeviceFoundCallbackHandle, DeviceFoundCallback> m_deviceFoundCallbackManager;
    core::TTriggerCallbackManager<DeviceLostCallbackHandle, DeviceLostCallback> m_deviceLostCallbackManager;
    core::TTriggerCallbackManager<DeviceDisconnectedCallbackHandle, DeviceDisconnectedCallback>
        m_deviceDisconnectedCallbackManager;
    core::TTriggerCallbackManager<DeviceReconnectedCallbackHandle, DeviceReconnectedCallback>
        m_deviceReconnectedCallbackManager;
    core::TTriggerCallbackManager<DeviceListChangedCallbackHandle, DeviceListChangedCallback>
        m_deviceListChangedCallbackManager;

    std::mutex m_producerLibrariesToAddMutex;
    mutable std::mutex m_systemsMutex;
    mutable std::recursive_mutex m_interfacesMutex;
    mutable std::recursive_mutex m_devicesMutex;
    mutable std::mutex m_openedDevicesMutex;

#ifndef _MSC_VER
    template <typename T,
        typename std::enable_if<(noexcept(std::declval<T>()())
                                    || (std::is_pointer<typename std::decay<T>::type>::value && __cplusplus < 201703L)
                                    // NOTE pre c++17 there is no way to check whether a function pointer is noexcept
                                    )
                && std::is_same<void, decltype(std::declval<T>()())>::value && std::is_move_constructible<T>::value
                && std::is_nothrow_move_constructible<T>::value,
            int>::type = 0>
#else
    template <typename T>
#endif
    class RAIIGuard
    {
        T m_fun;

    public:
        explicit RAIIGuard(T&& fun) noexcept
            : m_fun(std::forward<T>(fun))
        {}
        RAIIGuard(RAIIGuard const&) = delete;
        RAIIGuard(RAIIGuard&& other) noexcept
            : m_fun(std::move(other.m_fun))
        {}
        ~RAIIGuard() noexcept
        {
            m_fun();
        }
    };
    template <typename T>
    static RAIIGuard<T> makeRAIIGuard(T&& fun) noexcept
    {
        return { std::forward<T>(fun) };
    }

    template <typename T = DeviceManager>
    class LifetimeTracker
    {
    public:
        static void Create();
        static void Destroy();
        static T& Instance();

    private:
        static T* m_instance;
        static std::mutex m_mtx;
        static size_t m_initializedCount;
    };
};

} /* namespace peak */

/* Implementation */
namespace peak
{

template <typename T>
T* DeviceManager::LifetimeTracker<T>::m_instance = {};

template <typename T>
std::mutex DeviceManager::LifetimeTracker<T>::m_mtx = {};

template <typename T>
size_t DeviceManager::LifetimeTracker<T>::m_initializedCount = {};

template <typename T>
void DeviceManager::LifetimeTracker<T>::Create()
{
    std::lock_guard<decltype(m_mtx)> lock{ m_mtx };

    if (!m_instance)
    {
        m_instance = new T{};
    }

    m_initializedCount++;
}

template <typename T>
void DeviceManager::LifetimeTracker<T>::Destroy()
{
    std::lock_guard<decltype(m_mtx)> lock{ m_mtx };

    if (0 == m_initializedCount)
    {
        return;
    }

    m_initializedCount--;

    if (0 == m_initializedCount)
    {
        if (m_instance)
        {
            delete m_instance;
            m_instance = nullptr;
        }
    }
}

template <typename T>
T& DeviceManager::LifetimeTracker<T>::Instance()
{
    if (m_instance == nullptr)
    {
        throw core::NotInitializedException(
            "Library is not initialized. Call peak::Library::Initalize() first!");
    }
    return *m_instance;
}

inline DeviceManager& DeviceManager::Instance()
{
    return LifetimeTracker<>::Instance();
}

inline DeviceManager::~DeviceManager()
{
    Reset(ResetPolicy::IgnoreOpenDevices);
}

inline void DeviceManager::AddProducerLibrary(const std::string& ctiPath)
{
    std::lock_guard<std::mutex> lock(m_producerLibrariesToAddMutex);

    m_producerLibrariesToAdd.emplace_back(ctiPath);
}

inline void DeviceManager::AddProducerLibrary(const std::shared_ptr<core::ProducerLibrary>& library)
{
    std::lock(m_updateMutex, m_systemsMutex);

    std::lock_guard<std::mutex> lockUpdate(m_updateMutex, std::adopt_lock);

    std::shared_ptr<core::System> newSystem{};
    {
        std::lock_guard<std::mutex> lockSystems(m_systemsMutex, std::adopt_lock);

        const auto systemDescriptor = library->System();
        const auto ctiPath = systemDescriptor->CTIFullPath();

        // Check if system is already added
        if (m_producerLibraries.count(ctiPath) == 0)
        {
            m_producerLibraries.emplace(ctiPath, library);

            newSystem = systemDescriptor->IsOpened() ? systemDescriptor->OpenedSystem() : systemDescriptor->OpenSystem();
            AddSystemCallbacks(newSystem);
            m_systems.emplace_back(newSystem);
        }
    }

    if (newSystem)
    {
        TriggerSystemFoundCallbacks(newSystem);
    }
}

inline void DeviceManager::Update(UpdatePolicy updatePolicy /*= UpdatePolicy::ScanEnvironmentForProducerLibraries*/,
    const UpdateErrorCallback& errorCallback /*= UpdateErrorCallback()*/)
{
    std::lock_guard<std::mutex> lockUpdate(m_updateMutex);

    std::vector<std::shared_ptr<core::System>> allFoundSystems;
    std::vector<std::shared_ptr<core::Interface>> allFoundInterfaces;
    std::vector<std::string> allLostInterfaces;
    std::vector<std::shared_ptr<core::DeviceDescriptor>> allFoundDevices;
    std::vector<std::string> allLostDevices;

    auto interfaceUpdateTimeout_ms = m_interfaceUpdateTimeout_ms.load();
    auto deviceUpdateTimeout_ms = m_deviceUpdateTimeout_ms.load();

    std::vector<std::string> ctiPaths;
    // We look for all producer libraries in the current environment if we have to
    if (updatePolicy == UpdatePolicy::ScanEnvironmentForProducerLibraries)
    {
        ctiPaths = core::EnvironmentInspector::CollectCTIPaths();
    }

    // We look for the manually added producer libraries
    {
        std::lock_guard<std::mutex> lock(m_producerLibrariesToAddMutex);

        ctiPaths.reserve(ctiPaths.size() + m_producerLibrariesToAdd.size());
        std::move(
            std::begin(m_producerLibrariesToAdd), std::end(m_producerLibrariesToAdd), std::back_inserter(ctiPaths));

        m_producerLibrariesToAdd.clear();
    }

    for (const auto& ctiPath : ctiPaths)
    {
        if (m_producerLibraries.count(ctiPath) == 0)
        {
            try
            {
                auto producerLibrary = core::ProducerLibrary::Open(ctiPath);
                auto system = producerLibrary->System()->OpenSystem();

                AddSystemCallbacks(system);

                m_producerLibraries.emplace(ctiPath, producerLibrary);
                allFoundSystems.emplace_back(system);
            }
            catch (const core::NotAvailableException&)
            {
                // TL not available or not supported.
                continue;
            }
            catch (const core::NotInitializedException&)
            {
                // TODO: check this on creation of DeviceManager?
                throw; // if this exception occurs, nothing else will work. so we stop here
            }
            catch (const core::Exception& ex)
            {
                if (errorCallback)
                {
                    try
                    {
                        errorCallback("[CTI - " + ctiPath + "]: " + ex.what());
                    }
                    catch (...)
                    {}
                }
            }
        }
    }

    auto systemUpdateChildren = [&](const std::shared_ptr<core::System>& system) {
        try
        {
            system->UpdateInterfaces(interfaceUpdateTimeout_ms);
        }
        catch (const core::Exception& ex)
        {
            if (errorCallback)
            {
                try
                {
                    errorCallback("[System - " + system->Key() + "]: " + ex.what());
                }
                catch (...)
                {}
            }
        }
    };

    // We do an update on all available systems
    constexpr auto launchPolicy = std::launch::async; // switching this to std::launch::deferred makes debugging easier

    std::vector<std::future<void>> systemUpdateChildrenTasks;

    // add already found systems
    for (const auto& system : m_systems)
    {
        systemUpdateChildrenTasks.emplace_back(
            std::async(launchPolicy, systemUpdateChildren, std::const_pointer_cast<core::System>(system)));
    }

    // now add newly found systems
    for (const auto& system : allFoundSystems)
    {
        systemUpdateChildrenTasks.emplace_back(std::async(launchPolicy, systemUpdateChildren, system));
    }
    
    // wait for tasks to be done
    systemUpdateChildrenTasks.clear();

    // copy results to our local lists
    for (auto& keyValue : m_systemCallbackContainers)
    {
        auto& container = keyValue.second;

        std::lock_guard<std::mutex> lock(container.mutex);

        allFoundInterfaces.reserve(allFoundInterfaces.size() + container.foundInterfaces.size());
        for (const auto& foundInterface : container.foundInterfaces)
        {
            AddInterfaceCallbacks(foundInterface);
            allFoundInterfaces.emplace_back(foundInterface);
        }
        container.foundInterfaces.clear();

        allLostInterfaces.reserve(allLostInterfaces.size() + container.lostInterfaces.size());
        for (const auto& lostInterface : container.lostInterfaces)
        {
            auto interfaceIt = std::find_if(std::begin(m_interfaces), std::end(m_interfaces),
                [&lostInterface](const auto& intf) { return intf->Key() == lostInterface; });

            if (interfaceIt != std::end(m_interfaces))
            {
                RemoveInterfaceCallbacks(std::const_pointer_cast<core::Interface>(*interfaceIt));
            }

            allLostInterfaces.emplace_back(lostInterface);
        }
        container.lostInterfaces.clear();
    }

    auto interfaceUpdateChildren = [&deviceUpdateTimeout_ms, &errorCallback](
                                       const std::shared_ptr<core::Interface>& iface) {
        try
        {
            iface->UpdateDevices(deviceUpdateTimeout_ms);
        }
        catch (const core::Exception& ex)
        {
            if (errorCallback)
            {
                try
                {
                    errorCallback("[Interface - " + iface->Key() + "]: " + ex.what());
                }
                catch (...)
                {}
            }
        }
    };

    // We do an update on all available interfaces
    std::vector<std::future<void>> interfaceUpdateChildrenTasks;

    // add already found interfaces
    for (const auto& iface : m_interfaces)
    {
        interfaceUpdateChildrenTasks.emplace_back(
            std::async(launchPolicy, interfaceUpdateChildren, std::const_pointer_cast<core::Interface>(iface)));
    }

    // add newly found interfaces
    for (const auto& iface : allFoundInterfaces)
    {
        interfaceUpdateChildrenTasks.emplace_back(std::async(launchPolicy, interfaceUpdateChildren, iface));
    }

    // wait for tasks to be done
    interfaceUpdateChildrenTasks.clear();

    // copy results to our local lists
    for (auto& keyValue : m_interfaceCallbackContainers)
    {
        auto& container = keyValue.second;
        std::lock_guard<std::mutex> lock(container.mutex);

        decltype(container.foundDevices) foundDevices;
        std::swap(container.foundDevices, foundDevices);
        allFoundDevices.reserve(allFoundDevices.size() + foundDevices.size());
        std::move(std::begin(foundDevices), std::end(foundDevices), std::back_inserter(allFoundDevices));

        decltype(container.lostDevices) lostDevices;
        std::swap(container.lostDevices, lostDevices);
        allLostDevices.reserve(allLostDevices.size() + lostDevices.size());
        std::move(std::begin(lostDevices), std::end(lostDevices), std::back_inserter(allLostDevices));
    }

    // We apply the update changes to our internal lists
    {
        std::lock(m_systemsMutex, m_interfacesMutex, m_devicesMutex);

        std::lock_guard<std::mutex> lockSystems(m_systemsMutex, std::adopt_lock);
        std::lock_guard<std::recursive_mutex> lockInterfaces(m_interfacesMutex, std::adopt_lock);
        std::lock_guard<std::recursive_mutex> lockDevices(m_devicesMutex, std::adopt_lock);

        m_systems.reserve(m_systems.size() + allFoundSystems.size());
        std::copy(std::begin(allFoundSystems), std::end(allFoundSystems), std::back_inserter(m_systems));

        for (const auto& lostInterfaceKey : allLostInterfaces)
        {
            m_interfaces.erase(std::remove_if(std::begin(m_interfaces), std::end(m_interfaces),
                                   [&lostInterfaceKey](const auto& interfaceToCheck) {
                                       return interfaceToCheck->Key() == lostInterfaceKey;
                                   }),
                std::end(m_interfaces));
        }
        m_interfaces.reserve(m_interfaces.size() + allFoundInterfaces.size());
        std::copy(std::begin(allFoundInterfaces), std::end(allFoundInterfaces), std::back_inserter(m_interfaces));

        for (const auto& lostDeviceKey : allLostDevices)
        {
            m_devices.erase(
                std::remove_if(std::begin(m_devices), std::end(m_devices),
                    [&lostDeviceKey](const auto& deviceToCheck) { return deviceToCheck->Key() == lostDeviceKey; }),
                std::end(m_devices));
        }
        m_devices.reserve(m_devices.size() + allFoundDevices.size());
        std::copy(std::begin(allFoundDevices), std::end(allFoundDevices), std::back_inserter(m_devices));
    }

    // We trigger all callbacks
    for (const auto& foundSystem : allFoundSystems)
    {
        TriggerSystemFoundCallbacks(foundSystem);
    }

    for (const auto& foundInterface : allFoundInterfaces)
    {
        TriggerInterfaceFoundCallbacks(foundInterface);
    }

    for (const auto& lostInterfaceKey : allLostInterfaces)
    {
        TriggerInterfaceLostCallbacks(lostInterfaceKey);
    }

    for (const auto& foundDevice : allFoundDevices)
    {
        TriggerDeviceFoundCallbacks(foundDevice);
    }

    GetLostDevicesFromLostInterfaces(allLostInterfaces, allLostDevices);
    for (const auto& lostDeviceKey : allLostDevices)
    {
        TriggerDeviceLostCallbacks(lostDeviceKey);
    }
}

inline void DeviceManager::Reset(ResetPolicy resetPolicy)
{
    std::lock_guard<std::mutex> lockUpdate(m_updateMutex);

    if (resetPolicy == ResetPolicy::ErrorOnOpenDevices)
    {
        // check if any devices are running
        CheckDevicesOpened();
    }

    {
        std::lock(m_systemsMutex, m_interfacesMutex, m_devicesMutex);

        std::lock_guard<std::mutex> lockSystem(m_systemsMutex, std::adopt_lock);
        std::lock_guard<std::recursive_mutex> lockInterfaces(m_interfacesMutex, std::adopt_lock);
        std::lock_guard<std::recursive_mutex> lockDevices(m_devicesMutex, std::adopt_lock);

        // Note: the list must be swapped because the devices must be removed from the list before the callback is
        // called
        decltype(m_devices) lostDevicesList{};
        std::swap(m_devices, lostDevicesList);
        for (const auto& lostDevice : lostDevicesList)
        {
            TriggerDeviceLostCallbacks(lostDevice->Key());
        }

        decltype(m_interfaces) lostInterfacesList{};
        std::swap(m_interfaces, lostInterfacesList);
        for (const auto& lostInterface : lostInterfacesList)
        {
            TriggerInterfaceLostCallbacks(lostInterface->Key());
            RemoveInterfaceCallbacks(std::const_pointer_cast<core::Interface>(lostInterface));
        }

        decltype(m_systems) lostSystemsList{};
        std::swap(m_systems, lostSystemsList);
        for (const auto& lostSystem : lostSystemsList)
        {
            RemoveSystemCallbacks(std::const_pointer_cast<core::System>(lostSystem));
        }

        m_producerLibraries.clear();
    }

    {
        std::lock_guard<std::mutex> lock(m_callbackContainerMutex);

        m_systemCallbackContainers.clear();
        m_interfaceCallbackContainers.clear();
    }

    {
        std::lock_guard<std::mutex> lock(m_producerLibrariesToAddMutex);
        m_producerLibrariesToAdd.clear();
    }
}

inline void DeviceManager::AddSystemCallbacks(const std::shared_ptr<core::System>& addSystem)
{
    auto SystemKey = addSystem->Key();

    const auto foundCallbackHandle = addSystem->RegisterInterfaceFoundCallback(
        [this, SystemKey](const std::shared_ptr<core::InterfaceDescriptor>& foundInterface) {
            const auto systemIt = m_systemCallbackContainers.find(SystemKey);
            if (systemIt != std::end(m_systemCallbackContainers))
            {
                try
                {
                    auto& container = systemIt->second;
                    std::lock_guard<std::mutex> lock(container.mutex);

                    const auto openedInterface = foundInterface->OpenInterface();

                    container.foundInterfaces.emplace_back(openedInterface);
                    container.interfaceIdKeyMap.emplace(openedInterface->ID(), openedInterface->Key());
                }
                catch (const std::exception&)
                {}
            }
        });

    const auto lostCallbackHandle = addSystem->RegisterInterfaceLostCallback(
        [this, SystemKey](const std::string& lostInterfaceId) {
            const auto systemIt = m_systemCallbackContainers.find(SystemKey);
            if (systemIt != std::end(m_systemCallbackContainers))
            {
                auto& container = systemIt->second;
                std::lock_guard<std::mutex> lock(container.mutex);

                const auto interfaceKeyIt = container.interfaceIdKeyMap.find(lostInterfaceId);
                if (interfaceKeyIt != std::end(container.interfaceIdKeyMap))
                {
                    container.lostInterfaces.emplace_back(interfaceKeyIt->second);
                    container.interfaceIdKeyMap.erase(interfaceKeyIt);
                }
            }
        });

    std::lock_guard<std::mutex> lock(m_callbackContainerMutex);

    m_systemCallbackContainers.emplace(
        addSystem->Key(), SystemCallbackContainer{ foundCallbackHandle, lostCallbackHandle });
}

inline void DeviceManager::RemoveSystemCallbacks(const std::shared_ptr<core::System>& removeSystem)
{
    std::lock_guard<std::mutex> lock(m_callbackContainerMutex);

    const auto SystemKey = removeSystem->Key();

    const auto systemIt = m_systemCallbackContainers.find(SystemKey);

    if (systemIt != std::end(m_systemCallbackContainers))
    {
        const auto& container = systemIt->second;
        {
            std::lock_guard<std::mutex> lockContainer(container.mutex);

            removeSystem->UnregisterInterfaceFoundCallback(container.foundCallbackHandle);
            removeSystem->UnregisterInterfaceLostCallback(container.lostCallbackHandle);
        }

        m_systemCallbackContainers.erase(systemIt);
    }
}

inline void DeviceManager::AddInterfaceCallbacks(const std::shared_ptr<core::Interface>& addInterface)
{
    auto InterfaceKey = addInterface->Key();

    const auto foundCallbackHandle = addInterface->RegisterDeviceFoundCallback(
        [this, InterfaceKey](const std::shared_ptr<core::DeviceDescriptor>& foundDevice) {
            const auto interfaceIt = m_interfaceCallbackContainers.find(InterfaceKey);

            if (interfaceIt != std::end(m_interfaceCallbackContainers))
            {
                auto& container = interfaceIt->second;
                std::lock_guard<std::mutex> lock(container.mutex);

                container.foundDevices.emplace_back(foundDevice);
                container.deviceIdKeyMap.emplace(foundDevice->ID(), foundDevice->Key());
            }

            // also keep track about opened devices
            foundDevice->RegisterDeviceOpenedCallback([this](const std::shared_ptr<core::Device>& openedDevice) {
                std::lock_guard<std::mutex> lock(m_openedDevicesMutex);

                // clear out closed devices
                m_openedDevices.erase(std::remove_if(std::begin(m_openedDevices), std::end(m_openedDevices),
                                          [](const OpenedDevice& dev) { return dev.device.expired(); }),
                    std::end(m_openedDevices));

                // add the new one
                OpenedDevice device;
                device.device = openedDevice;
                device.displayName = openedDevice->DisplayName();
                m_openedDevices.push_back(device);
            });
        });

    const auto lostCallbackHandle = addInterface->RegisterDeviceLostCallback(
        [this, InterfaceKey](const std::string& lostDeviceId) {
            const auto interfaceIt = m_interfaceCallbackContainers.find(InterfaceKey);

            if (interfaceIt != std::end(m_interfaceCallbackContainers))
            {
                auto& container = interfaceIt->second;
                std::lock_guard<std::mutex> lock(container.mutex);

                const auto lostDeviceKeyIt = container.deviceIdKeyMap.find(lostDeviceId);
                if (lostDeviceKeyIt != std::end(container.deviceIdKeyMap))
                {
                    container.lostDevices.emplace_back(lostDeviceKeyIt->second);
                    container.deviceIdKeyMap.erase(lostDeviceKeyIt);
                }
            }
        });

    const auto reconnectedHandle = addInterface->RegisterDeviceReconnectedCallback(
        [this](const std::shared_ptr<core::DeviceDescriptor>& reconnectedDevice,
            core::DeviceReconnectInformation reconnectInformation) {
            return m_deviceReconnectedCallbackManager.TriggerCallbacks(reconnectedDevice, reconnectInformation);
        });

    const auto disconnectedHandle = addInterface->RegisterDeviceDisconnectedCallback(
        [this](const std::shared_ptr<core::DeviceDescriptor>& disconnectedDevice) {
            return m_deviceDisconnectedCallbackManager.TriggerCallbacks(disconnectedDevice);
        });

    const auto deviceListChangedHandle = addInterface->RegisterDeviceListChangedCallback(
        [this]() { return m_deviceListChangedCallbackManager.TriggerCallbacks(); });

    std::lock_guard<std::mutex> lock(m_callbackContainerMutex);

    m_interfaceCallbackContainers.emplace(addInterface->Key(),
        InterfaceCallbackContainer{
            foundCallbackHandle, lostCallbackHandle, disconnectedHandle, reconnectedHandle, deviceListChangedHandle });
}

inline void DeviceManager::RemoveInterfaceCallbacks(const std::shared_ptr<core::Interface>& removeInterface)
{
    std::lock_guard<std::mutex> lock(m_callbackContainerMutex);

    const auto interfaceIt = m_interfaceCallbackContainers.find(removeInterface->Key());

    if (interfaceIt != std::end(m_interfaceCallbackContainers))
    {
        const auto& container = interfaceIt->second;
        {
            std::lock_guard<std::mutex> lockContainer(container.mutex);

            removeInterface->UnregisterDeviceFoundCallback(container.foundCallbackHandle);
            removeInterface->UnregisterDeviceLostCallback(container.lostCallbackHandle);
            removeInterface->UnregisterDeviceDisconnectedCallback(container.disconnectedCallbackHandle);
            removeInterface->UnregisterDeviceReconnectedCallback(container.reconnectedCallbackHandle);
            removeInterface->UnregisterDeviceListChangedCallback(container.deviceListChangedCallbackHandle);
        }
        m_interfaceCallbackContainers.erase(interfaceIt);
    }
}

inline void DeviceManager::CheckDevicesOpened() const
{
    size_t openDevicesFound = 0;
    std::string deviceDisplayNames;

    {
        std::lock_guard<std::mutex> lock(m_openedDevicesMutex);

        for (const auto& openedDevice : m_openedDevices)
        {
            auto openDevice = openedDevice.device.lock();
            if (openDevice)
            {
                ++openDevicesFound;
                deviceDisplayNames += openedDevice.displayName;
                deviceDisplayNames += " ";
            }
        }
    }

    if (openDevicesFound > 0)
    {
        throw core::InternalErrorException(
            "Can't reset the device manager while there are devices open. The following devices ("
            + std::to_string(openDevicesFound) + ") are open: " + deviceDisplayNames);
    }
}

inline core::Timeout DeviceManager::InterfaceUpdateTimeout() const
{
    return m_interfaceUpdateTimeout_ms;
}

inline void DeviceManager::SetInterfaceUpdateTimeout(core::Timeout timeout_ms)
{
    m_interfaceUpdateTimeout_ms = timeout_ms;
}

inline core::Timeout DeviceManager::DeviceUpdateTimeout() const
{
    return m_deviceUpdateTimeout_ms;
}

inline void DeviceManager::SetDeviceUpdateTimeout(core::Timeout timeout_ms)
{
    m_deviceUpdateTimeout_ms = timeout_ms;
}

inline std::vector<std::shared_ptr<const core::System>> DeviceManager::Systems() const
{
    std::lock_guard<std::mutex> lock(m_systemsMutex);

    return m_systems;
}

inline std::vector<std::shared_ptr<const core::Interface>> DeviceManager::Interfaces() const
{
    std::lock_guard<std::recursive_mutex> lock(m_interfacesMutex);

    return m_interfaces;
}

inline std::vector<std::shared_ptr<core::DeviceDescriptor>> DeviceManager::Devices() const
{
    std::lock_guard<std::recursive_mutex> lock(m_devicesMutex);

    return m_devices;
}

inline DeviceManager::SystemFoundCallbackHandle DeviceManager::RegisterSystemFoundCallback(
    const DeviceManager::SystemFoundCallback& callback)
{
    return m_systemFoundCallbackManager.RegisterCallback(callback);
}

inline void DeviceManager::UnregisterSystemFoundCallback(DeviceManager::SystemFoundCallbackHandle callbackHandle)
{
    m_systemFoundCallbackManager.UnregisterCallback(callbackHandle);
}

inline DeviceManager::InterfaceFoundCallbackHandle DeviceManager::RegisterInterfaceFoundCallback(
    const DeviceManager::InterfaceFoundCallback& callback)
{
    return m_interfaceFoundCallbackManager.RegisterCallback(callback);
}

inline void DeviceManager::UnregisterInterfaceFoundCallback(DeviceManager::InterfaceFoundCallbackHandle callbackHandle)
{
    m_interfaceFoundCallbackManager.UnregisterCallback(callbackHandle);
}

inline DeviceManager::InterfaceLostCallbackHandle DeviceManager::RegisterInterfaceLostCallback(
    const DeviceManager::InterfaceLostCallback& callback)
{
    return m_interfaceLostCallbackManager.RegisterCallback(callback);
}

inline void DeviceManager::UnregisterInterfaceLostCallback(DeviceManager::InterfaceLostCallbackHandle callbackHandle)
{
    m_interfaceLostCallbackManager.UnregisterCallback(callbackHandle);
}

inline DeviceManager::DeviceFoundCallbackHandle DeviceManager::RegisterDeviceFoundCallback(
    const DeviceManager::DeviceFoundCallback& callback)
{
    return m_deviceFoundCallbackManager.RegisterCallback(callback);
}

inline void DeviceManager::UnregisterDeviceFoundCallback(DeviceManager::DeviceFoundCallbackHandle callbackHandle)
{
    m_deviceFoundCallbackManager.UnregisterCallback(callbackHandle);
}

inline DeviceManager::DeviceLostCallbackHandle DeviceManager::RegisterDeviceLostCallback(
    const DeviceManager::DeviceLostCallback& callback)
{
    return m_deviceLostCallbackManager.RegisterCallback(callback);
}

inline void DeviceManager::UnregisterDeviceLostCallback(DeviceManager::DeviceLostCallbackHandle callbackHandle)
{
    m_deviceLostCallbackManager.UnregisterCallback(callbackHandle);
}

inline DeviceManager::DeviceDisconnectedCallbackHandle DeviceManager::RegisterDeviceDisconnectedCallback(
    const DeviceManager::DeviceDisconnectedCallback& callback)
{
    return m_deviceDisconnectedCallbackManager.RegisterCallback(callback);
}

inline void DeviceManager::UnregisterDeviceDisconnectedCallback(
    DeviceManager::DeviceDisconnectedCallbackHandle callbackHandle)
{
    m_deviceDisconnectedCallbackManager.UnregisterCallback(callbackHandle);
}

inline DeviceManager::DeviceReconnectedCallbackHandle DeviceManager::RegisterDeviceReconnectedCallback(
    const DeviceManager::DeviceReconnectedCallback& callback)
{
    return m_deviceReconnectedCallbackManager.RegisterCallback(callback);
}

inline void DeviceManager::UnregisterDeviceReconnectedCallback(
    DeviceManager::DeviceReconnectedCallbackHandle callbackHandle)
{
    m_deviceReconnectedCallbackManager.UnregisterCallback(callbackHandle);
}

inline DeviceManager::DeviceListChangedCallbackHandle DeviceManager::RegisterDeviceListChangedCallback(
    const DeviceManager::DeviceListChangedCallback& callback)
{
    return m_deviceListChangedCallbackManager.RegisterCallback(callback);
}

inline void DeviceManager::UnregisterDeviceListChangedCallback(
    DeviceManager::DeviceListChangedCallbackHandle callbackHandle)
{
    m_deviceListChangedCallbackManager.UnregisterCallback(callbackHandle);
}

inline void DeviceManager::GetLostDevicesFromLostInterfaces(
    const std::vector<std::string>& lostInterfaces, std::vector<std::string>& lostDevices)
{
    std::lock_guard<std::recursive_mutex> lock(m_devicesMutex);

    for (const auto& device : m_devices)
    {
        auto resultIt = std::find(
            std::begin(lostInterfaces), std::end(lostInterfaces), device->ParentInterface()->Key());
        if (resultIt != std::end(lostInterfaces))
        {
            lostDevices.emplace_back(device->Key());
        }
    }
}

inline void DeviceManager::TriggerSystemFoundCallbacks(const std::shared_ptr<const core::System>& foundSystem) const
{
    m_systemFoundCallbackManager.TriggerCallbacks(foundSystem);
}

inline void DeviceManager::TriggerInterfaceFoundCallbacks(
    const std::shared_ptr<const core::Interface>& foundInterface) const
{
    m_interfaceFoundCallbackManager.TriggerCallbacks(foundInterface);
}

inline void DeviceManager::TriggerInterfaceLostCallbacks(const std::string& lostInterfaceKey) const
{
    m_interfaceLostCallbackManager.TriggerCallbacks(lostInterfaceKey);
}

inline void DeviceManager::TriggerDeviceFoundCallbacks(const std::shared_ptr<core::DeviceDescriptor>& foundDevice) const
{
    m_deviceFoundCallbackManager.TriggerCallbacks(foundDevice);
}

inline void DeviceManager::TriggerDeviceLostCallbacks(const std::string& lostDeviceKey) const
{
    m_deviceLostCallbackManager.TriggerCallbacks(lostDeviceKey);
}

} /* namespace peak */
