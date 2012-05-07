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
//  connection.cc
//  David Meisner (davidmax@gmail.com)
//

#include "cachebash/connection.h"

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "cachebash/request.h"
#include "cachebash/response.h"
#include "cachebash/scoped_ptr.h"
#include "cachebash/util.h"

namespace cachebash {

// A function for printing byte buffers to the screen.
// Useful for comparing to the protocol in:
// http://code.google.com/p/memcached/wiki/MemcacheBinaryProtocol
void PrintBuffer(char* buffer, int buffer_size, bool show_ascii) {
  int bytes_per_line = 4;
  int byte_lines = buffer_size / bytes_per_line;
  if (buffer_size % bytes_per_line != 0) {
    byte_lines++;
  }
  string break_line = "        "
                      "+---------------+---------------"
                      "+---------------+---------------+";

  printf("Buffer size %d Byte lines %d Show ASCII %d\n",
         buffer_size,
         byte_lines,
         show_ascii);
  printf("Byte    "
         "|       0       |       1       "
         "|       2       |       3       |\n"
         "        "
         "|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7"
         "|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|\n");

  for (int i = 0; i < byte_lines; i++) {
    printf("%s\n", break_line.c_str());
    int offset = 4 * i;
    int bytes_in_line = 4;
    if (i + 1 == byte_lines && buffer_size % bytes_per_line != 0) {
      bytes_in_line = buffer_size % bytes_per_line;
    }
    const int kBufferSize = 100;
    char str_buf[kBufferSize];
    int byte_num = i * 4;
    snprintf(str_buf, buffer_size, "%8d", byte_num);
    printf("%s", str_buf);
    for (int j = 0; j < bytes_in_line; j++) {
      unsigned char word = *(buffer + offset + j);
      if (show_ascii) {
        snprintf(str_buf, buffer_size, "|%#11x (%c)", word, word);
      } else {
        snprintf(str_buf, buffer_size, "|%#15x", word);
      }
      printf("%s", str_buf);
    }
    printf("|\n");
  }
  printf("%s\n", break_line.c_str());
  printf("Total Bytes: %d\n", buffer_size);
}

// Writes a block of memory to a give file descriptor.
void WriteBlock(int fd, char* buffer, int buffer_size, int debug_packets) {
  if (debug_packets) {
    printf("Write:\n");
    PrintBuffer(buffer, buffer_size, true);
  }
  int total_bytes_written = 0;
  while (total_bytes_written != buffer_size) {
    int bytes_written = write(fd,
                              buffer + total_bytes_written,
                              buffer_size - total_bytes_written);
    // Write syscall failed.
    if (bytes_written < 0) {
      printf("Attempted write size %d\n", buffer_size - total_bytes_written);
      string sys_error = string(strerror(errno));
      LOG_FATAL("Write syscall failed: " + sys_error);
    }
    total_bytes_written += bytes_written;
  }
  // Make sure all bytes have been written to the fd.
  if (total_bytes_written < buffer_size) {
    LOG_FATAL("Write loop exited before all bytes were written."
              "This should never happen");
  }
}

// Reads a block of memory to a give file descriptor.
void ReadBlock(int fd, char* buffer, int buffer_size, bool debug_packets) {
  int total_bytes_read = 0;
  while (total_bytes_read != buffer_size) {
    int bytes_read = read(fd,
                             buffer + total_bytes_read,
                             buffer_size - total_bytes_read);
    // Read syscall failed.
    if (bytes_read < 0) {
      string sys_error = string(strerror(errno));
      LOG_FATAL("Read syscall failed: " + sys_error);
    }
    total_bytes_read += bytes_read;
  }

  // Make sure all bytes have been read from the fd.
  if (total_bytes_read < buffer_size) {
    LOG_FATAL("Read loop exited before all bytes were written."
              "This should never happen");
  }
  if (debug_packets) {
    printf("Read:\n");
    PrintBuffer(buffer, buffer_size, true);
  }
}

Connection::Connection(ConnectionType connection_type, bool debug_packets)
    : connection_type_(connection_type),
      debug_packets_(debug_packets) {}

int Connection::GetSocketFd() {
  return sock_;
}

// Opens a TCP socket to the specified address for the connection
// on the specified port. If |disable_nagels| is true, will
// prevent TCP batching.
void Connection::OpenTcpSocket(const string& ip_address,
                               int port,
                               bool disable_nagles) {
  // Create a new socket.
  sock_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_ < 0) {
    LOG_FATAL("ERROR: Couldn't create a socket");
  }

  struct sockaddr_in server_info;
  // Use IPv4.
  server_info.sin_family = AF_INET;

  // Convert IP address to network order
  if (inet_pton(AF_INET,
                ip_address.c_str(),
                &server_info.sin_addr.s_addr) < 0) {
    LOG_FATAL("IP address error");
  }

  // Connect to the supplied port.
  server_info.sin_port = htons(port);
  int error = connect(sock_,
                      reinterpret_cast<struct sockaddr*>(&server_info),
                      sizeof(server_info));
  if (error < 0) {
    printf("Connection error: %s\n", strerror(errno));
    LOG_FATAL("Connection error");
  }

  // Disable nagles algorithm preventing batching.
  if (disable_nagles) {
    int flag = 1;
    int err = setsockopt(sock_,
                         IPPROTO_TCP,
                         TCP_NODELAY,
                         reinterpret_cast<char*>(&flag),
                         sizeof(int));
    if (err < 0) {
      LOG_FATAL("Couldn't set tcp_nodelay");
    }
  }
}

// Receive a response over the connection.
Response* Connection::ReceiveResponse() {
  ResponseHeader response_header;
  ReadBlock(sock_,
            reinterpret_cast<char*>(&response_header),
            sizeof(ResponseHeader),
            debug_packets_);
  if (response_header.magic != kMagicResponse) {
    LOG_FATAL("On read Incorrect magic number.");
  }

  int extras_size = static_cast<int>(response_header.extras_size);
  scoped_array<char> extras(new char[extras_size]);

  int key_size = 0;
  key_size |= response_header.key_size[1];
  key_size |= response_header.key_size[0] << 8;
  scoped_array<char> key(new char[key_size]);

  int body_size = 0;
  body_size |= response_header.total_body_size[3] & 0xFF;
  body_size |= (response_header.total_body_size[2] & 0xFF) << 8;
  body_size |= (response_header.total_body_size[1] & 0xFF) << 16;
  body_size |= (response_header.total_body_size[0] & 0xFF) << 24;
  int value_size = body_size - key_size - extras_size;
  scoped_array<char> value(new char[value_size]);

  ReadBlock(sock_, extras.Get(), extras_size, debug_packets_);
  ReadBlock(sock_, key.Get(), key_size, debug_packets_);
  ReadBlock(sock_, value.Get(), value_size, debug_packets_);

  Response* response = Response::CreateResponseFromHeader(response_header);

  return response;
}

// Send a request over the connection.
void Connection::SendRequest(Request* request) {
  int request_size_bytes;
  scoped_array<char> request_buffer(
                     request->ConstructRequestPacket(&request_size_bytes));
  WriteBlock(sock_,
             request_buffer.Get(),
             request_size_bytes,
             debug_packets_);
}

}  // namespace cachebash
