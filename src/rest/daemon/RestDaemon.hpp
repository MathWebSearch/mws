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
#ifndef _RESTDAEMON_HPP
#define _RESTDAEMON_HPP

/**
  * @brief File containing the header of the RestDaemon class.
  * @file RestDaemon.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 20 Jun 2011
  *
  * License: GPL v3
  *
  */

// System includes

#include <string>
#include "microhttpd_linux.h"           // Linux MicroHTTPd library includes

/**
  * @brief Class encapsulating a basic RESTful interface daemon.
  */
class RestDaemon
{
public:
    struct Config {
        int         restPort;
        int         mwsPort;
        std::string mwsHost;
    };
private:
    struct MHD_Daemon* _daemonHandler;
    struct Config      _config;
public:
    /**
      * @brief Default Constructor of the RestDaemon class.
      */
    RestDaemon();

    /**
      * @brief Default Destructor of the RestDaemon class.
      */
    ~RestDaemon();

    /**
      * @brief Method to start a rest daemon.
      * @param aPort the port where to run.
      * @return 0 on success and -1 on failure.
      */
    int startAsync(const Config& config);

    /**
      * @brief Method to stop a running daemon.
      */
    void stop();
};


#endif // _RESTDAEMON_HPP
