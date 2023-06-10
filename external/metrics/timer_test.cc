// Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include <base/time.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "metrics_library_mock.h"
#include "timer.h"
#include "timer_mock.h"

using ::testing::_;
using ::testing::Return;

namespace chromeos_metrics {

class TimerTest : public testing::Test {
 public:
  TimerTest() : clock_wrapper_mock_(new ClockWrapperMock()) {}

 protected:
  virtual void SetUp() {
    EXPECT_FALSE(timer_.is_started_);
    stime += base::TimeDelta::FromMilliseconds(1500);
    etime += base::TimeDelta::FromMilliseconds(3000);
  }

  virtual void TearDown() {}

  Timer timer_;
  std::unique_ptr<ClockWrapperMock> clock_wrapper_mock_;
  base::TimeTicks stime, etime;
};

TEST_F(TimerTest, StartStop) {
  EXPECT_CALL(*clock_wrapper_mock_, GetCurrentTime())
      .WillOnce(Return(stime))
      .WillOnce(Return(etime));
  timer_.clock_wrapper_.reset(clock_wrapper_mock_.release());
  ASSERT_TRUE(timer_.Start());
  ASSERT_TRUE(timer_.start_time_ == stime);
  ASSERT_TRUE(timer_.HasStarted());
  ASSERT_TRUE(timer_.Stop());
  ASSERT_EQ(timer_.elapsed_time_.InMilliseconds(), 1500);
  ASSERT_FALSE(timer_.HasStarted());
}

TEST_F(TimerTest, ReStart) {
  EXPECT_CALL(*clock_wrapper_mock_, GetCurrentTime())
      .WillOnce(Return(stime))
      .WillOnce(Return(etime));
  timer_.clock_wrapper_.reset(clock_wrapper_mock_.release());
  timer_.Start();
  base::TimeTicks buffer = timer_.start_time_;
  timer_.Start();
  ASSERT_FALSE(timer_.start_time_ == buffer);
}

TEST_F(TimerTest, Reset) {
  EXPECT_CALL(*clock_wrapper_mock_, GetCurrentTime())
      .WillOnce(Return(stime));
  timer_.clock_wrapper_.reset(clock_wrapper_mock_.release());
  timer_.Start();
  ASSERT_TRUE(timer_.Reset());
  ASSERT_FALSE(timer_.HasStarted());
}

TEST_F(TimerTest, SeparatedTimers) {
  base::TimeTicks stime2, etime2;
  stime2 += base::TimeDelta::FromMilliseconds(4200);
  etime2 += base::TimeDelta::FromMilliseconds(5000);
  EXPECT_CALL(*clock_wrapper_mock_, GetCurrentTime())
      .WillOnce(Return(stime))
      .WillOnce(Return(etime))
      .WillOnce(Return(stime2))
      .WillOnce(Return(etime2));
  timer_.clock_wrapper_.reset(clock_wrapper_mock_.release());
  ASSERT_TRUE(timer_.Start());
  ASSERT_TRUE(timer_.Stop());
  ASSERT_EQ(timer_.elapsed_time_.InMilliseconds(), 1500);
  ASSERT_TRUE(timer_.Start());
  ASSERT_TRUE(timer_.start_time_ == stime2);
  ASSERT_TRUE(timer_.Stop());
  ASSERT_EQ(timer_.elapsed_time_.InMilliseconds(), 800);
  ASSERT_FALSE(timer_.HasStarted());
}

TEST_F(TimerTest, InvalidStop) {
  EXPECT_CALL(*clock_wrapper_mock_, GetCurrentTime())
      .WillOnce(Return(stime))
      .WillOnce(Return(etime));
  timer_.clock_wrapper_.reset(clock_wrapper_mock_.release());
  ASSERT_FALSE(timer_.Stop());
  // Now we try it again, but after a valid start/stop.
  timer_.Start();
  timer_.Stop();
  base::TimeDelta elapsed_time = timer_.elapsed_time_;
  ASSERT_FALSE(timer_.Stop());
  ASSERT_TRUE(elapsed_time == timer_.elapsed_time_);
}

TEST_F(TimerTest, InvalidElapsedTime) {
  base::TimeDelta elapsed_time;
  ASSERT_FALSE(timer_.GetElapsedTime(&elapsed_time));
}

static const char kMetricName[] = "test-timer";
static const int kMinSample = 0;
static const int kMaxSample = 120 * 1E6;
static const int kNumBuckets = 50;

class TimerReporterTest : public testing::Test {
 public:
  TimerReporterTest() : timer_reporter_(kMetricName, kMinSample, kMaxSample,
                                        kNumBuckets),
                        clock_wrapper_mock_(new ClockWrapperMock()) {}

 protected:
  virtual void SetUp() {
    timer_reporter_.set_metrics_lib(&lib_);
    EXPECT_EQ(timer_reporter_.histogram_name_, kMetricName);
    EXPECT_EQ(timer_reporter_.min_, kMinSample);
    EXPECT_EQ(timer_reporter_.max_, kMaxSample);
    EXPECT_EQ(timer_reporter_.num_buckets_, kNumBuckets);
    stime += base::TimeDelta::FromMilliseconds(1500);
    etime += base::TimeDelta::FromMilliseconds(3000);
  }

  virtual void TearDown() {
    timer_reporter_.set_metrics_lib(NULL);
  }

  TimerReporter timer_reporter_;
  MetricsLibraryMock lib_;
  std::unique_ptr<ClockWrapperMock> clock_wrapper_mock_;
  base::TimeTicks stime, etime;
};

TEST_F(TimerReporterTest, StartStopReport) {
  EXPECT_CALL(*clock_wrapper_mock_, GetCurrentTime())
      .WillOnce(Return(stime))
      .WillOnce(Return(etime));
  timer_reporter_.clock_wrapper_.reset(clock_wrapper_mock_.release());
  EXPECT_CALL(lib_, SendToUMA(kMetricName, 1500, kMinSample, kMaxSample,
                              kNumBuckets)).WillOnce(Return(true));
  ASSERT_TRUE(timer_reporter_.Start());
  ASSERT_TRUE(timer_reporter_.Stop());
  ASSERT_TRUE(timer_reporter_.ReportMilliseconds());
}

TEST_F(TimerReporterTest, InvalidReport) {
  ASSERT_FALSE(timer_reporter_.ReportMilliseconds());
}

}  // namespace chromeos_metrics

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
