

all: submodules balltrack auton vcontrol tests

submodules: 
	git submodule init
	git submodule update
	(cd external/iniparser; make)
	(cd external/yavta; make)

tests: vcontrol
	(cd system_tests; make)

balltrack: submodules
	echo "compiling balltrack"
	(cd balltrack; make)

auton: submodules
	(cd auton; make)

vcontrol: submodules balltrack auton
	(cd vcontrol; make)


clean:
	cd external/iniparser; make clean
	cd external/yavta; make clean
	cd balltrack; make clean
	cd auton; make clean
	cd vcontrol; make clean
	cd system_tests; make clean

