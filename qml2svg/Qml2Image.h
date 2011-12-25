#ifndef QML2IMAGE_H
#define QML2IMAGE_H

#include <QtDeclarative/QDeclarativeView>
#include <QtSvg/QSvgGenerator>

class Qml2Image : public QObject
{
    Q_OBJECT

public:
    Qml2Image(const QString &qmlFile, const QString &outFile);
    virtual ~Qml2Image();

private slots:
    void maybeDone(const QDeclarativeView::Status status);
    void slotRenderLater();
    void slotSave();

private:
    QSvgGenerator *generator;
    QImage *image;
    QPainter painter;
    QString outFile;
    QDeclarativeView qmlView;
};

#endif // QML2IMAGE_H
