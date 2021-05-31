#!/bin/sh

set -eu

cd "`dirname "$0"`"

if [ -d 'build-kernel' ]
then
	cd build-kernel
else
	mkdir build-kernel
	cd build-kernel
	cmake -G Ninja ../kernel
fi

ninja
cd ..
mkdir -p qemu-workdir/input
cp build-kernel/kernel.elf qemu-workdir/input
