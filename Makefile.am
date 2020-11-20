# Copyright (C) 2009-2014 David Sugar, Tycho Softworks.
# Copyright (C) 2015 Cherokees of Idaho.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

MAINTAINERCLEANFILES = Makefile.in Makefile
RELEASE = -version-info $(LT_VERSION)
AM_CXXFLAGS = -I$(top_srcdir)/inc @CCAUDIO_FLAGS@

lib_LTLIBRARIES = libccaudio2.la

libccaudio2_la_LDFLAGS = @CCAUDIO_LIBS@ $(RELEASE)
libccaudio2_la_SOURCES = audiobase.cpp audiofile.cpp audiobuffer.cpp \
	codec.cpp detect.cpp dialers.cpp fileio.cpp friends.cpp \
	mapper.cpp oss.cpp osx.cpp resample.cpp stream.cpp w32.cpp \
	teltones.cpp tone.cpp 


