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

SRC_URI += "file://caam.cfg"
