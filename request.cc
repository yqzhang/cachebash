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

#include "cachebash/request.h"

#include <string.h>
#include <string>

#include "cachebash/util.h"

namespace cachebash {

Request::Request(string key, string value)
    : extras_(NULL),
      extras_size_(0),
      key_(key),
      value_(value) {}

Request::~Request() {
  if (extras_size_ != 0) {
    delete[] extras_;
  }
}

// Creates a buffer with the binary formatted request for memcached.
// Sets |request_size_bytes| to the size of the buffer in bytes.
// Returns |request_buffer| to point to the buffer.
// The caller is responsible for deleting |request_buffer|.
char* Request::ConstructRequestPacket(int* request_size_bytes) {
  // TODO(davidmax@gmail.com) Clean this up
  // to use the Request class information.
  int key_size = key_.size();
  int value_size = value_.size();
  int body_size = extras_size_ + key_size + value_size;

  // Construct the request header.
  struct RequestHeader request_header;

  // All requests have the same magic byte.
  request_header.magic = MAGIC_REQUEST;
  request_header.opcode = this->op_code();
  request_header.key_size[0] = ((unsigned int)(key_size & 0xff00)) >> 8;
  request_header.key_size[1] = (key_size & 0xff);
  request_header.extras_size = extras_size_;
  request_header.data_type = 0;  // Reserved for future use. Just set to 0.
  request_header.reserved[0] = 0;
  request_header.reserved[1] = 0;
  request_header.total_body_size[3] = (body_size & 0xff);
  request_header.total_body_size[2]
    = ((unsigned int)(body_size & 0xff00)) >> 8;
  request_header.total_body_size[1]
    = ((unsigned int)(body_size & 0xff0000)) >> 16;
  request_header.total_body_size[0]
    = ((unsigned int)(body_size & 0xff000000)) >> 24;
  request_header.opaque[0] = 0;
  request_header.opaque[1] = 0;
  request_header.opaque[2] = 0;
  request_header.opaque[3] = 0;
  request_header.cas[0] = 0;
  request_header.cas[1] = 0;
  request_header.cas[2] = 0;
  request_header.cas[3] = 0;
  request_header.cas[4] = 0;
  request_header.cas[5] = 0;
  request_header.cas[6] = 0;
  request_header.cas[7] = 0;

  // Allocate the request buffer and copy the data over.
  *request_size_bytes = sizeof(struct RequestHeader)
                        + extras_size_ + key_size + value_size;
  char* request_buffer = new char[*request_size_bytes];
  char* ptr = request_buffer;
  memcpy(ptr, &request_header, sizeof(struct RequestHeader));
  ptr += sizeof(struct RequestHeader);
  memcpy(ptr, extras_, extras_size_);
  ptr += extras_size_;
  memcpy(ptr, key_.c_str(), key_size);
  ptr += key_size;
  memcpy(ptr, value_.c_str(), value_size);

  return request_buffer;
}

int Request::CalculateRequestSize() const {
  int request_size_bytes = sizeof(struct RequestHeader) + extras_size_
                           + key_.size() + value_.size();
  return request_size_bytes;
}

GetRequest::GetRequest(string key) : Request(key, "") {}

void GetRequest::UpdateStatistics(StatisticsCollection* statistics_collection) {
  statistics_collection->AddSample("get_requests", 1);
  statistics_collection->AddSample("get_request_size", CalculateRequestSize());
}

void GetRequest::Print() {
  printf("Get Request:\n");
  printf("  Key: %s\n", key_.c_str());
}

SetRequest::SetRequest(string key, string value) : Request(key, value) {
  extras_ = new char[8];
  extras_[0] = 0xde;
  extras_[1] = 0xad;
  extras_[2] = 0xbe;
  extras_[3] = 0xef;
  extras_[4] = 0x00;
  extras_[5] = 0x00;
  extras_[6] = 0x00;
  extras_[7] = 0x00;
  extras_size_ = 8;
}

void SetRequest::UpdateStatistics(StatisticsCollection* statistics_collection) {
  statistics_collection->AddSample("set_requests", 1);
  statistics_collection->AddSample("set_request_size", CalculateRequestSize());
}

void SetRequest::Print() {
  printf("Set Request:\n");
  printf("  Key: %s\n", key_.c_str());
  printf("  Value: %s\n", value_.c_str());
}

}  // namespace cachebash
