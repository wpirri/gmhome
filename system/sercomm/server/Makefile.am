sbin_PROGRAMS = gmh_svsercomm
gmh_svsercomm_SOURCES = server.cc sercomm.cc serdev.cc
gmh_svsercomm_LDADD = -lgmqw
gmh_svsercomm_CPPFLAGS =

AM_CXXFLAGS = \
 -Wall $(gmon_debug_global)

install-data-local:
	./update-tables.sh . $(localstatedir)/lib/gmonitor server $(sbindir)/gmh_svsercomm
	./update-tables.sh . $(localstatedir)/lib/gmonitor funcion
