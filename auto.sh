#!/bin/bash

# function build
build() {
	if [ ! -d "build/" ];
	then
		echo "Create Build Directory!"
		mkdir build
	fi
	
	echo "Entering build directory!"
	cd build
	
	echo "Generate Makefile!"
	cmake ..
	
	echo "Compiling..."
	make
}

# function install
install() {
	if [ ! -d "build/" ];
	then
		echo "Please Build First!"
		exit 1
	fi

	echo "Install trbuf"
	cd build
	make install
}

# function uninstall
uninstall() {
	if [ ! -d "build/" ];
	then
		echo "Please Build First!"
		exit 1
	fi

	echo "Uninstall trbuf"
	for line in `cat build/install_manifest.txt`
	do
		echo "-- Uninstalling: $line"
		rm $line
	done
}

# switch case
case "$1" in

build)
	build
	;;
install)
	install
	;;
uninstall)
	uninstall
	;;
*)
	echo "Usage: ./auto.sh {build|install|uninstall}"
	exit 1
	;;
esac
exit 0
