#
# Copyright (C)2024 KOAN sas - <https://koansoftware.com>
#

# change the NXP repo with the System Electronics custom one
LINUX_IMX_SRC = "git://github.com/System-Electronics/linux-imx-lf-6.6.52;protocol=https;branch=${SRCBRANCH}"
SRCBRANCH = "main"
SRCREV = "5894246f3ad37da9bfb26224497e0b5656bfdc00"

# set local version
LOCALVERSION = "-sysele"

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://caam.cfg"
