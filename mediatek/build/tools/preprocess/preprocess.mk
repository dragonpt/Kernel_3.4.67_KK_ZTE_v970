# *************************************************************************
# Set shell align with Android build system
# *************************************************************************
SHELL        := /bin/bash

.PHONY : run-preprocess remove-preprocess clean-preprocess clean-preprocessed gen-preprocessed remove-preprocessed

HAVE_PREPROCESS_FLOW := true
HAVE_PREPROCESS_FLOW := $(strip $(HAVE_PREPROCESS_FLOW))
MP_REMOVE_PREPROCESS := $(strip $(MP_REMOVE_PREPROCESS))

temp_all_preprocess_list := $(shell python mediatek/build/tools/preprocess/getPreprocessFiles.py; \
                                    if [ $$? != 0 ]; then echo GET_PREPROCESS_FILES_PY_FAIL; fi \
                              )
ifneq ($(filter GET_PREPROCESS_FILES_PY_FAIL, $(temp_all_preprocess_list)),)
  $(error mediatek/build/tools/getPreprocessFiles.py run fail, please check whiteList.xml)
else
  ALL_NEED_PREPROCESS_FILES := $(strip $(temp_all_preprocess_list))
endif

ALL_CTXT_FILES := $(filter %.ctxt, $(ALL_NEED_PREPROCESS_FILES))
ALL_GEN_CTXT_FILES := $(patsubst %.ctxt,%.txt,$(ALL_CTXT_FILES))

ALL_CXML_FILES := $(filter %.cxml, $(ALL_NEED_PREPROCESS_FILES))
ALL_GEN_CXML_FILES := $(patsubst %.cxml,%.xml,$(ALL_CXML_FILES))

ALL_CJAVA_FILES := $(filter %.cjava, $(ALL_NEED_PREPROCESS_FILES))
ALL_GEN_CJAVA_FILES := $(patsubst %.cjava,%.java,$(ALL_CJAVA_FILES))

ALL_UNPREPROCESS_FILES := $(ALL_CTXT_FILES) $(ALL_CXML_FILES) $(ALL_CJAVA_FILES)
ALL_PREPROCESS_GEN_FILES := $(ALL_GEN_CTXT_FILES) $(ALL_GEN_CXML_FILES) $(ALL_GEN_CJAVA_FILES)

include mediatek/build/tools/preprocess/preprocessFeatureList.mk

define mtk.custom.generate-macros.preprocess
$(strip $(foreach t,$(AUTO_ADD_PREPROCESS_DEFINE_BY_NAME),$(call .if-cfg-on,$(t),-D$(strip $(t))=$(strip $(t)))))
endef

ALL_PREPROCESS_DEFINITIONS := $(call mtk.custom.generate-macros.preprocess)

ifneq ($(ALL_GEN_CJAVA_FILES),)
$(ALL_GEN_CJAVA_FILES) : %.java : %.cjava
	@echo pre-process $< $(DEAL_STDOUT)
	@gcc -x c -P $(ALL_PREPROCESS_DEFINITIONS) -C -E $< -o $@ $(DEAL_STDOUT)
endif

ifneq ($(ALL_GEN_CXML_FILES),)
$(ALL_GEN_CXML_FILES) : %.xml : %.cxml
	@echo pre-process $< $(DEAL_STDOUT)
	@gcc -x c -P -C $(ALL_PREPROCESS_DEFINITIONS) -E $< -o $@ $(DEAL_STDOUT)
endif

ifneq ($(ALL_GEN_CTXT_FILES),)
$(ALL_GEN_CTXT_FILES) : %.txt : %.ctxt
	@echo pre-process $< $(DEAL_STDOUT)
	@gcc -x c -P -C $(ALL_PREPROCESS_DEFINITIONS) -E $< -o $@ $(DEAL_STDOUT)
endif

remove-preprocessed clean-preprocessed:
ifneq ($(ALL_PREPROCESS_GEN_FILES),)
	@rm -f $(ALL_PREPROCESS_GEN_FILES) $(DEAL_STDOUT)
endif
	@echo Done clean pre-processed files

gen-preprocessed-and-remove-preprocess: gen-preprocessed
ifneq ($(ALL_UNPREPROCESS_FILES),)
	@rm -f $(ALL_UNPREPROCESS_FILES) $(DEAL_STDOUT)
endif
	@echo Done clean pre-process files

gen-preprocessed: $(ALL_PREPROCESS_GEN_FILES)
	@echo Done Gen pre-process

ifeq ($(MP_REMOVE_PREPROCESS),true)
run-preprocess: gen-preprocessed-and-remove-preprocess
endif
run-preprocess: gen-preprocessed
	@echo Done pre-process
