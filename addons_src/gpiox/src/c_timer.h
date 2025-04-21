/*
 * timer handling
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_timer.h
 *
 */

#pragma once

#include <stdint.h>
#include <time.h>
#include <poll.h>
#include <unistd.h>

#include "c_event.h"

#define NSEC_PER_SEC 1000000000l

class c_timer
{
public:
    c_timer() {}

    /**
     * @brief stops timer
     * @param sec wait in ms for stop
     * @note if t_ms is 0, then function returns immediate
     * @note if t_ms is negative, then waits infinite
     */
    inline void stop(int32_t t_ms = -1) { m_stopevent.stop(t_ms); }

    /**
     * @brief check for stop
     * @returns true on continue false on stop
     */
    inline bool check() { return m_stopevent.check(); }

    // acknowledge stop
    inline void ack_stop() { m_stopevent.ack_stop(); }

    /**
     * @brief delays current thread
     * @param sec delay time in seconds
     * @param nsec delay time in nano-seconds
     * @returns true on continue false on stop
     * @note time is adjusted on overflow
     * @note blocks current thread
     */
    bool delay(int64_t sec, int64_t nsec)
    {
        timespec ts;
        ts.tv_sec = sec;
        ts.tv_nsec = nsec;

        // adjust time on overflow
        adj_time(ts);

        // get actual time
        timespec ts_now;
        clock_gettime(CLOCK_REALTIME, &ts_now);

        // calculate end time
        ts.tv_sec =  ts_now.tv_sec + ts.tv_sec;
        ts.tv_nsec = ts_now.tv_nsec + ts.tv_nsec;

        // adjust time on overflow
        adj_time(ts);

        // wait until end time or stop request
        do
        {
            if (!m_stopevent.check())
                return false;

            clock_gettime(CLOCK_REALTIME, &ts_now);

        } while((ts_now.tv_sec < ts.tv_sec) || (ts_now.tv_nsec < ts.tv_nsec));

        return true;
    }

    // delay functions for us, ms and seconds
    inline bool delay_us(int64_t usec) { return delay(0l, usec * 1000l); };
    inline bool delay_ms(int64_t msec) { return delay(0l, msec * 1000000l); };
    inline bool delay_s(int64_t sec) { return delay(sec, 0l); };

    /**
     * @brief sleeps current thread
     * @param sec sleep time in seconds
     * @param nsec sleep time in nano-seconds
     * @returns true on continue false on stop
     * @note time is adjusted on overflow
     */
    bool sleep(int64_t sec, int64_t nsec)
    {
        timespec ts;
        ts.tv_sec = sec;
        ts.tv_nsec = nsec;

        // adjust time on overflow
        adj_time(ts);

        pollfd pfd = { .fd = m_stopevent.get_fd(), .events = POLLIN, .revents = 0 };

        // now sleep
        if (ppoll(&pfd, 1, &ts, NULL) > 0)
            return false;

        return true;
    }

    // sleep functions for us, ms and seconds
    inline bool sleep_us(int64_t usec) { return sleep(0l, usec * 1000l); };
    inline bool sleep_ms(int64_t msec) { return sleep(0l, msec * 1000000l); };
    inline bool sleep_s(int64_t sec) { return sleep(sec, 0l); };

private:
    c_event m_stopevent; // for stop timer

    /**
     * @brief adjust time on overflow
     * @param ts time to adjust
     */
    inline void adj_time(timespec &ts)
    {
        //
        while(ts.tv_nsec >= NSEC_PER_SEC)
        {
            ts.tv_sec++;
            ts.tv_nsec -= NSEC_PER_SEC;
        }
    }
};
