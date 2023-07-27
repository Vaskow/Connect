#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QCharRef>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    portReceive = 0;    //порты не заданы
    portTransmit = 0;

    msgLength = 0;
    msgStatus = FirstMsg;
    codCommand = SendMsg; //изначально стоит на передаче сообщений
    winner = false;
    loser = false;
    msgLastSend = false; //последнее сообщение отправлено

    contrsum = 0;
    contrsumCommon = 0;
    connect(&udpSocketWait, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sendDatagram()
{
    QByteArray datagram;

    QDataStream out(&datagram, QIODevice::WriteOnly);

    if (winner && !msgLastSend)
    {
        ui->statusSoft->setText(tr("Победа"));
        msgLength = 0;
        codCommand = SendMsg;
        out << (short)codCommand << msgLength << (short)msgStatus << text;
        udpSocket.writeDatagram(datagram, QHostAddress::LocalHost, (quint16)portTransmit);
        msgLastSend = true;
    }
    else if (loser && !msgLastSend)
    {
        codCommand = SendCs; //передача КС
        ui->statusSoft->setText(tr("Поражение"));
        out << (short)codCommand << msgLength << (short)msgStatus << contrsum;
        udpSocket.writeDatagram(datagram, QHostAddress::LocalHost, (quint16)portTransmit);
        msgLastSend = true;
    }
    else if (msgStatus == FirstMsg || msgStatus == IntermedMsg)
    {
        out << (short)codCommand << msgLength << (short)msgStatus << text;
        udpSocket.writeDatagram(datagram, QHostAddress::LocalHost, (quint16)portTransmit);
    }
}

void MainWindow::processPendingDatagrams()
{
    short _codComand = 0;
    short _msgLength = 0;
    short _msgStatus = 0;
    int _contrsum = 0;
    QByteArray datagram;

    do {
        datagram.resize(udpSocketWait.pendingDatagramSize());
        udpSocketWait.readDatagram(datagram.data(), datagram.size());
    } while (udpSocketWait.hasPendingDatagrams());

    QDataStream in(&datagram, QIODevice::ReadOnly);

    if (!winner)
    {
        in >> _codComand >>_msgLength >> _msgStatus  >> text;
    }
    else if (winner)
    {
        in >> _codComand >>_msgLength >> _msgStatus  >> _contrsum;
    }

    if (_codComand == SendMsg  && _msgStatus == LastMsg)
    {
        loser = true;
        winner = false;
        msgStatus = LastMsg;
        int csWinner = contrsumCommon - contrsum;
        ui->csOtherSoft->setText(QString::number(csWinner)); //посчитали КС победителя
    }
    else if (_codComand == SendCs && _msgStatus == LastMsg) //последнее сообщение от ПО 2
    {
        ui->csOtherSoft->setText(QString::number(_contrsum));   //получили КС от ПО №2
    }
    else msgStatus = IntermedMsg;

    if (contrsumCommon == 0) {
        controlSum(text, contrsumCommon); //подсчет общей контрольной суммы для ПО №2
        ui->csCommon->setText(QString::number(contrsumCommon));
    }

    if (msgStatus != LastMsg) deletText();

    msgLength = (short)text.length();   //устанавливаем длину передаваемого сообщения

    if (!msgLastSend) //последнее сообщение не было отправлено
    {
        sendDatagram();
    }
}

void MainWindow::deletText()
{
    QString newText = " "; //новая строка, после удаления
    int contrsumOperat = 0; //контрольная сумма удаленных слов за одну операцию
    countDeletWords = 0;
    QChar letter = text[0]; //записываем букву

    int i = 0;

    QVector<int> vec;
    QStringList listWord = text.split(" ");

    while(i != listWord.length())
    {
        if (listWord[i][0] == letter.toLower() || listWord[i][0] == letter.toUpper()) //слово начинаяется с нужной буквы
        {
            vec.push_back(i); //запоминаем слова для удаления
            countDeletWords++;

            controlSum(listWord[i], contrsumOperat);
        }
        i++;
    }

    int ind_del = 0;
    for(int elem : vec) //удаление элементов списка слов
    {
        listWord.removeAt(elem - ind_del);
        ind_del++;
    }

    if (listWord.length() > 1) newText = listWord.join(" "); //объединение оставшихся слов
    else if (listWord.length() == 1) newText = listWord[0];
    else if (listWord.isEmpty()) newText = nullptr;

    if (newText.isEmpty())
    {
        winner = true;
        msgStatus = LastMsg;
    }
    text.swap(newText); //обновим значение нашей строки перед передачей

    contrsum += contrsumOperat; //считаем все удаленные слова
    ui->csCurSoft->setNum(contrsum);

    ui->tableWidget->setRowCount(ui->tableWidget->rowCount() + 1);

    QTableWidgetItem* item1 = new QTableWidgetItem;
    item1->setText((QString)letter);
    item1->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 0, item1);

    QTableWidgetItem* item2 = new QTableWidgetItem;
    item2->setText(QString::number(countDeletWords));
    item2->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 1, item2);

    QTableWidgetItem* item3 = new QTableWidgetItem;
    item3->setText(QString::number(contrsumOperat));
    item3->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 2, item3);

    ui->tableWidget->resizeColumnsToContents();
}

void MainWindow::controlSum(QString word, int& csumma)
{
    QByteArray wordByteArr;
    wordByteArr = word.toLocal8Bit();

    for (int indw = 0; indw < wordByteArr.size(); ++indw)
    {
        if (word[indw].isLetter()) {
            int cs = static_cast<unsigned char>(wordByteArr[indw]);
            csumma += cs;
        }
    }
}

void MainWindow::on_readFile_clicked()
{
    if (!portReceive || !portTransmit)
    {
        QMessageBox msgBox;
        msgBox.setText("Введите значения для портов приёма и передачи!");
        msgBox.exec();
        return;
    }

    QString filename = QFileDialog::getOpenFileName(this, QString("Open file"), QString(), QString("Text files (*.txt)"));
    if (!filename.isEmpty())
    {
        QFile fileText(filename);
        fileText.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream out(&fileText);
        while(!out.atEnd())
        {
            text = text + out.readLine() + ' ';
        }

        fileText.close();

        controlSum(text, contrsumCommon); //подсчет общей контрольной суммы
        ui->csCommon->setText(QString::number(contrsumCommon));

        msgLength = (short)text.length(); //устанавливаем длину передаваемого сообщения
        sendDatagram();
    }
}


void MainWindow::on_receivPort_editingFinished()
{
    portReceive = ui->receivPort->text().toUInt();
    udpSocketWait.bind((quint16)portReceive);
}


void MainWindow::on_transmitPort_editingFinished()
{
    portTransmit = ui->transmitPort->text().toUInt();
}




