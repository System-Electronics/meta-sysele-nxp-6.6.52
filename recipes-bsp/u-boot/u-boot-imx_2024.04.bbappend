#
# Copyright (C)2024 KOAN sas - <https://koansoftware.com>
#

# change the NXP repo with the System Electronics custom one
UBOOT_SRC = "git://github.com/System-Electronics/uboot-imx-lf-6.6.52;protocol=https;branch=${SRCBRANCH}"
SRCBRANCH = "main"
SRCREV = "aca50c09c32806fb6288afe70a864957ccf73164"

# set local version
LOCALVERSION = "-sysele"
