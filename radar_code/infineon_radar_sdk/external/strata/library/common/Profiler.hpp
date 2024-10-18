/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <chrono>


class StrataProfiler
{
public:
    using Unit = std::chrono::microseconds;

    StrataProfiler();

    /**
     * @brief Store the current time point as timing reference start
     */
    void restart();

    /**
     * @brief Store the current time point as timing reference finish
     */
    void finish();

    /**
     * @brief Return the time difference between timing reference start and finish.
     *        If finish() has not been called so far, it will be automatically called first.
     */
    std::chrono::steady_clock::rep getElapsed();

    /**
     * @brief Print to the log the time difference between timing reference start and finish.
     *        If finish() has not been called so far, it will be automatically called first.
     */
    void logElapsed();

private:
    std::chrono::steady_clock::time_point m_tic, m_toc;
};
