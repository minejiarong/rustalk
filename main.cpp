#include "mainwindow.h"
#include "rustalk_core.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[]) 
{ 
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    QApplication a(argc, argv);

    rustalk_init("rustalk.db");

    MainWindow w;
    w.show();

    int exitCode = a.exec();
    rustalk_shutdown();
    return exitCode;
 }
