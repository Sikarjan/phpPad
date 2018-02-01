#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <singleapplication.h>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(qrc);

    QCoreApplication::setOrganizationName("innoBiz");
    QCoreApplication::setApplicationName("phpPad");
    QCoreApplication::setApplicationVersion(APP_VERSION);

    SingleApplication app(argc, argv, true);

    MainWindow mainWin;

    if( app.isSecondary() ) {
        for(int i = 1; i<app.arguments().count(); i++){
            app.sendMessage( app.arguments().at(i).toUtf8() );
        }
        return 0;
    }

    QObject::connect(&app,&SingleApplication::receivedMessage,&mainWin, &MainWindow::handleAppOpenMessage);
//    QObject::connect(&app, SIGNAL(receivedMessage(const quint32, const QByteArray)), &mainWin,SLOT(handleAppOpenMessage(const quint32, const QByteArray&)));
    QObject::connect( &app, &SingleApplication::instanceStarted, [ &mainWin ]() {
            mainWin.raise();
            mainWin.activateWindow();
    });

    app.setStyleSheet("QSplitter::handle { background-color: #999999 }");

    QTranslator translator;
    translator.load("translations/phpPad_" + QLocale::system().name() + ".ts");
    app.installTranslator(&translator);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(app);

    foreach (const QString &fileName, parser.positionalArguments())
        mainWin.handleAppOpenMessage(app.instanceId(), fileName.toUtf8());

    mainWin.show();

    return app.exec();
}
