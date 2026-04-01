WORKFLOW := build.yml
OUT_DIR  := firmware

# macOS: XIAO nRF52840 mounts as XIAO-SENSE in bootloader mode.
# Override if your system mounts it differently.
BOOT_LEFT   ?= /Volumes/XIAO-SENSE
BOOT_RIGHT  ?= /Volumes/XIAO-SENSE
BOOT_DONGLE ?= /Volumes/XIAO-SENSE

.PHONY: help build status download flash-left flash-right flash-dongle

help:
	@echo "TOTEM ZMK — dongle workflow"
	@echo ""
	@echo "  make build          Trigger GitHub Actions build"
	@echo "  make status         Show recent build runs"
	@echo "  make download       Download latest firmware artifacts → $(OUT_DIR)/"
	@echo "  make flash-left     Copy left UF2 to mounted bootloader drive"
	@echo "  make flash-right    Copy right UF2 to mounted bootloader drive"
	@echo "  make flash-dongle   Copy dongle UF2 to mounted bootloader drive"
	@echo ""
	@echo "Keymap:  config/totem.keymap"
	@echo ""
	@echo "Typical workflow:"
	@echo "  1. Edit config/totem.keymap"
	@echo "  2. make build"
	@echo "  3. make download   (once Actions completes)"
	@echo "  4. Double-tap reset on each device to enter bootloader, then flash:"
	@echo "     make flash-left && make flash-right && make flash-dongle"
	@echo ""
	@echo "Artifacts: totem_left (BLE peripheral), totem_right (BLE peripheral),"
	@echo "           totem_dongle (USB central + screen + ZMK Studio)"
	@echo ""
	@echo "First-time setup: flash settings_reset to both halves before"
	@echo "flashing normal firmware to clear any stale BLE bonds."
	@echo "  cp $(OUT_DIR)/settings_reset/zmk.uf2 /Volumes/XIAO-SENSE/zmk.uf2"

build:
	gh workflow run $(WORKFLOW)
	@echo "✓ Build triggered — run 'make status' to check progress"

status:
	gh run list --workflow=$(WORKFLOW) --limit=5

download:
	@mkdir -p $(OUT_DIR)
	$(eval RUN_ID := $(shell gh run list --workflow=$(WORKFLOW) --status=success --limit=1 --json databaseId -q '.[0].databaseId'))
	@test -n "$(RUN_ID)" || (echo "Error: no successful run found" && exit 1)
	gh run download $(RUN_ID) --dir $(OUT_DIR)
	@echo "✓ Artifacts downloaded to $(OUT_DIR)/"

flash-left:
	@test -d "$(BOOT_LEFT)" || (echo "Error: $(BOOT_LEFT) not mounted. Double-tap reset on left half first." && exit 1)
	cp $(OUT_DIR)/totem_left/zmk.uf2 "$(BOOT_LEFT)/zmk.uf2"
	@echo "✓ Left half flashed"

flash-right:
	@test -d "$(BOOT_RIGHT)" || (echo "Error: $(BOOT_RIGHT) not mounted. Double-tap reset on right half first." && exit 1)
	cp $(OUT_DIR)/totem_right/zmk.uf2 "$(BOOT_RIGHT)/zmk.uf2"
	@echo "✓ Right half flashed"

flash-dongle:
	@test -d "$(BOOT_DONGLE)" || (echo "Error: $(BOOT_DONGLE) not mounted. Double-tap reset on dongle first." && exit 1)
	cp $(OUT_DIR)/totem_dongle/zmk.uf2 "$(BOOT_DONGLE)/zmk.uf2"
	@echo "✓ Dongle flashed"
