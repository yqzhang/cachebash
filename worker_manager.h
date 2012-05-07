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
// worker_manager.h
// David Meisner (davidmax@gmail.com)
//

#ifndef WORKER_MANAGER_H_
#define WORKER_MANAGER_H_

#include <vector>

#include "cachebash/util.h"

using std::vector;

namespace cachebash {

class Config;
class Generator;
class StatisticsCollection;
class WorkerThread;

class WorkerManager {
 public:
  // TODO(davidmax@gmail) can any of these be const refs?
  WorkerManager(Generator* generator, Config* config);
  void CreateAndInitializeWorkerThreads(
          const StatisticsCollection& base_collection);
  void StartWorkerThreads();
  vector<WorkerThread*>* worker_threads();
  void Warmup();

 private:
  vector<WorkerThread*> worker_threads_;
  Generator* generator_;
  Config* config_;

  DISALLOW_COPY_AND_ASSIGN(WorkerManager);
};

}  // namespace

#endif  // WORKER_MANAGER_H_
