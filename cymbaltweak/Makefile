ARCHS = arm64 arm64e

include $(THEOS)/makefiles/common.mk

TWEAK_NAME = cymbalTweak
cymbalTweak_FILES = $(wildcard ../cymbal/*c) Tweak.x
cymbalTweak_CFLAGS = -I../cymbal/

include $(THEOS_MAKE_PATH)/tweak.mk
