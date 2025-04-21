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
#include <sys/epoll.h>
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
        m_fd2 = -1;
    }

    ~c_event()
    {
        close(m_fd_ack_stop);
        close(m_fd);
    }

    // set file handle to check
    void set_handle(int32_t fd) { m_fd2 = fd; }

    // check for event
    bool poll_handle()
    {
		pollfd pfd[2];
		pfd[0].fd = m_fd; pfd[0].events = POLLIN; pfd[0].revents = 0;
		pfd[1].fd = m_fd2; pfd[1].events = POLLIN; pfd[1].revents = 0;

		if (poll(pfd, 2, -1) <= 0)
	    	return false;

	    if (pfd[0].revents >= POLLIN)
	    	return false;

	    return pfd[1].revents >= POLLIN;
    }

	// acknowledge stop
    inline void ack_stop() { eventfd_write(m_fd_ack_stop, 1); }

    // stop wait and waits ms
    // return true if stopped, false on timeout
    // if t_ms is 0, then function returns immediate
    // if t_ms is negative, then waits infinite
    bool stop(int32_t t_ms = -1)
    {
    	eventfd_write(m_fd, 1);
		pollfd pfd = { .fd = m_fd_ack_stop, .events = POLLIN, .revents = 0 };
        return poll(&pfd, 1, t_ms) > 0;
    }

private:
    int32_t m_fd;
    int32_t m_fd2; // handle to check
    int32_t m_fd_ack_stop;
};
