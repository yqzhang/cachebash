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
//  cachebash.cc
//  David Meisner (davidmax@gmail.com)
//
// Protocol based on http://code.google.com/p/memcached/wiki/MemcacheBinaryProtocol

#include <stdio.h>
#include <string>

#include "cachebash/config.h"
#include "cachebash/connection.h"
#include "cachebash/generator.h"
#include "cachebash/request.h"
#include "cachebash/response.h"
#include "cachebash/size_key_distribution.h"
#include "cachebash/scoped_ptr.h"
#include "cachebash/statistic.h"
#include "cachebash/statistic_manager.h"
#include "cachebash/util.h"
#include "cachebash/warmup_sequence.h"
#include "cachebash/worker_thread.h"
#include "cachebash/worker_manager.h"

namespace cachebash {

void PrintUsage() {
  printf("usage: loader [-option]\n"
    "     [-c arg  connections per worker]\n"
    "     [-d enable packet debugging]\n"
    "     [-f arg  size/key object distribution file]\n"
    "     [-F arg  fixed object size]\n"
    "     [-g arg  fraction of requests that are gets (The rest are sets)]\n"
    "     [-h prints this message]\n"
    "     [-l arg use a fixed number of gets per multiget]\n"
    "     [-m arg fraction of requests that are multiget]\n"
    "     [-n enable naggle's algorithm]\n"
    "     [-r ATTEMPTED requests per second (default: max out rps)]\n"
    "     [-s server to load]\n"
    "     [-t arg  runtime of loadtesting in seconds (default: run forever)]\n"
    "     [-T arg  interval between stats printing (default: 1)]\n"
    "     [-w number of worker threads]\n");
}

void ParseArguments(int argc, char** argv, Config* config) {
  int c;
  while ((c = getopt(argc, argv, "c:dg:hf:F:l:m:nr:s:t:T:w:")) != -1) {
    switch (c) {
      case 'c':
        config->n_connections_per_worker_ = atoi(optarg);
        break;
      case 'd':
        config->debug_ = true;
        break;
      case 'f':
        config->size_key_distribution_
          = SizeKeyDistribution::LoadFile(string(optarg));
        // TODO(davidmax@gmail.com) There's probably a better way to do this.
        config->warmup_sequence_
          = new WarmupSequence(*config->size_key_distribution_);
        break;
      case 'F':
        config->fixed_object_size_ = atoi(optarg);
        break;
      case 'g':
        config->fraction_gets_ = atof(optarg);
        break;
      case 'h':
        PrintUsage();
        exit(0);
      case 'l':
        config->multiget_n_gets_ = atoi(optarg);
        break;
      case 'm':
        config->fraction_multiget_ = atof(optarg);
        break;
      case 'n':
        config->use_naggles_ = true;
        break;
      case 'r':
        config->rps_ = atof(optarg);
        break;
      case 's':
        config->server_ip_address_ = nslookup(string(optarg));
        break;
      case 't':
        config->runtime_ = atof(optarg);
        break;
      case 'T':
        config->stat_print_interval_ = atof(optarg);
        break;
      case 'w':
        config->n_worker_threads_ = atoi(optarg);
        break;
    }
  }
}

void CacheBash(int argc, char** argv) {
  printf("\ncachebash - a memcached loadtester\n"
         "David Meisner (davidmax@gmail.com)\n"
         "University of Michigan\n\n");

  Config config;
  ParseArguments(argc, argv, &config);
  config.Print();

  Generator generator(&config);

  WorkerManager worker_manager(&generator,
                               &config);

  StatisticsCollection base_collection(&config);

  base_collection.RegisterStatistic("get_requests", false);
  base_collection.AddStatisticPrinter("get_requests", new CountPrinter());

  base_collection.RegisterStatistic("get_request_size", false);
  base_collection.AddStatisticPrinter("get_request_size",
                                      new AveragePrinter());
  base_collection.AddStatisticPrinter("get_request_size", new MinPrinter());
  base_collection.AddStatisticPrinter("get_request_size", new MaxPrinter());

  base_collection.RegisterStatistic("set_requests", false);
  base_collection.AddStatisticPrinter("set_requests", new CountPrinter());

  base_collection.RegisterStatistic("set_request_size", false);
  base_collection.AddStatisticPrinter("set_request_size",
                                      new AveragePrinter());
  base_collection.AddStatisticPrinter("set_request_size", new MinPrinter());
  base_collection.AddStatisticPrinter("set_request_size", new MaxPrinter());

  base_collection.RegisterStatistic("latency", false);
  base_collection.AddStatisticPrinter("latency", new AveragePrinter());
  base_collection.AddStatisticPrinter("latency", new QuantilePrinter(0.50));
  base_collection.AddStatisticPrinter("latency", new QuantilePrinter(0.90));
  base_collection.AddStatisticPrinter("latency", new QuantilePrinter(0.95));
  base_collection.AddStatisticPrinter("latency", new QuantilePrinter(0.99));

  StatisticManager statistic_manager(&base_collection,
                                       &config,
                                       &worker_manager);
  worker_manager.CreateAndInitializeWorkerThreads(base_collection);

  // Perform a warmup with only one thread.
  worker_manager.Warmup();

  // Now that everything is setup, we can start.
  worker_manager.StartWorkerThreads();
  statistic_manager.StatisticsLoop();
}

}  // namespace cachebash

int main(int argc, char** argv) {
  cachebash::CacheBash(argc, argv);
  return 0;
}
