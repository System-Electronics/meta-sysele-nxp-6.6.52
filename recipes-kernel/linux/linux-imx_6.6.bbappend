#
# Copyright (C)2024 KOAN sas - <https://koansoftware.com>
#

# change the NXP repo with the System Electronics custom one
LINUX_IMX_SRC = "git://${TOPDIR}/../linux-imx-lf-6.6.52;branch=${SRCBRANCH}"
SRCBRANCH = "main"
SRCREV = "6450eeb46f6e2f06c6f3f1753de345ae8f0f335b"

# set local version
LOCALVERSION = "-sysele"

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://caam.cfg"
