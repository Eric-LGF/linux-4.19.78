#!/bin/bash -e

make LOADADDR=0x50008000 UIMAGE_ENTRYADDR=0x50008040 uImage

cp arch/arm/boot/uImage ~/tftpboot/

