//  Redistributions of any form whatsoever must retain and/or include the
//  following acknowledgment, notices and disclaimer:
//
//  This product includes software developed by the University of Michigan.
//
//  Copyright 2011 by David Meisner
//  at the University of Michigan
//
//  You may not use the name "University of Michigan" or derivations
//  thereof to endorse or promote products derived from this software.
//
//  If you modify the software you must place a notice on or within any
//  modified version provided or made available to any third party stating
//  that you have modified the software.  The notice shall include at least
//  your name, address, phone number, email address and the date and purpose
//  of the modification.
//
//  THE SOFTWARE IS PROVIDED "AS-IS" WITHOUT ANY WARRANTY OF ANY KIND, EITHER
//  EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO ANY WARRANTY
//  THAT THE SOFTWARE WILL CONFORM TO SPECIFICATIONS OR BE ERROR-FREE AND ANY
//  IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
//  TITLE, OR NON-INFRINGEMENT.  IN NO EVENT SHALL THE UNIVERSITY OF MICHIGAN
//  BE LIABLE FOR ANY DAMAGES, INCLUDING BUT NOT LIMITED TO DIRECT, INDIRECT,
//  SPECIAL OR CONSEQUENTIAL DAMAGES, ARISING OUT OF, RESULTING FROM, OR IN
//  ANY WAY CONNECTED WITH THIS SOFTWARE (WHETHER OR NOT BASED UPON WARRANTY,
//  CONTRACT, TORT OR OTHERWISE).
//
// connection.h
// David Meisner (davidmax@gmail.com)
//

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <string>

#include "cachebash/util.h"

using std::string;

namespace cachebash {

class Request;
class Response;

enum ConnectionType {
  TCP,
  UDP,
};

// A class to represent a connection to a server
class Connection {
 public:
  Connection(ConnectionType connection_type, bool debug_packets_);
  int GetSocketFd();
  void OpenTcpSocket(const string& ip_address, int port, bool disable_nagles);
  Response* ReceiveResponse();
  void SendRequest(Request* request);
 private:
  ConnectionType connection_type_;
  bool debug_packets_;
  int sock_;
  DISALLOW_COPY_AND_ASSIGN(Connection);
};

}  // namespace cachebash

#endif  // CONNECTION_H_
