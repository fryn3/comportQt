#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QElapsedTimer>
#include <QMainWindow>
#include "settingsdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected slots:
    void slApply();
    void slOpenSerialPort();
    void slCloseSerialPort();
    void slSendData(QByteArray datagram);
    void on_btnSend_clicked();
    void slReadData();
private:
    Ui::MainWindow *ui;
    SettingsDialog *setDialog;
    SettingsDialog::Settings m_settings;
    QSerialPort *m_serial;
    int m_indexHistory = 0;
    QStringList m_historyTx;
    QElapsedTimer m_msgTimer;
};

#endif // MAINWINDOW_H
