#include "QtWidgetsApplication3.h"
#include <QApplication>
#include <QPushButton>
#include <QTextEdit>
#include <QUdpSocket>
#include <QVBoxLayout>
#include <QThread>
#include <QFileDialog>
#include <QLabel>
#include <QSqlDatabase> 
#include <QSqlQuery> 

class UdpChat : public QWidget {
    Q_OBJECT

public:
    UdpChat(QWidget* parent = nullptr) : QWidget(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);
        messageInput = new QTextEdit(this);
        sendButton = new QPushButton("Send", this);
        fileButton = new QPushButton("Pin file", this);
        incomingBox = new QTextEdit(this);
        outgoingBox = new QTextEdit(this);
        incomingBox->setReadOnly(true);
        outgoingBox->setReadOnly(true);

        layout->addWidget(new QLabel("Incoming Messages", this));
        layout->addWidget(incomingBox);
        layout->addWidget(new QLabel("Outgoing Messages", this));
        layout->addWidget(outgoingBox);
        layout->addWidget(messageInput);
        layout->addWidget(fileButton);
        layout->addWidget(sendButton);

        socket = new QUdpSocket(this);
        socket->bind(QHostAddress::LocalHost, 1234);

        connect(sendButton, &QPushButton::clicked, this, &UdpChat::sendMessage);
        connect(fileButton, &QPushButton::clicked, this, &UdpChat::sendFile);
        connect(socket, &QUdpSocket::readyRead, this, &UdpChat::receiveMessage);

        //  Попытка соединения с sql сервером
        /*
        db = QSqlDatabase::addDatabase("QPSQL");
        db.setHostName("localhost");
        db.setDatabaseName("bd_1"); 
        db.setUserName("postgres"); 
        db.setPassword("123"); 
       
        bool ok = db.open();
        if (!ok) {
            // Обработка ошибки соединения
            outgoingBox->append("Connection not established.");
        }
        */
    }

private slots:
    void sendMessage() {
        QByteArray message = messageInput->toPlainText().toUtf8();
        int packetSize = 65507; // Ìàêñèìàëüíûé ðàçìåð UDP-ïàêåòà

        for (int i = 0; i < message.size(); i += packetSize) {
            QByteArray datagram = message.mid(i, packetSize);
            socket->writeDatagram(datagram, QHostAddress::LocalHost, 1234);
            QThread::msleep(10); // Задержка между пакетами
        }

        messageInput->clear();
        outgoingBox->append("Message delivered.");

        // Сохранение сообщения в базу данных
        /*
        QSqlQuery query;
        query.prepare("INSERT INTO chat_log (message, direction) VALUES (:message, :direction)");
        query.bindValue(":message", message);
        query.bindValue(":direction", "outgoing");
        query.exec();
        */
    }

    void sendFile() {
        QString fileName = QFileDialog::getOpenFileName(this, "Choose file");
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray fileData = file.readAll();
            socket->writeDatagram(fileData, QHostAddress::LocalHost, 5432);
            outgoingBox->append("File " + fileName + " send.");

            // Сохранение сообщения в базу данных
            /*
            QSqlQuery query;
            query.prepare("INSERT INTO chat_log (message, direction) VALUES (:message, :direction)");
            query.bindValue(":message", "File " + fileName + " send.");
            query.bindValue(":direction", "outgoing");
            query.exec();
            */
        }
    }

    void receiveMessage() {
        while (socket->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(socket->pendingDatagramSize());
            socket->readDatagram(datagram.data(), datagram.size());
            QString message = QString::fromUtf8(datagram);
            incomingBox->append("Recived: " + message);

            // Сохранение сообщения в базу данных
            /*
            QSqlQuery query;
            query.prepare("INSERT INTO chat_log (message, direction) VALUES (:message, :direction)");
            query.bindValue(":message", message);
            query.bindValue(":direction", "incoming");
            query.exec();
            */
        }
    }

private:
    QTextEdit* messageInput;
    QPushButton* sendButton;
    QPushButton* fileButton;
    QTextEdit* incomingBox;
    QTextEdit* outgoingBox;
    QUdpSocket* socket;
    QSqlDatabase db; 
};

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");
    QApplication app(argc, argv);

    UdpChat chat;
    chat.show();

    return app.exec();
}

#include "main.moc"
