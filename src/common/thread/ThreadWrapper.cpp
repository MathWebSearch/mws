/*

Copyright (C) 2010-2013 KWARC Group <kwarc.info>

This file is part of MathWebSearch.

MathWebSearch is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MathWebSearch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MathWebSearch.  If not, see <http://www.gnu.org/licenses/>.

*/
/**
  * @brief File containing the implementation of the ThreadWrapper class.
  *
  * @file ThreadWrapper.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date 03 May 2011
  *
  * License: GPL v3
  */

// System includes

#include "ThreadWrapper.hpp"           // ThreadWrapper class definition

// Static definitions

pthread_mutex_t ThreadWrapper::lock;
pthread_cond_t  ThreadWrapper::threadReleased;
bool            ThreadWrapper::active;
unsigned int    ThreadWrapper::alive;


// Datatype used to pass Thread Data to the active thread
struct thread_data
{
    void* (*start_routine)(void*);
    void* arg;
    ThreadWrapper* thread_wrapper;
};


int ThreadWrapper::init()
{
    if (pthread_mutex_init(&lock, NULL))
        return -1;

    if (pthread_cond_init(&threadReleased, NULL))
    {
        pthread_mutex_destroy(&lock);
        return -2;
    }

    active = true;
    alive = 0;

    return 0;
}


void ThreadWrapper::clean()
{
    pthread_mutex_lock(&lock);
    while (alive)
    {
        pthread_cond_wait(&threadReleased,
                          &lock);
    }
    active = false;
    pthread_mutex_unlock(&lock);
}


void* ThreadWrapper::startWrapper(void* ptr)
{
    struct thread_data* data = (struct thread_data*) ptr;

    (data->start_routine)(data->arg);

    pthread_mutex_lock(&ThreadWrapper::lock);
    delete data->thread_wrapper;
    delete data;
    pthread_mutex_unlock(&ThreadWrapper::lock);

    return NULL;
}


ThreadWrapper::ThreadWrapper(void* (*start_routine)(void*),
                             void* arg)
{
    thread_data*   data;
    pthread_attr_t attr;
    int err;

    pthread_mutex_lock(&lock);
    if (active)
    {
        data = new thread_data;
        data->start_routine = start_routine;
        data->arg = arg;
        data->thread_wrapper = this;

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        err = pthread_create(&currentThread,
                             &attr,
                             ThreadWrapper::startWrapper,
                             data);
        if (err)
        {
            pthread_attr_destroy(&attr);
            pthread_mutex_unlock(&lock);
            throw err;
        }

        alive++;

        pthread_attr_destroy(&attr);
    }
    pthread_mutex_unlock(&lock);
}


ThreadWrapper::~ThreadWrapper()
{
    alive--;
    if (alive == 0)
        pthread_cond_signal(&threadReleased);
}


int ThreadWrapper::run(void* (*start_routine)(void*),
                       void* arg)
{
    try
    {
        new ThreadWrapper(start_routine, arg);
    }
    catch (int err)
    {
        return err;
    }

    return 0;
}
