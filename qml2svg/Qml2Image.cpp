#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include "Qml2Image.h"


Qml2Image::Qml2Image(const QString &qmlFile, const QString &outFile_):
    generator(0), image(0), outFile(outFile_), qmlView(QUrl::fromLocalFile(qmlFile))
{
    //qmlView.resize(QSize(800, 1200));

    QTimer::singleShot(500, this, SLOT(slotRenderLater()));
}

void Qml2Image::maybeDone(const QDeclarativeView::Status status)
{
    if (status == QDeclarativeView::Error) {
        qFatal("qml2image: QML viewer returned fatal error\n");
        QCoreApplication::exit(1);
    } else if (status == QDeclarativeView::Ready) {
        slotSave();
    }
}

void Qml2Image::slotRenderLater()
{
    if (outFile.endsWith(".svg")) {
        generator = new QSvgGenerator();
        generator->setFileName(outFile);
        generator->setSize(qmlView.size());
        generator->setViewBox(QRect(QPoint(0, 0), qmlView.size()));
    } else {
        image = new QImage(qmlView.size(), QImage::Format_ARGB32);
    }

    connect(&qmlView, SIGNAL(statusChanged(QDeclarativeView::Status)), this, SLOT(maybeDone(QDeclarativeView::Status)));
    maybeDone(qmlView.status());
}

void Qml2Image::slotSave()
{
    if (generator) {
        painter.begin(generator);
    } else {
        painter.begin(image);
    }

    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter.setViewport(0, 0, qmlView.width(), qmlView.height());;
    qmlView.render(&painter);
    painter.end();
    if (image) {
        image->save(outFile);
    }
    QCoreApplication::exit(0);
}

Qml2Image::~Qml2Image()
{
    delete generator;
    delete image;
}
