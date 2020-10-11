sbin_PROGRAMS = gmh_svupsmon
gmh_svupsmon_SOURCES = statups.cc wpower.cc
gmh_svupsmon_LDADD = -lgmqw
gmh_svupsmon_CPPFLAGS =

AM_CXXFLAGS = \
 -DLOG_FILES=\"$(localstatedir)/log/gmhome\" \
 -Wall $(gmon_debug_global)

install-data-local:
	./update-tables.sh . $(localstatedir)/lib/gmonitor server $(sbindir)/gmh_svupsmon
	./update-tables.sh . $(localstatedir)/lib/gmonitor funcion
	$(mkinstalldirs) $(localstatedir)/log/gmhome
