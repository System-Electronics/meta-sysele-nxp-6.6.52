#
# Copyright (C)2024 KOAN sas - <https://koansoftware.com>
#

# change the NXP repo with the System Electronics custom one
UBOOT_SRC = "git://${TOPDIR}/../uboot-imx-lf-6.6.52;branch=${SRCBRANCH}"
SRCBRANCH = "main"
SRCREV = "0eb96aeac7fe58d82c9ba0b48db29595eb6812bc"

# set local version
LOCALVERSION = "-sysele"
