#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (argc != 3)
        return 1;

    const QString inputFile = QString::fromLocal8Bit(argv[1]);
    const QString outputFile = QString::fromLocal8Bit(argv[2]);

    QSvgRenderer renderer(inputFile);

    if (!renderer.isValid())
        return 2;

    QImage image(256, 256, QImage::Format_ARGB32);

    image.fill(Qt::transparent);

    QPainter painter(&image);

    renderer.render(&painter);

    painter.end();

    if (!image.save(outputFile, "PNG"))
        return 3;

    return 0;
}