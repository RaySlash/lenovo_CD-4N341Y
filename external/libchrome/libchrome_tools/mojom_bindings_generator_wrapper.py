#!/usr/bin/env python2
# -*- coding: utf-8 -*-
#
# Copyright 2018 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Thin wrapper of Mojo's mojom_bindings_generator.py.

To generate C++ files from mojom, it is necessary to run
mojom_bindings_generator.py three times for each mojom file.

Usage:
  python mojom_bindings_generator_wrapper.py ${MOJOM_BINDINGS_GENERATOR} \
    [... and more args/flags to be passed to the mojom_bindings_generator.py]
"""

from __future__ import print_function

import subprocess
import sys


def main(argv):
  # *_mojom.h *_mojom.cc
  subprocess.check_call([sys.executable] + argv[1:])
  # *_mojom-shared.h *_mojom-shared.cc
  subprocess.check_call(
      [sys.executable] + argv[1:] + ['--generate_non_variant_code'])
  # *_mojom-shared-message-ids.h
  subprocess.check_call(
      [sys.executable] + argv[1:] + ['--generate_non_variant_code', '--generate_message_ids'])


if __name__ == '__main__':
  main(sys.argv)
