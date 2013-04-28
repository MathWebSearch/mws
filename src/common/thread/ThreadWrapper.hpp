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
#ifndef _THREADWRAPPER_HPP
#define _THREADWRAPPER_HPP

/**
  * @brief File containing the header of the ThreadWrapper class.
  *
  * @file ThreadWrapper.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 03 May 2011
  *
  * License: GPL v3
  */

// System includes

#include <pthread.h>                   // POSIX Threads library header


/**
  * @brief Class to run self cleaning threads
  */
class ThreadWrapper
{
private:
    /// Global mutex to ensure exclusive access to class variables
    static pthread_mutex_t lock;
    /// Condition to signal the fact that a thread was released
    static pthread_cond_t  threadReleased;
    /// Boolean flag to signal if new threads are admited
    static bool            active;
    /// Counter for the number of active instances (threads)
    static unsigned int    alive;

    /// Instace thread variable
    pthread_t              currentThread;

    /**
      * @brief Constructor of the class
      * @param start_routine is the function taking a void* and returning a
      * void*
      * @param arg is the argument passed to the function
      */
    ThreadWrapper(void* (*start_routine)(void*),
                  void* arg);

    /// Destructor of the class
    ~ThreadWrapper();

    /// Class method used to wrap new thread function calls
    static void* startWrapper(void* arg);
public:
    // Class Methods

    /**
      * Note that this is not thread-safe.
      * @brief Method to initialize the ThreadWrapper class.
      * @return 0 if successfull and a negative value on error.
      */
    static int init();

    /**
      * @brief Method to run a function in a new thread.
      * @param start_routine is the function to be run.
      * @param arg is the argument of the function to be run.
      * @return 0 if successfull and a non-zero value on error.
      */
    static int run(void* (*start_routine)(void*),
                   void* arg);

    /**
      * This will be called when ThreadWrapper is not needed anymore.
      * @brief Method to wait for running threads and clean the ThreadWrapper
      * class.
      */
    static void clean();

};

#endif // _THREADWRAPPER_HPP
