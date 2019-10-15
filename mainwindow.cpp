#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QCharRef>
#include <QTextCodec>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    kodCommand = 1; //изначально стоит на передаче сообщений
    winner = false;
    loser = false;
    messLastSent = false; //последнее сообщение отправлено

    number = 1; //номер операции
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

    out.setVersion(QDataStream::Qt_4_1);

    if(winner && !messLastSent)
    {
        ui->label_4->setText(trUtf8("Победа"));
        messLeng = 0;
        out << (short)kodCommand << messLeng << (short)messStatus << text;
        udpSocket.writeDatagram(datagram, QHostAddress::LocalHost, (quint16)portBroadc);
        messLastSent = true;
    }
    else if(loser && !messLastSent)
    {
        kodCommand = 2; //передача КС
        ui->label_4->setText(trUtf8("Поражение"));
        out << (short)kodCommand << messLeng << (short)messStatus << contrsum;
        udpSocket.writeDatagram(datagram, QHostAddress::LocalHost, (quint16)portBroadc);
        messLastSent = true;
    }
    else if(messStatus == 1 || messStatus == 2)
    {
        out << (short)kodCommand << messLeng << (short)messStatus << text;
        udpSocket.writeDatagram(datagram, QHostAddress::LocalHost, (quint16)portBroadc);
    }



}


void MainWindow::processPendingDatagrams()
{

 short _kodComand = 0;
 short _messLeng = 0;
 short _messStatus = 0;
 int _contrsum = 0;
 QByteArray datagram;

 do {

    datagram.resize(udpSocketWait.pendingDatagramSize());

    udpSocketWait.readDatagram(datagram.data(), datagram.size());

 } while (udpSocketWait.hasPendingDatagrams());

 QDataStream in(&datagram, QIODevice::ReadOnly);

 in.setVersion(QDataStream::Qt_4_1);

 if(!winner)
 {
    in >> _kodComand >>_messLeng >> _messStatus  >> text;
 }
 else if(winner)
 {
    in >> _kodComand >>_messLeng >> _messStatus  >> _contrsum;
 }


 if(_kodComand == 1  && _messLeng == 0)
 {
     loser = true;
     winner = false;
     ui->label_5->setText(QString::number(loser));
 }
 else if (_kodComand == 2 && _messStatus == 3) //последнее сообщение от ПО 2
 {
    ui->label_5->setText(QString::number(_contrsum)); //получили КС от ПО №2
    ui->label_2->setText(QString::number(contrsumCommon));
 }
 if(!winner && !loser)
 deletText();

 messLeng = (short)text.length(); //устанавливаем длину передаваемого сообщения
 if(messLeng > maxLengMes)
 {
     QMessageBox *msgBox = new QMessageBox(QMessageBox::Information,trUtf8("Предупреждение"),trUtf8("Максимальный размер передавамого сообщения превышен"),
     QMessageBox::Ok| QMessageBox::Cancel);

        if(msgBox->exec() == QMessageBox::Yes)
        {
            this->close();
        }

       delete msgBox;
 }
 if (winner || loser) {messStatus = 3;} //последнее сообщение
 else messStatus = 2; //промежуточное сообщение

   if(!messLastSent) //последнее сообщение не было отправлено
   {
    sendDatagram();
   }

}

void MainWindow::deletText()
{
    QTextCodec* codec = QTextCodec::codecForName("Windows-1251");
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForLocale(codec);
    QString newText = " "; //новая строка, после удаления
    int contrsumOperat = 0; //контрольная сумма удаленных слов за одну операцию
    countDeletWord = 0;
    QCharRef letter = text[0]; //записываем букву

    int i = 0, j = 0;
    bool space = false;

    while(i != text.length())
    {
        if ( (space || i == 0) && (text[i] == letter || text[i] == letter.toLower() || text[i] == letter.toUpper() ) )
        {
            while(text[i] != ' ')
            {
                contrsumOperat = controlSum(text[i], contrsumOperat); //считаем контрольную сумму удаленных слов
                i++;
            }
            countDeletWord++;
            i++;
            space = true;

        }
        else
        {
            if(text[i] == ' ') {
                space = true;
            }
            else {
                space = false;
            }
            newText[j] = text[i];
            i++;
            j++;
        }
    }


    contrsum += contrsumOperat; //считаем все удаленные слова
    ui->label_3->setNum(contrsum);

    ui->tableWidget->setRowCount(ui->tableWidget->rowCount() + 1);
    QTableWidgetItem* item = new QTableWidgetItem;
    item->setText(QString::number(number));
    item->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 0, item);
    QTableWidgetItem* item1 = new QTableWidgetItem;
    item1->setText((QString)letter);
    ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 1, item1);
    QTableWidgetItem* item2 = new QTableWidgetItem;
    item2->setText(QString::number(countDeletWord));
    ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 2, item2);
    QTableWidgetItem* item3 = new QTableWidgetItem;
    item3->setText(QString::number(contrsumOperat));
    ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 3, item3);
    ui->tableWidget->resizeColumnsToContents();
    number++;

    if (newText[0] != ' '){
    text.swap(newText); //обновим значение нашей строки перед передачей
    }
    else {
        winner = true;
    }
}

int MainWindow::controlSum(QCharRef let, int csumma)
{
    char code = let.toAscii();
    int c = 0;


    if( (code >= 65 && code <= 90) || (code >=97 && code <= 122) )
    {
        csumma += code;
    }
    else if (code >= -64 && code < 0)
    {
        c = 256 + code; //для русских символов
        csumma += c;
    }
    return csumma;

}

void MainWindow::on_pushButton_clicked()
{
    //QString text = "";

    QString filename = QFileDialog::getOpenFileName(this, QString("Open file"), QString(), QString("Text files (*.txt)"));
    if (!filename.isEmpty())
    {
        QFile fileText(filename);
        fileText.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream out(&fileText);
        out.setCodec("Unicode");//установка кодека
        while(!out.atEnd())
        {
            text=text+out.readLine() + ' ';
        }
        //text = out.readAll();
        //ui->label->setText(text);
        fileText.close();

        QTextCodec* codec = QTextCodec::codecForName("Windows-1251");
        QTextCodec::setCodecForTr(codec);
        QTextCodec::setCodecForCStrings(codec);
        QTextCodec::setCodecForLocale(codec);

        int i = 0;
        while(i != text.length())
        {
            if(text[i] != ' ')
            {
                contrsumCommon = controlSum(text[i], contrsumCommon); //подсчет общей контрольной суммы
            }
            i++;
        }

        messLeng = (short)text.length(); //устанавливаем длину передаваемого сообщения
        messStatus = 1; //первое сообщение
        sendDatagram();
    }
}

void MainWindow::on_lineEdit_editingFinished()
{
    portRecept = ui->lineEdit->text().toUInt();
    udpSocketWait.bind((quint16)portRecept);
}

void MainWindow::on_lineEdit_2_editingFinished()
{
    portBroadc = ui->lineEdit_2->text().toUInt();
}

void MainWindow::on_lineEdit_3_editingFinished()
{
    maxLengMes = ui->lineEdit_3->text().toUInt();
}
