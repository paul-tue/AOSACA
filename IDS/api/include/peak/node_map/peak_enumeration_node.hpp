/*!
 * \file    peak_enumeration_node.hpp
 *
 * \author  IDS Imaging Development Systems GmbH
 * \date    2019-05-01
 * \since   1.0
 *
 * Copyright (c) 2019 - 2025, IDS Imaging Development Systems GmbH. All rights reserved.
 */

#pragma once


#include <peak/backend/peak_backend.h>
#include <peak/dll_interface/peak_dll_interface_util.hpp>
#include <peak/node_map/peak_enumeration_entry_node.hpp>
#include <peak/node_map/peak_node.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>


namespace peak
{
namespace core
{
namespace nodes
{

/*!
 * \brief Represents a GenAPI enumeration node.
 *
 */
class EnumerationNode : public Node
{
public:
    EnumerationNode() = delete;
    ~EnumerationNode() override = default;
    EnumerationNode(const EnumerationNode& other) = delete;
    EnumerationNode& operator=(const EnumerationNode& other) = delete;
    EnumerationNode(EnumerationNode&& other) = delete;
    EnumerationNode& operator=(EnumerationNode&& other) = delete;

    /*!
     * \brief Returns the current entry.
     *
     * \param[in] cacheUsePolicy A flag telling whether the value should be read using the internal cache.
     *
     * \return Current entry
     *
     * \since 1.0
     *
     * \throws IOException A communication error has occured, e.g. when the device is disconnected.
     * \throws InternalErrorException An internal error has occurred.
     */
    std::shared_ptr<EnumerationEntryNode> CurrentEntry(
        NodeCacheUsePolicy cacheUsePolicy = NodeCacheUsePolicy::UseCache) const;

    /*!
     * \brief Sets the current entry.
     *
     * \param[in] entry The entry to set as current entry.
     *
     * \since 1.0
     *
     * \throws InvalidArgumentException There is no matching entry in this enumeration node.
     * \throws IOException A communication error has occured, e.g. when the device is disconnected.
     * \throws InternalErrorException An internal error has occurred.
     */
    void SetCurrentEntry(const std::shared_ptr<EnumerationEntryNode>& entry);

    /*!
     * \brief Sets the current entry to an entry with the given symbolic value.
     *
     * \param[in] symbolicValue The symbolic value of the entry to set.
     *
     * \since 1.0
     *
     * \throws InvalidArgumentException There is no entry with this symbolicValue in this enumeration node.
     * \throws IOException A communication error has occured, e.g. when the device is disconnected.
     * \throws InternalErrorException An internal error has occurred.
     */
    void SetCurrentEntry(const std::string& symbolicValue);

    /*!
     * \brief Sets the current entry to an entry with the given value.
     *
     * \param[in] value The value of the entry to set.
     *
     * \since 1.0
     *
     * \throws InvalidArgumentException There is no entry with this value in this enumeration node.
     * \throws IOException A communication error has occured, e.g. when the device is disconnected.
     * \throws InternalErrorException An internal error has occurred.
     */
    void SetCurrentEntry(int64_t value);

    /*!
     * \brief Checks whether the enumeration node contains an entry with the given symbolic value.
     *
     * \param[in] symbolicValue The symbolic value of the entry to find.
     *
     * \since 1.10.0
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    bool HasEntry(const std::string& symbolicValue) const;

    /*!
     * \brief Checks whether the enumeration node contains an entry with the given value.
     *
     * \param[in] value The value of the entry to find.
     *
     * \since 1.10.0
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    bool HasEntry(int64_t value) const;

    /*!
     * \brief Finds an entry with the given symbolic value.
     *
     * \param[in] symbolicValue The symbolic value of the entry to find.
     *
     * \return Found entry.
     * \return If no entry with the given name exists, a NotFoundException is thrown.
     *
     * \since 1.0
     *
     * \throws NotFoundException There is no entry with the given name.
     * \throws InternalErrorException An internal error has occurred.
     */
    std::shared_ptr<EnumerationEntryNode> FindEntry(const std::string& symbolicValue) const;

    /*!
     * \brief Finds an entry with the given numeric value.
     *
     * \param[in] value The value of the entry to find.
     *
     * \return Found entry.
     * \return If no entry with the given numeric value, a NotFoundException is thrown.
     *
     * \since 1.0
     *
     * \throws NotFoundException There is no entry with the given numeric value.
     * \throws InternalErrorException An internal error has occurred.
     */
    std::shared_ptr<EnumerationEntryNode> FindEntry(int64_t value) const;

    /*!
     * \brief Tries to find an entry with the given symbolic value.
     *
     * \param[in] symbolicValue The symbolic value of the entry to find.
     *
     * \return Found entry or nullptr if no entry with the given name exists.
     *
     * \since 1.10.0
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    std::shared_ptr<EnumerationEntryNode> TryFindEntry(const std::string& symbolicValue) const;

    /*!
     * \brief Tries to find a entry with the given numeric value.
     *
     * \param[in] value The value of the entry to find.
     *
     * \return Found entry or nullptr if no entry with the given name exists.
     *
     * \since 1.10.0
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    std::shared_ptr<EnumerationEntryNode> TryFindEntry(const int64_t value) const;

    /*!
     * \brief Returns the entries.
     *
     * \return Entries
     *
     * \since 1.0
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    std::vector<std::shared_ptr<EnumerationEntryNode>> Entries() const;

    /*!
     * \brief Returns the currently selectable entries.
     *
     * \return Selectable entries
     *
     * \since 1.10.0
     *
     * \throws InternalErrorException An internal error has occurred.
     */
    std::vector<std::shared_ptr<EnumerationEntryNode>> AvailableEntries() const;

private:
    friend ClassCreator<EnumerationNode>;
    EnumerationNode(PEAK_ENUMERATION_NODE_HANDLE enumerationNodeHandle, const std::weak_ptr<NodeMap>& parentNodeMap);
    PEAK_ENUMERATION_NODE_HANDLE m_backendHandle;
};

} /* namespace nodes */
} /* namespace core */
} /* namespace peak */

/* Implementation */
namespace peak
{
namespace core
{
namespace nodes
{

inline EnumerationNode::EnumerationNode(
    PEAK_ENUMERATION_NODE_HANDLE enumerationNodeHandle, const std::weak_ptr<NodeMap>& parentNodeMap)
    : Node(QueryNumericFromCInterfaceFunction<PEAK_NODE_HANDLE>([&](PEAK_NODE_HANDLE* nodeHandle) {
        return PEAK_C_ABI_PREFIX PEAK_EnumerationNode_ToNode(enumerationNodeHandle, nodeHandle);
    }),
        parentNodeMap)
    , m_backendHandle(enumerationNodeHandle)
{}

inline std::shared_ptr<EnumerationEntryNode> EnumerationNode::CurrentEntry(
    NodeCacheUsePolicy cacheUsePolicy /* = NodeCacheUsePolicy::UseCache */) const
{
    auto enumerationEntryNodeHandle = QueryNumericFromCInterfaceFunction<PEAK_ENUMERATION_ENTRY_NODE_HANDLE>(
        [&](PEAK_ENUMERATION_ENTRY_NODE_HANDLE* _enumerationEntryNodeHandle) {
            return PEAK_C_ABI_PREFIX PEAK_EnumerationNode_GetCurrentEntry(m_backendHandle,
                static_cast<PEAK_NODE_CACHE_USE_POLICY>(cacheUsePolicy), _enumerationEntryNodeHandle);
        });

    auto nodeHandle = QueryNumericFromCInterfaceFunction<PEAK_NODE_HANDLE>([&](PEAK_NODE_HANDLE* _nodeHandle) {
        return PEAK_C_ABI_PREFIX PEAK_EnumerationEntryNode_ToNode(enumerationEntryNodeHandle, _nodeHandle);
    });

    auto nodeName = GetNodeName(nodeHandle);

    return std::dynamic_pointer_cast<EnumerationEntryNode>(ParentNodeMap()->FindNode(nodeName));
}

inline void EnumerationNode::SetCurrentEntry(const std::shared_ptr<EnumerationEntryNode>& entry)
{
    if (entry == nullptr)
    {
        throw InvalidArgumentException("The given entry is a nullptr!");
    }

    CallAndCheckCInterfaceFunction([&] {
        return PEAK_C_ABI_PREFIX PEAK_EnumerationNode_SetCurrentEntry(m_backendHandle, entry->m_backendHandle);
    });
}

inline void EnumerationNode::SetCurrentEntry(const std::string& symbolicValue)
{
    CallAndCheckCInterfaceFunction([&] {
        return PEAK_C_ABI_PREFIX PEAK_EnumerationNode_SetCurrentEntryBySymbolicValue(
            m_backendHandle, symbolicValue.c_str(), symbolicValue.size() + 1);
    });
}

inline void EnumerationNode::SetCurrentEntry(int64_t value)
{
    CallAndCheckCInterfaceFunction(
        [&] { return PEAK_C_ABI_PREFIX PEAK_EnumerationNode_SetCurrentEntryByValue(m_backendHandle, value); });
}

inline bool EnumerationNode::HasEntry(const std::string& symbolicValue) const
{
    return QueryNumericFromCInterfaceFunction<PEAK_BOOL8>([&](PEAK_BOOL8* hasEntry) {
        return PEAK_C_ABI_PREFIX PEAK_EnumerationNode_GetHasEntryBySymbolicValue(
            m_backendHandle, symbolicValue.c_str(), symbolicValue.size() + 1, hasEntry);
    }) > 0;
}

inline bool EnumerationNode::HasEntry(int64_t value) const
{
    return QueryNumericFromCInterfaceFunction<PEAK_BOOL8>([&](PEAK_BOOL8* hasEntry) {
        return PEAK_C_ABI_PREFIX PEAK_EnumerationNode_GetHasEntryByValue(m_backendHandle, value, hasEntry);
    }) > 0;
}

inline std::shared_ptr<EnumerationEntryNode> EnumerationNode::FindEntry(const std::string& symbolicValue) const
{
    auto enumerationEntryNodeHandle = QueryNumericFromCInterfaceFunction<PEAK_ENUMERATION_ENTRY_NODE_HANDLE>(
        [&](PEAK_ENUMERATION_ENTRY_NODE_HANDLE* _enumerationEntryNodeHandle) {
            return PEAK_C_ABI_PREFIX PEAK_EnumerationNode_FindEntryBySymbolicValue(
                m_backendHandle, symbolicValue.c_str(), symbolicValue.size() + 1, _enumerationEntryNodeHandle);
        });

    auto nodeHandle = QueryNumericFromCInterfaceFunction<PEAK_NODE_HANDLE>([&](PEAK_NODE_HANDLE* _nodeHandle) {
        return PEAK_C_ABI_PREFIX PEAK_EnumerationEntryNode_ToNode(enumerationEntryNodeHandle, _nodeHandle);
    });

    auto nodeName = GetNodeName(nodeHandle);

    return std::dynamic_pointer_cast<EnumerationEntryNode>(ParentNodeMap()->FindNode(nodeName));
}

inline std::shared_ptr<EnumerationEntryNode> EnumerationNode::FindEntry(int64_t value) const
{
    auto enumerationEntryNodeHandle = QueryNumericFromCInterfaceFunction<PEAK_ENUMERATION_ENTRY_NODE_HANDLE>(
        [&](PEAK_ENUMERATION_ENTRY_NODE_HANDLE* _enumerationEntryNodeHandle) {
            return PEAK_C_ABI_PREFIX PEAK_EnumerationNode_FindEntryByValue(
                m_backendHandle, value, _enumerationEntryNodeHandle);
        });

    auto nodeHandle = QueryNumericFromCInterfaceFunction<PEAK_NODE_HANDLE>([&](PEAK_NODE_HANDLE* _nodeHandle) {
        return PEAK_C_ABI_PREFIX PEAK_EnumerationEntryNode_ToNode(enumerationEntryNodeHandle, _nodeHandle);
    });

    auto nodeName = GetNodeName(nodeHandle);

    return std::dynamic_pointer_cast<EnumerationEntryNode>(ParentNodeMap()->FindNode(nodeName));
}


inline std::shared_ptr<EnumerationEntryNode> EnumerationNode::TryFindEntry(const std::string& symbolicValue) const
{
    auto enumerationEntryNodeHandle = QueryNumericFromCInterfaceFunction<PEAK_ENUMERATION_ENTRY_NODE_HANDLE>(
        [&](PEAK_ENUMERATION_ENTRY_NODE_HANDLE* _enumerationEntryNodeHandle) {
            return PEAK_C_ABI_PREFIX PEAK_EnumerationNode_TryFindEntryBySymbolicValue(
                m_backendHandle, symbolicValue.c_str(), symbolicValue.size() + 1, _enumerationEntryNodeHandle);
        });

    if(enumerationEntryNodeHandle == nullptr)
    {
        return nullptr;
    }

    auto nodeHandle = QueryNumericFromCInterfaceFunction<PEAK_NODE_HANDLE>([&](PEAK_NODE_HANDLE* _nodeHandle) {
        return PEAK_C_ABI_PREFIX PEAK_EnumerationEntryNode_ToNode(enumerationEntryNodeHandle, _nodeHandle);
    });

    auto nodeName = GetNodeName(nodeHandle);

    return std::dynamic_pointer_cast<EnumerationEntryNode>(ParentNodeMap()->FindNode(nodeName));
}

inline std::shared_ptr<EnumerationEntryNode> EnumerationNode::TryFindEntry(const int64_t value) const
{
    auto enumerationEntryNodeHandle = QueryNumericFromCInterfaceFunction<PEAK_ENUMERATION_ENTRY_NODE_HANDLE>(
        [&](PEAK_ENUMERATION_ENTRY_NODE_HANDLE* _enumerationEntryNodeHandle) {
            return PEAK_C_ABI_PREFIX PEAK_EnumerationNode_TryFindEntryByValue(
                m_backendHandle, value, _enumerationEntryNodeHandle);
        });

    if(enumerationEntryNodeHandle == nullptr)
    {
        return nullptr;
    }

    auto nodeHandle = QueryNumericFromCInterfaceFunction<PEAK_NODE_HANDLE>([&](PEAK_NODE_HANDLE* _nodeHandle) {
        return PEAK_C_ABI_PREFIX PEAK_EnumerationEntryNode_ToNode(enumerationEntryNodeHandle, _nodeHandle);
    });

    auto nodeName = GetNodeName(nodeHandle);

    return std::dynamic_pointer_cast<EnumerationEntryNode>(ParentNodeMap()->FindNode(nodeName));
}

inline std::vector<std::shared_ptr<EnumerationEntryNode>> EnumerationNode::Entries() const
{
    auto numEntries = QueryNumericFromCInterfaceFunction<size_t>([&](size_t* _numEntries) {
        return PEAK_C_ABI_PREFIX PEAK_EnumerationNode_GetNumEntries(m_backendHandle, _numEntries);
    });

    std::vector<std::shared_ptr<EnumerationEntryNode>> entries;
    for (size_t x = 0; x < numEntries; ++x)
    {
        auto enumerationEntryNodeHandle = QueryNumericFromCInterfaceFunction<PEAK_ENUMERATION_ENTRY_NODE_HANDLE>(
            [&](PEAK_ENUMERATION_ENTRY_NODE_HANDLE* _enumerationEntryNodeHandle) {
                return PEAK_C_ABI_PREFIX PEAK_EnumerationNode_GetEntry(
                    m_backendHandle, x, _enumerationEntryNodeHandle);
            });

        auto nodeHandle = QueryNumericFromCInterfaceFunction<PEAK_NODE_HANDLE>(
            [&](PEAK_NODE_HANDLE* _nodeHandle) {
                return PEAK_C_ABI_PREFIX PEAK_EnumerationEntryNode_ToNode(
                    enumerationEntryNodeHandle, _nodeHandle);
            });

        auto nodeName = GetNodeName(nodeHandle);

        entries.emplace_back(std::dynamic_pointer_cast<EnumerationEntryNode>(ParentNodeMap()->FindNode(nodeName)));
    }

    return entries;
}

inline std::vector<std::shared_ptr<EnumerationEntryNode>> EnumerationNode::AvailableEntries() const
{
    const auto& entries = Entries();
    std::vector<std::shared_ptr<EnumerationEntryNode>> availableEntries{};
    std::copy_if(entries.begin(), entries.end(), std::back_inserter(availableEntries),
        [](const std::shared_ptr<EnumerationEntryNode>& entry) { return entry->IsAvailable(); });
    return availableEntries;
}

} /* namespace nodes */
} /* namespace core */
} /* namespace peak */
