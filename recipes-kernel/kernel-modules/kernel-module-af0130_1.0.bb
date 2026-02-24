SUMMARY = "ON Semiconductor AF0130 MIPI camera driver"
DESCRIPTION = "Out-of-tree V4L2 sensor driver for the ON Semi AF0130"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${THISDIR}/af0130/git/vvcam/common/vvsensor.h;md5=8bc1118cd3c0aebd5d4dd90cfa5b8155"

inherit module

KERNEL_MODULE_PACKAGE_SUFFIX = ""

# Declare that this module depends on the kernel being built first
DEPENDS += "virtual/kernel"
RDEPENDS:${PN} += "kernel-modules"

PACKAGE_ARCH = "${MACHINE_ARCH}"
FILESEXTRAPATHS:prepend := "${THISDIR}/af0130:"

SRC_URI = " \
    file://git \
"

# Source directory containing the module Makefile + driver sources
S = "${WORKDIR}/git/vvcam/v4l2/sensor/af0130"

COMMON_DIR = "${WORKDIR}/git/vvcam/common"

# Add include path for your header files
EXTRA_OEMAKE += "EXTRA_CFLAGS='-I${COMMON_DIR}'"
EXTRA_OEMAKE += "INSTALL_MOD_PATH=${D}"

FILES:${PN} = "${kernel_release_dir}/updates/af0130.ko"

