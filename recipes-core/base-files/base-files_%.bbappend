do_install:append() {
    # create "/home/root" folder as symlink to ${ROOT_HOME}=/root
    ROOT_SYM="/home/root"
    if [ "${ROOT_HOME}" != "${ROOT_SYM}" ]; then
        if [ -d "${D}${ROOT_SYM}" ]; then
            bbfatal "Error while trying to create ${ROOT_SYM} as symlink to ${ROOT_HOME}, it is already a folder!"
        fi
        ln -sfr ${D}${ROOT_HOME} ${D}${ROOT_SYM}
    fi
}
