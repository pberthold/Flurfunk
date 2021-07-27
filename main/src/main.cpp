#include "MainWindow.h"

#include <QApplication>
#include "Core.h"

#ifdef ANDROID
#include <QtAndroid>
#include <QtAndroidExtras>
#endif

int main(int argc, char *argv[])
{
    #ifdef ANDROID
        QtAndroid::requestPermissionsSync(QStringList({"android.permission.RECORD_AUDIO"}));
    #endif

    QApplication a(argc, argv);
    Core core;

    MainWindow w(&core);
    w.show();
    return a.exec();
}
