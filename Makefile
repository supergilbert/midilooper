all : PYTHONAPI

CAPI :
	@make -C C

PYTHONAPI : CAPI
	@make -C midilooper

clean :
	@make -C C clean
	@make -C midilooper clean

re : clean all
