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
//  request_test.cc
//  David Meisner (davidmax@gmail.com)
//

#include "cachebash/request.h"

#include <string>

#include "gtest/gtest.h"

using cachebash::GetRequest;
using cachebash::SetRequest;
using std::string;

namespace {

class GetRequestTest : public ::testing::Test {
 protected:
  GetRequestTest() {
  }

  virtual ~GetRequestTest() {
  }

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};

class SetRequestTest : public ::testing::Test {
 protected:
  SetRequestTest() {
  }

  virtual ~SetRequestTest() {
  }

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};

// Test getter and setter methods.
TEST_F(SetRequestTest, Accessors) {
  string key = "foo";
  string value = "bar";
  SetRequest request(key, value);
  EXPECT_STREQ(key.c_str(), request.key().c_str());
  EXPECT_STREQ(value.c_str(), request.value().c_str());
}

// Test formatting of get request packet.
TEST_F(GetRequestTest, RequestPacketConstruction) {
  string key = "foo";
  GetRequest request(key);
  int packet_size = 0;
  char* packet = request.ConstructRequestPacket(&packet_size);
  char expected_packet[] =
  { 0x80, 0x00, 0x00, 0x3,   // 0-3   magic, type, key length(2)
    0x00, 0x00, 0x00, 0x00,  // 4-7   extra_length, data type, reserved(2)
    0x00, 0x00, 0x00, 0x03,  // 8-11  total body (4)
    0x00, 0x00, 0x00, 0x00,  // 12-15 opaque (4)
    0x00, 0x00, 0x00, 0x00,  // 16-19 CAS (8)
    0x00, 0x00, 0x00, 0x00,  // 20-13
    key[0],   key[1],   key[2] };  // Key and value variable length

  EXPECT_EQ(static_cast<int>(sizeof(expected_packet)), packet_size);
  for (int i = 0; i < static_cast<int>(sizeof(expected_packet)); i++) {
    EXPECT_EQ(expected_packet[i], packet[i]) << i << "th packet is wrong";
  }
}

// Test getter and setter methods.
TEST_F(GetRequestTest, Accessors) {
  string key = "foo";
  GetRequest request(key);
  EXPECT_STREQ(key.c_str(), request.key().c_str());
}

// Test formatting of set packet.
TEST_F(SetRequestTest, RequestPacketConstruction) {
  string key = "foo";
  string value = "bar";
  SetRequest request(key, value);
  int packet_size = 0;
  char* packet = request.ConstructRequestPacket(&packet_size);
  char expected_packet[] =
  { 0x80, 0x01, 0x00, 0x3,   // 0-3   magic, type, key length(2)
    0x08, 0x00, 0x00, 0x00,  // 4-7   extra_length, data type, reserved(2)
    0x00, 0x00, 0x00, 0x0E,  // 8-11  total body (4)
    0x00, 0x00, 0x00, 0x00,  // 12-15 opaque (4)
    0x00, 0x00, 0x00, 0x00,  // 16-19 CAS (8)
    0x00, 0x00, 0x00, 0x00,  // 20-13
    0xde, 0xad, 0xbe, 0xef,  // 24-27 Extras(8) (4 for flags)
    0x00, 0x00, 0x00, 0x00,  // 28-31 (4 for expir)
    key[0],   key[1],   key[2],  // Key and value variable length
    value[0], value[1], value[2]    };
  EXPECT_EQ(static_cast<int>(sizeof(expected_packet)), packet_size);
  for (int i = 0; i < static_cast<int>(sizeof(expected_packet)); i++) {
    EXPECT_EQ(expected_packet[i], packet[i]) << i << "th packet is wrong";
  }
}
}  // namespace
