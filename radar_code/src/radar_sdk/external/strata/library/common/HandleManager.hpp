/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <map>
#include <memory>
#include <set>
#include <type_traits>
#include <typeindex>
#include <utility>

#include <components/interfaces/IComponent.hpp>
#include <modules/interfaces/IModule.hpp>


using DefaultIdentifierType = int;


template <typename ClassType, typename IdentifierType = DefaultIdentifierType>
class HandleManager :
    public std::map<IdentifierType, std::unique_ptr<ClassType>>
{
public:
    HandleManager() :
        m_lastIdentifier {m_invalidIdentifier}
    {
    }

    /**
     * Add an instance to the list and return its handle
     * @param instance The instance to be managed
     *
     * @return The handle for the instance
     */
    inline IdentifierType addInstance(std::unique_ptr<ClassType> &&instance)
    {
        if (instance)
        {
            const auto handle = ++m_lastIdentifier;
            this->emplace(handle, std::move(instance));
            return handle;
        }
        else
        {
            return m_invalidIdentifier;
        }
    }

    /**
     * Get the pointer to an instance in the list
     * @param handle The obtained handle
     *
     * @return Pointer to instance
     * @throws out of range exception if handle not found
     */
    inline ClassType *getInstance(IdentifierType handle)
    {
        return at(handle).get();
    }

    /**
     * Destroy an instance and remove it from the list
     * @param handle The obtained handle
     */
    inline void deleteInstance(IdentifierType handle)
    {
        this->erase(handle);
    }

protected:
    static constexpr IdentifierType m_invalidIdentifier = 0;
    IdentifierType m_lastIdentifier;
};


template <typename ClassType, typename AssociatedType, typename IdentifierType = DefaultIdentifierType>
class AssociatedHandleManager :
    public HandleManager<std::pair<std::unique_ptr<ClassType>, std::unique_ptr<AssociatedType>>, IdentifierType>
{
public:
    IdentifierType addInstance(std::pair<std::unique_ptr<ClassType>, std::unique_ptr<AssociatedType>> &&) = delete;

    /**
     * Add an instance and a secondary (possibly dependent) instance to the list and return its handle
     * @param instance The instance to be managed
     * @param associatedInstance The secondary instance to be managed jointly
     *
     * @return The handle for the instance
     */
    inline IdentifierType addInstance(std::unique_ptr<ClassType> &&instance, std::unique_ptr<AssociatedType> &&associatedInstance)
    {
        return addInstance(std::make_pair(std::move(instance), std::move(associatedInstance)));
    }

    /**
     * Get the pointer to an instance in the list
     * @param handle The obtained handle
     *
     * @return Pointer to instance
     * @throws out of range exception if handle not found
     */
    inline ClassType *getInstance(IdentifierType handle)
    {
        return at(handle)->first.get();
    }

    /**
     * Get the pointer to the associated instance in the list
     * @param handle The obtained handle
     *
     * @return Pointer to associated instance
     * @throws out of range exception if handle not found
     */
    inline ClassType *getAssociatedInstance(IdentifierType handle)
    {
        return at(handle)->second.get();
    }
};


using IdentifierType = uintptr_t;

template <typename ClassType>
class HandleManagerPtr :
    public HandleManager<ClassType, IdentifierType>
{
public:
    /**
     * Add an instance to the list and return its handle
     * @param instance The instance to be managed
     *
     * @return The handle for the instance
     */
    inline IdentifierType addInstance(std::unique_ptr<ClassType> &&instance)
    {
        if (instance)
        {
            const auto handle = reinterpret_cast<IdentifierType>(instance.get());
            this->emplace(handle, std::move(instance));
            return handle;
        }
        else
        {
            return HandleManager<ClassType, IdentifierType>::m_invalidIdentifier;
        }
    }

    /**
     * Get the pointer to an instance in the list.
     * It is not checked whether the handle's instance has been deleted already,
     * so it is the callers responsibility to ensure that the pointer is not used in that case.
     *
     * @param handle The obtained handle
     *
     * @return Pointer to instance
     */
    inline ClassType *getInstance(IdentifierType handle)
    {
        return reinterpret_cast<ClassType *>(handle);
    }
};

///
/// \brief The AssociatedHandleTracker class tracks the lifetime of handles.
/// \details When the owner of a handle is removed then all its descendants become invalidated.
///          Thus you can use this class to make sure that only handles are accessed that are still valid.
///
class AssociatedHandleTracker
{
public:
    AssociatedHandleTracker()
    {
    }

    ///
    /// \brief get the handle for a pointer derived from IComponent
    /// \return the handle for the given instance
    ///
    template <typename T, typename Type = typename std::remove_pointer<T>::type>
    inline typename std::enable_if<std::is_base_of<IComponent, Type>::value, uintptr_t>::type
    getInstanceHandle(T instance)
    {
        IComponent *ptr = instance;
        return reinterpret_cast<uintptr_t>(ptr);
    }

    ///
    /// \brief get the handle for a pointer derived from IModule
    /// \return the handle for the given instance
    ///
    template <typename T, typename Type = typename std::remove_pointer<T>::type>
    inline typename std::enable_if<std::is_base_of<IModule, Type>::value, uintptr_t>::type
    getInstanceHandle(T instance)
    {
        IModule *ptr = instance;
        return reinterpret_cast<uintptr_t>(ptr);
    }

    ///
    /// \brief get the handle for a pointer to a generic class
    /// \return the handle for the given instance
    ///
    template <typename T, typename Type = typename std::remove_pointer<T>::type>
    inline typename std::enable_if<!std::is_base_of<IModule, Type>::value && !std::is_base_of<IComponent, Type>::value, uintptr_t>::type
    getInstanceHandle(T instance)
    {
        return reinterpret_cast<uintptr_t>(instance);
    }

    inline uintptr_t getInstanceHandle(std::nullptr_t)
    {
        return 0;
    }

    ///
    /// \brief get the type index to use for a given type
    /// \return the instance for the given handle
    /// \retval nullptr when the handle does not exist or you specified the wrong type
    ///
    template <typename T>
    inline std::type_index getTypeIndex()
    {
        if (std::is_base_of<IComponent, T>::value)
        {
            return typeid(IComponent);
        }
        else if (std::is_base_of<IModule, T>::value)
        {
            return typeid(IModule);
        }

        return typeid(T);
    }

    ///
    /// \brief Track instance and return a handle for it.
    /// \details instance will be marked to be owned by the top-most ascendent of parent.
    ///          If there is no ascendent of parent then parent will be the owner of instance.
    ///          If the owner of the instance was removed, see removeInstance(),
    ///          then the handle to instance is invalidated.
    /// \param instance to track
    /// \param parent The parent of instance, can be nullptr if instance is free-standing.
    /// \return the handle to instance, which you can use in getInstance() and removeInstance()
    /// \retval 0 when adding the instance failed. E.g. when instance is nullptr,
    /// 		when instance was added before, or when the parent was not added itself before.
    /// \note The parent needs to be tracked before adding any children.
    ///
    template <typename T, typename P>
    inline uintptr_t addInstance(T *instance, P parent)
    {
        return addInstanceInternal(getInstanceHandle(instance), getInstanceHandle(parent), getTypeIndex<T>());
    }

    ///
    /// \brief get the instance pointer for the given handle and handleType of a certain interface type
    /// \return the instance for the given handle
    /// \retval nullptr when the handle does not exist or you specified the wrong type
    ///
    template <typename I, typename T>
    inline T *getInstancePointerFromInterface(uintptr_t handle, std::type_index handleType)
    {
        if (handleType != std::type_index(typeid(I)))
            return nullptr;

        auto ptr = reinterpret_cast<I *>(handle);
        return dynamic_cast<T *>(ptr);
    }

    ///
    /// \brief get the instance pointer for the given handle and handleType of an IComponent
    /// \return the instance for the given handle
    /// \retval nullptr when the handle does not exist or you specified the wrong type
    ///
    template <typename T>
    inline typename std::enable_if<std::is_base_of<IComponent, T>::value, T *>::type
    getInstancePointer(uintptr_t handle, std::type_index handleType)
    {
        return getInstancePointerFromInterface<IComponent, T>(handle, handleType);
    }

    ///
    /// \brief get the instance pointer for the given handle and handleType of an IModule
    /// \return the instance for the given handle
    /// \retval nullptr when the handle does not exist or you specified the wrong type
    ///
    template <typename T>
    inline typename std::enable_if<std::is_base_of<IModule, T>::value, T *>::type
    getInstancePointer(uintptr_t handle, std::type_index handleType)
    {
        return getInstancePointerFromInterface<IModule, T>(handle, handleType);
    }

    ///
    /// \brief get the instance pointer for the given handle and handleType of a generic type
    /// \return the instance for the given handle
    /// \retval nullptr when the handle does not exist or you specified the wrong type
    ///
    template <typename T>
    inline typename std::enable_if<!std::is_base_of<IModule, T>::value && !std::is_base_of<IComponent, T>::value, T *>::type
    getInstancePointer(uintptr_t handle, std::type_index handleType)
    {
        if (handleType != std::type_index(typeid(T)))
            return nullptr;

        return reinterpret_cast<T *>(handle);
    }

    ///
    /// \brief get the instance for the given handle
    /// \return the instance for the given handle
    /// \retval nullptr when the handle does not exist or you specified the wrong type
    ///
    template <typename T>
    inline T *getInstance(uintptr_t handle)
    {
        auto it = m_handles.find(handle);
        if (it == m_handles.end())
            return nullptr;

        return getInstancePointer<T>(handle, it->second.handleType);
    }

    ///
    /// \brief remove the instance and all the handles it owns
    /// \details The handle instance becomes invalidated as well.
    /// \retval true on success
    /// \retval false if instance was never added before via addInstance()
    ///
    template <typename T>
    inline bool removeInstance(T *instance)
    {
        return removeInstance(reinterpret_cast<uintptr_t>(instance));
    }

    ///
    /// \brief remove the handle and all the handles it owns
    /// \retval true on success
    /// \retval false if handle is not known
    ///
    inline bool removeInstance(uintptr_t handle)
    {
        auto it = m_handles.find(handle);
        if (it == m_handles.end())
            return false;

        // Remove all handles owned by handle.
        // Do this by recursively calling this function but avoiding keeping references.
        std::set<uintptr_t> owned(std::move(it->second.owned));
        m_handles.erase(it);
        for (uintptr_t h : owned)
            removeInstance(h);

        return true;
    }

private:
    struct HandleInfo
    {
        uintptr_t owner;
        std::type_index handleType;
        std::set<uintptr_t> owned;

    public:
        friend bool operator==(const HandleInfo &lhs, const HandleInfo &rhs)
        {
            return lhs.owner == rhs.owner && lhs.handleType == rhs.handleType && lhs.owned == rhs.owned;
        }
    };

    inline uintptr_t addInstanceInternal(uintptr_t handle, uintptr_t parent, std::type_index handleType)
    {
        if (!handle)
            return 0U;

        if (!parent)
        {
            // the handle is free-standing
            const bool worked = m_handles.insert({handle, {0U, handleType, {}}}).second;
            return worked ? handle : 0U;
        }
        else
        {
            auto itOwner = determineOwner(parent);
            if (itOwner == m_handles.end())
                return 0U;

            const HandleInfo newHandleInfo {itOwner->first, handleType, {}};
            if (m_handles.insert({handle, newHandleInfo}).second)
            {
                // If the insertion of the new handle worked, add it to its parent and on success
                // return the new handle
                const bool worked = itOwner->second.owned.insert(handle).second;
                return worked ? handle : 0U;
            }
            // If the insertion of the new handle didn't work, chances are that the handle already exists.
            // In that case check if the new HandleInfo data is the same as the existing one and if the
            // new handle is owned by its parent.
            const bool handleInfoGood = newHandleInfo == m_handles.at(handle);
            const bool isOwned        = itOwner->second.owned.find(handle) != itOwner->second.owned.end();

            return handleInfoGood && isOwned ? handle : 0U;
        }
    }

    inline std::map<uintptr_t, HandleInfo>::iterator determineOwner(uintptr_t parent)
    {
        auto it = m_handles.find(parent);
        if (it != m_handles.end() && it->second.owner)
            return determineOwner(it->second.owner);

        return it;
    }

private:
    std::map<uintptr_t, HandleInfo> m_handles;
};
