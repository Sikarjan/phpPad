#-------------------------------------------------
#
# Project created by QtCreator 2017-01-05T07:43:22
#
#-------------------------------------------------

QT       += core gui xml network widgets

greaterThan(QT_MAJOR_VERSION, 8): QT += widgets

include(plugins/singleapplication/singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication

TARGET = phpPad
TEMPLATE = app
VERSION = 0.2.2
GIT_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD rev-list --count HEAD)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Define the preprocessor macro to get the application version in our application.
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"

SOURCES += main.cpp\
    mainwindow.cpp \
    components/codeeditor.cpp \
    components/highlighter.cpp \
    components/projectmodel.cpp \
    components/toolBox.cpp \
    dialogs/newproject.cpp \
    dialogs/newtabledialog.cpp \
    dialogs/preferencedialog.cpp \
    components/completerdelegate.cpp \
    dialogs/insertimagedialog.cpp

HEADERS  += mainwindow.h \
    components/codeeditor.h \
    components/highlighter.h \
    components/projectmodel.h \
    components/toolBox.h \
    dialogs/newproject.h \
    dialogs/newtabledialog.h \
    dialogs/preferencedialog.h \
    components/completerdelegate.h \
    dialogs/insertimagedialog.h

FORMS    += mainwindow.ui \
    dialogs/newproject.ui \
    dialogs/newtabledialog.ui \
    dialogs/preferencedialog.ui \
    components/toolBox.ui \
    dialogs/insertimagedialog.ui

DISTFILES += \
    appIcon/phpPad.rc \
    Erungenschaften.txt \
    xml/html.xml \
    README.md \
    LICENSE \
    installers/phpPadInstaller.exe \
    installers/phpPadInstaller.app.zip

RESOURCES += \
    qrc.qrc

ICON = appIcon/phpPad.icns

RC_FILE = appIcon/phpPad.rc

TRANSLATIONS = translations/phpPad_de.ts
