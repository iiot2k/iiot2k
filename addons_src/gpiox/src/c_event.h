/*
 * event handling
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_event.h
 *
 */

#pragma once

#include <sys/eventfd.h>
#include <unistd.h>
#include <poll.h>
#include <unistd.h>

class c_event
{
public:
    c_event()
    {
        m_fd = eventfd(0, 0);
        m_fd_ack_stop = eventfd(0, 0);
    }

    ~c_event()
    {
        close(m_fd);
        close(m_fd_ack_stop);
    }

    // wait for ms
    // return false on stop request
    // if t_ms is 0, then function returns immediate
    // if t_ms is negative, then waits infinite timeout
    bool wait(int32_t t_ms)
    {
        pollfd pfd = { .fd = m_fd, .events = POLLIN, .revents = 0 };

        if (poll(&pfd, 1, t_ms) > 0)
        {
            ack_stop();
            return false;
        }

        return true;
    }

    // check if stop signaled
    inline bool check() { return wait(0); }

    // return event fd
    inline int32_t get_fd() { return m_fd; }

    // acknowledge stop
    inline void ack_stop() { eventfd_write(m_fd_ack_stop, 1); }

    // stop wait and waits ms
    // return true if stopped, false on timeout
    // if t_ms is 0, then function returns immediate
    // if t_ms is negative, then waits infinite
    bool stop(int32_t t_ms)
    {
        eventfd_write(m_fd, 1);

        pollfd pfd = { .fd = m_fd_ack_stop, .events = POLLIN, .revents = 0 };
        return poll(&pfd, 1, t_ms) > 0;
    }

private:
    int32_t m_fd;
    int32_t m_fd_ack_stop;
};
