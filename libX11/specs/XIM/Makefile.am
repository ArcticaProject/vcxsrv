
if ENABLE_SPECS
if HAVE_XMLTO

# Main DocBook/XML files (DOCTYPE book)
docbook = xim.xml

# Included chapters, appendix, images
chapters =				\
	dynamicflowsampleseq.svg	\
	dynamicflow.svg			\
	eventflow.svg			\
	sampleprotocolflow1.svg		\
	sampleprotocolflow2.svg		\
	staticflowsampleseq.svg		\
	staticflow.svg

# The location where the DocBook/XML files and their generated formats are installed
shelfdir = $(docdir)/XIM

# Generate DocBook/XML output formats with or without stylesheets
include $(top_srcdir)/docbook.am

endif HAVE_XMLTO
endif ENABLE_SPECS
