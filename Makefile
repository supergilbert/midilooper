# Trick to enable "make -f"
current_dir := $(patsubst %/,%,$(dir $(firstword $(MAKEFILE_LIST))))

.PHONY: deb_pkg test help

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
	@echo " - The 'src' directory contain a python setup.py that can be used for"
	@echo "   custom installations instead of this Makefile."

test:
	@echo "Running midilooper_dev. (more option available with the script ./src/midilooper_dev.sh)"
	@$(current_dir)/src/midilooper_dev.sh

deb_src:
	@cd $(current_dir)/src && \
	./setup.py --command-packages=stdeb.command sdist_dsc && \
	cd -

deb_check:
	@dpkg -s dpkg-dev > /dev/null
	@cd $(current_dir)/src && dpkg-checkbuilddeps && cd -

$(current_dir)/src/debian/changelog:
	@$(current_dir)/src/gen_debchangelog.sh

deb_changelog: $(current_dir)/src/debian/changelog

deb_pkg: deb_check deb_changelog
	@cd $(current_dir)/src && debuild --preserve-envvar=OLDJACKAPI -b -us -uc && cd -

clean:
	@make -f $(current_dir)/src/midiseq_ext_dev.mk clean
	@cd $(current_dir)/src && debuild clean || echo -n && cd -
	@cd $(current_dir)/src && ./setup.py clean && cd -
	@rm -rf $(current_dir)/src/build $(current_dir)/src/MANIFEST \
	$(current_dir)/src/deb_dist $(current_dir)/src/dist \
	$(current_dir)/src/*tar.gz $(current_dir)/midilooper* \
	$(current_dir)/src/debian/changelog
	@find $(current_dir) -iname "*~" -exec rm -f {} +
