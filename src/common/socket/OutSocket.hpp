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
 * @file    OutSocket.hpp
 * @brief   Client  socket API
 * @author  Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
 * @date    2012-02-05
 *
 * License: GPLv3
 */

#ifndef _OUTSOCKET_HPP
#define _OUTSOCKET_HPP

/****************************************************************************/
/* Includes                                                                 */
/****************************************************************************/

#include <netinet/in.h>                 // Network related types: sockaddr_in
#include <string>                       // STL string

/****************************************************************************/
/* Prototypes                                                               */
/****************************************************************************/


struct SocketInfo
{
    std::string hostname;
    std::string service;
};

/**
 * @class OutSocket
 * OutSocket is the general implementation of a socket which
 * will be immediatly connected and used for read/write
 * opperations. The best examples are the kind of sockets a
 * client would use, or the resulting "sockets" of an accepted
 * connection (same socket, different file descriptors).
 */
class OutSocket
{
// Typedefs

private:
    typedef struct sockaddr_in _SockAddr;   /**< Socket address container   */

// Members

private:
    int         _fd;                    /**< File Descriptor of the socket  */
    _SockAddr   _outsideAddr;           /**< Outside socket physical address*/
    int         _outsideLen;            /**< Outside socket address length  */
    std::string _host;                  /**< Host name                      */
    int         _port;                  /**< Port number                    */
    bool        _isOpen;                /**< Socket open / closed state     */

// Constructors and destructors

private:
    /**
     * @brief Private constructor of the OutSocket class.
     */
    OutSocket();

public:
    /**
     * @brief Public constructor of the OutSocket class.
     */
    OutSocket(std::string aHost, int aPort);

    /**
     * @brief Public constructor of the OutSocket class.
     */
    OutSocket(int aPort);

    /**
     * @brief Destructor of the OutSocket class
     */
    ~OutSocket();

// Class Methods

public:
    /**
     * @brief    Method enable connection to a endpoint address.
     *
     * @return   0 on succes, -1 on failure.
     */
    int enable();

    /**
     * @brief   Method to write data to the endpoint of the OutSocket.
     *
     * @param   pAddr   pointer to the address from where to write data
     * @param   nrBytes maximum number of bytes to be written
     * @return  number of bytes written on success
     * @return  -1 on failure
     */
    int write(const void* pAddr, int rrBytes);

    /**
     * @brief   Method to read data from the endpoint of the OutSocket.
     *
     * @param   pAddr   pointer to the address where to place read data
     * @param   nrBytes maximum number of bytes to be read
     * @return  the number of bytes read on success
     * @return  -1 on failure
     */
    int read(void* pAddr, int nrBytes);

    /**
     * @brief Accessor method for file descriptor
     * @return the file descriptor corresponding to the connection.
     * @return -1 if the file descriptor is not open.
     */
    int getFd() const;

    /**
     * @brief Retrieve socket information
     * @return SocketInfo data
     */
    SocketInfo getInfo() const;

// Friend methods

    /**
     * @brief Method to build an OutSocket by accepting a connection
     *        on a specified socket.
     * @param a
     */
    friend  OutSocket* buildAcceptedConnection(int aFd);
};


OutSocket* buildAcceptedConnection(int aFd);

#endif // ! _OUTSOCKET_HPP
