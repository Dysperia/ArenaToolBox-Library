#include <QtTest/QTest>
#include <utils/CompressionTest.h>
#include <QCoreApplication>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setAttribute(Qt::AA_Use96Dpi, true);
    CompressionTest compressionTest;

    return QTest::qExec(&compressionTest, argc, argv);
}
