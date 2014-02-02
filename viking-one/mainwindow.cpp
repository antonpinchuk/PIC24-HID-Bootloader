#include "mainwindow.h"
#include "ui_mainwindow.h"
//------------------------------------------------------------------------------------------------------------//
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    updateAvailableDialog = new UpdateAvailableDialog(this);
    UpdateScheduler       = new TUpdateScheduler(this);
    SystemTrayMenu        = new QMenu("tray menu");


    QAction* checkUpdateAction = SystemTrayMenu->addAction("check update");
    QAction* quitAction        = SystemTrayMenu->addAction("quit");

    connect(quitAction       , SIGNAL(triggered()), this, SLOT(Exit()) );
    connect(checkUpdateAction, SIGNAL(triggered()), UpdateScheduler, SLOT(CheckUpdate()) );

    Ico = new QSystemTrayIcon(this);
    Ico->setIcon(QIcon(":/new/ico/app.ico"));
    Ico->setContextMenu(SystemTrayMenu);
    Ico->show();

    connect(Ico , SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT  (TrayIcoClick(QSystemTrayIcon::ActivationReason)) );



    connect( ui->CheckVersionBtn , SIGNAL(clicked()), UpdateScheduler, SLOT(CheckUpdate()) );
    connect( UpdateScheduler, SIGNAL(NeedUpdate(QString,QString)), this, SLOT(NeedUpdate(QString,QString)) );

    /* Bootloader */
    bootloader = new Bootloader();
    bootloader->writeFlash       = true;
    bootloader->writeConfig      = false; //Force user to manually re-enable it every time they re-launch the application.  Safer that way.
    bootloader->eraseDuringWrite = true;

    connect(bootloader, SIGNAL(setConnected(bool)), this, SLOT(setConnected(bool)));
    connect(bootloader, SIGNAL(setBootloadEnabled(bool)), this, SLOT(setBootloadEnabled(bool)));
    connect(bootloader, SIGNAL(setBootloadBusy(bool)), this, SLOT(setBootloadBusy(bool)));
    connect(bootloader, SIGNAL(setProgressBar(int)), this, SLOT(updateProgressBar(int)));
    connect(bootloader, SIGNAL(message(Bootloader::MessageType, QString)), this, SLOT(onMessage(Bootloader::MessageType, QString)));
    connect(bootloader, SIGNAL(messageClear()), this, SLOT(onMessageClear()));

    setConnected(false);
    enabledBtn(false);
    /* */

    QPalette palette = ui->StatusLbl->palette();
    palette.setColor(ui->StatusLbl->backgroundRole(), Qt::red);
    palette.setColor(ui->StatusLbl->foregroundRole(), Qt::red);
    ui->StatusLbl->setPalette(palette);
    ui->CheckVersionBtn->setVisible(false);

}
//------------------------------------------------------------------------------------------------------------//
MainWindow::~MainWindow()
{
    delete ui;
    delete Ico;
    delete UpdateScheduler;
    delete updateAvailableDialog;
    delete SystemTrayMenu;
}
//------------------------------------------------------------------------------------------------------------//
void MainWindow::changeEvent( QEvent * event )
{
    // упрятывание приложения в трей
    if(event->type() == QEvent::WindowStateChange) {
        if(isMinimized()) {
            this->hide();
            event->ignore();
        }
    }
    QMainWindow::changeEvent(event);
}
//------------------------------------------------------------------------------------------------------------//
void MainWindow::TrayIcoClick(QSystemTrayIcon::ActivationReason Reason)
{
    if(Reason == QSystemTrayIcon::Trigger){
        if(!this->isHidden()){
            this->hide();
        }
        else{
            this->showNormal();
            this->activateWindow();
        }
    }
}
//------------------------------------------------------------------------------------------------------------//
void MainWindow::NeedUpdate(QString AppFile, QString ReleaseNotes)
{
    updateAvailableDialog->ShowUpdateDialog(AppFile, ReleaseNotes);
}
//------------------------------------------------------------------------------------------------------------//
void MainWindow::Exit()
{
    QCoreApplication::exit();
}
//------------------------------------------------------------------------------------------------------------//
void MainWindow::enabledBtn(bool enable)
{
    ui->WriteRunePackBtn->setEnabled(enable);
    ui->ReadRunePackBtn->setEnabled(enable);
}
//------------------------------------------------------------------------------------------------------------//
void MainWindow::setConnected(bool connected)
{
    if (connected) {
        ui->StatusLbl->setText("Status:Connected");

        QPalette palette = ui->StatusLbl->palette();
        palette.setColor(ui->StatusLbl->backgroundRole(), Qt::darkGreen);
        palette.setColor(ui->StatusLbl->foregroundRole(), Qt::darkGreen);
        ui->StatusLbl->setPalette(palette);
        enabledBtn(true);
    } else {
        ui->StatusLbl->setText("Status:Disconnected");

        QPalette palette = ui->StatusLbl->palette();
        palette.setColor(ui->StatusLbl->backgroundRole(), Qt::red);
        palette.setColor(ui->StatusLbl->foregroundRole(), Qt::red);
        ui->StatusLbl->setPalette(palette);
        enabledBtn(false);
    }
}
//------------------------------------------------------------------------------------------------------------//
void MainWindow::setBootloadEnabled(bool enable)
{
}
//------------------------------------------------------------------------------------------------------------//
void MainWindow::setBootloadBusy(bool busy)
{
    if (busy) {
        QApplication::setOverrideCursor(Qt::BusyCursor);
    } else {
        QApplication::restoreOverrideCursor();
    }
    enabledBtn(!busy);
}
//------------------------------------------------------------------------------------------------------------//
void MainWindow::onMessage(Bootloader::MessageType type, QString value) {
    if (type == Bootloader::Warning) {
        QMessageBox::warning(this, "Warning!", value, QMessageBox::AcceptRole, QMessageBox::AcceptRole);
        value = "Warning: " + value;
    }
    if (type == Bootloader::Error) {
        QMessageBox::critical(this, "Error!", value, QMessageBox::AcceptRole, QMessageBox::AcceptRole);
        value = "Error: " + value;
    }
    ui->plainTextEdit->appendPlainText(value);
}
//------------------------------------------------------------------------------------------------------------//
void MainWindow::onMessageClear() {
    ui->plainTextEdit->clear();
}
//------------------------------------------------------------------------------------------------------------//
void MainWindow::updateProgressBar(int newValue) {
    ui->progressBar->setValue(newValue);
}
//------------------------------------------------------------------------------------------------------------//
void MainWindow::on_WriteRunePackBtn_clicked()
{
    QString newFileName;
    HexImporter::ErrorCode result;
    onMessageClear();

    newFileName = QFileDialog::getOpenFileName(this, "Open Hex File", fileName, "Hex Files (*.hex *.ehx)");

    if (newFileName.isEmpty()) {
        return;
    }
    result = bootloader->LoadFile(newFileName);

    if (result == HexImporter::Success) {

        future = QtConcurrent::run(bootloader, &Bootloader::WriteDevice);

        onMessage(Bootloader::Info, "Starting Erase/Program/Verify Sequence.");
        onMessage(Bootloader::Info, "Do not unplug device or disconnect power until the operation is fully complete.");
        onMessage(Bootloader::Info, " ");
    }

    QApplication::restoreOverrideCursor();
}
