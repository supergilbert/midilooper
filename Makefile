all : CAPI PYTHONAPI

CAPI :
	make -C C

PYTHONAPI :
	make -C python

clean :
	make -C C clean
	make -C python clean