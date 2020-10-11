sbin_PROGRAMS = gmh_homectrl
gmh_homectrl_SOURCES = server.cc gmstring.cc
#gmh_homectrl_LDADD = -lgmq -lmysqlclient
gmh_homectrl_LDADD = -lgmq -lpq
gmh_homectrl_CPPFLAGS =

AM_CXXFLAGS = \
 -Wall $(gmon_debug_global)

install-data-local:
	./update-tables.sh . $(localstatedir)/lib/gmonitor server $(sbindir)/gmh_homectrl
	./update-tables.sh . $(localstatedir)/lib/gmonitor funcion
