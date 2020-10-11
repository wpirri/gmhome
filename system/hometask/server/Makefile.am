sbin_PROGRAMS = gmh_hometask
gmh_hometask_SOURCES = server.cc hometask.cc
gmh_hometask_LDADD = -lgmq -lpq
gmh_hometask_CPPFLAGS =

AM_CXXFLAGS = \
 -Wall $(gmon_debug_global)

install-data-local:
	./update-tables.sh . $(localstatedir)/lib/gmonitor server $(sbindir)/gmh_hometask
	./update-tables.sh . $(localstatedir)/lib/gmonitor funcion
