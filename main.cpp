#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QString>

#if defined(Q_OS_WIN)
#include <Windows.h>
#endif

int main(int argc, char *argv[])
{
    #if defined(Q_OS_WIN)
    // disable QuickEdit mode in Console
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD prev_mode;
    GetConsoleMode(hInput, &prev_mode);
    SetConsoleMode(hInput, prev_mode & ~ENABLE_QUICK_EDIT_MODE);
    #endif

    QApplication a(argc, argv);
    MainWindow w;
    w.showMaximized();
    return a.exec();
}
