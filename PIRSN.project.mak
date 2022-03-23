####################################################################
# Automatically-generated file. Do not edit!                       #
# Makefile Version 10                                              #
####################################################################

BASE_SDK_PATH = C:/Users/█████████/SimplicityStudio/SDKs/gecko_sdk
UNAME:=$(shell uname -s | sed -e 's/^\(CYGWIN\).*/\1/' | sed -e 's/^\(MINGW\).*/\1/')
ifeq ($(UNAME),MINGW)
# Translate "C:/super" into "/C/super" for MinGW make.
SDK_PATH := /$(shell echo $(BASE_SDK_PATH) | sed s/://)
endif
SDK_PATH ?= $(BASE_SDK_PATH)
COPIED_SDK_PATH ?= gecko_sdk_4.0.2

# This uses the explicit build rules below
PROJECT_SOURCE_FILES =

C_SOURCE_FILES   += $(filter %.c, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cpp, $(PROJECT_SOURCE_FILES))
CXX_SOURCE_FILES += $(filter %.cc, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.s, $(PROJECT_SOURCE_FILES))
ASM_SOURCE_FILES += $(filter %.S, $(PROJECT_SOURCE_FILES))
LIB_FILES        += $(filter %.a, $(PROJECT_SOURCE_FILES))

C_DEFS += \

ASM_DEFS += \

INCLUDES += \

GROUP_START =-Wl,--start-group
GROUP_END =-Wl,--end-group

PROJECT_LIBS = \
 -lc \
 -lm

LIBS += $(GROUP_START) $(PROJECT_LIBS) $(GROUP_END)

LIB_FILES += $(filter %.a, $(PROJECT_LIBS))

C_FLAGS += \
 -std=c99 \
 -Wall \
 -Wextra \
 -Os \
 -g

CXX_FLAGS += \
 -std=c++11 \
 -Wall \
 -Wextra \
 -Os \
 -g

ASM_FLAGS += \
 -x assembler-with-cpp

LD_FLAGS += \
 -Xlinker -Map=$(OUTPUT_DIR)/$(PROJECTNAME).map


####################################################################
# SDK Build Rules                                                  #
####################################################################
# Automatically-generated Simplicity Studio Metadata
# Please do not edit or delete these lines!
# SIMPLICITY_STUDIO_METADATA=eJztWW1PGzkQ/ivRqh/aK9kFrpXKCqhOCZVyIk2Ul+NOXbRybBN82V2vbC+EIv77jb0veduES1ISKhWhJB6Px48fz9hj+9Fqd1p/XtR6frfV79QuupZrnX4eh0HljgrJeHTmWUf2oWdVaIQ5YdEQBP3el+onz/p87gkvOo0F/5diVYFGkXRDTmgAOrdKxa7j3N/f25IFaCBtzENHSqerEsK4TTEXFMxC65gK9dDF8A3tMmueZYxXKqc3PCBUVCIU6mqUKD6kUVGtFVhA82obC+xjHt2wof4J9hPBQK51XKfmOn0Jw3KuuBhJhRSMz6lTOVI8dhrH9a7TbnS6X52sE2fBmpODclJUeTHVSYSxmGOp0xuUBDCSCgzfcDKRxEioGg9jaDBgAVMPUAtixXmAbxGLSuokGc1LgdGcXKi1YUQYPhPg0/1gH9rHtn90fPzh5OTTx8OjacbMHLmESixYrCGfnzqLsnyoM4NLp9zJZsmUrAOr22i2Lxu1Ru8fv9vr1xstv9mq9y+NN3179CxBQ35HiWe5NyiQ9MCzBgkLFIsuxjhIoFOo+XY9EXd5IvBECtiSAJzF9azc29xm0wgzr8ukZ563wvPaqVLmeqBZybozzZRIUiEjppxgO+3XllQlsTYS84hGqmuK9hBjox6TcKb9uQekGOZYBIPTMSRhvpWiIu3I/k1/OplewWU+onPPmhABY9Z2nw62I7GIm0WXfjX8Jjhv/1NR+2r4I+niAt4JLYbyF4sbemEtDY5f9G1GX4+GcYAU3TGB16W7ULveLBIaz5tPaUBSmtSYfieJDahlO/pqZ9H2lmYzTmG28nJoZrnfN56yDXMlpsI2KMH/acAGAomHL5C7TXrB09hXKYYlijq/apnsBkq60AA3tiZS4NAOWDSiwiDlRrhUx4YNlUJaRJsoNn3rNncoSHT3xts3Q6CJA3PPYMi1bOxD4hcRJMgMAHxysqP+x+MlCN6/PzraDYZ7JCKz66Eg2NM0FBDoWAm0bxAxJShSDM/gMPvKridEUJ8KwYXcFxStEbLv5vgyg0Gy7/8fQohGVJ8iYUChXsqm/H8exwrVZcFalYqcrROxK/qI4zUAxfHy+E1BrRXEP4il3Hv8+XCuXhnJ7kl6eUQb8bO41lSvMtkeOXpRVOvwtDT0qy25D4ZeBk95xrJErUha/LTshyiehfJ3Zs+rVCGxOXvzttXvtfs9v97ovHPevM3uC7/+0bx4Z5vG2/K4gJxJvJANTpm+ZFIV5gvYwxIgzgTJhhiRlDQc/DCQYygUJqv3TN1WjRNtj3zqLpARSMHZ78cpyUTBLEVoSIk+MhFbt7CHUWIXHjFAks4PbwtzmAmcBLCZ0JhGBM4QD5snQ69nVBGH3RAOGtukMRt0D4tGeXhvY2vvU1Qsis9M0px2qmTr0LO5uqUigCBbPwhLjpDPtrkJqZQwlmpAo6G6PTvcU8zG8XrkTev/hPRNvTS89HVF6f3pvu9Q5i7F17o+SZ+oJranns7KJu9m5vZk4Z0AqoqLwGyBAFGeTJDlq0bxVLZ0Ssuv7jqt9kWn1zBvSI/6Bcz14NuDYxu6o6SrOB79hQRDsJNKLXb1h1bQfxAHMQMtMrrkOE23sgo3/zGkeMR9SUa+eSvLxU/ph75iLJ7rdtvzE/CRGtGXSlI/oV1bT/8BBiaI7w===END_SIMPLICITY_STUDIO_METADATA
# END OF METADATA