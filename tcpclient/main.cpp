#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Set application name and organization for proper settings storage
    QCoreApplication::setApplicationName("TcpClient");
    QCoreApplication::setOrganizationName("MyCompany");
    
    MainWindow w;
    w.show();
    return a.exec();
}
