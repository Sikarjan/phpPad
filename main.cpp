#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <qtsingleapplication.h>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(qrc);

    QtSingleApplication app(argc, argv);

    if (app.isRunning()){
        QString message;
        for (int a = 1; a < argc; ++a) {
            message += argv[a];

        if (a < argc-1)
            message += " ";
        }

        return !app.sendMessage(message);
    }

    app.setStyleSheet("QSplitter::handle { background-color: #999999 }");

    QTranslator translator;
    translator.load("translations/phpPad_" + QLocale::system().name() + ".ts");
    app.installTranslator(&translator);

    QCoreApplication::setOrganizationName("innoBiz");
    QCoreApplication::setApplicationName("phpPad");
    QCoreApplication::setApplicationVersion(APP_VERSION);
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(app);

    MainWindow mainWin;
    app.setActivationWindow(&mainWin);

    if (!parser.positionalArguments().isEmpty()){
        mainWin.handleAppOpenMessage(parser.positionalArguments().first());
    }

    mainWin.show();

    QObject::connect(&app, SIGNAL(messageReceived(const QString&)), &mainWin,SLOT(handleAppOpenMessage(const QString&)));
    QObject::connect(&app, SIGNAL(openFile(QString)), &mainWin, SLOT(handleAppOpenMessage(const QString&)));

    return app.exec();
}
