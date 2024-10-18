/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>

const char LoggerDateTimeFormat[] = "[%Y-%m-%d %H:%M:%S] ";
const char LoggerFileName[]       = "";


#define LOG(X)           LoggerInstance.log(Logger::LOG_##X)
#define LOG_LEVEL(X)     LoggerInstance.setLevel(Logger::LOG_##X)
#define LOG_FILE_OPEN(F) LoggerInstance.setFile(F)
#define LOG_FILE_CLOSE() LoggerInstance.setFile(nullptr)
#define LOG_BUFFER(X, buf, count)                                                                    \
    {                                                                                                \
        auto L = LoggerInstance.log(Logger::LOG_##X);                                                \
        L << "buffer \"" << #buf << "\"";                                                            \
        for (unsigned int i = 0; i < (count); i++)                                                   \
        {                                                                                            \
            L << " " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((buf)[i]); \
            if (!((i + 1) % 16))                                                                     \
            {                                                                                        \
                L << std::endl;                                                                      \
            }                                                                                        \
        }                                                                                            \
    }


/**
 * @brief Store the current time point as a global reference
 */
void tic();

/**
 * @brief Print to the log the time difference between making this call and the last call to tic()
 */
void toc();


class Logger
{
    class Line
    {
    public:
        Line(Logger *logger);
        Line(Line &&ref);
        ~Line();

        Line()       = delete;
        Line(Line &) = delete;

        Line &operator=(const Line &) = delete;
        Line &operator=(Line &&) = delete;

        template <typename T>
        Line &operator<<(const T &t);
        template <typename T>
        Line &write(const T *t, std::size_t count);

        // this is the signature for the std::endl function
        using EndLine = std::basic_ostream<char, std::char_traits<char>> &(*)(std::basic_ostream<char, std::char_traits<char>> &);

        // operator<< to take in std::endl
        Line &operator<<(EndLine);

    private:
        Logger *m_logger;
    };

public:
    enum LogLevel : uint8_t
    {
        LOG_NONE  = 0,
        LOG_ERROR = 1,
        LOG_WARN  = 2,
        LOG_INFO  = 3,
        LOG_DEBUG = 4
    };

    Logger(LogLevel level, const char *filename = nullptr);
    ~Logger();

    Logger()               = delete;
    Logger(Logger const &) = delete;

    Logger &operator=(const Logger &) = delete;
    Logger &operator=(Logger &&) = delete;

    void setLevel(LogLevel level);
    void setFile(const char *filename);

    Line log(LogLevel level);

protected:
    std::mutex m_lineLock;

private:
    LogLevel m_logLevel;
    std::ofstream m_outFile;
};

#ifdef NDEBUG
const Logger::LogLevel LoggerLevelDefault = Logger::LOG_INFO;
#else
const Logger::LogLevel LoggerLevelDefault = Logger::LOG_DEBUG;
#endif


// template function defintions have to be known externally, so they have to be placed in header file instead of source file

template <typename T>
Logger::Line &Logger::Line::operator<<(const T &t)
{
    if (m_logger)
    {
        try
        {
            std::cout << t;
            if (m_logger->m_outFile.is_open())
            {
                m_logger->m_outFile << t;
            }
        }
        catch (...)
        {
        }
    }
    return *this;
}

template <typename T>
Logger::Line &Logger::Line::write(const T *t, std::size_t count)
{
    if (m_logger)
    {
        std::cout.write(t, count);
        if (m_logger->m_outFile.is_open())
        {
            m_logger->m_outFile.write(t, count);
        }
    }
    return *this;
}


extern Logger LoggerInstance;
