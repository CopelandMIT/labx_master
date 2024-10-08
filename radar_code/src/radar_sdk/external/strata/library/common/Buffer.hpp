/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <algorithm>


namespace strata
{
    template <typename T>
    inline std::size_t size_aligned(std::size_t size)
    {
        return (size + sizeof(T) - 1) / sizeof(T);
    }

    template <typename T>
    inline T *new_aligned(std::size_t size)
    {
        return new T[size_aligned<T>(size)];
    }

    /**
     * @brief This class handles a buffer of any type. It takes care of allocation and release of the data
     */
    template <typename T>
    class buffer
    {
        using size_type      = std::size_t;
        using value_type     = T;
        using iterator       = T *;
        using const_iterator = T const *;

    public:
        buffer(const buffer &) = delete;
        buffer &operator=(const buffer &) = delete;

        buffer(buffer &&other) noexcept
        {
            m_size       = other.m_size;
            m_data       = other.m_data;
            other.m_size = 0;
            other.m_data = nullptr;
        }

        buffer &operator=(buffer &&other) noexcept
        {
            if (this != &other)
            {
                m_size       = other.m_size;
                m_data       = other.m_data;
                other.m_size = 0;
                other.m_data = nullptr;
            }
            return *this;
        }

        /**
         * @brief Construct an empty buffer with zero length
         */
        buffer() :
            m_size {0},
            m_data {nullptr}
        {}

        /**
         * @brief Construct a buffer with a specified length
         * @note The data will be uninitialized
         * @param size The number of elements to be allocated in the buffer
         */
        buffer(size_type size) :
            m_size {size},
            m_data {new T[size]}
        {}

        /**
         * @brief Destruct the buffer and free the allocated data
         */
        ~buffer()
        {
            if (m_data)
            {
                delete[] m_data;
            }
        }

        /**
         * @brief Reset the buffer to zero length. This frees any allocated resources.
         */
        void clear()
        {
            resize(0);
        }

        /**
         * @brief Resize the buffer to a specified size of elements.
         * @note If the size already fits, nothing will be done
         * @note If the size changes, all data will be uninitialized
         * @param size The number of elements to be allocated in the buffer (total size)
         */
        void resize(size_type size)
        {
            if (m_size == size)
            {
                return;
            }
            delete[] m_data;
            m_size = size;
            if (m_size)
            {
                m_data = new T[m_size];
            }
            else
            {
                m_data = nullptr;
            }
        }

        /**
         * @brief Resize the buffer to a specified size of elements.
         * @note If the size already fits, nothing will be done
         * @param size The number of elements to be allocated in the buffer (total size)
         * @param retainData Specifies if the data in the buffer shall be copied to the new buffer. In case the new buffer is smaller, only the fitting part will be copied.
         */
        inline void resize(size_type size, bool retainData)
        {
            if (m_size == size)
            {
                return;
            }
            T *previousData              = m_data;
            const size_type previousSize = m_size;
            m_size                       = size;
            if (m_size)
            {
                m_data = new T[m_size];
            }
            else
            {
                m_data = nullptr;
            }

            if (previousData)
            {
                //There was data before
                if (retainData && m_data)
                {
                    //We shall retain the data and we have a new buffer
                    const size_type copySize = previousSize < m_size ? previousSize : m_size;
                    std::copy(previousData, previousData + copySize, m_data);
                }
                delete[] previousData;
            }
        }

        /**
         * @brief Resize the buffer to a specified size of bytes.
         * @note If the size already fits, nothing will be done
         * @note Any new data will be uninitialized.
         * @param size The number of elements to be allocated in the buffer (resulting complete size)
         * @param retainData Specifies if the data in the buffer shall be copied to the new buffer. In case the new buffer is smaller, only the fitting part will be copied.
         */
        inline void resize_bytes(size_type size, bool retainData = false)
        {
            resize((size + sizeof(value_type) - 1) / sizeof(value_type), retainData);
        }

        /**
         * @brief Retrieve the size (number of elements) in the buffer
         * @return The size (element count)
         */
        inline size_type size() const
        {
            return m_size;
        }

        /**
         * @brief Retrieve the actual size in bytes of the buffer
         * @return The size in bytes
         */
        inline size_type byte_size() const
        {
            return m_size * sizeof(value_type);
        }

        /**
         * @brief Retrieve a pointer to the buffer's data
         * @return A pointer to the stored data
         */
        inline value_type *data()
        {
            return m_data;
        }

        /**
         * @brief Retrieve a const pointer to the buffer's data
         * @return A const pointer to the stored data
         */
        inline value_type const *data() const
        {
            return m_data;
        }

        /**
         * @brief Get one of the elements of the buffer
         * @param x The 0-based index to retrieve
         * @return The element at the specified position
         */
        inline value_type &operator[](size_type x) const
        {
            return m_data[x];
        }

        /**
         * @brief Retrieve an iterator pointing at the buffer's first element
         * @return The iterator
         */
        iterator begin()
        {
            return m_data;
        }

        /**
         * @brief Retrieve an iterator pointing at the buffer's end
         * @return The iterator
         */
        iterator end()
        {
            return m_data + m_size;
        }

        /**
         * @brief Retrieve a const iterator pointing at the buffers first element
         * @return The iterator
         */
        const_iterator begin() const
        {
            return m_data;
        }

        /**
         * @brief Retrieve a const iterator pointing at the buffer's end
         * @return The iterator
         */
        const_iterator end() const
        {
            return m_data + m_size;
        }

        /**
         * @brief Fill the buffer with a specified value. All bytes will have this same value then.
         * @note The size of the buffer won't be changed.
         */
        inline void fill(const value_type &val = 0)
        {
            std::fill(begin(), end(), val);
        }

        /**
         * @brief Set the buffer to specific data. It will be resized to exactly fit the provided data.
         * @param first Iterator pointing to the first value
         * @param last Iterator pointing to the end
         */
        template <typename InputIterator>
        inline void assign(InputIterator first, InputIterator last)
        {
            resize(last - first);
            std::copy(first, last, begin());
        }

        /**
         * @brief Resize the buffer and set all elements to one specific value
         * @param n The number of elements to be in the buffer
         * @param val The value to write to each element
         */
        inline void assign(size_type n, const value_type &val)
        {
            resize(n);
            std::fill(begin(), end(), val);
        }

    private:
        size_type m_size;
        T *m_data;
    };

    template <typename T>
    class buffer_aligned :
        public buffer<T>
    {
    public:
        buffer_aligned(size_t size) :
            buffer<T>(size_aligned<T>(size))
        {}
    };

}


// TODO: remove this after other projects have been adapted
namespace stdext
{
    using namespace strata;
}
