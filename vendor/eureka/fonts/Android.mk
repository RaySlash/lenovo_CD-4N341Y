#
# Copyright (C) 2012 The Android Open Source Project
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
#

LOCAL_PATH := $(call my-dir)

# If USE_MINIMALIST_FONTS is defined, install single small fonts.
# Otherwise, install full fonts.
include $(CLEAR_VARS)
FONTS_PATH := $(LOCAL_PATH)
LOCAL_MODULE := fonts
LOCAL_MODULE_TAGS := optional
LOCAL_REQUIRED_MODULES := acp
include $(BUILD_PHONY_PACKAGE)

.PHONY: copy_fonts
copy_fonts:
	mkdir -p $(TARGET_OUT)/usr/share/fonts/
	rsync -a $(FONTS_PATH)/etc $(TARGET_OUT)
ifeq ($(USE_MINIMALIST_FONTS),true)
	rsync -a $(FONTS_PATH)/usr/share/fonts/DroidSans.ttf \
	  $(TARGET_OUT)/usr/share/fonts/
	rsync -a $(FONTS_PATH)/cache/minimalist/* \
	  $(TARGET_OUT)/etc/fonts
else
	rsync -a $(FONTS_PATH)/usr/share/fonts/* \
	  $(TARGET_OUT)/usr/share/fonts/
	rsync -a $(FONTS_PATH)/cache/complete/* \
	  $(TARGET_OUT)/etc/fonts
endif # USE_MINIMALIST_FONTS

$(LOCAL_BUILT_MODULE): copy_fonts

# NOTICE file
# PHONY_PACKAGE rule generates unreadable description. Use make-notice-file instead.
$(eval $(call make-notice-file,/system/etc/fonts/,$(LOCAL_PATH)/NOTICE.fonts))
$(eval $(call make-notice-file,/system/usr/share/fonts/,$(LOCAL_PATH)/NOTICE.fonts))
