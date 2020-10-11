sbin_PROGRAMS = gmh_voicesrv
gmh_voicesrv_SOURCES = voicesrv.cc
gmh_voicesrv_LDADD = -lgmqw \
		-ldl -lncurses \
		-L/usr/local/festival/festival/src/lib -lFestival \
		-L/usr/local/festival/speech_tools/lib -lestools -lestbase -leststring

gmh_voicesrv_CPPFLAGS = -I/usr/local/festival/festival/src/include \
		-I/usr/local/festival/speech_tools/include \
		-Wno-deprecated

AM_CXXFLAGS = \
 -Wall $(gmon_debug_global)

install-data-local:
	./update-tables.sh . $(localstatedir)/lib/gmonitor server $(sbindir)/gmh_voicesrv
	./update-tables.sh . $(localstatedir)/lib/gmonitor funcion
