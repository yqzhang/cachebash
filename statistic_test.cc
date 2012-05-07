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
//  statistic_test.cc
//  David Meisner (davidmax@gmail.com)
//

#include "cachebash/statistic.h"

#include <string>

#include "cachebash/scoped_ptr.h"
#include "gtest/gtest.h"

using cachebash::Histogram;
using cachebash::Statistic;

namespace {

class StatisticTest : public ::testing::Test {
 protected:
  StatisticTest() : statistic_("", false) {
  }

  virtual ~StatisticTest() {
  }

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }

  Statistic statistic_;
};

// Test average.
TEST_F(StatisticTest, Average) {
  float count = 0.0;
  float avg = 0.0;
  EXPECT_EQ(0.0, statistic_.GetAverage());

  statistic_.AddSample(1.0);
  avg += 1;
  count += 1.0;
  EXPECT_EQ(avg / count, statistic_.GetAverage());

  statistic_.AddSample(1.0);
  avg += 1;
  count += 1.0;
  EXPECT_EQ(avg / count, statistic_.GetAverage());

  statistic_.AddSample(-9.0);
  avg += -9.0;
  count += 1.0;
  EXPECT_EQ(avg / count, statistic_.GetAverage());
}

TEST_F(StatisticTest, Copy) {
  statistic_.AddSample(0.1);
  statistic_.AddSample(0.2);
  statistic_.AddSample(0.3);
  statistic_.AddSample(0.4);
  statistic_.AddSample(1);
  statistic_.AddSample(2);
  statistic_.AddSample(3);
  statistic_.AddSample(4);
  scoped_ptr<Statistic> statistic_copy(statistic_.Copy());
  EXPECT_EQ(statistic_copy->GetAverage(), statistic_.GetAverage());
  EXPECT_EQ(statistic_copy->GetStandardDeviation(),
            statistic_.GetStandardDeviation());
  EXPECT_EQ(statistic_copy->GetQuantile(0.25), statistic_.GetQuantile(0.25));
  EXPECT_EQ(statistic_copy->GetQuantile(0.75), statistic_.GetQuantile(0.75));
}

TEST_F(StatisticTest, Count) {
  EXPECT_EQ(0, statistic_.GetCount());
  statistic_.AddSample(0);
  EXPECT_EQ(1, statistic_.GetCount());
  statistic_.AddSample(1);
  EXPECT_EQ(2, statistic_.GetCount());
  statistic_.AddSample(2);
  EXPECT_EQ(3, statistic_.GetCount());
}

TEST_F(StatisticTest, GetQuantile) {
  statistic_.AddSample(1);
  statistic_.AddSample(2);
  statistic_.AddSample(2);
  statistic_.AddSample(2);
  statistic_.AddSample(3);
  statistic_.AddSample(3);
  statistic_.AddSample(3);
  statistic_.AddSample(3);
  EXPECT_NEAR(1, statistic_.GetQuantile(0.125), 1e-6);
  EXPECT_NEAR(2, statistic_.GetQuantile(0.5), 1e-6);
  EXPECT_NEAR(3, statistic_.GetQuantile(1.0), 1e-6);

  Statistic second_statistic("", false);
  second_statistic.AddSample(1);
  second_statistic.AddSample(2);
  EXPECT_NEAR(1, second_statistic.GetQuantile(0.5), 1e-1);
  EXPECT_NEAR(2, second_statistic.GetQuantile(1.0), 1e-1);
  second_statistic.AddSample(3);
  second_statistic.AddSample(4);
  EXPECT_NEAR(1, second_statistic.GetQuantile(0.25), 1e-1);
  EXPECT_NEAR(2, second_statistic.GetQuantile(0.5), 1e-1);
  EXPECT_NEAR(3, second_statistic.GetQuantile(0.75), 1e-1);
  EXPECT_NEAR(4, second_statistic.GetQuantile(1.0), 1e-1);
  second_statistic.AddSample(0.001);
  second_statistic.AddSample(0.002);
  second_statistic.AddSample(0.003);
  second_statistic.AddSample(0.004);
  EXPECT_NEAR(0.001, second_statistic.GetQuantile(0.125), 1e-4);
  EXPECT_NEAR(0.002, second_statistic.GetQuantile(0.25), 1e-4);
  EXPECT_NEAR(0.003, second_statistic.GetQuantile(0.375), 1e-4);
  EXPECT_NEAR(0.004, second_statistic.GetQuantile(0.5), 1e-4);
  second_statistic.AddSample(1e-6);
  second_statistic.AddSample(2e-6);
  second_statistic.AddSample(3e-6);
  second_statistic.AddSample(4e-6);
  second_statistic.AddSample(1e-6);
  second_statistic.AddSample(2e-6);
  second_statistic.AddSample(3e-6);
  second_statistic.AddSample(4e-6);
  EXPECT_NEAR(1e-6, second_statistic.GetQuantile(0.125), 1e-7);
  EXPECT_NEAR(2e-6, second_statistic.GetQuantile(0.25), 1e-7);
  EXPECT_NEAR(3e-6, second_statistic.GetQuantile(0.375), 1e-7);
  EXPECT_NEAR(4e-6, second_statistic.GetQuantile(0.5), 1e-7);
}

TEST_F(StatisticTest, GetSampleStandardDeviation) {
  statistic_.AddSample(1);
  statistic_.AddSample(1);
  EXPECT_NEAR(0.0, statistic_.GetSampleStandardDeviation(), .001);
  statistic_.AddSample(2);
  EXPECT_NEAR(0.577350, statistic_.GetSampleStandardDeviation(), .001);
  statistic_.AddSample(3);
  EXPECT_NEAR(0.957427, statistic_.GetSampleStandardDeviation(), .001);
}

TEST_F(StatisticTest, GetStandardDeviation) {
  statistic_.AddSample(1);
  statistic_.AddSample(1);
  EXPECT_NEAR(0.0, statistic_.GetStandardDeviation(), .001);
  statistic_.AddSample(2);
  EXPECT_NEAR(0.471404, statistic_.GetStandardDeviation(), .001);
  statistic_.AddSample(3);
  EXPECT_NEAR(0.829156, statistic_.GetStandardDeviation(), .001);
}

TEST_F(StatisticTest, Max) {
  statistic_.AddSample(-10);
  EXPECT_EQ(-10, statistic_.GetMax());
  statistic_.AddSample(0);
  EXPECT_EQ(0, statistic_.GetMax());
  statistic_.AddSample(100);
  EXPECT_EQ(100, statistic_.GetMax());
  statistic_.AddSample(10);
  EXPECT_EQ(100, statistic_.GetMax());
  statistic_.AddSample(100);
  EXPECT_EQ(100, statistic_.GetMax());
}

TEST_F(StatisticTest, EmptyStatistic) {
  // TODO(davidmax@gmail.com) Make sure min and max make sense.
  EXPECT_TRUE(false);
}

TEST_F(StatisticTest, MergeWithStatistic) {
  statistic_.AddSample(1);
  statistic_.AddSample(2);
  statistic_.AddSample(3);

  Statistic second_statistic("", false);
  second_statistic.AddSample(4);
  second_statistic.AddSample(5);
  second_statistic.AddSample(6);

  statistic_.MergeWithStatistic(second_statistic);

  EXPECT_EQ(3.5, statistic_.GetAverage());
  EXPECT_EQ(1, statistic_.GetMin());
  EXPECT_EQ(6, statistic_.GetMax());

  Statistic third_statistic("", false);
  third_statistic.AddSample(7);
  third_statistic.AddSample(8);
  third_statistic.AddSample(9);
  third_statistic.AddSample(10);
  for (int i = 0; i < 10; i++) {
    third_statistic.AddSample(1e-3 * (i + 1));
  }
  for (int i = 0; i < 10; i++) {
    third_statistic.AddSample(1e-6 * (i + 1));
  }
  statistic_.MergeWithStatistic(third_statistic);

  EXPECT_NEAR(1.8351685, statistic_.GetAverage(), 1e-6);
  EXPECT_NEAR(1e-6, statistic_.GetMin(), 1e-6);
  EXPECT_EQ(10, statistic_.GetMax());

  // Make sure an empty statistic does not change anything.
  Statistic fourth_statistic("", false);
  statistic_.MergeWithStatistic(fourth_statistic);
  EXPECT_NEAR(1.8351685, statistic_.GetAverage(), 1e-6);
  EXPECT_NEAR(1e-6, statistic_.GetMin(), 1e-6);
  EXPECT_EQ(10, statistic_.GetMax());
}

TEST_F(StatisticTest, Min) {
  statistic_.AddSample(100);
  EXPECT_EQ(100, statistic_.GetMin());
  statistic_.AddSample(10);
  EXPECT_EQ(10, statistic_.GetMin());
  statistic_.AddSample(100);
  EXPECT_EQ(10, statistic_.GetMin());
  statistic_.AddSample(0);
  EXPECT_EQ(0, statistic_.GetMin());
  statistic_.AddSample(-10);
  EXPECT_EQ(-10, statistic_.GetMin());
}

TEST_F(StatisticTest, Reset) {
  statistic_.AddSample(100);
  statistic_.AddSample(200);
  statistic_.AddSample(300);
  statistic_.AddSample(400);
  EXPECT_EQ(4, statistic_.GetCount());
  statistic_.Reset();
  EXPECT_EQ(0, statistic_.GetCount());
  statistic_.AddSample(1);
  statistic_.AddSample(2);
  statistic_.AddSample(3);
  statistic_.AddSample(4);
  EXPECT_EQ(3, statistic_.GetQuantile(.75));
  EXPECT_EQ(4, statistic_.GetMax());
  EXPECT_EQ(4, statistic_.GetCount());
}

TEST(HistogramTest, AddSample) {
  float min = 1e-3;
  float max = 1.0;
  int n_bins = 10;
  Histogram histogram(min, max, n_bins);
  histogram.AddSample(1e-3);
  EXPECT_EQ(1, histogram.histogram()[0]);
  histogram.AddSample(5e-3);
  EXPECT_EQ(2, histogram.histogram()[0]);
  histogram.AddSample(.101);
  EXPECT_EQ(1, histogram.histogram()[1]);
  histogram.AddSample(0.999999);
  EXPECT_EQ(1, histogram.histogram()[n_bins - 1]);

  // Make sure the correct number of samples is recorded.
  EXPECT_EQ(4, histogram.n_samples());
}

TEST(HistogramTest, Copy) {
  float min = 1e-3;
  float max = 1.0;
  int n_bins = 10;
  Histogram histogram(min, max, n_bins);
  histogram.AddSample(1e-3);
  histogram.AddSample(5e-3);
  histogram.AddSample(.101);
  histogram.AddSample(0.999999);
  Histogram* histogram_copy = histogram.Copy();
  for (int i = 0; i < n_bins; i++) {
    EXPECT_EQ(histogram.histogram()[i], histogram_copy->histogram()[i]);
  }
  EXPECT_EQ(histogram.n_bins(), histogram_copy->n_bins());
  EXPECT_EQ(histogram.min(), histogram_copy->min());
  EXPECT_EQ(histogram.max(), histogram_copy->max());
  EXPECT_EQ(histogram.n_samples(), histogram_copy->n_samples());
}

TEST(HistogramTest, GetQuantile) {
  float min = 1e-3;
  float max = 1.0;
  int n_bins = 10;
  Histogram histogram(min, max, n_bins);
  histogram.AddSample(1e-3);
  histogram.AddSample(0.1);
  histogram.AddSample(0.2);
  histogram.AddSample(0.3);
  // 1e-3 gets trucated.
  EXPECT_NEAR(0.0, histogram.GetQuantile(0.25), 1e-6);
  EXPECT_NEAR(0.1, histogram.GetQuantile(0.5), 1e-6);
  EXPECT_NEAR(0.2, histogram.GetQuantile(0.75), 1e-6);
  EXPECT_NEAR(0.3, histogram.GetQuantile(1.00), 1e-6);
  histogram.AddSample(0.4);
  histogram.AddSample(0.5);
  histogram.AddSample(0.6);
  histogram.AddSample(0.7);
  histogram.AddSample(0.8);
  histogram.AddSample(0.9);
  // Again, 1e-3 gets truncated.
  EXPECT_NEAR(0.0, histogram.GetQuantile(0.1), 1e-6);
  EXPECT_NEAR(0.3, histogram.GetQuantile(0.4), 1e-6);
  EXPECT_NEAR(0.6, histogram.GetQuantile(0.7), 1e-6);
  EXPECT_NEAR(0.9, histogram.GetQuantile(1.0), 1e-6);
}

TEST(HistogramTest, MergeWithHistogram) {
  float min = 1e-3;
  float max = 1.0;
  int n_bins = 10;
  Histogram histogram(min, max, n_bins);
  histogram.AddSample(1e-3);
  histogram.AddSample(5e-3);
  histogram.AddSample(.101);
  histogram.AddSample(0.999999);

  Histogram second_histogram(min, max, n_bins);
  second_histogram.AddSample(1e-3);
  second_histogram.AddSample(5e-3);
  second_histogram.AddSample(.101);

  histogram.MergeWithHistogram(second_histogram);

  EXPECT_EQ(4, histogram.histogram()[0]);
  EXPECT_EQ(2, histogram.histogram()[1]);
  EXPECT_EQ(1, histogram.histogram()[9]);
  EXPECT_EQ(7, histogram.n_samples());
}

TEST(HistogramTest, Reset) {
  float min = 1e-3;
  float max = 1.0;
  int n_bins = 10;
  Histogram histogram(min, max, n_bins);
  histogram.AddSample(1e-3);
  histogram.AddSample(5e-3);
  histogram.AddSample(.101);
  histogram.AddSample(0.999999);
  histogram.Reset();
  for (int i = 0; i < n_bins; i++) {
    EXPECT_EQ(0, histogram.histogram()[i]) << " i=" << i;
  }
  EXPECT_EQ(histogram.n_samples(), 0);
}

}  // namespace
