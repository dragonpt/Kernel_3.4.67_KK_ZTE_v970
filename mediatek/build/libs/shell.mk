include $(_prefix_)/mediatek/build/Makefile
$(call codebase-path,$(_postfix_),$(_prefix_))

# export variables to shell environments
ifeq ($(strip $(MTK_HYBRID_PRODUCTFILE)),)
    $(eval ADDITIONAL_LIST := $(strip $(foreach p,$(MTK_PROJECT_CONFIGS),\
        $(shell cat $p | grep -v "^\s*#" | sed 's/\s*=\s*.*//g'))))
else
    $(eval ADDITIONAL_LIST := $(strip $(foreach p,$(MTK_PROJECT_CONFIGS),\
        $(shell cat $p | grep -P "\b\s*=\s*\b" | grep -v -P "^\s*#|:|\+" | sed 's/\s*=\s*.*//g'))))
    ADDITIONAL_LIST += MTK_HYBRID_PRODUCTFILE
endif

# TODO this should be done automatically to avoid issues
VARIABLE_LIST := \
    TO_ROOT MTK_PROJECT MTK_ROOT MTK_ROOT_CUSTOM_OUT MKT_ROOT_CONFIG_OUT MTK_ROOT_PLATFORM MTK_ROOT_CONFIG \
    MTK_ROOT_BUILD MTK_ROOT_SOURCE MTK_ROOT_CUSTOM MTK_PATH_PLATFORM \
    MTK_PATH_BUILD MTK_PATH_SOURCE MTK_PATH_CUSTOM MTK_ROOT_GEN_CONFIG \
    MTK_CUSTOM_FOLDERS MTK_CONFIG_FOLDERS MTK_PROJECT_CONFIGS FULL_PROJECT \
    $(ADDITIONAL_LIST)

$(foreach v,$(VARIABLE_LIST), $(info export $(v)="$($v)"))

ifneq ($(strip $(MTK_HYBRID_PRODUCTFILE)),)
    $(foreach f,$(strip $(shell cat $(call relative-path,$(addprefix ../../../,$(strip $(MTK_HYBRID_PRODUCTFILE)))) | \
        grep -P "\b\s*=\s*\b" | grep -v -P "^\s*#|:|\+" | sed 's/\s*=\s*/=/g')), $(eval export $(f)))
endif

all:
	@echo "export MTK_LOAD_CONFIG=1"
