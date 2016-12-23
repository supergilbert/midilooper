# Trick to enable "make -f"
current_dir := $(patsubst %/,%,$(dir $(firstword $(MAKEFILE_LIST))))

.PHONY: deb_pkg deb_src deb_changelog test help clean

.DEFAULT_GOAL=help

help:
	@echo "Makefile targets:"
	@echo " test      Compile and run midilooper (without installing it)"
	@echo "           (more option available with the script ./src/midilooper_dev.sh)"
	@echo " deb_check Check for debian package build dependencies"
	@echo " deb_pkg   Build a debian package"
	@echo " clean     Remove all compilated files"
	@echo " help      Show this help"
	@echo ""
	@echo "Note:"
	@echo " - For old jack api, the environment variable OLDJACKAPI must be set."
	@echo "   (ex: \"export OLDJACKAPI=yes\" or \"OLDJACKAPI=yes make <TARGET>\")"
	@echo " - The 'src' directory contain a python setup.py that can be used for"
	@echo "   custom installations instead of this Makefile."

test:
	@echo "Running midilooper_dev. (more option available with the script ./test_midilooper.sh"
	@$(current_dir)/test_midilooper.sh

deb_dir:
	@cd $(current_dir)/src && \
	rm -rf debian && \
	./setup.py --command-packages=stdeb.command debianize && \
	cd -

deb_check: deb_dir
	@dpkg -s dpkg-dev > /dev/null
	@cd $(current_dir)/src && dpkg-checkbuilddeps && cd -

$(current_dir)/src/debian/changelog: deb_dir
	@$(current_dir)/src/gen_debchangelog.sh

deb_changelog: $(current_dir)/src/debian/changelog

deb_pkg: deb_check deb_changelog
	@cd $(current_dir)/src && DEB_BUILD_OPTIONS=nocheck debuild --preserve-envvar=OLDJACKAPI -b -us -uc && cd -

clean:
	@make -f $(current_dir)/src/midiseq_ext_dev.mk clean
	@rm -rf $(current_dir)/src/build \
	$(current_dir)/src/MANIFEST \
	$(current_dir)/midilooper* \
	$(current_dir)/src/.pybuild \
	$(current_dir)/python3-midilooper* \
	$(current_dir)/src/debian
	@find $(current_dir) -iname "*~" -exec rm -f {} +
