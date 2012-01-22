all : CAPI PYTHONAPI

CAPI :
	make -C C

PYTHONAPI :
	make -C python/module

clean :
	make -C C clean
	make -C python/module clean
