#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    unsigned int portRecept; //порт приема
    unsigned int portBroadc; //порт передачи
    unsigned int maxLengMes; //максимальный размер передаваемых сообщений

    char kodCommand; //код команды
    short messLeng; //длина передаваемого сообщения
    char messStatus; //статус передаваемого сообщения

    QString text;
    bool winner;
    bool loser;
    bool messLastSent;

    int countDeletWord; //количество удаленных слов
    int number; //номер операции
    int contrsum; //контрольная сумма всех удаленных слов
    int contrsumCommon; //контрольная сумма всех слов
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void processPendingDatagrams();
    void sendDatagram();
    void deletText();
    int controlSum(QCharRef let, int contrsum); //вычисление контрольной суммы по буквам

    void on_lineEdit_editingFinished();

    void on_lineEdit_2_editingFinished();

    void on_lineEdit_3_editingFinished();

private:
    Ui::MainWindow *ui;
    QUdpSocket udpSocket;
    QUdpSocket udpSocketWait;
};

#endif // MAINWINDOW_H
