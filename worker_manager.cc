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
// worker_manager.cc
// David Meisner (davidmax@gmail.com)
//

#include "cachebash/worker_manager.h"

#include "cachebash/config.h"
#include "cachebash/worker_thread.h"

namespace cachebash {

WorkerManager::WorkerManager(Generator* generator,
                             Config* config)
    : generator_(generator),
      config_(config) {}

void WorkerManager::CreateAndInitializeWorkerThreads(
       const StatisticsCollection& base_collection) {

  // TODO(davidmax@gmail.com) figure out CPU affinity.
  // #ifdef __gnu_linux__
  // int n_cpus = sysconf(_SC_NPROCESSORS_ONLN);
  // #else
  // // TODO(davidmax@gmail.com) This should probably be one.
  // int n_cpus = 2;
  // #endif

  for (int i = 0; i < config_->n_worker_threads_; i++) {
    printf("Creating thread %d\n", i);
    WorkerThread* worker_thread = new WorkerThread(config_,
                                                   generator_,
                                                   base_collection.Copy());
    worker_thread->Init();
    worker_threads_.push_back(worker_thread);
  }
}

void WorkerManager::Warmup() {
  printf("Warming up...");
  WarmupWorkerThread warmup_worker_thread(config_, config_->warmup_sequence_);
  warmup_worker_thread.Init();
  warmup_worker_thread.MainLoop();
  printf("... done warming up\n");
}

void WorkerManager::StartWorkerThreads() {
  int i = 0;
  for (vector<WorkerThread*>::iterator it = worker_threads_.begin();
       it != worker_threads_.end();
       it++) {
         i++;
         (*it)->Start();
       }
  bool running = true;
  while (running) {
    sleep(1);
  }
}

vector<WorkerThread*>* WorkerManager::worker_threads() {
  return &worker_threads_;
}

}  // namespace
