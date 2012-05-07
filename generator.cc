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
// generator.cc
// David Meisner (davidmax@gmail.com)

#include "cachebash/generator.h"

#include <string>

#include "cachebash/config.h"
#include "cachebash/request.h"
#include "cachebash/size_key_distribution.h"
#include "cachebash/util.h"

namespace cachebash {

Generator::Generator(Config* config) : config_(config) {}

string Generator::GenerateRandomString(int max_length) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    // TODO(davidmax@gmail.com) fix rand()
    int length = (rand() % (max_length - 1)) + 1;
    string s;
    s.reserve(length);
    for (int i = 0; i < length; i++) {
        s.push_back(alphanum[rand() % (sizeof(alphanum) - 1)]);
    }
    return s;
}

Request* Generator::GenerateNextRequest() {
  Request* request = NULL;
  string key = "";
  string value = "";
  // Check if we've been provided a size/key distribution file.
  if (config_->size_key_distribution_ != NULL) {
    SizeKeyEntry* size_key_entry
      = config_->size_key_distribution_->GetRandomEntry(RandomInt());
    key = size_key_entry->key;
    value = Generator::GenerateRandomString(size_key_entry->size);
  } else {
    key = Generator::GenerateRandomString(MAX_KEY_SIZE);
    value = Generator::GenerateRandomString(MAX_VALUE_SIZE);
  }

  float random = RandomFloat();
  if (random < config_->fraction_gets_) {
    request = new GetRequest(key);
  } else {
    request = new SetRequest(key, value);
  }
  return request;
}

}  // namespace cachebash
