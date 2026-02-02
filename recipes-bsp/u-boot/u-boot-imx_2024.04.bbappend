#
# Copyright (C)2024 KOAN sas - <https://koansoftware.com>
#

# change the NXP repo with the System Electronics custom one
UBOOT_SRC = "git://${TOPDIR}/../uboot-imx-lf-6.6.52;branch=${SRCBRANCH}"
SRCBRANCH = "main"
SRCREV = "005b37d97327761ed9182c4835ad58b86d76065d"

# set local version
LOCALVERSION = "-sysele"
