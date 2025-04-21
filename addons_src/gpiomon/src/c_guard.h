/*
 * thread lock guard
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_guard.h
 *
 */

#pragma once

#include <mutex>
using namespace std;

class c_guard
{
public:

	// lock on constructor call
	c_guard(mutex* mtx)
	{
		m_mtx = mtx;

		if (m_mtx != NULL)
			m_mtx->lock();
	}

	// unlock on destructor call
	~c_guard()
	{
		if (m_mtx != NULL)
			m_mtx->unlock();
	}

private:
    mutex* m_mtx;
};
