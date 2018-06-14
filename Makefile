# Trick to enable "make -f"
current_dir := $(patsubst %/,%,$(dir $(firstword $(MAKEFILE_LIST))))

.PHONY: help test deb_dir deb_check deb_pkg clean

.DEFAULT_GOAL=help

help:
	@echo "Makefile targets:"
	@echo " test       Compile and run midilooper (without installing it)"
	@echo "            (Arguments can be passed with MIDILOOPER_ARGS environment variable"
	@echo "            more option available with the script ./test_midilooper.sh)"
	@echo " test_build Build midilooper for testing."
	@echo " deb_check  Check for debian package build dependencies"
	@echo " deb_pkg    Build a debian package"
	@echo " clean      Remove all compilated files"
	@echo " help       Show this help"
	@echo ""
	@echo "Note:"
	@echo " - For old jack api, the environment variable OLDJACKAPI must be set."
	@echo "   (ex: \"export OLDJACKAPI=yes\" or \"OLDJACKAPI=yes make <TARGET>\")"
	@echo " - The 'src' directory contain a python setup.py that can be used for"
	@echo "   custom installations instead of this Makefile."

test:
	@echo "Running midilooper_dev. (more option available with the script ./test_midilooper.sh)"
	@$(current_dir)/test_midilooper.sh $(MIDILOOPER_ARGS)

test_build:
	@echo "Building midilooper_dev. (for ./test_midilooper.sh)"
	make -f $(current_dir)/src/midiseq_ext_dev.mk

$(current_dir)/src/debian:
	cp -Rf $(current_dir)/src/debian.in $(current_dir)/src/debian
	@$(current_dir)/src/update_debchangelog.sh

# Waiting for "stdeb.command debianize" in debian stable
# $(current_dir)/src/debian:
# 	@cd $(current_dir)/src && \
# 	rm -rf debian && \
# 	./setup.py --command-packages=stdeb.command debianize && \
# 	cd -
# 	@sed -i "s/python3-midilooper/midilooper/" $(current_dir)/src/debian/control
#	@cp -f $(current_dir)/debian_changelog.in $(current_dir)/debian/changelog
# 	@$(current_dir)/src/update_debchangelog.sh

deb_dir: $(current_dir)/src/debian

deb_check: deb_dir
	@dpkg -s dpkg-dev > /dev/null
	@cd $(current_dir)/src && dpkg-checkbuilddeps && cd -

deb_pkg: deb_check
	@cd $(current_dir)/src && \
	DEB_BUILD_OPTIONS=nocheck debuild --preserve-envvar=OLDJACKAPI -b -us -uc && \
	cd -

install:
	@if [ -z "$(PREFIX)" ]; then echo '$$PREFIX directory variable needed' >&2; false; fi
	@if [ ! -d "$(PREFIX)" ]; then echo "Unable to read $(PREFIX) directory" >&2; false; fi
	@cd $(current_dir)/src && ./setup.py install --prefix $(PREFIX) && cd -

clean:
	@make -f $(current_dir)/src/midiseq_ext_dev.mk clean
	@rm -rf $(current_dir)/src/build \
	$(current_dir)/src/MANIFEST \
	$(current_dir)/midilooper* \
	$(current_dir)/src/.pybuild \
	$(current_dir)/src/debian
	@find $(current_dir) -iname "*~" -exec rm -f {} +
