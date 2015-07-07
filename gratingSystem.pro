QT += gui core

TARGET=sevenPsg
TEMPLATE=app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp \
    serial/qextserialport_win.cpp \
    serial/qextserialport.cpp\
    mythread.cpp \
    FILEWR.cpp \
    define.cpp \
    inifile.cpp \
    findcomputercom.cpp

HEADERS += \
    mainwindow.h \
    qcustomplot.h \
    serial/qextserialport_global.h \
    serial/qextserialport.h\
    mythread.h \
    define.h \
    FILEWR.h \
    define.h \
    inifile.h \
    findcomputercom.h


RC_FILE+=myico.rc

RESOURCES += \
    src.qrc
