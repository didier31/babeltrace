check_SCRIPTS = test_intersection \
		bt_python_helper.py \
		test_multi_trace_intersection.py

dist_noinst_SCRIPTS = test_multi_trace_intersection.py
EXTRA_DIST=test_multi_trace_intersection.py

all-local:
	@srcdir_id=$(shell ls -id ${srcdir} | cut -f1 -d' '); \
	builddir_id=$(shell ls -id ${builddir} | cut -f1 -d' '); \
	if [ $${srcdir_id} != $${builddir_id} ]; then \
		for script in $(EXTRA_DIST); do \
			cp -f $(srcdir)/$$script $(builddir); \
		done; \
	fi

clean-local:
	@srcdir_id=$(shell ls -id ${srcdir} | cut -f1 -d' '); \
	builddir_id=$(shell ls -id ${builddir} | cut -f1 -d' '); \
	if [ $${srcdir_id} != $${builddir_id} ]; then \
		for script in $(EXTRA_DIST); do \
			rm -f $(builddir)/$$script; \
		done; \
	fi
