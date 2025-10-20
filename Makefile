VERSION := 5
STARTLOC :=
ARGS := -Cu -v${VERSION} ${ADDITIONAL_ARGS}

ifneq ($(STARTLOC),)
	ARGS += --define STARTLOC=${STARTLOC}
endif

.PHONY: all hl2

all: hl2

hl2: abbreviations.h
	$(MAKE) VERSION=5 hl2.zn

abbreviations.h:
	@echo "! this is a generated file, DO NOT EDIT UNLESS YOU KNOW WHAT YOU ARE DOING" > hl2util.h
	inform -e -u hl2.inf | grep --color=never "^ObviousExitsSub" >> hl2util.h

%.zn: %.inf
	inform ${ARGS} $*.inf

clean:
	rm -f *.z* *.log *.dbg inform-tests/*.z*