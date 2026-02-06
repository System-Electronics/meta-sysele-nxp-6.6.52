ENABLE_SE05 ?= "0"

do_install:append() {
    if [ "${ENABLE_SE05}" = "1" ]; then
        mv ${D}${bindir}/ex_ecc ${D}${bindir}/ex_ecc_imx
    fi
}

