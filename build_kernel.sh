#! /bin/bash

CUR_PATH=$PWD

source=${CUR_PATH}/kernel
out=${CUR_PATH}/out-kernel

export CROSS_COMPILE=aarch64-linux-gnu-
export PATH=${CUR_PATH}/prebuilt/toolchain/gcc-linaro-6.3.1-2017.02-x86_64_aarch64-linux-gnu/bin:$PATH

rm -fr ${out}

make ARCH=arm64 -C ${source} O=${out} meson64_smarthome_defconfig
if [ $? -ne 0 ]; then
   echo ">>>"
   echo ">>>Kernel Compile error!!!!"
   echo ">>>"
   exit -1
fi

make ARCH=arm64 -C ${source} O=${out} -j8 Image  UIMAGE_LOADADDR=0x1080000 2>&1 | tee build_kernel.log 
if [ $? -ne 0 ]; then
   echo ">>>"
   echo ">>>Kernel Compile error!!!!"
   echo ">>>"
   exit -1
fi

make ARCH=arm64 -C ${source} O=${out} -j8 Image.gz  UIMAGE_LOADADDR=0x1080000 
if [ $? -ne 0 ]; then
   echo ">>>"
   echo ">>>Kernel Compile error!!!!"
   echo ">>>"
   exit -1
fi

make ARCH=arm64 -C ${source} O=${out} -j8 axg_s420_v03gva.dtb
if [ $? -ne 0 ]; then
   echo ">>>"
   echo ">>>Kernel Compile error!!!!"
   echo ">>>"
   exit -1
fi

echo ">>>"
echo ">>>Kernel Compile success!"
echo ">>>"

