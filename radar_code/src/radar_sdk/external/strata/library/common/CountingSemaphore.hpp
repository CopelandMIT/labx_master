#pragma once

#include <condition_variable>
#include <mutex>

/// \brief Counting semaphore with convenience functions
///
/// \details Provides a counting semaphore with some convenience functions, like:
///  test without blocking, wait with timeout, wait for value with timeout
class CountingSemaphore
{
public:
    CountingSemaphore() :
        _counter(0)
    {}

    /// \brief Signal (give) semaphore
    /// \details Increments semaphore value by 1
    void signal()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _counter++;
        _condition.notify_one();
    }

    /// \brief Wait for semaphore (take), blocking for indefinite time
    /// \details Decrements semaphore value by 1 upon return
    void wait()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while (0 == _counter)  // Handle spurious wake-ups.
        {
            _condition.wait(lock);
        }
        _counter--;
    }

    /// \brief Test semaphore (take), without blocking
    /// \return True if semaphore value not 0, false otherwise
    /// \details Decrements semaphore value by 1 upon return
    bool test()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_counter > 0)
        {
            _counter--;
            return true;
        }
        return false;
    }

    /// \brief Wait for semaphore (take) with a timeout
    /// \param timeOutMs Timeout in ms
    /// \return True if semaphore could be taken, false if timeout
    /// \details Decrements semaphore value by 1 upon success
    bool wait_for(uint32_t timeOutMs)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        auto checkFlag = [this] {
            return _counter > 0;
        };
        bool res = _condition.wait_for(lock, std::chrono::milliseconds(timeOutMs), checkFlag);
        _counter = (_counter > 0) ? _counter - 1 : 0;

        return res;
    }

    /// \brief Wait for specific semaphore value (take) with a timeout
    /// \param count Semaphore value to wait for
    /// \param timeOutMs Timeout in ms
    /// \return True if semaphore value reached count in timeOutMs, false if timed out
    /// \details Decrements semaphore value by count upon success
    bool wait_for(uint32_t count, uint32_t timeOutMs)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        auto checkFlag = [this, count] {
            return _counter >= count;
        };
        bool res = _condition.wait_for(lock, std::chrono::milliseconds(timeOutMs), checkFlag);
        _counter = (_counter >= count) ? _counter - count : _counter;

        return res;
    }

private:
    std::mutex _mutex;
    std::condition_variable _condition;
    uint32_t _counter = false;
};