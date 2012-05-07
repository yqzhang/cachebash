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
// response.h
// David Meisner (davidmax@gmail.com)
//

#ifndef RESPONSE_H_
#define RESPONSE_H_

#include "cachebash/util.h"

namespace cachebash {

const char  kMagicResponse = static_cast<char>(0x81);

// memcached error codes.
const char kNoError = static_cast<char>(0x0000);
const char kKeyNotFound = static_cast<char>(0x0001);
const char kKeyExists = static_cast<char>(0x0002);
const char kValueTooLarge = static_cast<char>(0x0003);
const char kInvalidArgument = static_cast<char>(0x0004);
const char kItemNotStored = static_cast<char>(0x0005);
const char kIncDecNonNum = static_cast<char>(0x0006);
const char kUnknownCommand = static_cast<char>(0x0081);
const char kOutOfMemory = static_cast<char>(0x0082);

struct ResponseHeader {
  char magic;
  char opcode;
  char key_size[2];
  char extras_size;
  char data_type;
  char status[2];
  char total_body_size[4];
  char opaque[4];
  char cas[8];
};

class Request;

class Response {
 public:
  Response();
  virtual ~Response();
  static Response* CreateResponseFromHeader(
                     const ResponseHeader& response_header);
  void set_request(Request* request) { request_ = request; }
  Request* request() const { return request_; }
  void set_request_latency(float latency) { response_latency_ = latency; }
  float request_latency() const { return response_latency_; }

 private:
  Request* request_;
  float response_latency_;

  DISALLOW_COPY_AND_ASSIGN(Response);
};

class GetResponse : public Response {
 public:
 private:
  DISALLOW_COPY_AND_ASSIGN(GetResponse);
};

class SetResponse : public Response {
 public:
 private:
  DISALLOW_COPY_AND_ASSIGN(SetResponse);
};

}  // namespace

#endif  // RESPONSE_H_
