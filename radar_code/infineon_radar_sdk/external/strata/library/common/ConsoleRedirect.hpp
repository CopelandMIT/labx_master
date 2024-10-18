/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <iostream>
#include <thread>


// redirect stdout to custom implementation, optionally only within the same thread

class IStreamRedirect
{
public:
    virtual ~IStreamRedirect()                                = default;
    virtual void onOutput(const std::streambuf::char_type *s) = 0;
};


class StreamOverride :
    public std::streambuf
{
public:
    StreamOverride(IStreamRedirect *redirect) :
        m_redirect {redirect}
    {
        m_s[1] = 0;
    }

protected:
    virtual std::streamsize xsputn(const char_type *s, std::streamsize count) override
    {
        m_redirect->onOutput(s);
        return count;
    }

    virtual int_type overflow(int_type ch = std::char_traits<char_type>::eof()) override
    {
        if (ch != std::char_traits<char_type>::eof())
        {
            m_s[0] = std::char_traits<char_type>::to_char_type(ch);
            m_redirect->onOutput(m_s);
        }
        return ch;
    }

private:
    IStreamRedirect *m_redirect;
    char_type m_s[2];
};


class StreamOverrideThreaded :
    public StreamOverride
{
public:
    StreamOverrideThreaded(IStreamRedirect *redirect, std::thread::id threadId) :
        StreamOverride(redirect),
        m_threadId {threadId}
    {
    }

protected:
    virtual std::streamsize xsputn(const char_type *s, std::streamsize count) override
    {
        const auto thisThreadId = std::this_thread::get_id();
        if (thisThreadId != m_threadId)
        {
            return count;
        }

        return StreamOverride::xsputn(s, count);
    }

    virtual int_type overflow(int_type ch = std::char_traits<char_type>::eof()) override
    {
        const auto thisThreadId = std::this_thread::get_id();
        if (thisThreadId != m_threadId)
        {
            return ch;
        }

        return StreamOverride::overflow(ch);
    }

private:
    std::thread::id m_threadId;
};


class ConsoleRedirect :
    public IStreamRedirect
{
public:
    ConsoleRedirect() :
        m_out(this)
    {
        original_buf = std::cout.rdbuf();
        std::cout.rdbuf(&m_out);
    }
    ~ConsoleRedirect()
    {
        std::cout.rdbuf(original_buf);
    }

private:
    StreamOverride m_out;
    std::streambuf *original_buf;
};
