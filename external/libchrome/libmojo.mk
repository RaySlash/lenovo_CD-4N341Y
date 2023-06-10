# Copyright 2015 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

LOCAL_PATH:= $(call my-dir)

# Build native shared library.
include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE := libmojo
LOCAL_MODULE_TAGS := optional
LOCAL_CPP_EXTENSION := .cc
LOCAL_C_INCLUDES := external/gtest/include

LOCAL_MOJOM_FILES := \
	chromecast/external_mojo/public/mojom/connector.mojom \
	ipc/ipc.mojom \
	mojo/public/interfaces/bindings/interface_control_messages.mojom \
	mojo/public/interfaces/bindings/native_struct.mojom \
	mojo/public/interfaces/bindings/pipe_control_messages.mojom \
	mojo/public/mojom/base/big_buffer.mojom \
	mojo/public/mojom/base/big_string.mojom \
	mojo/public/mojom/base/file.mojom \
	mojo/public/mojom/base/file_error.mojom \
	mojo/public/mojom/base/file_info.mojom \
	mojo/public/mojom/base/file_path.mojom \
	mojo/public/mojom/base/process_id.mojom \
	mojo/public/mojom/base/read_only_buffer.mojom \
	mojo/public/mojom/base/ref_counted_memory.mojom \
	mojo/public/mojom/base/shared_memory.mojom \
	mojo/public/mojom/base/string16.mojom \
	mojo/public/mojom/base/text_direction.mojom \
	mojo/public/mojom/base/thread_priority.mojom \
	mojo/public/mojom/base/time.mojom \
	mojo/public/mojom/base/unguessable_token.mojom \
	mojo/public/mojom/base/values.mojom \
	ui/gfx/geometry/mojo/geometry.mojom \
	ui/gfx/range/mojo/range.mojom \

LOCAL_MOJOM_TYPEMAP_FILES := \
	mojo/public/cpp/base/big_buffer.typemap \
	mojo/public/cpp/base/big_string.typemap \
	mojo/public/cpp/base/file.typemap \
	mojo/public/cpp/base/file_error.typemap \
	mojo/public/cpp/base/file_info.typemap \
	mojo/public/cpp/base/file_path.typemap \
	mojo/public/cpp/base/process_id.typemap \
	mojo/public/cpp/base/read_only_buffer.typemap \
	mojo/public/cpp/base/ref_counted_memory.typemap \
	mojo/public/cpp/base/shared_memory.typemap \
	mojo/public/cpp/base/string16.typemap \
	mojo/public/cpp/base/text_direction.typemap \
	mojo/public/cpp/base/thread_priority.typemap \
	mojo/public/cpp/base/time.typemap \
	mojo/public/cpp/base/unguessable_token.typemap \
	mojo/public/cpp/base/values.typemap \
	ui/gfx/geometry/mojo/geometry.typemap \
	ui/gfx/range/mojo/range.typemap \

LIB_MOJO_ROOT := $(LOCAL_PATH)
LOCAL_SOURCE_ROOT := $(LOCAL_PATH)

# Compiles all local mojom files.
include $(LOCAL_PATH)/generate_mojom_sources.mk

LOCAL_SRC_FILES := \
	chromecast/external_mojo/public/cpp/common.cc \
	chromecast/external_mojo/public/cpp/external_connector.cc \
	chromecast/external_mojo/public/cpp/external_service.cc \
	ipc/ipc_message.cc \
	ipc/ipc_message_attachment.cc \
	ipc/ipc_message_attachment_set.cc \
	ipc/ipc_message_utils.cc \
	ipc/ipc_mojo_handle_attachment.cc \
	ipc/ipc_mojo_message_helper.cc \
	ipc/ipc_mojo_param_traits.cc \
	ipc/ipc_platform_file_attachment_posix.cc \
	ipc/native_handle_type_converters.cc \
	mojo/core/broker_host.cc \
	mojo/core/broker_posix.cc \
	mojo/core/channel.cc \
	mojo/core/channel_posix.cc \
	mojo/core/configuration.cc \
	mojo/core/connection_params.cc \
	mojo/core/core.cc \
	mojo/core/data_pipe_consumer_dispatcher.cc \
	mojo/core/data_pipe_control_message.cc \
	mojo/core/data_pipe_producer_dispatcher.cc \
	mojo/core/dispatcher.cc \
	mojo/core/entrypoints.cc \
	mojo/core/handle_table.cc \
	mojo/core/invitation_dispatcher.cc \
	mojo/core/message_pipe_dispatcher.cc \
	mojo/core/mojo_core.cc \
	mojo/core/node_channel.cc \
	mojo/core/node_controller.cc \
	mojo/core/platform_handle_dispatcher.cc \
	mojo/core/platform_handle_in_transit.cc \
	mojo/core/platform_handle_utils.cc \
	mojo/core/platform_shared_memory_mapping.cc \
	mojo/core/request_context.cc \
	mojo/core/scoped_process_handle.cc \
	mojo/core/shared_buffer_dispatcher.cc \
	mojo/core/user_message_impl.cc \
	mojo/core/watch.cc \
	mojo/core/watcher_dispatcher.cc \
	mojo/core/watcher_set.cc \
	mojo/core/embedder/embedder.cc \
	mojo/core/embedder/scoped_ipc_support.cc \
	mojo/core/ports/event.cc \
	mojo/core/ports/message_queue.cc \
	mojo/core/ports/name.cc \
	mojo/core/ports/node.cc \
	mojo/core/ports/port.cc \
	mojo/core/ports/port_locker.cc \
	mojo/core/ports/port_ref.cc \
	mojo/core/ports/user_message.cc \
	mojo/public/c/system/thunks.cc \
	mojo/public/cpp/base/big_buffer.cc \
	mojo/public/cpp/base/big_buffer_mojom_traits.cc \
	mojo/public/cpp/base/big_string_mojom_traits.cc \
	mojo/public/cpp/base/file_info_mojom_traits.cc \
	mojo/public/cpp/base/file_mojom_traits.cc \
	mojo/public/cpp/base/file_path_mojom_traits.cc \
	mojo/public/cpp/base/process_id_mojom_traits \
	mojo/public/cpp/base/read_only_buffer_mojom_traits.cc \
	mojo/public/cpp/base/ref_counted_memory_mojom_traits.cc \
	mojo/public/cpp/base/shared_memory_mojom_traits.cc \
	mojo/public/cpp/base/string16_mojom_traits.cc \
	mojo/public/cpp/base/text_direction_mojom_traits.cc \
	mojo/public/cpp/base/thread_priority_mojom_traits.cc \
	mojo/public/cpp/base/time_mojom_traits.cc \
	mojo/public/cpp/base/unguessable_token_mojom_traits.cc \
	mojo/public/cpp/base/values_mojom_traits.cc \
	mojo/public/cpp/bindings/lib/array_internal.cc \
	mojo/public/cpp/bindings/lib/associated_binding.cc \
	mojo/public/cpp/bindings/lib/associated_group.cc \
	mojo/public/cpp/bindings/lib/associated_group_controller.cc \
	mojo/public/cpp/bindings/lib/associated_interface_ptr.cc \
	mojo/public/cpp/bindings/lib/associated_interface_ptr_state.cc \
	mojo/public/cpp/bindings/lib/binding_state.cc \
	mojo/public/cpp/bindings/lib/buffer.cc \
	mojo/public/cpp/bindings/lib/connector.cc \
	mojo/public/cpp/bindings/lib/control_message_handler.cc \
	mojo/public/cpp/bindings/lib/control_message_proxy.cc \
	mojo/public/cpp/bindings/lib/filter_chain.cc \
	mojo/public/cpp/bindings/lib/fixed_buffer.cc \
	mojo/public/cpp/bindings/lib/interface_endpoint_client.cc \
	mojo/public/cpp/bindings/lib/interface_ptr_state.cc \
	mojo/public/cpp/bindings/lib/message.cc \
	mojo/public/cpp/bindings/lib/message_dumper.cc \
	mojo/public/cpp/bindings/lib/message_header_validator.cc \
	mojo/public/cpp/bindings/lib/message_internal.cc \
	mojo/public/cpp/bindings/lib/multiplex_router.cc \
	mojo/public/cpp/bindings/lib/native_struct_serialization.cc \
	mojo/public/cpp/bindings/lib/pipe_control_message_handler.cc \
	mojo/public/cpp/bindings/lib/pipe_control_message_proxy.cc \
	mojo/public/cpp/bindings/lib/scoped_interface_endpoint_handle.cc \
	mojo/public/cpp/bindings/lib/sequence_local_sync_event_watcher.cc \
	mojo/public/cpp/bindings/lib/serialization_context.cc \
	mojo/public/cpp/bindings/lib/sync_call_restrictions.cc \
	mojo/public/cpp/bindings/lib/sync_event_watcher.cc \
	mojo/public/cpp/bindings/lib/sync_handle_registry.cc \
	mojo/public/cpp/bindings/lib/sync_handle_watcher.cc \
	mojo/public/cpp/bindings/lib/task_runner_helper.cc \
	mojo/public/cpp/bindings/lib/unserialized_message_context.cc \
	mojo/public/cpp/bindings/lib/validation_context.cc \
	mojo/public/cpp/bindings/lib/validation_errors.cc \
	mojo/public/cpp/bindings/lib/validation_util.cc \
	mojo/public/cpp/platform/named_platform_channel.cc \
	mojo/public/cpp/platform/named_platform_channel_posix.cc \
	mojo/public/cpp/platform/platform_channel.cc \
	mojo/public/cpp/platform/platform_channel_endpoint.cc \
	mojo/public/cpp/platform/platform_channel_server_endpoint.cc \
	mojo/public/cpp/platform/platform_handle.cc \
	mojo/public/cpp/platform/socket_utils_posix.cc \
	mojo/public/cpp/system/buffer.cc \
	mojo/public/cpp/system/data_pipe_drainer.cc \
	mojo/public/cpp/system/data_pipe_utils.cc \
	mojo/public/cpp/system/file_data_pipe_producer.cc \
	mojo/public/cpp/system/handle_signal_tracker.cc \
	mojo/public/cpp/system/invitation.cc \
	mojo/public/cpp/system/isolated_connection.cc \
	mojo/public/cpp/system/message_pipe.cc \
	mojo/public/cpp/system/platform_handle.cc \
	mojo/public/cpp/system/scope_to_message_pipe.cc \
	mojo/public/cpp/system/simple_watcher.cc \
	mojo/public/cpp/system/string_data_pipe_producer.cc \
	mojo/public/cpp/system/trap.cc \
	mojo/public/cpp/system/wait.cc \
	mojo/public/cpp/system/wait_set.cc \

LOCAL_CFLAGS := \
	-DMOJO_CORE_LEGACY_PROTOCOL \
	-Wall \
	-Werror \
	-Wno-missing-field-initializers \
	-Wno-unused-parameter \

# We use OS_POSIX since we need to communicate with Chrome.
# We also pass NO_ASHMEM to make base::SharedMemory avoid using it and prefer
# the POSIX versions.
LOCAL_CPPFLAGS := \
	-DNO_ASHMEM \
	-Wno-extra \
	-Wno-ignored-qualifiers \
	-Wno-non-virtual-dtor \
	-Wno-sign-promo \
	-std=c++14 \

LOCAL_SHARED_LIBRARIES := libevent liblog libchrome libchrome-crypto

LOCAL_EXPORT_C_INCLUDE_DIRS += \
	external/gtest/include \

include $(BUILD_SHARED_LIBRARY)
