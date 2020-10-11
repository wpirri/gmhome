sbin_PROGRAMS = gmh_syshome
gmh_syshome_SOURCES = server.cc
#gmh_syshome_LDADD = -lgmq -lmysqlclient
gmh_syshome_LDADD = -lgmq -lpq
gmh_syshome_CPPFLAGS =

AM_CXXFLAGS = \
 -Wall $(gmon_debug_global)

install-data-local:
	./update-tables.sh . $(localstatedir)/lib/gmonitor server $(sbindir)/gmh_syshome
	./update-tables.sh . $(localstatedir)/lib/gmonitor funcion
