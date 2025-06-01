# toolchain.cmake for i.MX7 (Yocto SDK)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# SDK paths
set(SDK_HOST_SYSROOT /opt/fsl-imx-wayland/6.6-scarthgap/sysroots/x86_64-pokysdk-linux)
set(SDK_TARGET_SYSROOT /opt/fsl-imx-wayland/6.6-scarthgap/sysroots/cortexa7t2hf-neon-poky-linux-gnueabi)

# Compiler paths
set(CMAKE_C_COMPILER ${SDK_HOST_SYSROOT}/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER ${SDK_HOST_SYSROOT}/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-g++)

# Correct sysroot
set(CMAKE_SYSROOT ${SDK_TARGET_SYSROOT})

# ABI and FPU flags (prevent VFP mismatch)
set(CMAKE_C_FLAGS "--sysroot=${SDK_TARGET_SYSROOT} -march=armv7-a -mfloat-abi=hard -mfpu=neon" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "--sysroot=${SDK_TARGET_SYSROOT} -march=armv7-a -mfloat-abi=hard -mfpu=neon" CACHE STRING "" FORCE)

# Where to install
set(CMAKE_INSTALL_PREFIX ${SDK_TARGET_SYSROOT}/usr)

# Root path settings
set(CMAKE_FIND_ROOT_PATH ${SDK_TARGET_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
