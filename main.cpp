#include "mainwindow.h"

#include <QApplication>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QStringList>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName(QStringLiteral("简易文本编辑器"));
    QApplication::setApplicationVersion(QStringLiteral("1.0"));
    QApplication::setOrganizationName(QStringLiteral("DGUT-CS"));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/logo.svg")));

    // 安装 Qt 自带控件（颜色对话框、字体对话框、消息框按钮等）的中文翻译
    QTranslator qtTranslator;
    if (qtTranslator.load(QLocale(QLocale::Chinese, QLocale::China), QStringLiteral("qtbase"),
                          QStringLiteral("_"), QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&qtTranslator);
    }

    MainWindow window;
    window.show();

    // 支持在命令行中直接指定要打开的文件：TextEditor.exe <文件路径>
    const QStringList arguments = QCoreApplication::arguments();
    if (arguments.size() > 1)
        window.openFileFromPath(arguments.at(1));

    return QApplication::exec();
}
