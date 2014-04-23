TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    trunk/main.cpp
INCLUDEPATH += .
INCLUDEPATH += ./net-jes/

HEADERS += \
    trunk/containers/jsonobject.h \
    trunk/http/util.h \
    trunk/http/sslDevice.h \
    trunk/http/request.h \
    trunk/http/postdata.h \
    trunk/http/cookiejar.h \
    trunk/http/apis/dropbox/file.h \
    trunk/http/apis/dropbox/client.h \
    trunk/http/apis/9292/route.h \
    trunk/http/apis/9292/location.h \
    trunk/ssdp/service.h \
    trunk/ssdp/msearch.h \
    trunk/ssdp/message.h \
    trunk/ssdp/listener.h \
    trunk/ssdp/devicelist.h \
    trunk/ssdp/device.h

