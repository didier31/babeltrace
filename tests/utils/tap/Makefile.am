noinst_LTLIBRARIES = libtap.la
libtap_la_SOURCES = tap.c tap.h

SCRIPT_LIST = tap.sh

dist_noinst_SCRIPTS = $(SCRIPT_LIST)

all-local:
	@srcdir_id=$(shell ls -id ${srcdir} | cut -f1 -d' '); \
	builddir_id=$(shell ls -id ${builddir} | cut -f1 -d' '); \
	if [ $${srcdir_id} != $${builddir_id} ]; then \
		for script in $(SCRIPT_LIST); do \
			cp -f $(srcdir)/$$script $(builddir); \
		done; \
	fi

clean-local:
	@srcdir_id=$(shell ls -id ${srcdir} | cut -f1 -d' '); \
	builddir_id=$(shell ls -id ${builddir} | cut -f1 -d' '); \
	if [ $${srcdir_id} != $${builddir_id} ]; then \
		for script in $(SCRIPT_LIST); do \
			rm -f $(builddir)/$$script; \
		done; \
	fi
