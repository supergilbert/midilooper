all : CAPI PYTHONAPI

CAPI :
	@make -C C

PYTHONAPI :
	@make -C midilooper

clean :
	@make -C C clean
	@make -C midilooper clean

re : clean all
