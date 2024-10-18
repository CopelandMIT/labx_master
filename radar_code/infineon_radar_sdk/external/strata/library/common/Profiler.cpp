/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "Profiler.hpp"
#include "Logger.hpp"


StrataProfiler::StrataProfiler() :
    m_toc {}
{
    restart();
}

void StrataProfiler::restart()
{
    m_tic = std::chrono::steady_clock::now();
}

void StrataProfiler::finish()
{
    m_toc = std::chrono::steady_clock::now();
}

std::chrono::steady_clock::rep StrataProfiler::getElapsed()
{
    if (m_toc == decltype(m_toc) {})
    {
        finish();
    }

    return std::chrono::duration_cast<Unit>(m_toc - m_tic).count();
}

void StrataProfiler::logElapsed()
{
    const auto toc = getElapsed();
    LOG(DEBUG) << "Profiled duration = " << std::dec << toc << "us";
}
