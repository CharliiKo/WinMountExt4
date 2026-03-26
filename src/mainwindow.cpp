#include "mainwindow.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QDir>
#include <QStyle>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    initUI();
    initTray();
    refreshDisks();
    checkStartupStatus();
    applyModernStyle();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    setWindowTitle(tr("WinMountExt4 GUI"));
    setMinimumSize(600, 520);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->setSpacing(10);
    layout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel *titleLabel = new QLabel("WinMountExt4");
    titleLabel->setObjectName("titleLabel");
    layout->addWidget(titleLabel);

    // Disk Selection
    layout->addWidget(new QLabel(tr("Select Physical Disk:")));
    diskCombo = new QComboBox();
    connect(diskCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onDiskSelected);
    layout->addWidget(diskCombo);

    // Partition Selection
    layout->addWidget(new QLabel(tr("Select Partition (Ext4):")));
    partCombo = new QComboBox();
    layout->addWidget(partCombo);

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    mountBtn = new QPushButton(tr("Mount"));
    unmountBtn = new QPushButton(tr("Unmount"));
    refreshBtn = new QPushButton(tr("Refresh"));

    mountBtn->setObjectName("primaryBtn");
    
    connect(mountBtn, &QPushButton::clicked, this, &MainWindow::mountDisk);
    connect(unmountBtn, &QPushButton::clicked, this, &MainWindow::unmountDisk);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshDisks);

    btnLayout->addWidget(mountBtn);
    btnLayout->addWidget(unmountBtn);
    btnLayout->addWidget(refreshBtn);
    layout->addLayout(btnLayout);

    // Options
    QGridLayout *optLayout = new QGridLayout();
    startupCb = new QCheckBox(tr("Start on Windows Login"));
    minToTrayCb = new QCheckBox(tr("Minimize to Tray on Close"));
    startMinimizedCb = new QCheckBox(tr("Minimize to Tray on Open"));

    connect(startupCb, &QCheckBox::stateChanged, this, &MainWindow::toggleStartup);
    connect(minToTrayCb, &QCheckBox::stateChanged, this, &MainWindow::toggleTrayOption);
    connect(startMinimizedCb, &QCheckBox::stateChanged, this, &MainWindow::toggleTrayOption);

    optLayout->addWidget(startupCb, 0, 0);
    optLayout->addWidget(minToTrayCb, 0, 1);
    optLayout->addWidget(startMinimizedCb, 1, 0);

    // Language Selection
    QHBoxLayout *langLayout = new QHBoxLayout();
    langLayout->addWidget(new QLabel(tr("Language:")));
    langCombo = new QComboBox();
    langCombo->addItem("English", "en");
    langCombo->addItem("简体中文", "zh_CN");
    langCombo->addItem("繁體中文", "zh_TW");
    connect(langCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onLanguageChanged);
    langLayout->addWidget(langCombo);
    langLayout->addStretch();
    
    optLayout->addLayout(langLayout, 1, 1);
    layout->addLayout(optLayout);

    // Log Output
    layout->addWidget(new QLabel(tr("Log Output:")));
    logOutput = new QTextEdit();
    logOutput->setReadOnly(true);
    layout->addWidget(logOutput);
}

void MainWindow::initTray()
{
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(style()->standardIcon(QStyle::SP_DriveHDIcon));
    trayIcon->setToolTip("WinMountExt4");

    trayMenu = new QMenu(this);
    showAction = new QAction(tr("Open Main Window"), this);
    quitAction = new QAction(tr("Exit"), this);

    connect(showAction, &QAction::triggered, this, &MainWindow::showNormal);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    trayMenu->addAction(showAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);

    trayIcon->setContextMenu(trayMenu);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);
    
    trayIcon->show();
}

void MainWindow::applyModernStyle()
{
    QString qss = R"(
        QMainWindow { background-color: #f5f6f7; }
        #titleLabel { font-size: 24px; font-weight: bold; color: #2c3e50; margin-bottom: 10px; }
        QLabel { color: #34495e; font-size: 14px; }
        QComboBox { 
            border: 1px solid #dcdde1; border-radius: 4px; padding: 5px; background: white; min-height: 25px;
        }
        QComboBox:hover { border-color: #3498db; }
        QPushButton { 
            background-color: #ecf0f1; border: 1px solid #dcdde1; border-radius: 4px; padding: 8px 15px; 
            font-weight: bold; min-width: 80px; color: #2c3e50;
        }
        QPushButton:hover { background-color: #dcdde1; }
        #primaryBtn { background-color: #3498db; color: white; border: none; }
        #primaryBtn:hover { background-color: #2980b9; }
        QTextEdit { border: 1px solid #dcdde1; border-radius: 4px; background: #ffffff; color: #2f3640; }
        QCheckBox { spacing: 8px; color: #34495e; }
    )";
    this->setStyleSheet(qss);
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        if (isVisible()) {
            hide();
        } else {
            showNormal();
            activateWindow();
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (minToTrayCb->isChecked() && trayIcon->isVisible()) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::log(const QString &message)
{
    logOutput->append(message);
}

QString MainWindow::runCommand(const QString &program, const QStringList &arguments)
{
    QProcess process;
    process.start(program, arguments);
    process.waitForFinished();
    return QString::fromUtf8(process.readAllStandardOutput());
}

void MainWindow::refreshDisks()
{
    diskCombo->clear();
    log(tr("Refreshing disk list..."));

    QStringList psArgs;
    psArgs << "-NoProfile" << "-Command" << "Get-CimInstance -query 'SELECT * from Win32_DiskDrive' | Select-Object DeviceID, Model, Size | ConvertTo-Json";
    
    QString output = runCommand("powershell", psArgs);
    if (output.isEmpty()) {
        log(tr("No disks found or error running PowerShell."));
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
    QJsonArray disks;
    if (doc.isObject()) {
        disks.append(doc.object());
    } else {
        disks = doc.array();
    }

    for (int i = 0; i < disks.size(); ++i) {
        QJsonObject disk = disks[i].toObject();
        QString deviceId = disk["DeviceID"].toString();
        QString model = disk["Model"].toString();
        long long size = disk["Size"].toVariant().toLongLong();
        QString label = QString("%1 - %2 (%3 GB)").arg(deviceId).arg(model).arg(size / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
        diskCombo->addItem(label, deviceId);
    }
}

void MainWindow::onDiskSelected(int index)
{
    Q_UNUSED(index);
    partCombo->clear();
    QString diskId = diskCombo->currentData().toString();
    if (diskId.isEmpty()) return;

    log(tr("Probing partitions for %1...").arg(diskId));
    
    // Attach --bare
    runCommand("wsl", {"--mount", diskId, "--bare"});

    // Get partitions
    QString output = runCommand("wsl", {"-u", "root", "-e", "lsblk", "-p", "-o", "NAME,FSTYPE,SIZE", "-J"});
    
    QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
    QJsonObject root = doc.object();
    QJsonArray devices = root["blockdevices"].toArray();

    for (int i = 0; i < devices.size(); ++i) {
        QJsonObject dev = devices[i].toObject();
        QJsonArray children = dev["children"].toArray();
        for (int j = 0; j < children.size(); ++j) {
            QJsonObject part = children[j].toObject();
            QString name = part["name"].toString();
            QString fstype = part["fstype"].toString();
            QString size = part["size"].toString();
            
            if (fstype == "ext4" || fstype.isEmpty()) {
                QString label = QString("%1 (%2) - %3").arg(name).arg(fstype.isEmpty() ? tr("unknown") : fstype).arg(size);
                partCombo->addItem(label, name);
            }
        }
    }
}

void MainWindow::mountDisk()
{
    QString diskId = diskCombo->currentData().toString();
    QString partPath = partCombo->currentData().toString();

    if (diskId.isEmpty() || partPath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a disk and partition."));
        return;
    }

    log(tr("Mounting %1...").arg(partPath));
    runCommand("wsl", {"-u", "root", "-e", "mkdir", "-p", "/mnt/wsl/ext4_disk"});
    
    QProcess process;
    process.start("wsl", {"-u", "root", "-e", "mount", partPath, "/mnt/wsl/ext4_disk"});
    process.waitForFinished();

    if (process.exitCode() == 0) {
        log(tr("Successfully mounted!"));
        log(tr("Access at: \\\\wsl.localhost\\Ubuntu\\mnt\\wsl\\ext4_disk"));
        QMessageBox::information(this, tr("Success"), tr("Disk mounted successfully!"));
    } else {
        QString err = process.readAllStandardError();
        log(tr("Mount failed: %1").arg(err));
        QMessageBox::critical(this, tr("Error"), tr("Mount failed: %1").arg(err));
    }
}

void MainWindow::unmountDisk()
{
    QString diskId = diskCombo->currentData().toString();
    if (diskId.isEmpty()) return;

    log(tr("Unmounting %1...").arg(diskId));
    QProcess process;
    process.start("wsl", {"--unmount", diskId});
    process.waitForFinished();

    if (process.exitCode() == 0) {
        log(tr("Successfully unmounted!"));
        QMessageBox::information(this, tr("Success"), tr("Disk unmounted successfully!"));
    } else {
        QString err = process.readAllStandardError();
        log(tr("Unmount failed: %1").arg(err));
        QMessageBox::critical(this, tr("Error"), tr("Unmount failed: %1").arg(err));
    }
}

void MainWindow::checkStartupStatus()
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\WinMountExt4", QSettings::NativeFormat);
    minToTrayCb->setChecked(settings.value("minToTray", true).toBool());
    startMinimizedCb->setChecked(settings.value("startMinimized", false).toBool());
    
    QString lang = settings.value("language", "auto").toString();
    int idx = langCombo->findData(lang);
    if (idx != -1) langCombo->setCurrentIndex(idx);

    QSettings bootSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (bootSettings.contains("WinMountExt4")) {
        startupCb->setChecked(true);
    } else {
        startupCb->setChecked(false);
    }
}

void MainWindow::toggleStartup(int state)
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (state == Qt::Checked) {
        QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        settings.setValue("WinMountExt4", QString("\"%1\"").arg(appPath));
        log(tr("Added to startup."));
    } else {
        settings.remove("WinMountExt4");
        log(tr("Removed from startup."));
    }
}

void MainWindow::toggleTrayOption(int state)
{
    Q_UNUSED(state);
    QSettings settings("HKEY_CURRENT_USER\\Software\\WinMountExt4", QSettings::NativeFormat);
    settings.setValue("minToTray", minToTrayCb->isChecked());
    settings.setValue("startMinimized", startMinimizedCb->isChecked());
}

void MainWindow::onLanguageChanged(int index)
{
    QString lang = langCombo->itemData(index).toString();
    QSettings settings("HKEY_CURRENT_USER\\Software\\WinMountExt4", QSettings::NativeFormat);
    if (settings.value("language").toString() != lang) {
        settings.setValue("language", lang);
        QMessageBox::information(this, tr("Language Changed"), tr("Please restart the application to apply the new language."));
    }
}

bool MainWindow::shouldStartMinimized() const
{
    return startMinimizedCb->isChecked();
}
