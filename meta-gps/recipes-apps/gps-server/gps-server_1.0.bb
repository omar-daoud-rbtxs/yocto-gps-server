SUMMARY = "GPS TCP Server and QML Map"
LICENSE = "CLOSED"

inherit cmake_qt5 systemd

DEPENDS += "qtbase qtdeclarative qtlocation"

SRC_URI = " \
    file://src \
    file://gps-server.service \
"

S = "${WORKDIR}/src"

SYSTEMD_SERVICE:${PN} = "gps-server.service"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/gps_server ${D}${bindir}/

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/gps-server.service ${D}${systemd_system_unitdir}/
}