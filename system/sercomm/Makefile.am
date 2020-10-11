SUBDIRS = client server

build-server:
	(cd server && $(MAKE) ) || exit 1
build-client:
	(cd client && $(MAKE) ) || exit 1
install-server:
	(cd server && $(MAKE) install ) || exit 1
install-client:
	(cd client && $(MAKE) install ) || exit 1
uninstall-server:
	(cd server && $(MAKE) uninstall ) || exit 1
uninstall-client:
	(cd client && $(MAKE) uninstall ) || exit 1
