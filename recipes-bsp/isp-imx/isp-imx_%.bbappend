FILESEXTRAPATHS:prepend := "${THISDIR}/imx219:"

SYSTEMD_DISABLE = "imx8-isp.service"
SYSTEMD_AUTO_ENABLE = "disable"

SRC_URI += "file://0001-isp-imx-add-imx219.patch" 

FILES_SOLIBS_VERSIONED += " \
    ${libdir}/libimx219.so \
"
