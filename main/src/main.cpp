#include "MainWindow.h"

#include <QApplication>
#include "Core.h"

#ifdef ANDROID
#include <QtAndroid>
#include <QtAndroidExtras>
#include <QStyleFactory>
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

    #ifdef ANDROID
        qApp->setStyle(QStyleFactory::create("fusion"));
        QPalette palette;
        palette.setColor(QPalette::Window, QColor(53,53,53));
        palette.setColor(QPalette::WindowText, Qt::white);
        palette.setColor(QPalette::Base, QColor(15,15,15));
        palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
        palette.setColor(QPalette::ToolTipBase, Qt::white);
        palette.setColor(QPalette::ToolTipText, Qt::white);
        palette.setColor(QPalette::Text, Qt::white);
        palette.setColor(QPalette::Button, QColor(53,53,53));
        palette.setColor(QPalette::ButtonText, Qt::white);
        palette.setColor(QPalette::BrightText, Qt::red);

        palette.setColor(QPalette::Highlight, QColor(142,45,197).lighter());
        palette.setColor(QPalette::HighlightedText, Qt::black);

        palette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);
        qApp->setPalette(palette);
    #endif

    return a.exec();
}
