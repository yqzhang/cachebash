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
// KeySizeDistribution.h
// David Meisner (davidmax@gmail.com)
//
// Represents a popularity distribution over a set of object size/key pairs.
// Loads distributions with the format (cdf, size, key) where cdf is
// the cdf value of the popularity distribution for a given object
// size/key pair. Size is the size of the object and key is a string
// representing the object's key.


#ifndef SIZE_KEY_DISTRIBUTION_H_
#define SIZE_KEY_DISTRIBUTION_H_

#include <string>

#include "cachebash/util.h"

using std::string;

namespace cachebash {

struct SizeKeyEntry {
  float cdf;
  int size;
  string key;
};

class SizeKeyDistribution {
 public:
  struct SizeKeyEntry* GetRandomEntry(int random_int) const;
  static SizeKeyDistribution* LoadFile(string filename);
  int n_entries() const { return n_entries_; }
  // TODO(davidmax@gmail.com) Find a way to protect this.
  SizeKeyEntry** size_key_entries() const { return size_key_entries_; }
 private:
  SizeKeyDistribution(struct SizeKeyEntry** size_key_entries, int n_entries);
  struct SizeKeyEntry** size_key_entries_;
  int n_entries_;

  DISALLOW_COPY_AND_ASSIGN(SizeKeyDistribution);
};

}  // namespace cachebash

#endif  // SIZE_KEY_DISTRIBUTION_H_
