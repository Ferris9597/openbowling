#include <QtNetwork>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <iostream>

#include "namedpipe.h"

#define FIFO          "/tmp/ob_pipe"
#define TCP_SEND_PORT 12321

NamedPipe::NamedPipe( QObject *parent ) :
    QObject(parent)
{
    int i, value;

    connected = 0;

    value = ::mkfifo(FIFO, S_IRWXU | S_IRWXO | S_IRWXG);
    if (value == -1)
    {
        std::cout << "Failed to create FIFO (Named Pipe): " << FIFO << std::endl;
    }

    for (i = 0; i < LINE_LENGTH; i++) buffer[i] = '\0';

//        setSocketDescriptor(fd, QAbstractSocket::ConnectedState, QIODevice::ReadWrite);
//        setOpenMode (QIODevice::ReadWrite);
    tcpSocket = NULL;

    // find out which IP to connect to
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

    std::cout << "ipAddress: " << (char *)ipAddress.toLatin1().data() << "  port: " << TCP_SEND_PORT << std::endl;

#ifdef LKJH
    blockSize = 0;
    std::cout << "connectToServer: 1" << std::endl;
    if( tcpSocket != NULL )
    {
        tcpSocket->abort();
        delete tcpSocket;
    }

    std::cout << "connectToServer: 2" << std::endl;
    tcpSocket = new QTcpSocket(parent);

    std::cout << "connectToServer: 3" << std::endl;
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readFortune()));
    connect( tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)) );
    connect( tcpSocket, SIGNAL( connected() ), this, SLOT( tcpConnectionUp() ) );
    connect( tcpSocket, SIGNAL( disconnected() ), this, SLOT( tcpConnectionDown() ) );
#endif
//    connectToServer(parent);

}

void NamedPipe::connectToServer()
{

//    std::cout << "connectToHost" << std::endl;
//    tcpSocket->connectToHost(ipAddress, TCP_SEND_PORT);

}

void NamedPipe::readFortune()
{
        std::cout << "readFortune" << std::endl;
        connected = 1;
//        ReadPipe();
}

void NamedPipe::tcpConnectionUp()
{
        std::cout << "tcpConnectionUp" << std::endl;
        connected = 1;
//        ReadPipe();
}

void NamedPipe::tcpConnectionDown()
{
        std::cout << "tcpConnectionDown" << std::endl;
        tcpSocket = NULL;
        connected = 0;
}

void NamedPipe::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        std::cout << "ERROR: Remote host was closed." << FIFO << std::endl;
        break;
    case QAbstractSocket::HostNotFoundError:
        std::cout << "ERROR: The host was not found. " << (char *)tcpSocket->errorString().toLatin1().data() << std::endl;
        break;
    case QAbstractSocket::ConnectionRefusedError:
        std::cout << "ERROR: The connection was refused by the peer." << std::endl;
        break;
    default:
        std::cout << "ERROR: The following error occurred: " <<
                (char *)tcpSocket->errorString().toLatin1().data() << std::endl;
    }

}

void NamedPipe::ReadPipe()
{
    int m = O_RDONLY;
    int i, value;

 std::cout << "1**** " << std::endl;
#ifdef ASDGF
    fd = ::open(FIFO, m);
std::cout << "2**** " << std::endl;

    value = read(fd, &buffer, LINE_LENGTH);
    if (value < 0)
    {
        std::cout << "ERROR reading the pipe: " << errno << std::endl;
        sleep(2);
    }
    else if (value > 0)
    {
        std::cout << "INPUT value: " << value << " buffer: " << buffer << std::endl;

        // Clean buffer
        for (i = 0; i < LINE_LENGTH; i++) buffer[i] = '\0';
    }
    else std::cout << "buffer = 0" << std::endl;

    if (fd < 0)
    {
        ::close(fd);
    }
#endif

    const int Timeout = 5 * 1000;

     tcpSocket->connectToHost(ipAddress, TCP_SEND_PORT);

     if (!tcpSocket->waitForConnected(Timeout)) {
         std::cout << "ERROR2: " << (char *)tcpSocket->errorString().toLatin1().data() << std::endl;
         return;
     }

     while (tcpSocket->bytesAvailable() < (int)sizeof(quint16)) {
         if (!tcpSocket->waitForReadyRead(Timeout)) {
             std::cout << "ERROR3: " << (char *)tcpSocket->errorString().toLatin1().data() << std::endl;
             return;
         }
     }


    std::cout << "Make block" << std::endl;
    QString scoresToSend = "5,4,9";
    QByteArray block;
    QDataStream outData(&block, QIODevice::WriteOnly);
    outData.setVersion(QDataStream::Qt_4_6);

    outData << (quint16)0;	// placeholder which we will reset once we know the size of the payload
    outData << scoresToSend;
    outData.device()->seek(0);
    outData << (quint16)(block.size() - sizeof(quint16));

    std::cout << "OUTPUT sent" << std::endl;
    tcpSocket->write( block );

}