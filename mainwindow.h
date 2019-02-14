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
    void slRadioBtnClicked();
    void slSendComandChange(const QString &newText);
protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
private:
    Ui::MainWindow *ui;
    SettingsDialog *setDialog;
    SettingsDialog::Settings m_settings;
    QSerialPort *m_serial;
    int m_indexHistory = 0;
    struct HistoryStruct
    {
        bool isTx;
        QByteArray data;
        qint64 time;
    };
    QVector<HistoryStruct> m_historyRxTx;
    QVector<QByteArray> m_historyTx;
    QElapsedTimer m_msgTimer;
    void printMsg(bool isTx, QByteArray data, qint64 time);
    void printMsg(HistoryStruct histItem);
};

#endif // MAINWINDOW_H
