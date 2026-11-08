QT       += core gui xml svg axserver

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

DESTDIR = $$PWD/bin
VERSION = 0.1

CONFIG += c++17
win32-g++ {
}else{
QMAKE_CXXFLAGS += /utf-8
}

#CONFIG(debug,debug|release){
#}else{
##
#QMAKE_CFLAGS -= /GL
#QMAKE_LFLAGS -= /LTCG
#}


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS #_NO_FMA_TRAY #_WITH_EMF

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    fma/cmd/fmaeditop.cpp \
    fma/cmd/fmaopscommand.cpp \
    fma/fmadoc.cpp \
    fma/fmaenclose.cpp \
    fma/fmafrac.cpp \
    fma/fmalongdiv.cpp \
    fma/fmamath.cpp \
    fma/fmamime.cpp \
    fma/fmammltable.cpp \
    fma/fmamultiscripts.cpp \
    fma/fmaoperdict.cpp \
    fma/fmaroot.cpp \
    fma/fmarow.cpp \
    fma/fmarows.cpp \
    fma/fmastack.cpp \
    fma/fmasubsuperbase.cpp \
    fma/fmatable.cpp \
    fma/fmatext.cpp \
    fma/fmatextformats.cpp \
    fma/fmaunderover.cpp \
    fma/ifmabase.cpp \
    fma/ifmacontainer.cpp \
    fma/widget/fmaclipboard.cpp \
    fma/widget/fmamatrixinputdlg.cpp \
    fma/widget/fmaaboutdialog.cpp \
    fma/widget/fmaopsbutton.cpp \
    fma/widget/fmaopwidget.cpp \
    fma/widget/fmatoolbar.cpp \
    fma/widget/fmaeditview.cpp \
    fma/widget/fmatransdataobject.cpp \
    fma/wmf/fmawmf.cpp \
    fma/wmf/fmawmfgenerator.cpp \
    fma/wmf/fmawmfpaintengine.cpp \
    fmaaggregated.cpp \
    fmabinder.cpp \
    fmamain.cpp \
    fmatrayicon.cpp \
    main.cpp \
    prop/Properties.cpp \
    prop/PropertiesParser.cpp \
    prop/PropertiesUtils.cpp \
    fma/widget/fmadictpanel.cpp \
    util/lkutil.cpp \
    util/pathconfig.cpp \
    util/lktranslator.cpp \
    util/lktheme.cpp

HEADERS += \
    fma/cmd/fmaeditop.h \
    fma/cmd/fmaopscommand.h \
    fma/fmadoc.h \
    fma/fmaenclose.h \
    fma/fmafrac.h \
    fma/fmalongdiv.h \
    fma/fmamath.h \
    fma/fmamime.h \
    fma/fmammltable.h \
    fma/fmamultiscripts.h \
    fma/fmaoperdict.h \
    fma/fmaroot.h \
    fma/fmarow.h \
    fma/fmarows.h \
    fma/fmastack.h \
    fma/fmasubsuperbase.h \
    fma/fmatable.h \
    fma/fmatext.h \
    fma/fmatextformats.h \
    fma/fmaunderover.h \
    fma/ifmabase.h \
    fma/ifmacontainer.h \
    fma/widget/fmaclipboard.h \
    fma/widget/fmadictpanel.h \
    fma/widget/fmamatrixinputdlg.h \
    fma/widget/fmaaboutdialog.h \
    fma/widget/fmaopsbutton.h \
    fma/widget/fmaopwidget.h \
    fma/widget/fmatoolbar.h \
    fma/widget/fmaeditview.h \
    fma/widget/fmatransdataobject.h \
    fma/wmf/fmawmf.h \
    fma/wmf/fmawmfgenerator.h \
    fma/wmf/fmawmfpaintengine.h \
    fmaaggregated.h \
    fmabinder.h \
    fmamain.h \
    fmatrayicon.h \
    prop/Properties.h \
    prop/PropertiesParser.h \
    prop/PropertiesUtils.h \
    util/lkutil.h \
    util/nodeleter.h \
    util/pathconfig.h \
    util/lktranslator.h \
    util/lktheme.h

FORMS += \
    fma/widget/fmamatrixinputdlg.ui \
    fmamain.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_FILE  = fmamain.rc

RESOURCES += \
    fma/fma.qrc

TRANSLATIONS += \
    fma/translations/LkFormula_zh_CN.ts
