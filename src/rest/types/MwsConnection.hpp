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
#ifndef _MWSCONNECTION_HPP
#define _MWSCONNECTION_HPP

/**
  * @brief File containing the header of the MwsConnection class.
  * @file MwsConnection.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 29 Jul 2011
  *
  * License: GPL v3
  *
  */

#include <string>
#include <sstream>
#include <iomanip>
#include <sys/time.h>

#include "common/socket/OutSocket.hpp"
#include "common/types/ControlSequence.hpp"
#include "common/utils/ToString.hpp"

// Config files
#include "config.h"

// TODO documentation

class MwsConnection
{
private:
    OutSocket       _mwsSocket;
    ControlSequence _controlSequence;
    bool            _connected;
    ssize_t         _bytes_read;
    ssize_t         _bytes_written;
    struct timeval  _tv_start;

public:
    MwsConnection(std::string host = DEFAULT_MWS_HOST, int port = DEFAULT_MWS_PORT) :
        _mwsSocket(host, port),
        _connected     ( false ),
        _bytes_read    ( 0 ),
        _bytes_written ( 0 )
    {
        gettimeofday(&_tv_start, NULL);
    }

    inline int connect()
    {
        int ret;

        ret = _mwsSocket.enable();
        if (ret != -1)
        {
            _connected = true;
        }

        return ret;
    }

    inline bool isConnected()
    {
        return _connected;
    }

    inline bool isParsed()
    {
        return _controlSequence.isParsed();
    }

    inline DataFormat getOutputFormat()
    {
        return _controlSequence.getFormat();
    }

    inline int write(const void* data_ptr, size_t data_size)
    {
        ssize_t nbytes;

        if (_connected)
        {
            nbytes          = _mwsSocket.write(data_ptr, data_size);
            _bytes_written += nbytes;
        }
        else
        {
            nbytes = data_size;
        }

        return nbytes;
    }

    inline int read(void* data_ptr, size_t data_size)
    {
        ssize_t nbytes;

        if (_connected)
        {
            nbytes       = _mwsSocket.read(data_ptr, data_size);
            _bytes_read += nbytes;
        }
        else
        {
            nbytes = data_size;
        }

        return nbytes;
    }

    inline int signalEnd()
    {
        int ret;
        // Shutting down the WRITE end of the socket
        shutdown(_mwsSocket.getFd(), SHUT_WR); 

        // Receiving the control data and discarding if errors detected
        if ((ret = _controlSequence.recv(_mwsSocket.getFd()))
                != 0)
        {
            ret = -1;
        }

        return ret;
    }

    inline std::string getInfo()
    {
        std::stringstream ss;
        struct timeval tv_now;
        struct timeval tv_diff;

        gettimeofday(&tv_now, NULL);

        tv_diff.tv_sec  = tv_now.tv_sec  - _tv_start.tv_sec;
        tv_diff.tv_usec = tv_now.tv_usec - _tv_start.tv_usec;

        if (tv_diff.tv_usec < 0)
        {
            tv_diff.tv_sec  -= 1;
            tv_diff.tv_usec += 1000000;
        }

        if (!_connected)
        {
            ss << "Couldn't connect to MWS";
        }
        else if (!_controlSequence.isParsed())
        {
            ss << "Couldn't parse Mws Query request";
        }
        else
        {
            ss << "Response (" << ToString(getOutputFormat())
               << ") of " << _bytes_read << " bytes returned in "
               << tv_diff.tv_sec << ".";
            ss << std::setfill('0') << std::setw(3)
               << std::right << tv_diff.tv_usec / 1000 << "s";
        }

        return ss.str();
    }
};

#endif // _MWSCONNECTION_HPP
