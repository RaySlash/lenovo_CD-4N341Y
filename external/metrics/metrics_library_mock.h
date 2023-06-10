// Copyright (c) 2010 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef METRICS_LIBRARY_MOCK_H_
#define METRICS_LIBRARY_MOCK_H_

#include <string>

#include "metrics_library.h"

#include <gmock/gmock.h>

class MetricsLibraryMock : public MetricsLibraryInterface {
 public:
  MOCK_METHOD0(Init, void());
  MOCK_METHOD5(SendToUMA, bool(const std::string& name, int sample,
                               int min, int max, int nbuckets));
  MOCK_METHOD3(SendEnumToUMA, bool(const std::string& name, int sample,
                                   int max));
  MOCK_METHOD1(SendUserActionToUMA, bool(const std::string& action));
};

#endif  // METRICS_LIBRARY_MOCK_H_
