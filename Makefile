# Trick to enable "make -f"
current_dir := $(patsubst %/,%,$(dir $(firstword $(MAKEFILE_LIST))))

.PHONY: debpkg test help

.DEFAULT_GOAL=test

test:
	$(current_dir)/src/midilooper_dev.sh

debpkg:
	cd $(current_dir)/src && \
	./setup.py --command-packages=stdeb.command bdist_deb && \
	cd -
	cd $(current_dir)/src
	find $(current_dir)/src/deb_dist -iname "*.deb" -exec cp {} ./ \;

clean:
	@make -f $(current_dir)/src/midiseq_ext_dev.mk clean
	@cd src && $(current_dir)/setup.py clean && cd -
	@rm -rf $(current_dir)/src/build $(current_dir)/src/MANIFEST \
	$(current_dir)/src/deb_dist $(current_dir)/src/dist \
	$(current_dir)/src/*tar.gz
	@find $(current_dir) -iname "*.deb" -exec rm {} +
	@find $(current_dir) -iname "*~" -exec rm {} +

help:
	@echo "target:"
	@echo " debpkg Build a debian package"
	@echo " test   Compile and run midilooper (without installing it)"
	@echo "        (more option available with the script ./src/midilooper_dev.sh)"
	@echo " clean  Remove all compilated files"
	@echo " help   Show this help"
