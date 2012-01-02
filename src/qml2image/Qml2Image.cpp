#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include "Qml2Image.h"

Qml2Image::Qml2Image(const QString &qmlFile, const QString &outFile_):
    generator(0), image(0), outFile(outFile_), qmlView(QUrl::fromLocalFile(qmlFile))
{
    if (outFile.endsWith(".svg")) {
        generator = new QSvgGenerator();
        generator->setFileName(outFile);
    }

    connect(&qmlView, SIGNAL(statusChanged(QDeclarativeView::Status)), this, SLOT(maybeDone()));
    QTimer::singleShot(0, this, SLOT(maybeDone()));
}

void Qml2Image::maybeDone()
{
    const QDeclarativeView::Status status = qmlView.status();
    if (status == QDeclarativeView::Error) {
        qFatal("qml2image: QML viewer returned fatal error\n");
        QCoreApplication::exit(1);
    } else if (status == QDeclarativeView::Ready) {
        QTimer::singleShot(0, this, SLOT(slotSave()));
    }
}

void Qml2Image::slotSave()
{
    qDebug() << "qml2image: current view size" << qmlView.size();
    if (generator) {
        painter.begin(generator);
        generator->setSize(qmlView.size());
    } else {
        image = new QImage(qmlView.size(), QImage::Format_ARGB32);
        painter.begin(image);
    }

    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter.setViewport(0, 0, qmlView.width(), qmlView.height());;
    qmlView.render(&painter);
    painter.end();
    if (image) {
        image->save(outFile);
    }
    //qmlView.show();
    QCoreApplication::exit(0);
}

Qml2Image::~Qml2Image()
{
    delete generator;
    delete image;
}

#include "Qml2Image.moc"
