# Disable and mask all psplash systemd services at build time
do_install:append() {
    # Create symlinks to /dev/null to mask the systemd services
    for svc in psplash-systemd.service \
               psplash-start.service \
               psplash-quit.service \
               psplash-network.service \
               psplash-basic.service; do
        mkdir -p ${D}${systemd_system_unitdir}/
        ln -sf /dev/null ${D}${systemd_system_unitdir}/${svc}
    done
}

# Prevent Yocto from automatically enabling psplash services
SYSTEMD_AUTO_ENABLE = "disable"

