#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QSettings>
#include <windows.h>
#include <shlobj.h>

bool IsAdmin() {
    return IsUserAnAdmin();
}

int main(int argc, char *argv[])
{
    if (!IsAdmin()) {
        wchar_t szPath[MAX_PATH];
        if (GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath))) {
            SHELLEXECUTEINFOW sei = { sizeof(sei) };
            sei.lpVerb = L"runas";
            sei.lpFile = szPath;
            sei.hwnd = NULL;
            sei.nShow = SW_NORMAL;
            if (!ShellExecuteExW(&sei)) {
                return 1;
            }
        }
        return 0;
    }

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/assets/logo.ico"));

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    // Load Translation
    QSettings settings("HKEY_CURRENT_USER\\Software\\WinMountExt4", QSettings::NativeFormat);
    QString lang = settings.value("language", "auto").toString();
    
    QTranslator translator;
    bool loaded = false;
    if (lang == "auto") {
        loaded = translator.load(QLocale::system(), "winmountext4", "_", ":/translations");
    } else if (lang != "en") {
        loaded = translator.load("winmountext4_" + lang, ":/translations");
    }
    
    if (loaded) {
        a.installTranslator(&translator);
    }

    MainWindow w;
    if (w.shouldStartMinimized()) {
        w.hide();
    } else {
        w.show();
    }
    
    return a.exec();
}
