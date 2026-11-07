NUGGET := ../nugget
TARGET := projectname
TYPE := ps-exe
# REDUX is the path to the pcsx-redux executable.
# If it's not on the PATH, replace it with the full path
REDUX := $(shell which pcsx-redux)

DATADIR := DATA
AUDIODIR := AUDIO
SRCDIR := src

SRCS := $(NUGGET)/common/crt0/crt0.s $(SRCDIR)/main.c $(SRCDIR)/display.c $(SRCDIR)/player.c $(SRCDIR)/enemy.c

CPPFLAGS += -I$(NUGGET)/psyq/include
LDFLAGS += -L$(NUGGET)/psyq/lib
LDFLAGS += -Wl,--start-group
# Add any psyq libraries here as needed (e.g. -lcd, -lspu)
LDFLAGS += -lapi -lc -lc2 -lcard -lcd -letc -lgpu -lgs -lgte -lmath -lmcrd -lpad -lpress -lsio -lsnd -lspu
LDFLAGS += -Wl,--end-group

include $(NUGGET)/common.mk

# Generate bin and cue files using the mkpsxiso and the helper script
image: $(TARGET).bin $(TARGET).cue


# Multiple outputs are generated from a single invocation, so group them with `&:`
$(TARGET).bin $(TARGET).cue &: $(TARGET).ps-exe
	python ../mkpsxiso-helper/mkpsxiso-helper.py $(TARGET) -i $(TARGET).ps-exe -d $(DATADIR) -a $(AUDIODIR)/* -Sx

# Load disc image into pcsx-redux and play
play: $(TARGET).bin $(TARGET).cue
	$(REDUX) -iso=$(TARGET).cue -run

# Build and run the ps-exe directly (no ISO needed)
run: $(TARGET).ps-exe
	$(REDUX) -exe=$(TARGET).ps-exe -run

# Patch psyq headers to properly include what they use,
# so build does not depend on include order.
# HIGHLY RECOMMENDED to do this first, before working.
fix-headers: $(NUGGET)/psyq/include
	cp -r ../psyq_include_what_you_use/include/ $(NUGGET)/psyq/

clean::
	rm -f *.bin *.cue *.xml

.PHONY: image play run
