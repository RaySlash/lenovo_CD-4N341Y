# Generate source files for mojom sources $(LOCAL_MOJOM_FILES).
#
# Following path needs to be setup first by the consumer:
# $(LIB_MOJO_ROOT) - point to the root of libmojo source, where
# the script files live.
# $(LOCAL_SOURCE_ROOT) - point to the root of the project source
# $(LOCAL_MOJOM_FILES) - mojom files
# $(LOCAL_MOJOM_TYPEMAP_FILES) - mojom typemap files

include $(LIB_MOJO_ROOT)/mojom_template_tools.mk

generated_sources_dir := $(local-intermediates-dir)/gen
generated_templates_dir := $(generated_sources_dir)/templates
generated_typemap_dir := $(generated_sources_dir)/typemap
generated_pickles_dir := $(generated_sources_dir)/pickles

# Generate template files
gen_templates := $(generated_templates_dir)/.stamp
$(gen_templates) : PRIVATE_TOOL := $(MOJOM_BINDINGS_GENERATOR)
$(gen_templates) : PRIVATE_OUT_DIR := $(generated_templates_dir)
$(gen_templates) : $(MOJOM_TEMPLATE_TOOLS)
	@echo generate_mojo_templates: $(PRIVATE_OUT_DIR)
	$(hide) rm -rf $(dir $@)
	$(hide) mkdir -p $(dir $@)
	$(hide) python $(PRIVATE_TOOL) --use_bundled_pylibs precompile \
		-o $(PRIVATE_OUT_DIR)
	$(hide) touch $@

# Make the files that are actually generated depend on the .stamp file.
$(generated_templates_dir)/cpp_templates.zip: $(gen_templates)
	$(hide) touch $@

LOCAL_MOJOM_BINDINGS_GENERATOR_FLAGS := \
	--use_once_callback \
	--use_new_wrapper_types \
	--disallow_native_types \

# Generate typemapping file.
ifneq ($(LOCAL_MOJOM_TYPEMAP_FILES),)
typemap_sources := $(addprefix $(LOCAL_SOURCE_ROOT)/,$(LOCAL_MOJOM_TYPEMAP_FILES))
typemap_outfile := $(generated_typemap_dir)/common_custom_types__type_mappings
gen_typemap := $(generated_typemap_dir)/.stamp
$(gen_typemap) : PRIVATE_TOOL := $(LOCAL_MOJO_ROOT)/libchrome_tools/mojom_generate_type_mappings.py
$(gen_typemap) : PRIVATE_OUTPUT_FILE := $(typemap_outfile)
$(gen_typemap) : PRIVATE_SOURCES := $(typemap_sources)
$(gen_typemap) : $(MOJOM_TEMPLATE_TOOLS)
	@echo generate_mojo_typemap: $(PRIVATE_OUTPUT_FILE)
	python $(PRIVATE_TOOL) --output $(PRIVATE_OUTPUT_FILE) \
	$(PRIVATE_SOURCES)
	mkdir -p $(dir $@)
	touch $(dir $@)

LOCAL_MOJOM_BINDINGS_GENERATOR_FLAGS += \
	--typemap $(typemap_outfile)
else
gen_typemap := $(generated_typemap_dir)/.stamp
$(gen_typemap) : $(MOJOM_TEMPLATE_TOOLS)
	mkdir -p $(dir $@)
	touch $(dir $@)
endif

mojom_sources := $(addprefix $(LOCAL_SOURCE_ROOT)/,$(LOCAL_MOJOM_FILES))

# Generate pickle files.
gen_pickles := $(generated_sources_dir)/pickles/.stamp
$(gen_pickles) : $(mojom_sources)
$(gen_pickles) : PRIVATE_TOOL := $(MOJOM_BINDINGS_GENERATOR)
$(gen_pickles) : PRIVATE_OUTPUT_DIR := $(generated_sources_dir)
$(gen_pickles) : PRIVATE_SOURCE_ROOT := $(LOCAL_SOURCE_ROOT)
$(gen_pickles) : PRIVATE_SOURCES := $(mojom_sources)
$(gen_pickles) : PRIVATE_FLAGS := $(LOCAL_MOJOM_BINDINGS_GENERATOR_FLAGS)
$(gen_pickles) : $(MOJOM_TEMPLATE_TOOLS)
	@echo generate_mojo_pickles: $(PRIVATE_OUT_DIR)
	python $(PRIVATE_TOOL) --use_bundled_pylibs parse \
	$(PRIVATE_SOURCES) -o $(PRIVATE_OUTPUT_DIR) -d $(PRIVATE_SOURCE_ROOT) \
	$(PRIVATE_FLAGS)
	mkdir -p $(dir $@)
	touch $@

# Generate source files.
fullpath_mojom_files := $(addprefix $(generated_sources_dir)/,$(LOCAL_MOJOM_FILES))
gen_cc := $(addsuffix .cc,$(fullpath_mojom_files))
gen_h := $(addsuffix .h,$(fullpath_mojom_files))
gen_internal_h := $(addsuffix -internal.h,$(fullpath_mojom_files))
gen_shared_cc := $(addsuffix -shared.cc,$(fullpath_mojom_files))
gen_shared_h := $(addsuffix -shared.h,$(fullpath_mojom_files))
gen_src := $(gen_shared_cc) $(gen_cc)
gen_source := $(generated_sources_dir)/.stamp
$(gen_source) : $(mojom_sources)
$(gen_source) : PRIVATE_WRAPPER := $(LIB_MOJO_ROOT)/libchrome_tools/mojom_bindings_generator_wrapper.py
$(gen_source) : PRIVATE_TOOL := $(MOJOM_BINDINGS_GENERATOR)
$(gen_source) : PRIVATE_OUTPUT_DIR := $(generated_sources_dir)
$(gen_source) : PRIVATE_SOURCE_ROOT := $(LOCAL_SOURCE_ROOT)
$(gen_source) : PRIVATE_SOURCES := $(mojom_sources)
$(gen_source) : PRIVATE_TEMPLATES_DIR := $(generated_templates_dir)
$(gen_source) : PRIVATE_FLAGS := $(LOCAL_MOJOM_BINDINGS_GENERATOR_FLAGS)
$(gen_source) : $(MOJOM_TEMPLATE_TOOLS) $(gen_templates) $(gen_pickles) $(gen_typemap)
	python $(PRIVATE_WRAPPER) $(PRIVATE_TOOL) --use_bundled_pylibs generate \
		$(PRIVATE_SOURCES) -I $(PRIVATE_SOURCE_ROOT) -o $(PRIVATE_OUTPUT_DIR) \
		-d $(PRIVATE_SOURCE_ROOT) \
		-g c++ \
		--bytecode_path $(PRIVATE_TEMPLATES_DIR) \
		$(PRIVATE_FLAGS)
	mkdir -p $(dir $@)
	touch $@

$(gen_src) : $(gen_source)

LOCAL_GENERATED_SOURCES += $(gen_src)

LOCAL_C_INCLUDES += $(generated_sources_dir)
LOCAL_EXPORT_C_INCLUDE_DIRS += $(generated_sources_dir)

