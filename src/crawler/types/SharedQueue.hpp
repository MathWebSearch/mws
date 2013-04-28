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
#ifndef _QUEUE
#define _QUEUE

#include <string>
#include <vector>
#include <queue>
#include <pthread.h>

struct QueueNode
{
  std::string urlstart;
  int count;
  int dontcrawlnr;
  std::vector <std::string> dontcrawl;
};

struct SharedQueue
{
  std::queue <QueueNode> sharedQueue;
  pthread_mutex_t lock;
  SharedQueue()
  {
    // lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutex_init(&lock, &attr);
  }
};

#endif
