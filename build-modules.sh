#!/bin/bash -e

make modules

rootfs_dir="/home/eric/linux-develop/rootfs"

dir=`ls -A ${rootfs_dir}/lib/modules/`

if test -n "$dir"
then
    echo $dir
    rm -r ${rootfs_dir}/lib/modules/*
fi
make modules_install INSTALL_MOD_PATH=${rootfs_dir}


