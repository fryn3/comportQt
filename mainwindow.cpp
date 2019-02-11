#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    setDialog(new SettingsDialog),
    m_serial(new QSerialPort(this))
{
    ui->setupUi(this);
    setDialog->setModal(true);
    connect(ui->actConfigure, &QAction::triggered, setDialog, &MainWindow::show);
    connect(setDialog, &SettingsDialog::sigApply, this, &MainWindow::slApply);
    connect(ui->btnConnect, &QPushButton::clicked, this, &MainWindow::slOpenSerialPort);
    connect(ui->btnDisconnect, &QPushButton::clicked, this, &MainWindow::slCloseSerialPort);
    connect(ui->btnMonitorClear, &QPushButton::clicked, ui->pteMonitor, &QPlainTextEdit::clear);
    ui->lSelectPort->setText(setDialog->settings().name);
    slApply();
    m_msgTimer.restart();
}

void MainWindow::slApply()
{
    m_settings = setDialog->settings();
    ui->lSelectPort->setText(m_settings.name);
    qDebug() << m_settings.baudRate;
}

void MainWindow::slOpenSerialPort()
{
    const SettingsDialog::Settings p = m_settings;
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);
    if (m_serial->open(QIODevice::ReadWrite)) {
        ui->gbMonitor->setEnabled(true);
        ui->btnConnect->setEnabled(false);
        ui->btnDisconnect->setEnabled(true);
        ui->actConfigure->setEnabled(false);
        ui->statusBar->showMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } else {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());

        ui->statusBar->showMessage(tr("Open error"));
    }
}

void MainWindow::slSendData(QByteArray data)
{
    auto time = m_msgTimer.restart();
    m_serial->write(data);
    if (ui->rbHex->isChecked()) {
        ui->pteMonitor->appendHtml(QString("<div style='color:black;'>Tx (size = %1, time = %3ms):<pre>%2</pre></div>")
                      .arg(data.size())
                      .arg(data.toHex(' ').toUpper().data())
                      .arg(time));
    } else {
        ui->pteMonitor->appendHtml(QString("<div style='color:black;'>Tx (size = %1, time = %3ms):<pre>%2</pre></div>")
                      .arg(data.size())
                      .arg(QString(data))
                      .arg(time));
    }

    m_historyTx.removeAll(data);
    m_historyTx.append(data);
}

void MainWindow::slReadData()
{
    const QByteArray data = m_serial->readAll();
    auto time = m_msgTimer.restart();
    if (ui->rbHex->isChecked()) {
        ui->pteMonitor->appendHtml(QString("<div style='color:green;'>Rx (size = %1, time = %3ms):<pre>%2</pre></div>")
                      .arg(data.size())
                      .arg(data.toHex(' ').toUpper().data())
                      .arg(time));
    } else {
        ui->pteMonitor->appendHtml(QString("<div style='color:green;'>Rx (size = %1, time = %3ms):<pre>%2</pre></div>")
                      .arg(data.size())
                      .arg(QString(data))
                      .arg(time));
    }
}

void MainWindow::on_btnSend_clicked()
{
    auto data = ui->leSend->text().toLatin1();
    if (ui->rbHex->isChecked()) {
        slSendData(QByteArray::fromHex(data));
    } else {
        slSendData(data);
    }
    ui->leSend->clear();
    m_indexHistory = 0;
}

void MainWindow::slCloseSerialPort()
{
    if (m_serial->isOpen())
        m_serial->close();
    ui->gbMonitor->setEnabled(false);
    ui->btnConnect->setEnabled(true);
    ui->btnDisconnect->setEnabled(false);
    ui->actConfigure->setEnabled(true);
    ui->statusBar->showMessage(tr("Disconnected"));
}

MainWindow::~MainWindow()
{
    delete setDialog;
    delete ui;
}
