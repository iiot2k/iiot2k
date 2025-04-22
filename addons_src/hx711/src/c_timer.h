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

#define NSEC_PER_SEC 1000000000l

class c_timer
{
public:
    c_timer() {}

    /**
     * @brief delays current thread
     * @param sec delay time in seconds
     * @param nsec delay time in nano-seconds
     * @note time is adjusted on overflow
     * @note blocks current thread
     */
    void delay(int64_t sec, int64_t nsec)
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

        // wait until end time
        do
        {
            clock_gettime(CLOCK_REALTIME, &ts_now);

        } while((ts_now.tv_sec < ts.tv_sec) || (ts_now.tv_nsec < ts.tv_nsec));
    }

    // delay functions for us, ms and seconds
    inline void delay_us(int64_t usec) { delay(0l, usec * 1000l); };
    inline void delay_ms(int64_t msec) { delay(0l, msec * 1000000l); };
    inline void delay_s(int64_t sec) { delay(sec, 0l); };

    /**
     * @brief sleeps current thread
     * @param sec sleep time in seconds
     * @param nsec sleep time in nano-seconds
     * @note time is adjusted on overflow
     */
    void sleep(int64_t sec, int64_t nsec)
    {
        timespec ts;
        ts.tv_sec = sec;
        ts.tv_nsec = nsec;

        // adjust time on overflow
        adj_time(ts);

        // now sleep
        clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);
    }

    // sleep functions for us, ms and seconds
    inline void sleep_us(int64_t usec) { sleep(0l, usec * 1000l); };
    inline void sleep_ms(int64_t msec) { sleep(0l, msec * 1000000l); };
    inline void sleep_s(int64_t sec) { sleep(sec, 0l); };

private:
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
