

all: submodules bt auto vc tests calib

submodules: 
	git submodule init
	git submodule update
	(cd external/iniparser; make)
	(cd external/yavta; make)

tests: vcontrol
	cd system_tests; make

bt: submodules
	echo "compiling balltrack"
	cd balltrack; make

auto: submodules
	cd auton; make

vc: submodules balltrack auton
	cd vcontrol; make

calib:
	cd calibrate; make

clean:
	cd external/iniparser; make clean
	cd external/yavta; make clean
	cd balltrack; make clean
	cd auton; make clean
	cd vcontrol; make clean
	cd system_tests; make clean
	cd calibrate; make clean

