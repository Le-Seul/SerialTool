#include "vediobox.h"
#include <QtCore>
#include <QPainter>
#include <QClipboard>

VedioBox::VedioBox(QWidget *parent) : QDialog(parent)
{
    // ����ʾ�ʺ�
    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

    ui.setupUi(this);
    ui.label->setFixedSize(325, 245);
    setFixedSize(330, 279); // ���������ĶԻ���

    connect(ui.saveButton, SIGNAL(clicked()), this, SLOT(saveImage()));
    connect(ui.copyButton, SIGNAL(clicked()), this, SLOT(copyImage()));
}

// ���һ�ֽ�����
void VedioBox::addData(const QByteArray &arr)
{
    array.append(arr);
    qint32 i, len = array.size() - 1;
    const char *data = array.data();

    for (i = 0; i < len; ++i) {
        if ((quint8)data[i] == 0x0B && (quint8)data[i + 1] == 0xBB) {
            break;
        }
    }
    if (i != 0 && i < len) { // ƥ�䵽����
        array = array.mid(i); // ������������
    }
    // ����Ƿ���յ�һ��ͼ��
    if ((quint8)array.data()[0] == 0x0B
        && (quint8)array.data()[1] == 0xBB
        && array.size() >= 602) {
        QPixmap pixmap(80, 60);
        QPainter painter(&pixmap);
        
        pixmap.fill(Qt::white);
        data = array.data() + 2;
        for (int y = 0; y < 60; ++y) {
            for (int x = 0; x < 10; ++x) {
                quint8 byte = (quint8)*data++;
                for (i = 0; i < 8; ++i) {
                    if (byte & (1 << (7 - i))) {
                        painter.drawPoint(QPoint(x * 8 + i, y));
                    };
                }
            }
        }
        ui.label->setPixmap(pixmap.scaled(QSize(320, 240)));
        image = pixmap; // ����ͼ��
        memcpy(imageData, array.data() + 2, 600); // ���Ƶ�������
        array = array.mid(602); // ������������
    }
}

// ����ͼ��
void VedioBox::saveImage()
{
    QString str = QDateTime::currentDateTime().toString("yyMMddhhmmssz");
    image.save(filePath + "/" + str + ".bmp", "bmp");
}

void VedioBox::copyImage()
{
    QClipboard *board = QApplication::clipboard();
    QString str;
    char *data = imageData;

    str = "const unsigned char image_data[600] = {\n"
          "    ";
    for (int y = 0; y < 60; ++y) {
        for (int x = 0; x < 10; ++x) {
            str += QString::asprintf("0x%.2X", (quint8)*data++);
            if (x < 9) {
                str += ", ";
            }
        }
        if (y < 59) {
            str += ",\n    ";
        }
    }
    str += "\n};\n";
    board->setText(str);
}

// �����ļ�·��
void VedioBox::setFilePath(const QString &path)
{
    if (path.isEmpty()) {
        filePath = ".";
    } else {
        filePath = path;
    }
}
