/*
 * thread safe queue
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_tsqueue.h
 *
 * Based on
 * https://www.geeksforgeeks.org/implement-thread-safe-queue-in-c/
 *
 */

#pragma once

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
using namespace std;

template <typename T>
class c_tsqueue {

public:
	// frees all items
	~c_tsqueue()
	{
    	while(!m_queue.empty())
    		delete (T) m_queue.front();
	}

	// add item to queue bottom
    void Push(T item)
    {
        unique_lock<mutex> lock(m_mutex);
        m_queue.push(item);
        m_cond.notify_one();
    }

    // wait uitil item in queue and get item from top
    T Pop()
    {
        unique_lock<mutex> lock(m_mutex);
        m_cond.wait(lock, [this]() { return !m_queue.empty(); });
        T item = m_queue.front();
        m_queue.pop();

        return item;
    }

private:
    queue<T> m_queue;
    mutex m_mutex;
    condition_variable m_cond;
};
