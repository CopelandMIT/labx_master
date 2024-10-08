/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "Logger.hpp"
#include "Time.hpp"


//#define WIN_FORCE_CONSOLE


#if defined(_WIN32) && defined(WIN_FORCE_CONSOLE)
    #include <windows.h>
#endif


Logger LoggerInstance(LoggerLevelDefault, LoggerFileName);


namespace
{
    std::chrono::steady_clock::time_point log_tic;
}


void tic()
{
    LOG(DEBUG) << "tic ***";
    log_tic = std::chrono::steady_clock::now();
}

void toc()
{
    const auto log_toc = getElapsedTime(log_tic);
    LOG(DEBUG) << "*** toc: duration = " << std::dec << log_toc << "us";
}

Logger::Logger(Logger::LogLevel level, const char *filename)
{
    setLevel(level);
#if defined(_WIN32) && defined(WIN_FORCE_CONSOLE)
    const HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!handle)
    {
        AllocConsole();
        FILE *stream;
        freopen_s(&stream, "CONOUT$", "w", stdout);
        freopen_s(&stream, "CONOUT$", "w", stderr);
    }
#endif

    setFile(filename);
}

Logger::~Logger()
{
    try
    {
        Logger::Line(this) << std::endl;
        if (m_outFile.is_open())
        {
            m_outFile.close();
        }
    }
    catch (...)
    {
    }
}

void Logger::setLevel(Logger::LogLevel level)
{
    m_logLevel = level;
}

void Logger::setFile(const char *filename)
{
    if (m_outFile.is_open())
    {
        m_outFile.close();
    }

    if (filename != nullptr)
    {
        if (filename[0])
        {
            m_outFile.open(filename, std::ios::out);
        }
    }
}

Logger::Line Logger::log(Logger::LogLevel level)
{
    if (level > m_logLevel)
    {
        return Logger::Line(nullptr);
    }

    std::time_t logtime = std::time(nullptr);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &logtime);
#else
    localtime_r(&logtime, &tm);
#endif

    char timestamp[32];
    const auto s = std::strftime(timestamp, sizeof(timestamp), LoggerDateTimeFormat, &tm);

    Logger::Line line(this);
    try
    {
        if (s)
        {
            line << timestamp;
        }
        switch (level)
        {
            case LOG_INFO:
                line << "INFO: ";
                break;
            case LOG_DEBUG:
                line << "DEBUG: ";
                break;
            case LOG_WARN:
                line << "WARN: ";
                break;
            case LOG_ERROR:
                line << "ERROR: ";
                break;
            default:
                break;
        }
    }
    catch (...)
    {
    }

    return line;
}

Logger::Line::Line(Logger *logger) :
    m_logger {logger}
{
    if (m_logger)
    {
        m_logger->m_lineLock.lock();
    }
}

Logger::Line::Line(Line &&ref) :
    m_logger {ref.m_logger}
{
    ref.m_logger = nullptr;
}

Logger::Line::~Line()
{
    if (m_logger)
    {
        try
        {
            *this << "\n";
        }
        catch (...)
        {
        }
        m_logger->m_lineLock.unlock();
    }
}

Logger::Line &Logger::Line::operator<<(EndLine)
{
    if (m_logger)
    {
        std::cout << std::endl;
        if (m_logger->m_outFile.is_open())
        {
            m_logger->m_outFile << std::endl;
        }
    }
    return *this;
}
