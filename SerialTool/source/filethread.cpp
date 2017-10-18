#include "filethread.h"
#include "xmodem.h"

static XModemClass xmodem;

FileThread::FileThread()
{
    xmodem.setThread(this);
    transMode = StopMode;
}

// �����ļ�
void FileThread::setFileName(const QString &fileName)
{
    this->fileName = fileName;
}

// ���ô���Э��
void FileThread::setProtocol(Protocol mode)
{
    protocol = mode;
}

// �����շ�ģʽ
void FileThread::setTransMode(TransMode mode)
{
    transMode = mode;
}

// ��������
bool FileThread::startTransfer()
{
    bool res;
    file = new QFile(fileName);
    if (transMode == SendMode) {
        res = file->open(QFile::ReadOnly);
        switch (protocol) {
        case XModem:
            xmodem.startTransmit();
            break;
        default:
            break;
        }
    } else {
        res = file->open(QFile::WriteOnly);
        switch (protocol) {
        case XModem:
            xmodem.startReceive();
            break;
        default:
            break;
        }
    }
    fSize = file->size();
    fPos = 0;
    return res;
}

// ȡ������
bool FileThread::cancelTransfer()
{
    bool res = true;

    switch (protocol) {
    case XModem:
        res = xmodem.cancelTrans();
        break;
    default:
        break;
    }
    if (res) {
        file->close();
        delete file;
        file = nullptr;
        transMode = StopMode;
    }
    return res;
}

// ��ȡ�ļ���С
qint64 FileThread::fileSize()
{
    return fSize;
}

// ��ȡ�ļ�ƫ��
qint64 FileThread::filePos()
{
    return fPos;
}

// ��ȡ�������
char FileThread::progress()
{
    if (fSize == 0) {
        return 0;
    }
    return (char)(fPos * 100 / fSize);
}

// ��ȡ���ݲۺ���
void FileThread::readData(const QByteArray &array)
{
    char result = 0;

    if (transMode == SendMode) { // ����ģʽ
        switch (protocol) {
        case XModem:
            result = xmodem.transmit(array[0], fPos);
            break;
        default:
            break;
        }
    } else if (transMode == ReceiveMode) { // ����ģʽ
        switch (protocol) {
        case XModem:
            result = xmodem.receive(array, fPos);
            break;
        default:
            break;
        }
        fSize = fPos;
    }
    if (result) { // �������
        file->close();
        delete file;
        file = nullptr;
        transMode = StopMode;
        emit transFinsh();
    }
}

// ��������
void FileThread::sendPortData(const QByteArray &array)
{
    emit sendData(array);
}

// �̺߳���
void FileThread::run()
{
    exec();
}

void FileThread::writeFile(const char* buffer, int size)
{
    file->write(buffer, size);
}

int FileThread::readFile(char* buffer, int size)
{
    return file->read(buffer, size);
}
