// Copyright (c) 2010 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef METRICS_COUNTER_MOCK_H_
#define METRICS_COUNTER_MOCK_H_

#include <string>

#include <gmock/gmock.h>

#include "counter.h"

namespace chromeos_metrics {

class TaggedCounterMock : public TaggedCounter {
 public:
  MOCK_METHOD3(Init, void(const char* filename,
                          Reporter reporter, void* reporter_handle));
  MOCK_METHOD2(Update, void(int tag, int count));
  MOCK_METHOD0(Flush, void());
};

class TaggedCounterReporterMock : public TaggedCounterReporter {
 public:
  MOCK_METHOD5(Init, void(const char* filename,
                          const char* histogram_name,
                          int min,
                          int max,
                          int nbuckets));
  MOCK_METHOD2(Update, void(int32_t tag, int32_t count));
  MOCK_METHOD0(Flush, void());
};

class FrequencyCounterMock : public FrequencyCounter {
 public:
  MOCK_METHOD4(Init, void(const char* filename,
                          TaggedCounterInterface::Reporter reporter,
                          void* reporter_handle,
                          time_t cycle_duration));
  MOCK_METHOD1(Update, void(int32_t count));
  MOCK_METHOD0(FlushFinishedCycles, void());
};

}  // namespace chromeos_metrics

#endif  // METRICS_COUNTER_MOCK_H_
