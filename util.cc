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
// util.h
// David Meisner (davidmax@gmail.com)
//

#include "cachebash/util.h";

#include <arpa/inet.h>
#include <execinfo.h>
#include <netdb.h>
#include <stdlib.h>


namespace cachebash {

// Translate a hostname to an IP address.
string nslookup(string hostname) {
  struct hostent* host_info = 0;
  for (int attempt = 0; (host_info == 0) && (attempt < 3); attempt++) {
    host_info = gethostbyname(hostname.c_str());
  }

  char* ip_address;
  if (host_info) {
    struct in_addr* address = (struct in_addr*)host_info->h_addr;
    ip_address = inet_ntoa(*address);
    printf("Host: %s\n", host_info->h_name);
    printf("Address: %s\n", ip_address);
  } else {
    LOG_FATAL("DNS error");
  }
  return string(ip_address);
}

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, 2);
  exit(1);
}

void LogFatal(string error) {
  fprintf(stderr, "%s\n", error.c_str());
  signal(SIGSEGV, handler);   // install our handler
  int *foo = reinterpret_cast<int*>(-1);  // make a bad pointer
  printf("%d\n", *foo);  // causes segfault
  exit(-1);
}

void LogInfo(string msg) {
  printf("%s\n", msg.c_str());
}

// TODO(davidmax@gmail.com) This is no good.
int RandomInt() {
  return rand();
}

// Random number between 0.0 and 1.0.
float RandomFloat() {
  float random = (rand() % 1000000001) / 1000000000.0;
  return random;
}

}  // namespace cachebash
