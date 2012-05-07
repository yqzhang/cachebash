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
// worker_thread.h
// David Meisner (davidmax@gmail.com)
//

#ifndef WORKER_THREAD_H_
#define WORKER_THREAD_H_

#include "cachebash/worker_thread.h"

#include <pthread.h>
#include <event2/event.h>
#include <queue>

#include "cachebash/request.h"

using std::queue;

namespace cachebash {

class Config;
class Connection;
class Generator;
class Response;
class StatisticsCollection;

enum WorkerThreadState {
  WARM_UP,
  STEADY_STATE_LOADING
};

class WorkerThread {
 public:
  WorkerThread(Config* config,
               Generator* generator,
               StatisticsCollection* statistics_collection);
  virtual ~WorkerThread();
  StatisticsCollection* GetStatisticsCollection();
  void Init();
  // void WarmUpReceiveCallback();
  // void WarmUpSendCallback();
  virtual void ReceiveCallback();
  virtual void SendCallback();
  void SendRequest(Request* request);
  Response* ReceiveResponse();
  void MainLoop();
  void Start();

 protected:
  Config* config_;
  Generator* generator_;
  StatisticsCollection* statistics_collection_;
  queue<Request*> request_queue_;
  struct event_base* event_base_;

 private:
  Connection* connection_;
  struct timeval last_receive_time_;
  struct timeval last_send_time_;
  bool last_send_time_valid_;
  // Each WorkerThread has its own StatisticsCollection.
  pthread_t* thread_;

  DISALLOW_COPY_AND_ASSIGN(WorkerThread);
};

class WarmupSequence;

class WarmupWorkerThread : public WorkerThread {
 public:
  WarmupWorkerThread(Config* config,
                     WarmupSequence* warmup_sequence);
  //virtual ~WarmupWorkerThread();
  virtual void SendCallback();
  virtual void ReceiveCallback();

 private:
  WarmupSequence* warmup_sequence_;

  DISALLOW_COPY_AND_ASSIGN(WarmupWorkerThread);
};

// Hooks to interface between functional calls in pthreads or libevent
// to WorkerThread object member functions.
void SendCallbackHook(int fd, short event_type, void* args);
void ReceiveCallbackHook(int fd, short event_type, void* args);
void* MainLoopHook(void* arg);

}  // namespace cachebash

#endif  // WORKER_THREAD_H_
