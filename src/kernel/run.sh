#!/bin/bash

rm -f Nanvix.img
cp -f img/blank.img Nanvix.img
echo $1 | sudo -S chmod 777 Nanvix.img
echo $1 | sudo -S losetup /dev/loop2 Nanvix.img
echo $1 | sudo -S mount /dev/loop2 /mnt
echo $1 | sudo -S cp bin/kernel /mnt/kernel
echo $1 | sudo -S cp img/menu.lst /mnt/boot/grub/menu.lst
echo $1 | sudo -S umount /dev/loop2
echo $1 | sudo -S losetup -d /dev/loop2

echo $1 |sudo losetup /dev/loop1 Nanvix.img
echo $1 | sudo bochs -q -f bochsrc.txt
echo $1 |sudo losetup -d /dev/loop1