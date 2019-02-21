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
    connect(ui->rbHex, &QRadioButton::clicked, ui->actHex, &QAction::trigger);
    connect(ui->rbText, &QRadioButton::clicked, ui->actText, &QAction::trigger);
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::slReadData);
    connect(ui->leSend, &QLineEdit::textChanged, this, &MainWindow::slSendComandChange);
    connect(ui->actHex, &QAction::triggered, this, &MainWindow::slModeChange);
    connect(ui->actText, &QAction::triggered, this, &MainWindow::slModeChange);
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

void MainWindow::printMsg(bool isTx, QByteArray data, qint64 time)
{
    if (isTx) {
        ui->pteMonitor->appendHtml(QString("<div style='color:black;'>Tx (size = %1, time = %3ms):<pre>%2</pre></div>")
                          .arg(data.size())
                          .arg(convertToPrint(data, ui->rbHex->isChecked()))
                          .arg(time));
    } else {
        ui->pteMonitor->appendHtml(QString("<div style='color:green;'>Rx (size = %1, time = %3ms):<pre>%2</pre></div>")
                          .arg(data.size())
                          .arg(convertToPrint(data, ui->rbHex->isChecked()))
                          .arg(time));
    }
}

void MainWindow::printMsg(HistoryStruct histItem)
{
    printMsg(histItem.isTx, histItem.data, histItem.time);
}

void MainWindow::slSendData(QByteArray data)
{
    auto time = m_msgTimer.restart();
    m_serial->write(data);
    HistoryStruct item { true, data, time };
    printMsg(item);
    m_historyRxTx.append(item);
    m_historyTx.removeAll(data);
    m_historyTx.append(data);
}

void MainWindow::slReadData()
{
    const QByteArray data = m_serial->readAll();
    auto time = m_msgTimer.restart();

    HistoryStruct item { false, data, time };
    printMsg(item);

    m_historyRxTx.append(item);
}

QByteArray MainWindow::convertToSend(QString msg, bool isHex)
{
    if (isHex) {
        return QByteArray::fromHex(msg.toLatin1());
    } else {
        msg.replace("\\r", "\r");
        return msg.toLatin1();
    }
}

QString MainWindow::convertToPrint(QByteArray hex, bool isHex)
{
    if (isHex) {
        return hex.toHex(' ').toUpper().data();
    } else {
        return QString(hex).replace("\r", "\\r");
    }
}

void MainWindow::on_btnSend_clicked()
{
    slSendData(convertToSend(ui->leSend->text(), ui->rbHex->isChecked()));
    ui->leSend->clear();
    m_indexHistory = 0;
}

void MainWindow::slModeChange()
{
    static QAction *lastMode = nullptr;
    auto act = dynamic_cast<QAction *>(sender());

    Q_ASSERT(act);
    if (lastMode == act) {
        return;
    }
    lastMode = act;
    if (act == ui->actHex) {
        ui->rbHex->setChecked(true);
    } else {
        ui->rbText->setChecked(true);
    }
    ui->pteMonitor->clear();
    for (HistoryStruct item: m_historyRxTx) {
        printMsg(item);
    }
    QByteArray ar = convertToSend(ui->leSend->text(), !ui->rbHex->isChecked());
    ui->leSend->setText(convertToPrint(ar, ui->rbHex->isChecked()));
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    static QByteArray textLineEditUp;
    if (!event->text().isEmpty() && ui->leSend->isEnabled()) {
        if (event->key() == Qt::Key_Enter
                || event->key() == Qt::Key_Return) {
            on_btnSend_clicked();
        } else if (event->key() == Qt::Key_Escape) {
            ui->pteMonitor->clear();
        } else if ((ui->rbHex->isChecked()
                    && ((Qt::Key_0 <= event->key() && event->key() <= Qt::Key_9)
                   || (Qt::Key_A <= event->key() && event->key() <= Qt::Key_F)))
                   || (ui->rbText->isChecked()
                       && ((Qt::Key_Plus <= event->key() && event->key() <= Qt::Key_9)
                   || (Qt::Key_A <= event->key() && event->key() <= Qt::Key_Z))))
        {
            ui->leSend->setFocus();
            ui->leSend->setText(ui->leSend->text()
                                          .append(event->text()));
        }
    } else if (event->key() == Qt::Key_Up) {
        if (m_indexHistory == 0) {
            textLineEditUp = convertToSend(ui->leSend->text(), ui->rbHex->isChecked());
        }
        if (m_historyTx.size() > m_indexHistory) {
            m_indexHistory++;
            ui->leSend->setText(convertToPrint(m_historyTx.at(m_historyTx.size() - m_indexHistory), ui->rbHex->isChecked()));
        }
    } else if (event->key() == Qt::Key_Down) {
        if (m_indexHistory > 0) {
            m_indexHistory--;
            if (m_indexHistory == 0) {
                ui->leSend->setText(textLineEditUp);
            } else {
                ui->leSend->setText(convertToPrint(m_historyTx.at(m_historyTx.size() - m_indexHistory), ui->rbHex->isChecked()));
            }
        }
    } else {
        QWidget::keyPressEvent(event);
    }
}

void MainWindow::slSendComandChange(const QString &newText) {
    if (ui->rbHex->isChecked()) {
        int countSpaceLast = newText.count(" ");
        int cursPos = ui->leSend->cursorPosition();
        QString t = newText;
        t.remove(QRegExp("[^a-fA-F0-9]*"));
        int countSpace = 0;
        int len = t.length();
        while ((len - 1) / 2 > countSpace ) {
            t.insert(len - 2 * ++countSpace, ' ');
        }
        t = t.toUpper();
        if (newText != t) {
            ui->leSend->setText(t);
            ui->leSend->setCursorPosition(cursPos + countSpace - countSpaceLast);
        }
    } else if (ui->rbText->isChecked()) {
        // тут особых правил нет
    }
}

MainWindow::~MainWindow()
{
    delete setDialog;
    delete ui;
}
