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
 * @file    InSocket.hpp
 * @brief   Server socket API
 * @author  Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
 * @date    2012-02-05
 *
 * License: GPLv3
 */

#ifndef _INSOCKET_HPP
#define _INSOCKET_HPP

/****************************************************************************/
/* Includes                                                                 */
/****************************************************************************/

#include <netinet/in.h>                 // Network related types: sockaddr_in
#include "OutSocket.hpp"                // OutSocket class definition

/****************************************************************************/
/* Constants                                                                */
/****************************************************************************/

#define  SOCKET_DEFAULT_QUEUE_SIZE  3

/****************************************************************************/
/* Prototypes                                                               */
/****************************************************************************/

/**
 * InSocket is the general implementation of a server side socket which is
 * created, binded, set to listen and then just used to accept connections.
 *
 * @brief Server side socket.
 */
class InSocket
{
    // Typedefs
    typedef struct sockaddr_in _SockAddr;   /**< Socket address container   */

    // Private members
    int         _fd;                    /**< File Descriptor of the socket  */
    _SockAddr   _insideAddr;            /**< Inside socket physical address */
    int         _port;                  /**< Port number                    */
    int         _queueSize;             /**< Acceptance queue max size      */
    bool        _isOpen;                /**< Socket open / closed state     */

public:
    // Constructors and Destructors
    /**
     * @brief Constructor of the InSocket class.
     *
     * @param aPort the port where to listen for incoming connections.
     * @param aQueueSize maximum acceptance waiting connections.
     */
    InSocket(int aPort,
             int aQueueSize = SOCKET_DEFAULT_QUEUE_SIZE);
    /**
      * @brief Destructor of the InSocket class.
      */
    ~InSocket();

    // Public methods
    /**
     * Note that enable() is an idempotent operation.
     *
     * @brief Method to enable listening for incoming connections
     * @return 0 on success and -1 on error. On failure, errno will be set
     *         accordingly.
     */
    int enable();

    /**
     * Note that this is a blocking call.
     *
     * @brief Method to accept an incoming connection.
     * @return Pointer to the accepted OutSocket on success and NULL on failure
     *         or if the socket is not started.
     */
    OutSocket* accept();
};

#endif // ! _INSOCKET_HPP
