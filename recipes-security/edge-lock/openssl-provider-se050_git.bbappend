ENABLE_SE05 ?= "0"

do_install:append() {
    if [ "${ENABLE_SE05}" = "1" ]; then
        mv ${D}${libdir}/libsssProvider.so ${D}${libdir}/libsssProvider_imx.so
    fi
}

