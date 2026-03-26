#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCloseEvent>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool shouldStartMinimized() const;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void refreshDisks();
    void onDiskSelected(int index);
    void mountDisk();
    void unmountDisk();
    void toggleStartup(int state);
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void toggleTrayOption(int state);
    void onLanguageChanged(int index);

private:
    void initUI();
    void initTray();
    void log(const QString &message);
    QString runCommand(const QString &program, const QStringList &arguments);
    void checkStartupStatus();
    void applyModernStyle();

    QComboBox *diskCombo;
    QComboBox *partCombo;
    QComboBox *langCombo;
    QPushButton *mountBtn;
    QPushButton *unmountBtn;
    QPushButton *refreshBtn;
    QCheckBox *startupCb;
    QCheckBox *minToTrayCb;
    QCheckBox *startMinimizedCb;
    QTextEdit *logOutput;

    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *showAction;
    QAction *quitAction;
};

#endif // MAINWINDOW_H
