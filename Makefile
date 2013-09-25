
all:
	make -C ./LasCommon
	make -C ./LasTgBoard
	make -C ./LasLsBoard
	make -C ./LasFSM
	make -C ./LasCommissioner


install:
	make -C ./LasCommon install
	make -C ./LasTgBoard install
	make -C ./LasLsBoard install
	make -C ./LasFSM install
	make -C ./LasCommissioner install

clean:
	make -C ./LasCommon clean
	make -C ./LasTgBoard clean
	make -C ./LasLsBoard clean
	make -C ./LasFSM clean
	make -C ./LasCommissioner clean


cleanmake:
	rm -f LasTgBoard/Makefile
	rm -f LasLsBoard/Makefile
	rm -f LasFSM/Makefile

