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
//  scoped_ptr.h
//  David Meisner (davidmax@gmail.com)
//
//  Based heavily on Google's implementation of scoped_ptr in the
//  open-source protobuf implementation.
//  See: src/google/protobuf/stubs/common.h

#ifndef SCOPED_PTR_H_
#define SCOPED_PTR_H_

#include <assert.h>

#include "cachebash/util.h"

template <class T> class scoped_ptr {
 public:
  explicit scoped_ptr(T* ptr) : ptr_(ptr) {}

  ~scoped_ptr() {
    delete ptr_;
  }

  T& operator*() const {
    assert(ptr_ != NULL);
    return *ptr_;
  }

  T* operator->() const {
    assert(ptr_ != NULL);
    return ptr_;
  }

  bool operator==(T* ptr) const { return ptr == ptr_; }

  bool operator!=(T* ptr) const { return ptr != ptr_; }

  T* Get() const { return ptr_; }

 private:
  T* ptr_;

  DISALLOW_COPY_AND_ASSIGN(scoped_ptr);
};

template <class T> class scoped_array {
 public:
  explicit scoped_array(T* array) : array_(array) {}

  ~scoped_array() { delete[] array_; }

  T* Get() const { return array_; }

 private:
  T* array_;

  DISALLOW_COPY_AND_ASSIGN(scoped_array);
};

#endif  // SCOPED_PTR_H_
