//#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtGui/QApplication>
#include "Qml2Image.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (argc != 3) {
        qFatal("Usage: %s input.qml output.{svg|png}\n", argv[0]);
        return 1;
    }

    QString qmlFile = QString::fromLocal8Bit(argv[1]);
    QString outFile = QString::fromLocal8Bit(argv[2]);
    Qml2Image convertor(qmlFile, outFile);
    return a.exec();
}
