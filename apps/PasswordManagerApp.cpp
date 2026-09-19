#include <QApplication>

#include "MainWindow.h"
#include "MasterKey.h"

int main(int argc, char* argv[])
{
    if (sodium_init() < 0)
    {
        return 1;
    }

    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
