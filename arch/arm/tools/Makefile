#
# linux/arch/arm/tools/Makefile
#
# Copyright (C) 2001 Russell King
#

include/generated/mach-types.h: $(src)/gen-mach-types $(src)/mach-types FORCE
	@echo '  Generating $@'
	@mkdir -p $(dir $@)
	$(Q)set -e; tmp="$@.tmp.$$$$"; \
	trap 'rm -f "$$tmp"' EXIT HUP INT TERM; \
	$(AWK) -f $(filter-out FORCE,$^) > "$$tmp"; \
	test -s "$$tmp"; \
	cmp -s "$$tmp" "$@" || mv -f "$$tmp" "$@"
