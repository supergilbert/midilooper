all : PYTHONAPI

CAPI :
	@make -C c_api

PYTHONAPI : CAPI
	@make -C midilooper

clean :
	@make -C c_api clean
	@make -C midilooper clean

re : clean all
