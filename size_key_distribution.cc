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
// size_key_distribution.cc
// David Meisner (davidmax@gmail.com)
//

#include "cachebash/size_key_distribution.h"

#include <string.h>
#include <fstream>

#include "cachebash/util.h"

namespace cachebash {

SizeKeyDistribution::SizeKeyDistribution(struct SizeKeyEntry** size_key_entries,
                                         int n_entries)
    : size_key_entries_(size_key_entries), n_entries_(n_entries) {}

// Select a random entry from the SizeKeyDistribution.
// Selects a point in a uniform random space determines the CDF
// value that corresponds to the empirical CDF distribution of
// the SizeKeyDistribution.
struct SizeKeyEntry* SizeKeyDistribution::GetRandomEntry(int random_int) const {
  // TODO(davidmax@gmail.com) Change to a random function.
  double target_cdf = (random_int % 1000000) / 1000000.0;

  // Perform a binary search;
  int bottom = 0;
  int top = n_entries_ - 1;
  int current = (bottom + top) / 2;
  while (top != bottom) {
    // TODO(davidmax@gmail.com) double check search code.
    // We are below our search target.
    // B     C   V   T
    // 3 4 5 6 7 8 9 10
    if (size_key_entries_[current]->cdf < target_cdf) {
      bottom = current;
    } else if (size_key_entries_[current]->cdf < target_cdf) {
    // We are above our search target.
    // B   V C       T
    // 3 4 5 6 7 8 9 10
      top = current;
    } else {
      break;
    }

    // Pick a new current point in the middle.
    // B     C     T
    // 3 4 5 6 7 8 9
    // Pick 6 = (3 + 9) / 2
    current = (bottom + top) / 2 + 1;
  }
  return size_key_entries_[current];
}

SizeKeyDistribution* SizeKeyDistribution::LoadFile(string filename) {
  // TODO(davidmax@gmail.com) Change this to avoid using streams.
  std::ifstream file(filename.c_str());
  if (!file.is_open()) {
    LOG_FATAL("Could not open " + filename);
  }
  string line;
  int n_entries = 0;
  while (getline(file, line)) {
    n_entries++;
  }
  struct SizeKeyEntry** size_key_entries = new struct SizeKeyEntry*[n_entries];
  // TODO(davidmax@gmail.com) figure out why seek isn't working.
  file.close();
  file.open(filename.c_str());
  int i = 0;
  while (getline(file, line)) {
    // TODO(davidmax@gmail.com) is there a better way to do this?
    // Split Size/Key distribution line into cdf, size, and key.
    if (line.length() < 1) {
      continue;
    }
//    printf("line: %s \ni=%d\n", line.c_str(), i);
    char* line_str = new char[line.length() + 1];
    memcpy(line_str, line.c_str(), line.length() + 1);
    struct SizeKeyEntry* entry = new struct SizeKeyEntry();
    char* save_ptr;
    char* pch = strtok_r(line_str, ", ", &save_ptr);
    entry->cdf = atof(pch);
    pch = strtok_r(NULL, ", ", &save_ptr);
    entry->size = atoi(pch);
    pch = strtok_r(NULL, ", ", &save_ptr);
    entry->key = string(pch);
    size_key_entries[i] = entry;
    delete[] line_str;
    i++;
  }
  file.close();
  SizeKeyDistribution* size_key_distribution
                         = new SizeKeyDistribution(size_key_entries,
                                                   n_entries);
  return size_key_distribution;
}

}  // namespace cachebash
