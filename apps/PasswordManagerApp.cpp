#include <QApplication>

#include "MainWindow.h"
#include "MasterKey.h"

/**
* @brief The main entry point for the PasswordManager application.
* @param argc The number of command-line arguments.
* @param argv The array of command-line arguments.
* @return The exit status of the application.
*
* This function initializes the application, sets up the main window, and starts the event loop.
* It also initializes the sodium library for cryptographic operations.
* If the sodium library fails to initialize, the application will exit with a status of 1.
* 
 */

int main(int argc, char* argv[])
{
    if (sodium_init() < 0)
    {
        return 1;
    }

    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("PasswordManager");
    QCoreApplication::setApplicationName("");

    // Set the application style to "Fusion" for a consistent look across platforms.
    app.setStyle("Fusion");
    MainWindow window;
    window.show();

    return app.exec();
}
