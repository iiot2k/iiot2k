/*
 * C++ class definition of worker
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_worker.h
 *
 */

#pragma once

#include <thread>
using namespace std;

class c_worker
{
public:
    // worker destructor
    virtual ~c_worker() {}

    // called once in thread, overwrite on derived class
    virtual void Execute() {}

    // start thread in background
    // join: false -> don't wait thread ends, true -> wait until threads ends
    void Queue()
    {
        thread th(&c_worker::execute, this);
        th.detach();
    }

private:
    void execute()
    {
        Execute();
        delete this; // harakiri class
    }
};
