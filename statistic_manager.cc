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
// statistic_manager.cc
// David Meisner (davidmax@gmail.com)
//

#include "cachebash/statistic_manager.h"

#include <sys/time.h>

#include "cachebash/config.h"
#include "cachebash/scoped_ptr.h"
#include "cachebash/statistic.h"
#include "cachebash/worker_manager.h"
#include "cachebash/worker_thread.h"

namespace cachebash {

StatisticManager::StatisticManager(StatisticsCollection* base_collection,
                                   Config* config,
                                   WorkerManager* worker_manager)
    : base_collection_(base_collection),
      config_(config),
      worker_manager_(worker_manager) {}

void StatisticManager::StatisticsLoop() {
  struct timeval start_time;
  gettimeofday(&start_time, NULL);
  while (1) {
    sleep(config_->stat_print_interval_);

    // Combine all the worker threads' statistics collections.
    scoped_ptr<StatisticsCollection> base_collection_copy(
                                    base_collection_->Copy());
    vector<WorkerThread*>* worker_threads = worker_manager_->worker_threads();
    for (vector<WorkerThread*>::const_iterator it = worker_threads->begin();
         it != worker_threads->end();
         it++) {
           StatisticsCollection* current_statistics_collection
                                = (*it)->GetStatisticsCollection();
           base_collection_copy->MergeWithStatisticsCollection(
                                      *current_statistics_collection);
           current_statistics_collection->ResetNonCummulativeStatistics();
         }

    base_collection_copy->PrintStatInterval();

    // Check if we've loadtested for long enough.
    struct timeval timestamp, time_diff;
    gettimeofday(&timestamp, NULL);
    timersub(&timestamp, &start_time, &time_diff);
    double elapsed_time = time_diff.tv_usec * 1e-6  + time_diff.tv_sec;
    if (config_->runtime_ > 0 && elapsed_time > config_->runtime_) {
      break;
    }
  }
}

}  // namespace
