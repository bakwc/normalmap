#include <QApplication>

#include "application.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QGLFormat format(QGL::DoubleBuffer);
    format.setSampleBuffers(true);
    format.setSamples(4);
    TApplication window(format);
    window.show();
    app.exec();

    return 0;
}
