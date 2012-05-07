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
// request.h
// David Meisner (davidmax@gmail.com)
//

#ifndef REQUEST_H_
#define REQUEST_H_

#include <string>

#include "cachebash/statistic.h"

#define MAGIC_REQUEST  static_cast<char>(0x80)

#define OPCODE_STAT    static_cast<char>(0x10)
#define OPCODE_GET     static_cast<char>(0x00)
#define OPCODE_SET     static_cast<char>(0x01)
#define OPCODE_GETQ    static_cast<char>(0x09)
#define OPCODE_INCR    static_cast<char>(0x05)
#define OPCODE_DEL     static_cast<char>(0x04)
#define OPCODE_ADD     static_cast<char>(0x02)
#define OPCODE_REP     static_cast<char>(0x03)

namespace cachebash {

struct RequestHeader {
  char magic;
  char opcode;
  char key_size[2];
  char extras_size;
  char data_type;  // Reserved for future use. Just set to 0.
  char reserved[2];  // Unused for now.
  char total_body_size[4];

  // A value which can be set to be reflected back by the server.
  char opaque[4];
  char cas[8];
};

class Request {
 public:
  Request(string key, string value);
  virtual ~Request();
  int CalculateRequestSize() const;
  char* ConstructRequestPacket(int* request_size_bytes);
  char* extras() const { return extras_; }

  string key() const { return key_; }

  virtual char op_code() = 0;
  virtual void Print() = 0;
  struct timeval send_time() const { return send_time_; }

  void set_send_time(struct timeval send_time) { send_time_ = send_time; }

  virtual void UpdateStatistics(StatisticsCollection* statistic_collection) = 0;
  string value() const { return value_; }

 protected:
  char* extras_;
  int extras_size_;
  string key_;
  char op_code_;
  struct timeval send_time_;
  string value_;
};

class SetRequest : public Request {
 public:
  SetRequest(string key, string value);
  virtual char op_code() { return OPCODE_SET; }
  virtual void UpdateStatistics(StatisticsCollection* statistic_collection);
  virtual void Print();
};

class GetRequest : public Request {
 public:
  explicit GetRequest(string key);
  virtual char op_code() { return OPCODE_GET; }
  virtual void UpdateStatistics(StatisticsCollection* statistic_collection);
  virtual void Print();
};

}  // namespace cachebash

#endif  // REQUEST_H_
