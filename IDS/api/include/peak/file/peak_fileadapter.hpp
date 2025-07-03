/*!
* \file    peak_fileadapter.hpp
*
* \author  IDS Imaging Development Systems GmbH
* \date    2024-09-25
* \since   1.10
*
* Copyright (c) 2024 - 2025, IDS Imaging Development Systems GmbH. All rights reserved.
*/

#pragma once


#include <peak/backend/peak_backend.h>
#include <peak/common/peak_common_structs.hpp>
#include <peak/dll_interface/peak_dll_interface_util.hpp>

#include <cstdint>
#include <vector>

namespace peak
{
namespace core
{
class NodeMap;

namespace file
{

/*!
 * \brief The FileAdapter is a utility class designed for accessing files stored in a
 *        RemoteDevice's persistent memory.
 *
 * It provides convenient functions to \ref Read, \ref Write, and \ref Delete files.
 * \snippet{trimleft} doc_file_adapter_test.cpp file_adapter_class
 *
 * \note
 * The FileAdapter relies on certain nodes within the RemoteDevice.
 * If these nodes are not present, the functions may not work properly.
 * Check your device's manual to confirm the availability of these nodes.
 * - FileSelector
 * - FileOperationSelector
 * - FileOperationExecute
 * - FileSize
 * - FileAccessBuffer
 * - FileAccessLength
 * - FileAccessOffset
 * - ...
 *
 * The FileAdapter class encapsulates the functions of the GenApi::FileProtocolAdapter.
 *
 */
class FileAdapter
{
public:
    /*!
     * \brief Constructs a FileAdapter for a specific file name in the RemoteDevice's NodeMap.
     *
     * \snippet{trimleft} doc_file_adapter_test.cpp file_adapter_constructor
     *
     * File names may vary depending on the device.
     * To obtain a list of valid file names, you can either iterate through the
     * \link nodes::EnumerationNode::AvailableEntries AvailableEntries\endlink of the
     * 'FileSelector' \link nodes::EnumerationNode EnumerationNode\endlink
     * or use the static FileAdapter::AvailableFileNames function.
     * Some files may be read-only.
     *
     * Common file names include:
     * * UserData1
     * * UserData2
     * * CriticalEventLog
     *
     * \param nodeMap The NodeMap of the RemoteDevice
     * \param fileName A valid file name
     *
     * \since 1.10
     *
     * \throws InvalidArgumentException The nodeMap pointer is invalid.
     * \throws NotAvailableException The 'FileSelector' node or it's entry with the given file name is not available.
     * \throws InternalErrorException An internal error has occurred.
     */
    FileAdapter(const std::shared_ptr<NodeMap>& nodeMap, const std::string& fileName);
    ~FileAdapter();

    FileAdapter(const FileAdapter& other) = delete;
    FileAdapter& operator=(const FileAdapter& other) = delete;
    FileAdapter(FileAdapter&& other) = delete;
    FileAdapter& operator=(FileAdapter&& other) = delete;

    /*!
     * \brief Returns the contents of the file that the adapter references.
     *
     * To read the entire file, use FileAdapter::Size as the size argument.
     * \snippet{trimleft} doc_file_adapter_test.cpp file_adapter_read
     *
     * Alternatively, you can specify an offset and a smaller size to read only a part of the file content.
     *
     * \param size The amount of data to be read
     * \param offset The position from the beginning of the file where data reading should start
     *
     * \return File content
     *
     * \since 1.10
     *
     * \throws OutOfRangeException Size + offset exceeds the file's size.
     * \throws InternalErrorException An internal error has occurred.
     */
    PEAK_NO_DISCARD std::vector<uint8_t> Read(uint64_t size, uint64_t offset = 0) const;

    /*!
     * \brief Reads the contents of the file into the supplied buffer
     *
     * To read the entire file, use FileAdapter::Size as the size argument.
     *
     * Alternatively, you can specify an offset and a smaller size to read only a part of the file content.
     *
     * \param buffer The buffer to write to. The buffer needs to be at least the size supplied in \p size.
     * \param size   The amount of data to be read
     * \param offset The position from the beginning of the file where data reading should start
     *
     * \return the number of bytes read
     *
     * \since 1.10
     *
     * \throws OutOfRangeException Size + offset exceeds the file's size.
     * \throws InternalErrorException An internal error has occurred.
     */
    uint64_t Read(uint8_t* buffer, uint64_t size, uint64_t offset = 0) const;

    /*!
     * \brief Writes the data to the file the adapter references.
     *
     * \snippet{trimleft} doc_file_adapter_test.cpp file_adapter_write
     *
     * Ensure that the data size does not exceed FileAdapter::MaxSize.
     *
     * \note
     * If you don't want to write from the start of the file, add an offset in the function call.
     * This might fail if the RemoteDevice's FileAccessOffset node cannot set the specified offset.
     *
     * \param data   The data to be written
     * \param offset The position from the beginning of the file where data writing should start
     *
     * \since 1.10
     *
     * \throws OutOfRangeException Data size + offset exceeds the file's MaxSize or offset cannot be set.
     * \throws InternalErrorException An internal error has occurred.
     */
    void Write(const std::vector<uint8_t>& data, uint64_t offset = 0) const;

    /*!
     * \brief Writes the data supplied in the buffer to the file the adapter references.
     *
     * Ensure that the data size does not exceed FileAdapter::MaxSize.
     *
     * \note
     * If you don't want to write from the start of the file, add an offset in the function call.
     * This might fail if the RemoteDevice's FileAccessOffset node cannot set the specified offset.
     *
     * \param buffer     The buffer containing the bytes to write to the file.
     * \param bufferSize The size of the buffer supplied in \p buffer
     * \param offset     The position from the beginning of the file where data writing should start
     *
     * \since 1.10
     *
     * \throws OutOfRangeException Data size + offset exceeds the file's MaxSize or offset cannot be set.
     * \throws InternalErrorException An internal error has occurred.
     */
    void Write(const uint8_t* buffer, uint64_t bufferSize, uint64_t offset = 0) const;

    /*!
     * \brief Deletes the file the adapter references.
     *
     * \since 1.10
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    void Delete() const;

   /*!
    * \brief Returns the content size of the file the adapter references.
    * This is the maximum size that can be read.
    *
    * \return The content size
    *
    * \since 1.10
    *
    * \throws InternalErrorException An internal error has occurred.
    */
    PEAK_NO_DISCARD uint64_t Size() const;

    /*!
     * \brief Returns the maximum size of the file the adapter references.
     * This is the maximum size that can be written.
     *
     * \return The maximum size
     *
     * \since 1.10
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    PEAK_NO_DISCARD uint64_t MaxSize() const;

    /*!
     * \brief Returns a vector of available file names from the given NodeMap.
     * File names may vary depending on the device.
     * \snippet{trimleft} doc_file_adapter_test.cpp file_adapter_list
     *
     * \param nodeMap The NodeMap of the RemoteDevice
     *
     * \return Vector of available file names
     *
     * \since 1.10
     *
     * \throws InvalidArgumentException The NodeMap pointer is invalid.
     * \throws NotAvailableException The 'FileSelector' node is not available.
     * \throws InternalErrorException An internal error has occurred.
     */
    PEAK_NO_DISCARD static std::vector<std::string> AvailableFileNames(const std::shared_ptr<NodeMap>& nodeMap);

private:
    PEAK_FILE_ADAPTER_HANDLE m_backendHandle{};
};

inline FileAdapter::FileAdapter(const std::shared_ptr<NodeMap>& nodeMap, const std::string& fileName)
{
    if (nodeMap == nullptr)
    {
        throw InvalidArgumentException("The given nodeMap is a nullptr!");
    }

    peak::core::ExecuteAndMapReturnCodes([&] {
        return PEAK_C_ABI_PREFIX PEAK_FileAdapter_Construct(nodeMap->Handle(), fileName.c_str(), &m_backendHandle);
    });
}

inline FileAdapter::~FileAdapter()
{
    (void)PEAK_C_ABI_PREFIX PEAK_FileAdapter_Destruct(m_backendHandle);
}

inline std::vector<uint8_t> FileAdapter::Read(uint64_t size, uint64_t offset) const
{
    std::vector<uint8_t> array(static_cast<size_t>(size));
    uint64_t readBytes{};

    peak::core::ExecuteAndMapReturnCodes([&] {
        return PEAK_C_ABI_PREFIX PEAK_FileAdapter_Read(m_backendHandle, size, offset, array.data(), &readBytes);
    });

    array.resize(static_cast<size_t>(readBytes));

    return array;
}

inline uint64_t FileAdapter::Read(uint8_t* buffer, uint64_t size, uint64_t offset) const
{
    uint64_t readBytes{};

    peak::core::ExecuteAndMapReturnCodes([&] {
        return PEAK_C_ABI_PREFIX PEAK_FileAdapter_Read(m_backendHandle, size, offset, buffer, &readBytes);
    });

    return readBytes;
}

inline void FileAdapter::Write(const std::vector<uint8_t>& data, uint64_t offset) const
{
    peak::core::ExecuteAndMapReturnCodes([&] {
        return PEAK_C_ABI_PREFIX PEAK_FileAdapter_Write(m_backendHandle, data.data(), data.size(), offset);
    });
}

inline void FileAdapter::Write(const uint8_t* buffer, uint64_t bufferSize, uint64_t offset) const
{
    peak::core::ExecuteAndMapReturnCodes([&] {
        return PEAK_C_ABI_PREFIX PEAK_FileAdapter_Write(m_backendHandle, buffer, bufferSize, offset);
    });
}

inline void FileAdapter::Delete() const
{
    peak::core::ExecuteAndMapReturnCodes([&] {
        return PEAK_C_ABI_PREFIX PEAK_FileAdapter_Delete(m_backendHandle);
    });
}

inline uint64_t FileAdapter::Size() const
{
    uint64_t size{};
    peak::core::ExecuteAndMapReturnCodes([&] {
        return PEAK_C_ABI_PREFIX PEAK_FileAdapter_GetSize(m_backendHandle, &size);
    });

    return size;
}

inline uint64_t FileAdapter::MaxSize() const
{
    uint64_t size{};
    peak::core::ExecuteAndMapReturnCodes([&] {
        return PEAK_C_ABI_PREFIX PEAK_FileAdapter_GetMaxSize(m_backendHandle, &size);
    });

    return size;
}

inline std::vector<std::string> FileAdapter::AvailableFileNames(const std::shared_ptr<NodeMap>& nodeMap)
{
    if (nodeMap == nullptr)
    {
        throw InvalidArgumentException("The given nodeMap is a nullptr!");
    }

    const auto fileSelectorNode = nodeMap->TryFindNode<nodes::EnumerationNode>("FileSelector");
    if (fileSelectorNode == nullptr)
    {
        throw NotAvailableException("EnumerationNode with the name 'FileSelector' not found.");
    }
    std::vector<std::string> availableFileNames{};
    const auto entries = fileSelectorNode->AvailableEntries();
    availableFileNames.reserve(entries.size());

    for (const auto& entry : entries)
    {
        availableFileNames.emplace_back(entry->SymbolicValue());
    }
    return availableFileNames;
}

} /* namespace file */
} /* namespace core */
} /* namespace peak */
