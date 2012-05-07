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

#include "cachebash/statistic.h"

#include <math.h>
#include <sys/time.h>
#include <algorithm>
#include <limits>
#include <map>
#include <string>
#include <utility>

#include "cachebash/config.h"
#include "cachebash/scoped_ptr.h"
#include "cachebash/util.h"
#include "cachebash/worker_thread.h"
#include "cachebash/worker_manager.h"

using std::max;
using std::min;

namespace cachebash {

AveragePrinter::AveragePrinter() {}

void AveragePrinter::Print(Statistic* statistic) {
  printf("Avg: %f ", statistic->GetAverage());
}

StatisticPrinter* AveragePrinter::Copy() {
  return new AveragePrinter();
}

QuantilePrinter::QuantilePrinter(float quantile) : quantile_(quantile) {}

void QuantilePrinter::Print(Statistic* statistic) {
  printf("%.3fth: %.3f ", quantile_, statistic->GetQuantile(quantile_));
}

StatisticPrinter* QuantilePrinter::Copy() {
  return new QuantilePrinter(quantile_);
}

MaxPrinter::MaxPrinter() {}

void MaxPrinter::Print(Statistic* statistic) {
  printf("Max: %f ", statistic->GetMax());
}

StatisticPrinter* MaxPrinter::Copy() {
  return new MaxPrinter();
}

MinPrinter::MinPrinter() {}

void MinPrinter::Print(Statistic* statistic) {
  printf("Min: %f ", statistic->GetMin());
}

StatisticPrinter* MinPrinter::Copy() {
  return new MinPrinter();
}

CountPrinter::CountPrinter() {}

void CountPrinter::Print(Statistic* statistic) {
  printf("Count: %d ", statistic->GetCount());
}

StatisticPrinter* CountPrinter::Copy() {
  return new CountPrinter();
}

// Statistic functions.

Statistic::Statistic(string name, bool cummulative)
    : name_(name),
      s0_(0.0),
      s1_(0.0),
      s2_(0.0),
      min_(std::numeric_limits<float>::max()),
      max_(-std::numeric_limits<float>::max()),
      cummulative_(cummulative),
      microsecond_histogram_(new Histogram(0.0, 1e-3, kBinsPerHistogram)),
      millisecond_histogram_(new Histogram(1e-3, 1.0, kBinsPerHistogram)),
      second_histogram_(new Histogram(1.0, 1000, kBinsPerHistogram)) {}

Statistic::~Statistic() {
  for (vector<StatisticPrinter*>::iterator it = statistic_printers_.begin();
       it != statistic_printers_.end();
       it++) {
         delete (*it);
       }
}

void Statistic::AddStatisticPrinter(StatisticPrinter* statistic_printer) {
  statistic_printers_.push_back(statistic_printer);
}

void Statistic::Reset() {
  s0_ = 0.0;
  s1_ = 0.0;
  s2_ = 0.0;
  min_ = std::numeric_limits<float>::max();
  max_ = -std::numeric_limits<float>::max();
  microsecond_histogram_->Reset();
  millisecond_histogram_->Reset();
  second_histogram_->Reset();
}

bool Statistic::IsCummulative() {
  return cummulative_;
}

void Statistic::Print() {
  printf("%s - ", name_.c_str());
  for (vector<StatisticPrinter*>::iterator it = statistic_printers_.begin();
       it != statistic_printers_.end();
       it++) {
    (*it)->Print(this);
  }
}

// Distributions currently do not support negative numbers.
void Statistic::AddSample(float value) {
  s0_ += 1.0;
  s1_ += value;
  s2_ += value * value;
  min_ = min(min_, value);
  max_ = max(max_, value);

  // TODO(davidmax@gmail.com) Support negative numbers.
  if ( value < 0.0 ) {
    return;
  }
  // TODO(davidmax@gmail.com) Add tracking for dropped samples?
  // I.e., too big, too small.
  // TODO(davidmax@gmail.com) Consider replacing with InRange(value).
  if (value < 1e-3) {
    microsecond_histogram_->AddSample(value);
  } else if (value >= 1e-3 && value < 1.0) {
    millisecond_histogram_->AddSample(value);
  } else if (value >= 1.0 && value < 1000) {
    second_histogram_->AddSample(value);
  } else {
    // TODO(davidmax@gmail.com) Remove this.
    printf("value: %f\n", value);
    LOG_FATAL("Unhandled Statistic value");
  }
}


// Create a deep copy of the Statistic.
Statistic* Statistic::Copy() const {
  Statistic* statistic = new Statistic(name_, cummulative_);
  statistic->s0_ = s0_;
  statistic->s1_ = s1_;
  statistic->s2_ = s2_;
  statistic->min_ = min_;
  statistic->max_ = max_;

  statistic->microsecond_histogram_ = microsecond_histogram_->Copy();
  statistic->millisecond_histogram_ = millisecond_histogram_->Copy();
  statistic->second_histogram_ = second_histogram_->Copy();

  for (vector<StatisticPrinter*>::const_iterator
         it = statistic_printers_.begin();
       it != statistic_printers_.end();
       it++) {
    statistic->statistic_printers_.push_back((*it)->Copy());
  }
  return statistic;
}

int Statistic::GetCount() {
  return s0_;
}

float Statistic::GetQuantile(float quantile) {
  // TODO(davidmax@gmail.com) Need better testing of quantile code.
  // Still not convinced this is exactly correct.
  if (quantile < 0.0 || quantile > 1.0) {
    LOG_FATAL("Invalid quantile argument");
  }
  int n_samples_needed = quantile * s0_;
  // We can bypass a histogram if we know we need more samples than it has.
  int n_samples = 0;

  // Look at the microsecond histogram.
  // We still need more samples even after adding this histogram's.
  if (n_samples_needed > n_samples + microsecond_histogram_->n_samples()) {
    n_samples += microsecond_histogram_->n_samples();
  // The quantile falls in this histogram.
  } else {
    for (int i = 0; i < microsecond_histogram_->n_bins(); i++) {
      n_samples += microsecond_histogram_->histogram()[i];
      if (n_samples >= n_samples_needed) {
        return i * microsecond_histogram_->max()
               / static_cast<float>(microsecond_histogram_->n_bins());
      }
    }
  }
  // Look at the millisecond histogram.
  // We still need more samples even after adding this histogram's.
  if (n_samples_needed > n_samples + millisecond_histogram_->n_samples()) {
    n_samples += millisecond_histogram_->n_samples();
  // The quantile falls in this histogram.
  } else {
    for (int i = 0; i < millisecond_histogram_->n_bins(); i++) {
      n_samples += millisecond_histogram_->histogram()[i];
      if (n_samples >= n_samples_needed) {
        return i * millisecond_histogram_->max()
               / static_cast<float>(millisecond_histogram_->n_bins());
      }
    }
  }
  // Look at the second histogram.
  // We still need more samples even after adding this histogram's.
  if (n_samples_needed > n_samples + second_histogram_->n_samples()) {
    n_samples += second_histogram_->n_samples();
  // The quantile falls in this histogram.
  } else {
    for (int i = 0; i < second_histogram_->n_bins(); i++) {
      n_samples += second_histogram_->histogram()[i];
      if (n_samples >= n_samples_needed) {
        return i * second_histogram_->max()
               / static_cast<float>(second_histogram_->n_bins());
      }
    }
  }

  return (second_histogram_->n_bins() - 1) * second_histogram_->max()
           / static_cast<float>(second_histogram_->n_bins());
}

string Statistic::GetName() {
  return name_;
}

// Calculates the average values of the statistic.
float Statistic::GetAverage() {
  if (s0_ == 0) {
    return 0.0;
  } else {
    return s1_ / s0_;
  }
}

float Statistic::GetSampleStandardDeviation() {
  return sqrt((s0_ * s2_ - s1_ * s1_) / (s0_ * (s0_ - 1)));
}

// Calculates the standard deviation of the statistc.
// Method from http://en.wikipedia.org/wiki/Standard_deviation
float Statistic::GetStandardDeviation() {
  return sqrt(s0_ * s2_ - s1_ * s1_) / s0_;
}

void Statistic::MergeWithStatistic(const Statistic& statistic) {
  // This doesn't check if both statistics are cummulative or not.
  s0_ = s0_ + statistic.s0_;
  s1_ = s1_ + statistic.s1_;
  s2_ = s2_ + statistic.s2_;
  min_ = min(min_, statistic.min_);
  max_ = max(max_, statistic.max_);
  microsecond_histogram_->MergeWithHistogram(*statistic.microsecond_histogram_);
  millisecond_histogram_->MergeWithHistogram(*statistic.millisecond_histogram_);
  second_histogram_->MergeWithHistogram(*statistic.microsecond_histogram_);
}

Histogram::Histogram(float min, float max, int n_bins)
  : histogram_(new int[n_bins]),
    max_(max),
    min_(min),
    n_bins_(n_bins),
    n_samples_(0) {
      memset(histogram_, 0, sizeof(int) * n_bins);
    }

Histogram::~Histogram() {
  delete[] histogram_;
}

Histogram* Histogram::Copy() const {
  Histogram* histogram = new Histogram(min_, max_, n_bins_);
  histogram->MergeWithHistogram(*this);
  return histogram;
}

void Histogram::AddSample(float value) {
  // Add the sample to the historgram.
  // Histograms are organized as follows:
  // Assume we have 10 bins and cover the interval [1e-3, 1).
  // Each bin might cover 999e-3/10 = 9.99e-2 = .0999 of space.
  // However, it's much easier to understand if we have each
  // cover .100 of space and truncate the first bin by .001.
  // i.e.,
  // bin  values
  // 0    [.001, .100)
  // 1    [.100, .200)
  // 2    [.200, .300)
  // ...
  // 9    [.900, 1.0)
  // With this scheme, the appropriate bin can be determined by:
  // bin = int(value / .100)
  //     = int(value / (max / num_bins))
  // Assuming 1e-3 <= value < 1.
  //
  // Another example:
  // 10 bins range [1e-6, 1e-3)
  // bin  values
  // 0    [1e-6, 1e-4)
  // 1    [1e-4, 2e-4)
  // 2    [2e-4, 3e-4)
  // ...
  // 9    [9e-4, 1e-3)
  // bin = int(value / 1e-3/10)
  // TODO(davidmax@gmail.com) Explains what happens with min = 0.
  if (value < min_ || value >= max_) {
    printf("Value: %f\n", value);
    LOG_FATAL("Unexpected value");
  }
  int bin = static_cast<int>(value / (max_ / static_cast<float>(n_bins_)));
  histogram_[bin]++;
  n_samples_++;
}

// TODO(davidmax@gmail.com) Consider adding interpolation.
float Histogram::GetQuantile(float quantile) const {
  if (quantile < 0.0 || quantile > 1.0) {
    LOG_FATAL("Invalid quantile argument");
  }
  int n_samples_needed = static_cast<int>(quantile * n_samples_);
  int n_samples = 0;
  int bin = 0;
  for (int i = 0; i < n_bins_; i++) {
    n_samples += histogram_[i];
    if (n_samples >= n_samples_needed) {
      bin = i;
      break;
    }
  }
  float x_value = bin * (max_ / static_cast<float>(n_bins_));
  return x_value;
}

void Histogram::MergeWithHistogram(const Histogram& histogram) {
  // TODO(davidmax@gmail.com) Make sure the histograms match.
  for (int i = 0; i < n_bins_; i++) {
    histogram_[i] += histogram.histogram()[i];
  }
  n_samples_ += histogram.n_samples();
}

void Histogram::Reset() {
  memset(histogram_, 0, sizeof(int) * n_bins_);
  n_samples_ = 0;
}

StatisticsCollection::StatisticsCollection(Config* config)
    : config_(config) {}

StatisticsCollection::~StatisticsCollection() {
  // Destroy all the statistics this StatisticsCollection was tracking.
  for (map<string, Statistic*>::iterator it = statistics_.begin();
       it != statistics_.end();
       it++) {
    delete it->second;
  }
}

void StatisticsCollection::AddSample(string name, float value) {
  Statistic* statistic = statistics_.find(name)->second;
  if (statistic == NULL) {
    LOG_FATAL("Tried to access an unregistered statistic");
  }
  statistic->AddSample(value);
}

// TODO(davidmax@gmail.com) Probably should be moved to stats manager.
void StatisticsCollection::AddStatisticPrinter(
                          string name,
                          StatisticPrinter* statistic_printer) {
  GetStatistic(name)->AddStatisticPrinter(statistic_printer);
}

// Creates a deep copy of the StatisticsManager object.
StatisticsCollection* StatisticsCollection::Copy() const {
  StatisticsCollection* statistics_collection
      = new StatisticsCollection(config_);
  for (map<string, Statistic*>::const_iterator it = statistics_.begin();
       it != statistics_.end();
       it++) {
    statistics_collection->AddStatistic(it->second->Copy());
  }
  return statistics_collection;
}

Statistic* StatisticsCollection::GetStatistic(string name) const {
  return statistics_.find(name)->second;
}

void StatisticsCollection::MergeWithStatisticsCollection(
                          const StatisticsCollection& statistics_collection) {
  for (map<string, Statistic*>::iterator it = statistics_.begin();
       it != statistics_.end();
       it++) {
    Statistic* statistic = statistics_collection.GetStatistic(it->first);
    if (statistic == NULL) {
      LOG_FATAL("Tried to merge statistic collection with non-matching"
                " sets of statistics");
    }
    it->second->MergeWithStatistic(*(statistic));
  }
}

void StatisticsCollection::PrintStatInterval() {
  printf("==============================\n");
  for (map<string, Statistic*>::iterator it = statistics_.begin();
       it != statistics_.end();
       it++) {
    Statistic* statistic = it->second;
    statistic->Print();
    printf("\n");
  }
  printf("\n");
}

void StatisticsCollection::RegisterStatistic(string name, bool cummulative) {
  statistics_.insert(pair<string, Statistic*>(name,
                                              new Statistic(name,
                                                            cummulative)));
}

void StatisticsCollection::ResetNonCummulativeStatistics() {
  for (map<string, Statistic*>::iterator it = statistics_.begin();
       it != statistics_.end();
       it++) {
    Statistic* statistic = it->second;
    if (!statistic->IsCummulative()) {
      statistic->Reset();
    }
  }
}

void StatisticsCollection::AddStatistic(Statistic* statistic) {
  statistics_.insert(pair<string, Statistic*>(statistic->GetName(),
                                             statistic));
}

}  // namespace cachebash
