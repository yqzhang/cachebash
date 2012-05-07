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
// worker_thread.cc
// David Meisner (davidmax@gmail.com)
//

#include "cachebash/worker_thread.h"

#include <sched.h>
#include <stdio.h>

#include "cachebash/config.h"
#include "cachebash/connection.h"
#include "cachebash/generator.h"
#include "cachebash/request.h"
#include "cachebash/response.h"
#include "cachebash/statistic.h"
#include "cachebash/scoped_ptr.h"
#include "cachebash/util.h"
#include "cachebash/warmup_sequence.h"

namespace cachebash {

// Construct a WorkerThread.
// |cpu_id| - The cpu number (starting at 0) to bind the thread to.
WorkerThread::WorkerThread(Config* config,
                           Generator* generator,
                           StatisticsCollection* statistics_collection)
    : config_(config),
      generator_(generator),
      statistics_collection_(statistics_collection),
      event_base_(event_base_new()),
      connection_(new Connection(TCP, config->debug_)),
      last_send_time_valid_(false),
      thread_(new pthread_t()) {}

void WorkerThread::Init() {
  connection_->OpenTcpSocket(config_->server_ip_address_,
                             11211,
                             !config_->use_naggles_);
  // Set CPU affinity. This doesn't work on mac os x, so check
  // that we're running GNU Linux.
  // Can check macros with gcc -E -dM - </dev/null
  #ifdef __gnu_linux__
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(num, &cpuset);
  int s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
  if (s != 0) {
    LOG_FATAL("Couldn't set CPU affinity");
  }
  #endif
}

WorkerThread::~WorkerThread() {
  delete connection_;
  delete thread_;
}

StatisticsCollection* WorkerThread::GetStatisticsCollection() {
  return statistics_collection_;
}

// Create and initiate a new thread represented by the WorkerThread object.
void WorkerThread::Start() {
  int rc = pthread_create(thread_, NULL, MainLoopHook, this);
  if (rc) {
    LOG_FATAL("Thread failed to start");
  }
}

// Interfaces between libevent send callback and WorkerThread
// object's send functionality.
void SendCallbackHook(int fd, short event_type, void* args) {
  WorkerThread* worker_thread = static_cast<WorkerThread*>(args);
  worker_thread->SendCallback();
}

// Interfaces between libevent read callback and WorkerThread
// object's receive functionality.
void ReceiveCallbackHook(int fd, short event_type, void* args) {
  //WorkerThread* worker_thread = static_cast<WorkerThread*>(args);
  WorkerThread* worker_thread = static_cast<WorkerThread*>(args);
  worker_thread->ReceiveCallback();
}

// Interfaces between pthread's thread creation callback and WorkerThread
// object's main loop.
void* MainLoopHook(void* arg) {
  WorkerThread* worker_thread = static_cast<WorkerThread*>(arg);
  worker_thread->MainLoop();
  return NULL;
}

void WorkerThread::SendCallback() {
  // If a rps value has not been specified,
  // send requests as quickly as possible.
  float intersend_time = 0.0;
  // Send requests equally far apart to meet a rps target.
  if (config_->rps_ > 0) {
    intersend_time = (1 / config_->rps_) / config_->n_worker_threads_;
  }

  // Check if the inter-send time has been exceeded
  struct timeval timestamp, time_diff;
  gettimeofday(&timestamp, NULL);
  timersub(&timestamp, &last_send_time_, &time_diff);
  double time_since_last_call = time_diff.tv_usec * 1e-6  + time_diff.tv_sec;

  // If the time has not been exceeded just return,
  // this function will soon be called again.
  if (time_since_last_call < intersend_time && last_send_time_valid_) {
    return;
  }

  // We are now ready to generate and send a request.
  Request* request  = generator_->GenerateNextRequest();
  SendRequest(request);
}

void WorkerThread::SendRequest(Request* request) {
  struct timeval timestamp;
  gettimeofday(&timestamp, NULL);
  request->set_send_time(timestamp);

  if (config_->debug_) {
    request->Print();
  }

  connection_->SendRequest(request);
  request_queue_.push(request);
  last_send_time_ = timestamp;
  last_send_time_valid_ = true;
}

// void WorkerThread::HandleSendCallback() {
//   SendCallback();
// }

Response* WorkerThread::ReceiveResponse() {
  // Get the request that corresponds to this request.
  Request* request = request_queue_.front();
  request_queue_.pop();
  Response* response = connection_->ReceiveResponse();
  response->set_request(request);

  // Determine how long the request took.
  struct timeval timestamp, time_diff;
  gettimeofday(&timestamp, NULL);
  struct timeval send_time = request->send_time();
  timersub(&timestamp, &send_time, &time_diff);
  double request_latency = time_diff.tv_usec * 1e-6  + time_diff.tv_sec;
  response->set_request_latency(request_latency);
  return response;
}

void WorkerThread::ReceiveCallback() {
  scoped_ptr<Response> response(ReceiveResponse());
  statistics_collection_->AddSample("latency", response->request_latency());
  response->request()->UpdateStatistics(statistics_collection_);
}

// void WorkerThread::HandleReceiveCallback() {
//   ReceiveCallback();
// }

void WorkerThread::MainLoop() {
  // Register a send callback per Connection.
  struct event* send_event = event_new(event_base_,
                                       connection_->GetSocketFd(),
                                       EV_WRITE | EV_PERSIST,
                                       SendCallbackHook,
                                       this);
  // Lower priorities are serviced first.
  // Give send less of a priority than receive.
  event_priority_set(send_event, 2);
  event_add(send_event, NULL);

  // Register a receive callback per Connection.
  struct event* receive_event = event_new(event_base_,
                                          connection_->GetSocketFd(),
                                          EV_READ | EV_PERSIST,
                                          ReceiveCallbackHook,
                                          this);
  event_priority_set(receive_event, 1);
  event_add(receive_event, NULL);

  // Start the main event loop.
  printf("starting receive base loop\n");
  int error = event_base_loop(event_base_, 0);
  if (error == -1) {
    LOG_FATAL("Error starting libevent");
  } else if (error == 1) {
    LOG_FATAL("No events registered with libevent");
  }
}

WarmupWorkerThread::WarmupWorkerThread(Config* config,
                                       WarmupSequence* warmup_sequence)
    : WorkerThread(config,
                   NULL,
                   NULL),
                   warmup_sequence_(warmup_sequence) {}

void WarmupWorkerThread::SendCallback() {
  if (warmup_sequence_->HasNext()) {
    return;
  }

  SizeKeyEntry size_key_entry = warmup_sequence_->Next();
  string key = size_key_entry.key;
  string value = Generator::GenerateRandomString(size_key_entry.size);
  Request* request = new SetRequest(key, value);
  printf("Generating request\n");
  SendRequest(request);
}

void WarmupWorkerThread::ReceiveCallback() {
  scoped_ptr<Response> response(ReceiveResponse());
  // Check if we are now down with warmup.
  if (warmup_sequence_->HasNext() && request_queue_.empty()) {
    event_base_loopbreak(event_base_);
  }
}

}  // namespace cachebash
