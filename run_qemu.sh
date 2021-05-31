#!/bin/sh

set -eu

executable="`readlink -f "$1"`"

cd "`dirname "$0"`"
mkdir -p qemu-workdir
cd qemu-workdir

if [ ! -f disk.img ]
then
	qemu-img create -f raw disk.img 200M
	mkfs.fat -n 'MIKAN OS' -s 2 -f 2 -R 32 -F 32 disk.img
fi

if [ ! -f OVMF_VARS.fd ]
then
	cp /usr/share/edk2-ovmf/x64/OVMF_VARS.fd .
fi

mkdir -p mnt
sudo mount -o loop disk.img mnt
sudo mkdir -p mnt/EFI/BOOT
sudo cp "$executable" mnt/EFI/BOOT/BOOTX64.EFI
sudo umount mnt

qemu-system-x86_64 \
	-drive if=pflash,format=raw,readonly=on,file=/usr/share/edk2-ovmf/x64/OVMF_CODE.fd \
	-drive if=pflash,format=raw,file=OVMF_VARS.fd \
	-drive format=raw,index=0,media=disk,file=disk.img
