#include <QMessageBox>
#include <QDateTime>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    pPingTest = new PingTest();

    connect(pPingTest, SIGNAL(sigGetResponseTime(qlonglong)), this, SLOT(slotGetResponseTime(qlonglong)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnStartStop_clicked()
{
    if (true == ui->editIP->text().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Enter IP first.");
        msgBox.exec();
        return;
    }

    if (ui->btnStartStop->text().compare(tr("Start")) == 0) {
        pPingTest->setIP(ui->editIP->text());
        pPingTest->ping(true);
        ui->btnStartStop->setText(tr("Stop"));
    }
    else {
        pPingTest->ping(false);
        ui->btnStartStop->setText(tr("Start"));
    }
}

void MainWindow::slotGetResponseTime(qlonglong us)
{
    QDateTime currTime = QDateTime::currentDateTime();

    if (us >= 0) {
        ui->editPingTimeRecord->textCursor().insertText("[" + currTime.toString("yyyy-MM-ddT hh:mm:ss") +
                                                    "] " + QString::number(us) + "us\n");
    }
    else {
        ui->editPingTimeRecord->textCursor().insertText("[" + currTime.toString("yyyy-MM-ddT hh:mm:ss") +
                                                    "] timeout\n");
    }
}

