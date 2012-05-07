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
//  config.cc
//  David Meisner (davidmax@gmail.com)
//

#include "cachebash/config.h"

#include <stdio.h>

namespace cachebash {

Config::Config() {
  // Set the default values.
  debug_ = false;
  fixed_object_size_ = 1024;
  fraction_gets_ = 0.9;
  fraction_multiget_ = MULTIGET_DISABLED;
  multiget_n_gets_ = MULTIGET_DISABLED;
  n_cpus_ = 1;
  n_connections_per_worker_ = 1;
  n_worker_threads_ = 1;
  server_ip_address_ = "127.0.0.1";
  size_key_distribution_ = NULL;
  runtime_ = NO_RUNTIME_LIMIT;
  rps_ = -1.0;
  stat_print_interval_ = 1.0;
  use_naggles_ = false;
}

void Config::Print() {
  printf("Configuration:\n");
  printf("fraction_gets_: %f\n", fraction_gets_);
  printf("n_cpus: %d\n", n_cpus_);
  printf("n_connections_per_worker: %d\n", n_connections_per_worker_);
  printf("n_worker_threads: %d\n", n_worker_threads_);
  printf("server_ip_address: %s\n", server_ip_address_.c_str());
  // TODO(davidmax@gmail.com) Replace this with something more meaningful.
  printf("size_key_distribution: %p\n", size_key_distribution_);
  printf("stat_print_interval: %f\n", stat_print_interval_);
  printf("runtime: %f\n", runtime_);
  printf("use_naggles: %d\n", use_naggles_);
  printf("\n");
}

}  // namespace cachebash
