.PHONY: combadge flash

SOC ?= esp32s3
PORT ?= /dev/ttyUSB0
#CPPFLAGS ?=

ifeq "${SOC}" "esp32s3"
	override CPPFLAGS += -DSOC_ESP32_S3
else
	override CPPFLAGS += -DSOC_ESP32
endif

combadge:
	arduino-cli compile --clean --build-property build.extra_flags="${CPPFLAGS}" --fqbn esp32:esp32:${SOC}

flash:
	arduino-cli upload --fqbn esp32:esp32:${SOC} --port ${PORT}
