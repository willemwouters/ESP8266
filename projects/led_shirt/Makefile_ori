#
# Copyright (c) 2015, Lambertus Gorter <l.gorter@gmail.com>
# All rights reserved.
#
SDK_PATH      := ../..

include  $(SDK_PATH)/tools/host.mk

# The variable PROJECT_PATH is used by the SDK makefile to locate the project and its build location.
# It must be an absolute path.
export PROJECT_PATH := $(abspath .)


.PHONY: $(MAKEGOALS) distclean

ifdef MAKECMDGOALS
$(MAKECMDGOALS):
	$(MAKE) -C $(SDK_PATH) -f makefile.mk $@
else
all:
	$(MAKE) -C $(SDK_PATH) -f makefile.mk
endif
