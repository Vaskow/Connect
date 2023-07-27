#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
//#include <QTextCodec>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_readFile_clicked();
    void processPendingDatagrams();
    void sendDatagram();
    void deletText();
    void controlSum(QString word, int& contrsum); //вычисление контрольной суммы по буквам

    void on_receivPort_editingFinished();
    void on_transmitPort_editingFinished();

private:
    char codCommand; //код команды
    unsigned short msgLength; //длина передаваемого сообщения
    char msgStatus; //статус передаваемого сообщения
    unsigned int portReceive; //порт приема
    unsigned int portTransmit; //порт передачи

    enum StatusSendMsg {
        FirstMsg,       //первое передаваемое сообщение
        IntermedMsg,    //промежуточное передаваемое сообщение
        LastMsg         //последнее передаваемое сообщение
    };

    enum CodCmd {
        SendMsg,    //передача сообщения
        SendCs      //передача контрольной суммы
    };

    QString text;
    bool winner;
    bool loser;
    bool msgLastSend;   //последнее сообщение было отправлено

    int countDeletWords;    //количество удаленных слов
    int contrsum;           //контрольная сумма всех удаленных слов
    int contrsumCommon;     //контрольная сумма всех слов

    Ui::MainWindow *ui;
    QUdpSocket udpSocket;
    QUdpSocket udpSocketWait;
    QTextCodec* codec_cp1251;
};

#endif // MAINWINDOW_H
