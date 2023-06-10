// Copyright (c) 2009 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include <base/logging.h>
#include <base/string_util.h>
#include <gflags/gflags.h>
#include <rootdev/rootdev.h>

#include "metrics_daemon.h"

DEFINE_bool(daemon, true, "run as daemon (use -nodaemon for debugging)");

// Returns the path to the disk stats in the sysfs.  Returns the null string if
// it cannot find the disk stats file.
static
const std::string MetricsMainDiskStatsPath() {
  char dev_path_cstr[PATH_MAX];
  std::string dev_prefix = "/dev/";
  std::string dev_path;
  std::string dev_name;

  int ret = rootdev(dev_path_cstr, sizeof(dev_path_cstr), true, true);
  if (ret != 0) {
    LOG(WARNING) << "error " << ret << " determining root device";
    return "";
  }
  dev_path = dev_path_cstr;
  // Check that rootdev begins with "/dev/".
  if (!StartsWithASCII(dev_path, dev_prefix, false)) {
    LOG(WARNING) << "unexpected root device " << dev_path;
    return "";
  }
  // Get the device name, e.g. "sda" from "/dev/sda".
  dev_name = dev_path.substr(dev_prefix.length());
  return "/sys/class/block/" + dev_name + "/stat";
}

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  MetricsLibrary metrics_lib;
  metrics_lib.Init();
  MetricsDaemon daemon;
  daemon.Init(false, &metrics_lib, MetricsMainDiskStatsPath(), "/proc/vmstat");
  daemon.Run(FLAGS_daemon);
}
