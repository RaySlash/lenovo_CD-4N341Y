#! /bin/bash

export CROSS_COMPILE=/opt/gcc-linaro-6.3.1-2017.02-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-

make ARCH=arm meson64_a32_defconfig
make ARCH=arm  -j16 uImage  || echo "Compile Image Fail !!"
make ARCH=arm tm2_revb_pxp.dtb || echo "Compile dtb Fail !!"
make ARCH=arm tm2_t962e2_ab311.dtb || echo "Compile dtb Fail !!"
make ARCH=arm tm2_t962e2_ab319.dtb || echo "Compile dtb Fail !!"



