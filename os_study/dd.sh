#!/bin/sh

name=$1
seek=$2
root=$(cd $(dirname $0) && pwd)

source $HOST_HOME/bin/cmd.sh
if [ -z "$seek" ]; then
    exeCmd dd if=$root/$name of=$BOCHS_HOME/hd60M.img bs=512 count=1 conv=notrunc
else
    exeCmd dd if=$root/$name of=$BOCHS_HOME/hd60M.img bs=512 count=1 seek=$seek conv=notrunc
fi
