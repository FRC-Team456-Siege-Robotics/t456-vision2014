

all: submodules balltrack auton vcontrol

submodules: 
	git submodule init
	git submodule update
	(cd external/iniparser; make)
	(cd external/yavta; make)

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

