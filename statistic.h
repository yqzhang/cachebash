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
// statistic.h
// David Meisner (davidmax@gmail.com)
//

#ifndef STATISTIC_H_
#define STATISTIC_H_

#include <map>
#include <string>
#include <vector>

#include "cachebash/util.h"

using std::string;
using std::vector;
using std::map;
using std::pair;

namespace cachebash {

static const int kBinsPerHistogram = 1000;

class Config;
class Statistic;
class StatisticPrinter;
class StatisticsManager;
class WorkerManager;

class Histogram {
 public:
  Histogram(float min, float max, int n_bins);
  virtual ~Histogram();
  Histogram* Copy() const;
  void AddSample(float value);
  float GetQuantile(float quantile) const;
  int* histogram() const { return histogram_; }
  float max() const { return max_; }
  void MergeWithHistogram(const Histogram& histogram);
  float min() const { return min_; }
  int n_bins() const { return n_bins_; }
  int n_samples() const { return n_samples_; }
  void Reset();

 private:
  int* histogram_;
  float max_;
  float min_;
  int n_bins_;
  int n_samples_;

  DISALLOW_COPY_AND_ASSIGN(Histogram);
};

class Statistic {
 public:
  Statistic(string name, bool cummulative);
  virtual ~Statistic();
  void AddSample(float value);
  void AddStatisticPrinter(StatisticPrinter* statistic_printer);
  Statistic* Copy() const;
  float GetAverage();
  int GetCount();
  float GetQuantile(float quantile);
  float GetMin() const { return min_; }
  float GetMax() const { return max_; }
  string GetName();
  float GetSampleStandardDeviation();
  float GetStandardDeviation();
  void MergeWithStatistic(const Statistic& statistic);
  bool IsCummulative();
  void Print();
  void Reset();

 protected:
  string name_;
  float s0_;
  float s1_;
  float s2_;
  float min_;
  float max_;
  // TODO(davidmax@gmail.com) At some point,
  // should change this to a specialized subclass.
  bool cummulative_;

  Histogram* microsecond_histogram_;
  Histogram* millisecond_histogram_;
  Histogram* second_histogram_;
  vector<StatisticPrinter*> statistic_printers_;

 private:
  DISALLOW_COPY_AND_ASSIGN(Statistic);
};

class StatisticPrinter {
 public:
  StatisticPrinter() {}
  virtual ~StatisticPrinter() {}
  virtual void Print(Statistic* statistic) = 0;
  virtual StatisticPrinter* Copy() = 0;

 private:
  Statistic* statistic_;
  DISALLOW_COPY_AND_ASSIGN(StatisticPrinter);
};

class AveragePrinter :  public StatisticPrinter {
 public:
  AveragePrinter();
  virtual void Print(Statistic* statistic);
  virtual StatisticPrinter* Copy();

 private:
  DISALLOW_COPY_AND_ASSIGN(AveragePrinter);
};

class QuantilePrinter : public StatisticPrinter {
 public:
  explicit QuantilePrinter(float quantile);
  virtual void Print(Statistic* statistic);
  virtual StatisticPrinter* Copy();

 private:
  float quantile_;
  DISALLOW_COPY_AND_ASSIGN(QuantilePrinter);
};

class MinPrinter : public StatisticPrinter {
 public:
  MinPrinter();
  virtual void Print(Statistic* statistic);
  virtual StatisticPrinter* Copy();

 private:
  DISALLOW_COPY_AND_ASSIGN(MinPrinter);
};

class MaxPrinter : public StatisticPrinter {
 public:
  MaxPrinter();
  virtual void Print(Statistic* statistic);
  virtual StatisticPrinter* Copy();

 private:
  DISALLOW_COPY_AND_ASSIGN(MaxPrinter);
};

class CountPrinter : public StatisticPrinter {
 public:
  CountPrinter();
  virtual void Print(Statistic* statistic);
  virtual StatisticPrinter* Copy();

 private:
  DISALLOW_COPY_AND_ASSIGN(CountPrinter);
};

class StatisticsCollection {
 public:
  explicit StatisticsCollection(Config* config);
  virtual ~StatisticsCollection();
  void AddSample(string name, float value);
  void AddStatisticPrinter(string name,
                           StatisticPrinter* statistic_printer);
  StatisticsCollection* Copy() const;
  Statistic* GetStatistic(string name) const;
  void MergeWithStatisticsCollection(
          const StatisticsCollection& statistics_collection);
  void PrintStatInterval();
  void RegisterStatistic(string name, bool cummulative);
  void ResetNonCummulativeStatistics();

 protected:
  void AddStatistic(Statistic* statistic);

 private:
  Config* config_;
  map<string, Statistic*> statistics_;

  DISALLOW_COPY_AND_ASSIGN(StatisticsCollection);
};

class StatisticsManager {
 public:
  StatisticsManager(StatisticsCollection* base_collection,
                    Config* config,
                    WorkerManager* worker_manager);
  void StatisticsLoop();

 private:
  StatisticsCollection* base_collection_;
  Config* config_;
  WorkerManager* worker_manager_;

  DISALLOW_COPY_AND_ASSIGN(StatisticsManager);
};

}  // namespace

#endif  // STATISTIC_H_
