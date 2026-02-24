#
# Copyright (C)2024 KOAN sas - <https://koansoftware.com>
#

# change the NXP repo with the System Electronics custom one
LINUX_IMX_SRC = "git://github.com/System-Electronics/linux-imx-lf-6.6.52;protocol=https;branch=${SRCBRANCH}"
SRCBRANCH = "main"
SRCREV = "150ffc64090c41de59af3cc6a8713d37ea1fc191"

# set local version
LOCALVERSION = "-sysele"

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

# Apply only kernel-space patches (like ISI RAW support)
SRC_URI += "file://caam.cfg \
            file://0001-dts-add-support-for-onsemi-AF0130-camera-on-i2c6.patch \
            file://imx8-mipi-raw-support-Y12.patch \
            file://add-af0130-dts-makefile.patch \
            file://add-af0130-dts-file.patch "

EXTRA_DTBS += " freescale/imx8mp-evk-af0130.dtb"

KERNEL_DEVICETREE:append:use-nxp-bsp = " freescale/imx8mp-evk-af0130.dtb"

# Optional: bump PR so Yocto rebuilds kernel when you change patch
PR .= ".onsemi1"
