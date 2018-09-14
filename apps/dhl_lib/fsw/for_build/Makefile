###############################################################################
# File: CFS Library Makefile 
#
# $Id: Makefile 1.6 2009/07/09 11:41:26EDT rmcgraw Exp  $
#
# $Log: Makefile  $
# Revision 1.6 2009/07/09 11:41:26EDT rmcgraw 
# DCR8291:1 Changed CFE_MISSION_INC to CFS_MISSION_INC and added log if needed
#
###############################################################################
#
# Subsystem produced by this makefile.
#
APPTARGET = dhl_lib

# 
# Entry Point for task
# 
ENTRY_PT = DHL_LibInit 

#
# Object files required to build subsystem.
#
OBJS = dellingr_lib.o 
OBJS += cam_lib.o
OBJS += dagr_lib.o
OBJS += eps_lib.o
OBJS += cs_lib.o
OBJS += gps_lib.o
OBJS += inms_lib.o
OBJS += l3c_lib.o
OBJS += nm_lib.o
OBJS += rw_lib.o
OBJS += sol_lib.o
OBJS += ssc_lib.o
OBJS += tlc_lib.o
OBJS += deployable.o
OBJS += sim_input.o
OBJS += sim_lib.o
OBJS += mode_lib.o
OBJS += gyro_lib.o
OBJS += wallops_fss.o
OBJS += nano_fss.o

#
# Source files required to build subsystem; used to generate dependencies.
# As long as there are no assembly files this can be automated.
#
SOURCES = $(OBJS:.o=.c)


##
## Specify extra C Flags needed to build this subsystem
##
LOCAL_COPTS = 


##
## EXEDIR is defined here, just in case it needs to be different for a custom
## build
##
EXEDIR=../exe

##
## Certain OSs and Application Loaders require the following option for
## Shared libraries. Currently only needed for vxWorks 5.5 and RTEMS.
## For each shared library that this app depends on, you need to have an
## entry like the following:
##  -R../tst_lib/tst_lib.elf
##
SHARED_LIB_LINK = 

########################################################################
# Should not have to change below this line, except for customized 
# Mission and cFE directory structures
########################################################################

#
# Set build type to CFE_APP. This allows us to 
# define different compiler flags for the cFE Core and Apps.
# 
BUILD_TYPE = CFE_APP

## 
## Include all necessary cFE make rules
## Any of these can be copied to a local file and 
## changed if needed.
##
##
##       cfe-config.mak contains PSP and OS selection
##
include ../cfe/cfe-config.mak
##
##       debug-opts.mak contains debug switches
##
include ../cfe/debug-opts.mak
##
##       compiler-opts.mak contains compiler definitions and switches/defines
##
include $(CFE_PSP_SRC)/$(PSP)/make/compiler-opts.mak

##
## Setup the include path for this subsystem
## The OS specific includes are in the build-rules.make file
##
## If this subsystem needs include files from another app, add the path here.
##
INCLUDE_PATH = \
-I$(OSAL_SRC)/inc \
-I$(CFE_CORE_SRC)/inc \
-I$(CFE_CORE_SRC)/time \
-I$(CFE_PSP_SRC)/inc \
-I$(CFE_PSP_SRC)/$(PSP)/inc \
-I$(CFS_APP_SRC)/inc \
-I$(CFS_APP_SRC)/$(APPTARGET)/fsw/src \
-I$(CFS_MISSION_INC) \
-I$(CFS_PLATFORM_INC) \
-I../cfe/inc \
-I../inc \
-I../../../nanomind \
-I../../../nanomind/src \
-I../../../nanomind/build/include \
-I../../../nanomind/lib/libgomspace/include \
-I../../../nanomind/lib/libcsp/include \
-I../../../nanomind/lib/libio/include \
-I../../../nanomind/lib/libgomspace/include/freertos \
-I../../../nanomind/lib/libgomspace/include/dev \
-I../../../nanomind/lib/libgomspace/include/dev/arm \
-I../../../nanomind/lib/libgomspace/include/dev/util

##
## Define the VPATH make variable. 
## This can be modified to include source from another directory.
## If there is no corresponding app in the cfs-apps directory, then this can be discarded, or
## if the mission chooses to put the src in another directory such as "src", then that can be 
## added here as well.
##
VPATH := $(CFS_APP_SRC)/$(APPTARGET)/fsw/src/nanomind
VPATH += $(CFS_APP_SRC)/$(APPTARGET)/fsw/src/common

##
## Include the common make rules for building a cFE Application
##
include $(CFE_CORE_SRC)/make/app-rules.mak
