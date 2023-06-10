# Copyright (C) 2015 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Default values for the USE flags. Override these USE flags from your product
# by setting BRILLO_USE_* values. Note that we define local variables like
# local_use_* to prevent leaking our default setting for other packages.
local_use_dbus := $(if $(BRILLO_USE_DBUS),$(BRILLO_USE_DBUS),0)

LOCAL_PATH := $(call my-dir)

# Common variables
# ========================================================

# Set libchromeUseClang to "true" to force clang or "false" to force gcc.
libchromeUseClang :=
libchromeCommonCppExtension := .cc
libchromeTestCFlags := -Wno-unused-parameter -Wno-unused-function \
	-Wno-missing-field-initializers
libchromeCommonCFlags := \
	-Wall \
	-Werror \
	-Wno-deprecated-declarations \
	-Wno-implicit-fallthrough \
	-Wno-missing-field-initializers \
	-Wno-unused-parameter \

libchromeCommonCIncludes := \
	external/valgrind/include \
	external/valgrind \

libchromeExportedCIncludes := $(LOCAL_PATH)

libchromeCommonCppFlags := -std=gnu++14

libchromeCommonSrc := \
	base/at_exit.cc \
	base/barrier_closure.cc \
	base/base64.cc \
	base/base64url.cc \
	base/base_paths.cc \
	base/base_paths_posix.cc \
	base/base_switches.cc \
	base/big_endian.cc \
	base/build_time.cc \
	base/callback_helpers.cc \
	base/callback_internal.cc \
	base/command_line.cc \
	base/cpu.cc \
	base/debug/activity_tracker.cc \
	base/debug/alias.cc \
	base/debug/crash_logging.cc \
	base/debug/debugger.cc \
	base/debug/debugger_posix.cc \
	base/debug/dump_without_crashing.cc \
	base/debug/proc_maps_linux.cc \
	base/debug/profiler.cc \
	base/debug/stack_trace.cc \
	base/debug/stack_trace_posix.cc \
	base/debug/task_annotator.cc \
	base/environment.cc \
	base/feature_list.cc \
	base/files/file.cc \
	base/files/file_descriptor_watcher_posix.cc \
	base/files/file_enumerator.cc \
	base/files/file_enumerator_posix.cc \
	base/files/file_path.cc \
	base/files/file_path_constants.cc \
	base/files/file_path_watcher.cc \
	base/files/file_posix.cc \
	base/files/file_tracing.cc \
	base/files/file_util.cc \
	base/files/file_util_posix.cc \
	base/files/important_file_writer.cc \
	base/files/memory_mapped_file.cc \
	base/files/memory_mapped_file_posix.cc \
	base/files/scoped_file.cc \
	base/files/scoped_temp_dir.cc \
	base/guid.cc \
	base/hash.cc \
	base/json/json_file_value_serializer.cc \
	base/json/json_parser.cc \
	base/json/json_reader.cc \
	base/json/json_string_value_serializer.cc \
	base/json/json_value_converter.cc \
	base/json/json_writer.cc \
	base/json/string_escape.cc \
	base/lazy_instance_helpers.cc \
	base/location.cc \
	base/logging.cc \
	base/md5.cc \
	base/memory/aligned_memory.cc \
	base/memory/platform_shared_memory_region.cc \
	base/memory/platform_shared_memory_region_posix.cc \
	base/memory/read_only_shared_memory_region.cc \
	base/memory/ref_counted.cc \
	base/memory/ref_counted_memory.cc \
	base/memory/shared_memory_handle.cc \
	base/memory/shared_memory_handle_posix.cc \
	base/memory/shared_memory_helper.cc \
	base/memory/shared_memory_mapping.cc \
	base/memory/unsafe_shared_memory_region.cc \
	base/memory/weak_ptr.cc \
	base/memory/writable_shared_memory_region.cc \
	base/message_loop/incoming_task_queue.cc \
	base/message_loop/message_loop.cc \
	base/message_loop/message_loop_current.cc \
	base/message_loop/message_loop_task_runner.cc \
	base/message_loop/message_pump.cc \
	base/message_loop/message_pump_default.cc \
	base/message_loop/message_pump_libevent.cc \
	base/message_loop/watchable_io_message_pump_posix.cc \
	base/metrics/bucket_ranges.cc \
	base/metrics/dummy_histogram.cc \
	base/metrics/field_trial.cc \
	base/metrics/field_trial_param_associator.cc \
	base/metrics/histogram_base.cc \
	base/metrics/histogram.cc \
	base/metrics/histogram_functions.cc \
	base/metrics/histogram_samples.cc \
	base/metrics/histogram_snapshot_manager.cc \
	base/metrics/metrics_hashes.cc \
	base/metrics/persistent_histogram_allocator.cc \
	base/metrics/persistent_memory_allocator.cc \
	base/metrics/persistent_sample_map.cc \
	base/metrics/sample_map.cc \
	base/metrics/sample_vector.cc \
	base/metrics/sparse_histogram.cc \
	base/metrics/statistics_recorder.cc \
	base/native_library.cc \
	base/native_library_posix.cc \
	base/observer_list_threadsafe.cc \
	base/path_service.cc \
	base/pending_task.cc \
	base/pickle.cc \
	base/posix/file_descriptor_shuffle.cc \
	base/posix/global_descriptors.cc \
	base/posix/safe_strerror.cc \
	base/process/kill.cc \
	base/process/kill_posix.cc \
	base/process/launch.cc \
	base/process/launch_posix.cc \
	base/process/memory.cc \
	base/process/process_handle.cc \
	base/process/process_handle_posix.cc \
	base/process/process_iterator.cc \
	base/process/process_metrics.cc \
	base/process/process_metrics_posix.cc \
	base/process/process_posix.cc \
	base/rand_util.cc \
	base/rand_util_posix.cc \
	base/run_loop.cc \
	base/scoped_native_library.cc \
	base/sequence_checker_impl.cc \
	base/sequence_token.cc \
	base/sequenced_task_runner.cc \
	base/sha1.cc \
	base/strings/nullable_string16.cc \
	base/strings/pattern.cc \
	base/strings/safe_sprintf.cc \
	base/strings/string16.cc \
	base/strings/string_number_conversions.cc \
	base/strings/string_piece.cc \
	base/strings/string_split.cc \
	base/strings/string_util.cc \
	base/strings/string_util_constants.cc \
	base/strings/stringprintf.cc \
	base/strings/utf_string_conversions.cc \
	base/strings/utf_string_conversion_utils.cc \
	base/synchronization/atomic_flag.cc \
	base/synchronization/condition_variable_posix.cc \
	base/synchronization/lock.cc \
	base/synchronization/lock_impl_posix.cc \
	base/synchronization/waitable_event_posix.cc \
	base/sync_socket_posix.cc \
	base/sys_info.cc \
	base/sys_info_posix.cc \
	base/task/cancelable_task_tracker.cc \
	base/task_runner.cc \
	base/task_scheduler/scheduler_lock_impl.cc \
	base/task_scheduler/scoped_set_task_priority_for_current_thread.cc \
	base/task_scheduler/sequence.cc \
	base/task_scheduler/sequence_sort_key.cc \
	base/task_scheduler/task.cc \
	base/task_scheduler/task_traits.cc \
	base/third_party/dynamic_annotations/dynamic_annotations.c \
	base/third_party/icu/icu_utf.cc \
	base/third_party/nspr/prtime.cc \
	base/threading/platform_thread_posix.cc \
	base/threading/post_task_and_reply_impl.cc \
	base/threading/scoped_blocking_call.cc \
	base/threading/sequence_local_storage_map.cc \
	base/threading/sequence_local_storage_slot.cc \
	base/threading/sequenced_task_runner_handle.cc \
	base/threading/simple_thread.cc \
	base/threading/thread.cc \
	base/threading/thread_checker_impl.cc \
	base/threading/thread_collision_warner.cc \
	base/threading/thread_id_name_manager.cc \
	base/threading/thread_local_storage.cc \
	base/threading/thread_local_storage_posix.cc \
	base/threading/thread_restrictions.cc \
	base/threading/thread_task_runner_handle.cc \
	base/time/clock.cc \
	base/time/default_clock.cc \
	base/time/default_tick_clock.cc \
	base/time/tick_clock.cc \
	base/time/time.cc \
	base/time/time_conversion_posix.cc \
	base/time/time_exploded_posix.cc \
	base/time/time_now_posix.cc \
	base/time/time_override.cc \
	base/timer/elapsed_timer.cc \
	base/timer/timer.cc \
	base/unguessable_token.cc \
	base/value_iterators.cc \
	base/values.cc \
	base/version.cc \
	base/vlog.cc \
	device/bluetooth/bluetooth_advertisement.cc \
	device/bluetooth/bluetooth_uuid.cc \
	device/bluetooth/bluez/bluetooth_service_attribute_value_bluez.cc \
	ui/gfx/geometry/insets.cc \
	ui/gfx/geometry/insets_f.cc \
	ui/gfx/geometry/point.cc \
	ui/gfx/geometry/point_conversions.cc \
	ui/gfx/geometry/point_f.cc \
	ui/gfx/geometry/rect.cc \
	ui/gfx/geometry/rect_f.cc \
	ui/gfx/geometry/size.cc \
	ui/gfx/geometry/size_conversions.cc \
	ui/gfx/geometry/size_f.cc \
	ui/gfx/geometry/vector2d.cc \
	ui/gfx/geometry/vector2d_f.cc \
	ui/gfx/range/range.cc \
	ui/gfx/range/range_f.cc \

libchromeLinuxSrcNoAlloc := \
	base/files/file_path_watcher_linux.cc \
	base/files/file_util_linux.cc \
	base/memory/shared_memory_posix.cc \
	base/posix/unix_domain_socket.cc \
	base/process/internal_linux.cc \
	base/process/memory_linux.cc \
	base/process/process_handle_linux.cc \
	base/process/process_info_linux.cc \
	base/process/process_iterator_linux.cc \
	base/process/process_metrics_linux.cc \
	base/strings/sys_string_conversions_posix.cc \
	base/sys_info_linux.cc \
	base/threading/platform_thread_internal_posix.cc \
	base/threading/platform_thread_linux.cc \
	components/timers/alarm_timer_chromeos.cc \

libchromeLinuxSrc := \
	base/allocator/allocator_shim.cc \
  $(libchromeLinuxSrcNoAlloc)

libchromeCommonUnittestSrc := \
	base/at_exit_unittest.cc \
	base/atomicops_unittest.cc \
	base/base64_unittest.cc \
	base/base64url_unittest.cc \
	base/big_endian_unittest.cc \
	base/bind_unittest.cc \
	base/bits_unittest.cc \
	base/build_time_unittest.cc \
	base/callback_helpers_unittest.cc \
	base/callback_list_unittest.cc \
	base/callback_unittest.cc \
	base/cancelable_callback_unittest.cc \
	base/command_line_unittest.cc \
	base/cpu_unittest.cc \
        base/debug/activity_tracker_unittest.cc \
	base/debug/debugger_unittest.cc \
	base/debug/leak_tracker_unittest.cc \
	base/environment_unittest.cc \
	base/files/dir_reader_posix_unittest.cc \
	base/files/file_descriptor_watcher_posix_unittest.cc \
	base/files/file_enumerator_unittest.cc \
	base/files/file_path_unittest.cc \
	base/files/file_path_watcher_unittest.cc \
	base/files/file_unittest.cc \
	base/files/important_file_writer_unittest.cc \
	base/files/scoped_temp_dir_unittest.cc \
	base/gmock_unittest.cc \
	base/guid_unittest.cc \
	base/json/json_parser_unittest.cc \
	base/json/json_reader_unittest.cc \
	base/json/json_value_converter_unittest.cc \
	base/json/json_value_serializer_unittest.cc \
	base/json/json_writer_unittest.cc \
	base/json/string_escape_unittest.cc \
	base/lazy_instance_unittest.cc \
	base/logging_unittest.cc \
	base/md5_unittest.cc \
	base/memory/aligned_memory_unittest.cc \
	base/memory/linked_ptr_unittest.cc \
	base/memory/ref_counted_memory_unittest.cc \
	base/memory/ref_counted_unittest.cc \
	base/memory/singleton_unittest.cc \
	base/memory/weak_ptr_unittest.cc \
	base/message_loop/message_loop_task_runner_unittest.cc \
	base/message_loop/message_loop_unittest.cc \
	base/metrics/bucket_ranges_unittest.cc \
	base/metrics/field_trial_unittest.cc \
	base/metrics/histogram_base_unittest.cc \
	base/metrics/histogram_macros_unittest.cc \
	base/metrics/histogram_snapshot_manager_unittest.cc \
	base/metrics/histogram_unittest.cc \
	base/metrics/metrics_hashes_unittest.cc \
	base/metrics/persistent_histogram_allocator_unittest.cc \
	base/metrics/persistent_memory_allocator_unittest.cc \
	base/metrics/persistent_sample_map_unittest.cc \
	base/metrics/sample_map_unittest.cc \
	base/metrics/sample_vector_unittest.cc \
	base/metrics/sparse_histogram_unittest.cc \
	base/metrics/statistics_recorder_unittest.cc \
	base/observer_list_unittest.cc \
	base/optional_unittest.cc \
	base/pickle_unittest.cc \
	base/posix/file_descriptor_shuffle_unittest.cc \
	base/posix/unix_domain_socket_unittest.cc \
	base/process/process_info_unittest.cc \
	base/process/process_metrics_unittest.cc \
	base/rand_util_unittest.cc \
	base/scoped_clear_errno_unittest.cc \
	base/scoped_generic_unittest.cc \
	base/security_unittest.cc \
	base/sequence_checker_unittest.cc \
	base/sequence_token_unittest.cc \
	base/sha1_unittest.cc \
	base/stl_util_unittest.cc \
	base/strings/pattern_unittest.cc \
	base/strings/string16_unittest.cc \
	base/strings/string_number_conversions_unittest.cc \
	base/strings/string_piece_unittest.cc \
	base/strings/string_split_unittest.cc \
	base/strings/string_util_unittest.cc \
	base/strings/stringprintf_unittest.cc \
	base/strings/sys_string_conversions_unittest.cc \
	base/strings/utf_string_conversions_unittest.cc \
	base/synchronization/atomic_flag_unittest.cc \
	base/synchronization/condition_variable_unittest.cc \
	base/synchronization/lock_unittest.cc \
	base/synchronization/waitable_event_unittest.cc \
	base/sys_info_unittest.cc \
	base/task/cancelable_task_tracker_unittest.cc \
	base/task_runner_util_unittest.cc \
	base/task_scheduler/scheduler_lock_unittest.cc \
	base/task_scheduler/scoped_set_task_priority_for_current_thread_unittest.cc \
	base/task_scheduler/sequence_sort_key_unittest.cc \
	base/task_scheduler/sequence_unittest.cc \
	base/task_scheduler/task_traits.cc \
	base/template_util_unittest.cc \
	base/test/metrics/histogram_tester.cc \
	base/test/mock_entropy_provider.cc \
	base/test/multiprocess_test.cc \
	base/test/scoped_feature_list.cc \
	base/test/scoped_locale.cc \
	base/test/simple_test_tick_clock.cc \
	base/test/test_file_util.cc \
	base/test/test_file_util_linux.cc \
	base/test/test_file_util_posix.cc \
	base/test/test_io_thread.cc \
        base/test/test_mock_time_task_runner.cc \
	base/test/test_pending_task.cc \
	base/test/test_shared_memory_util.cc \
	base/test/test_simple_task_runner.cc \
	base/test/test_switches.cc \
	base/test/test_timeouts.cc \
	base/threading/platform_thread_unittest.cc \
	base/threading/simple_thread_unittest.cc \
	base/threading/thread_checker_unittest.cc \
	base/threading/thread_collision_warner_unittest.cc \
	base/threading/thread_id_name_manager_unittest.cc \
	base/threading/thread_local_storage_unittest.cc \
	base/threading/thread_local_unittest.cc \
	base/threading/thread_unittest.cc \
	base/time/pr_time_unittest.cc \
	base/time/time_unittest.cc \
	base/timer/hi_res_timer_manager_unittest.cc \
	base/timer/mock_timer.cc \
	base/tuple_unittest.cc \
	base/values_unittest.cc \
	base/version_unittest.cc \
	base/vlog_unittest.cc \
	testing/multiprocess_func_list.cc \
	testrunner.cc \
	ui/gfx/range/range_unittest.cc \

libchromeCryptoUnittestSrc := \
	crypto/secure_hash_unittest.cc \
	crypto/sha2_unittest.cc \

libchromeHostCFlags := -D__ANDROID_HOST__ -DDONT_EMBED_BUILD_METADATA

# TODO(bcf): Temporary until b/75024799 is resolved
libchromeHostCFlags += \
	-Wno-implicit-fallthrough \
	-Wno-format-truncation \
	-Wno-dangling-else \
	-Wno-type-limits \
	-Wno-extra

ifeq ($(HOST_OS),linux)
libchromeHostSrc := $(libchromeLinuxSrc) \
	base/allocator/allocator_shim_default_dispatch_to_glibc.cc
libchromeHostLdFlags := -ldl
endif

ifeq ($(HOST_OS),darwin)
libchromeHostSrc := $(libchromeMacSrc)
libchromeHostCFlags += -D_FILE_OFFSET_BITS=64 -Wno-deprecated-declarations
libchromeHostLdFlags := \
	-framework AppKit \
	-framework CoreFoundation \
	-framework Foundation \
	-framework Security
endif

ifeq ($(BUILD_EUREKA),true)
libchromeHostCFlags += -Wno-missing-field-initializers
endif

# libchrome shared library for target
# ========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := libchrome
LOCAL_SRC_FILES := \
	$(libchromeCommonSrc) \
	$(libchromeLinuxSrc) \
	base/allocator/allocator_shim_default_dispatch_to_glibc.cc \
	base/memory/shared_memory_android.cc \
	base/sys_info_chromeos.cc

LOCAL_CPP_EXTENSION := $(libchromeCommonCppExtension)
LOCAL_CFLAGS := $(libchromeCommonCFlags)
LOCAL_CPPFLAGS := $(libchromeCommonCppFlags)
LOCAL_CLANG := $(libchromeUseClang)
LOCAL_C_INCLUDES := $(libchromeCommonCIncludes)
LOCAL_EXPORT_SHARED_LIBRARY_HEADERS := libbase
LOCAL_EXPORT_STATIC_LIBRARY_HEADERS := libgtest_prod
LOCAL_SHARED_LIBRARIES :=  libbase libevent liblog libcutils
LOCAL_STATIC_LIBRARIES := libmodpb64 libgtest_prod
LOCAL_EXPORT_C_INCLUDE_DIRS := $(libchromeExportedCIncludes)
include $(BUILD_SHARED_LIBRARY)

# libchrome static library for target
# ========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := libchrome-static
LOCAL_SRC_FILES := \
	$(libchromeCommonSrc) \
	$(libchromeLinuxSrcNoAlloc) \
	base/memory/shared_memory_android.cc \
	base/sys_info_chromeos.cc
ifeq ($(USE_ASAN),1)
LIBCHROME_NO_ALLOCATOR:= 1
endif
ifeq ($(LIBCHROME_NO_ALLOCATOR),1)
LOCAL_CFLAGS += -DMEMORY_TOOL_REPLACES_ALLOCATOR=1
else
LOCAL_SRC_FILES += \
	base/allocator/allocator_shim_default_dispatch_to_glibc.cc
endif

LOCAL_CPP_EXTENSION := $(libchromeCommonCppExtension)
LOCAL_CFLAGS := $(libchromeCommonCFlags)
LOCAL_CPPFLAGS := $(libchromeCommonCppFlags)
LOCAL_CLANG := $(libchromeUseClang)
LOCAL_C_INCLUDES := $(libchromeCommonCIncludes)
LOCAL_WHOLE_STATIC_LIBRARIES := libmodpb64 libgtest_prod \
	libbase libevent-static liblog libcutils
LOCAL_EXPORT_STATIC_LIBRARY_HEADERS := $(LOCAL_STATIC_LIBRARIES)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(libchromeExportedCIncludes)


ifeq ($(USE_ASAN),1)
LOCAL_CFLAGS += -fsanitize=address -fno-omit-frame-pointer
LOCAL_LDFLAGS += -fsanitize=address -fno-omit-frame-pointer
endif

include $(BUILD_STATIC_LIBRARY)

# libchrome shared library for host
# ========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := libchrome-host
LOCAL_CFLAGS := $(libchromeCommonCFlags) $(libchromeHostCFlags)
LOCAL_CPPFLAGS := $(libchromeCommonCppFlags) -Wno-sign-promo
LOCAL_CLANG := $(libchromeUseClang)
LOCAL_CPP_EXTENSION := $(libchromeCommonCppExtension)
LOCAL_C_INCLUDES := $(libchromeCommonCIncludes)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(libchromeExportedCIncludes)
LOCAL_EXPORT_STATIC_LIBRARY_HEADERS := libgtest_prod
LOCAL_EXPORT_SHARED_LIBRARY_HEADERS := libbase
LOCAL_SHARED_LIBRARIES := libbase libevent-host
LOCAL_STATIC_LIBRARIES := libmodpb64-host libgtest_prod
LOCAL_SRC_FILES := \
	$(libchromeCommonSrc) \
	$(libchromeHostSrc) \

LOCAL_LDLIBS := -lpthread
ifeq ($(HOST_OS),linux)
LOCAL_LDLIBS += -lrt
endif
LOCAL_LDFLAGS := $(libchromeHostLdFlags)
include $(BUILD_HOST_SHARED_LIBRARY)

ifeq ($(local_use_dbus),1)

# libchrome-dbus shared library for target
# ========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := libchrome-dbus
LOCAL_SRC_FILES := \
	dbus/bus.cc \
	dbus/dbus_statistics.cc \
	dbus/exported_object.cc \
	dbus/message.cc \
	dbus/object_manager.cc \
	dbus/object_path.cc \
	dbus/object_proxy.cc \
	dbus/property.cc \
	dbus/scoped_dbus_error.cc \
	dbus/string_util.cc \
	dbus/util.cc \
	dbus/values_util.cc \

LOCAL_CPP_EXTENSION := $(libchromeCommonCppExtension)
LOCAL_CFLAGS := $(libchromeCommonCFlags)
LOCAL_CPPFLAGS := $(libchromeCommonCppFlags)
LOCAL_CLANG := $(libchromeUseClang)
LOCAL_C_INCLUDES := $(libchromeCommonCIncludes)
LOCAL_SHARED_LIBRARIES := \
	libchrome \
	libdbus \
	libprotobuf-cpp-lite \

LOCAL_STATIC_LIBRARIES := libgtest_prod
LOCAL_EXPORT_C_INCLUDE_DIRS := $(libchromeExportedCIncludes)
LOCAL_EXPORT_STATIC_LIBRARY_HEADERS := libgtest_prod
LOCAL_EXPORT_SHARED_LIBRARY_HEADERS := libchrome
include $(BUILD_SHARED_LIBRARY)

endif  # local_use_dbus == 1

# libchrome-crypto shared library for target
# ========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := libchrome-crypto
LOCAL_SRC_FILES := \
	crypto/openssl_util.cc \
	crypto/random.cc \
	crypto/secure_hash.cc \
	crypto/secure_util.cc \
	crypto/sha2.cc \

LOCAL_CPP_EXTENSION := $(libchromeCommonCppExtension)
LOCAL_CFLAGS := $(libchromeCommonCFlags) -Wno-unused-parameter
LOCAL_CPPFLAGS := $(libchromeCommonCppFlags)
LOCAL_CLANG := $(libchromeUseClang)
LOCAL_C_INCLUDES := $(libchromeCommonCIncludes) external/gtest/include
LOCAL_SHARED_LIBRARIES := \
	libchrome \
	libcrypto \
	libssl \

LOCAL_EXPORT_C_INCLUDE_DIRS := $(libchromeExportedCIncludes)
include $(BUILD_SHARED_LIBRARY)

# Helpers needed for unit tests.
# ========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := libchrome_test_helpers
LOCAL_SHARED_LIBRARIES := libchrome
LOCAL_CPP_EXTENSION := $(libchromeCommonCppExtension)
LOCAL_CFLAGS := $(libchromeCommonCFlags) $(libchromeTestCFlags)
LOCAL_CPPFLAGS := $(libchromeCommonCppFlags)
LOCAL_CLANG := $(libchromeUseClang)
LOCAL_C_INCLUDES := $(libchromeCommonCIncludes)
LOCAL_SRC_FILES := \
	base/test/simple_test_clock.cc \
	base/test/simple_test_tick_clock.cc \
	base/test/test_file_util.cc \
	base/test/test_file_util_linux.cc \
	base/test/test_switches.cc \
	base/test/test_timeouts.cc \

include $(BUILD_STATIC_LIBRARY)

ifeq ($(local_use_dbus),1)

# Helpers needed for D-Bus unit tests.
# ========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := libchrome_dbus_test_helpers
LOCAL_SHARED_LIBRARIES := libdbus libchrome-dbus
LOCAL_STATIC_LIBRARIES := libgmock
LOCAL_CPP_EXTENSION := $(libchromeCommonCppExtension)
LOCAL_CFLAGS := $(libchromeCommonCFlags) $(libchromeTestCFlags)
LOCAL_CPPFLAGS := $(libchromeCommonCppFlags)
LOCAL_CLANG := $(libchromeUseClang)
LOCAL_C_INCLUDES := $(libchromeCommonCIncludes)
LOCAL_SRC_FILES := \
	dbus/mock_bus.cc \
	dbus/mock_exported_object.cc \
	dbus/mock_object_proxy.cc \

include $(BUILD_STATIC_LIBRARY)

endif  # local_use_dbus == 1

# Helpers needed for unit tests (for host).
# ========================================================
ifeq ($(HOST_OS),linux)
include $(CLEAR_VARS)
LOCAL_MODULE := libchrome_test_helpers-host
LOCAL_SHARED_LIBRARIES := libchrome
LOCAL_CPP_EXTENSION := $(libchromeCommonCppExtension)
LOCAL_CFLAGS := $(libchromeCommonCFlags) $(libchromeTestCFlags)
LOCAL_CPPFLAGS := $(libchromeCommonCppFlags)
LOCAL_CLANG := $(libchromeUseClang)
LOCAL_C_INCLUDES := $(libchromeCommonCIncludes)
LOCAL_SRC_FILES := base/test/simple_test_clock.cc
include $(BUILD_HOST_STATIC_LIBRARY)

# Host unit tests. Run (from repo root) with:
# ./out/host/<arch>/bin/libchrome_test
# ========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := libchrome_test
LOCAL_SRC_FILES := $(libchromeCommonUnittestSrc)
LOCAL_CPP_EXTENSION := $(libchromeCommonCppExtension)
LOCAL_CFLAGS := $(libchromeCommonCFlags) $(libchromeTestCFlags) $(libchromeHostCFlags) -DUNIT_TEST
LOCAL_CPPFLAGS := $(libchromeCommonCppFlags)
LOCAL_CLANG := $(libchromeUseClang)
LOCAL_C_INCLUDES := $(libchromeCommonCIncludes)
LOCAL_SHARED_LIBRARIES := libchrome libevent
LOCAL_STATIC_LIBRARIES := libgmock_host libgtest_host
LOCAL_LDLIBS_linux := -lrt
include $(BUILD_HOST_NATIVE_TEST)
endif

# Native unit tests. Run with:
# adb shell /data/nativetest/libchrome_test/libchrome_test
# ========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := libchrome_test
LOCAL_SRC_FILES := $(libchromeCryptoUnittestSrc) $(libchromeCommonUnittestSrc)
LOCAL_CPP_EXTENSION := $(libchromeCommonCppExtension)
LOCAL_CFLAGS := $(libchromeCommonCFlags) $(libchromeTestCFlags) -DUNIT_TEST -DDONT_EMBED_BUILD_METADATA
LOCAL_CPPFLAGS := $(libchromeCommonCppFlags)
LOCAL_CLANG := $(libchromeUseClang)
LOCAL_C_INCLUDES := $(libchromeCommonCIncludes)
LOCAL_SHARED_LIBRARIES := libchrome libchrome-crypto libevent
LOCAL_STATIC_LIBRARIES := libgmock libgtest
include $(BUILD_NATIVE_TEST)

include $(LOCAL_PATH)/libmojo.mk
