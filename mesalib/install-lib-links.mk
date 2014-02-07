# Provide compatibility with scripts for the old Mesa build system for
# a while by putting a link to the driver into /lib of the build tree.

all-local : .libs/install-mesa-links

.libs/install-mesa-links : $(lib_LTLIBRARIES)
	$(MKDIR_P) $(top_builddir)/$(LIB_DIR)
	for f in $(lib_LTLIBRARIES:%.la=.libs/%.so*); do	\
		if test -h .libs/$$f; then			\
			cp -d $$f $(top_builddir)/$(LIB_DIR);	\
		else						\
			ln -f $$f $(top_builddir)/$(LIB_DIR);	\
		fi;						\
	done && touch $@
