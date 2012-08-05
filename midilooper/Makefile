PYMIDILIB=./midiseq.so

all : $(PYMIDILIB)

$(PYMIDILIB) :
	@make -C ./module all
	test -L $(PYMIDILIB) || ln -s ./module/midiseq.so

clean:
	find ./ -iname '*pyc' -exec rm -rf {} \;
	@make -C ./module clean
	rm -f $(PYMIDILIB)