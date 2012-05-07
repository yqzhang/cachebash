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
// generator.h
// David Meisner (david@gmail.com)

#ifndef GENERATOR_H_
#define GENERATOR_H_

// TODO(davidmax@gmail.com) put these to the correct values.
// #define MAX_KEY_SIZE 250
#define MAX_KEY_SIZE 10
// #define MAX_VALUE_SIZE (1024*1023)
#define MAX_VALUE_SIZE (10)

#include <string>

using std::string;

namespace cachebash {

class Request;
class Config;
class SizeKeyDistribution;

class Generator {
 public:
  explicit Generator(Config* config);
  Request* GenerateNextRequest();
  static string GenerateRandomString(int max_length);

 private:
  Config* config_;
};

}  // namespace cachebash

#endif  // GENERATOR_H_
