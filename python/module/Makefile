CC=gcc
PYMIDILIB=./midiseq.so

all : $(PYMIDILIB)

$(PYMIDILIB) :
	@python ./setup.py install --install-lib=./ --home=./

clean :
	@python ./setup.py clean
	@rm -rf ./build $(PYMIDILIB) ./midiseq-0.0.0.egg-info