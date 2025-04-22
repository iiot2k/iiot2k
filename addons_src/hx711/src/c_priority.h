/*
 * priority switching
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_priority.h
 *
 */

#pragma once

#include <sched.h>

class c_priority
{
public:
    // save actual priority
    // set priority to FIFO on create
    c_priority()
    {
        sched_getparam(0, &m_sched);
        sched_param sched;
        sched.sched_priority = sched_get_priority_max(SCHED_FIFO);
        sched_setscheduler(0, SCHED_FIFO, &sched);
    }

    // restore previous priority on destroy
    ~c_priority()
    {
        sched_setscheduler(0, SCHED_OTHER, &m_sched);
    }

private:
    sched_param m_sched;
};
