# Home Assistant ePaper Remote - Makefile
# Detects python command and provides convenient build targets

PYTHON := $(shell command -v python3 2>/dev/null || command -v python 2>/dev/null)

.PHONY: icons build-lilygo build-m5 upload-lilygo upload-m5 configurator clean help

help:
	@echo "Home Assistant ePaper Remote"
	@echo ""
	@echo "Usage: make <target>"
	@echo ""
	@echo "Targets:"
	@echo "  icons          Generate icons.h from PNG files in icons-buttons/"
	@echo "  configurator   Open the visual configurator"
	@echo "  build-lilygo   Build firmware for Lilygo T5 S3 Pro"
	@echo "  build-m5       Build firmware for M5Stack M5Paper S3"
	@echo "  upload-lilygo  Build and upload to Lilygo T5 S3 Pro"
	@echo "  upload-m5      Build and upload to M5Stack M5Paper S3"
	@echo "  clean          Remove build artifacts"
	@echo ""

icons:
ifndef PYTHON
	$(error Python not found. Please install Python 3 and Pillow: pip install Pillow)
endif
	@echo "Using Python: $(PYTHON)"
	$(PYTHON) generate-icons.py

configurator: icons
	@echo "Opening configurator..."
	@open tools/configurator.html 2>/dev/null || xdg-open tools/configurator.html 2>/dev/null || start tools/configurator.html

build-lilygo:
	platformio run -e lilygo-t5-s3

build-m5:
	platformio run -e m5-papers3

upload-lilygo:
	platformio run -e lilygo-t5-s3 --target upload

upload-m5:
	platformio run -e m5-papers3 --target upload

monitor:
	platformio run -e lilygo-t5-s3 --target monitor

clean:
	rm -rf .pio/build
