#include "oscilloscope.h"
#include <QTextStream>

#define SCALE   (1000.0 / _xRange)

Oscilloscope::Oscilloscope(QWidget *parent)
{
    ui.setupUi(parent);

    setupPlot();
    setupChannel();
    listViewInit();

    QRegExp rx("^(-?[0]|-?[1-9][0-9]{0,5})(?:\\.\\d{1,4})?$|(^\\t?$)");
    QRegExpValidator *pReg = new QRegExpValidator(rx, this);
    ui.xRangeBox->lineEdit()->setValidator(pReg);

    updataTimer.setInterval(10);

    connect(ui.customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));
    connect(ui.customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(yAxisChanged(QCPRange)));
    connect(ui.horizontalScrollBar, &QAbstractSlider::valueChanged, this, &Oscilloscope::horzScrollBarChanged);
    connect(ui.yOffsetBox, static_cast<void (QDoubleSpinBox::*)(double)>
        (&QDoubleSpinBox::valueChanged), this, &Oscilloscope::yOffsetChanged);
    connect(ui.yRangeBox, static_cast<void (QDoubleSpinBox::*)(double)>
        (&QDoubleSpinBox::valueChanged), this, &Oscilloscope::yRangeChanged);
    connect(ui.xRangeBox, &QComboBox::currentTextChanged, this, &Oscilloscope::xRangeChanged);
    connect(&updataTimer, &QTimer::timeout, this, &Oscilloscope::timeUpdata);

    setXRange(10);
    clear();
}

// ������������
void Oscilloscope::retranslate()
{
    ui.retranslateUi(this);
}


// ��ʼ��ʾ��������
void Oscilloscope::setupPlot()
{
    // �϶�ʱ�������
    ui.customPlot->setNoAntialiasingOnDrag(true);

    // ���ÿ̶���
    QSharedPointer<	QCPAxisTicker> xTicker(new QCPAxisTicker);
    QSharedPointer<	QCPAxisTicker> yTicker(new QCPAxisTicker);
    xTicker->setTickCount(5);
    ui.customPlot->xAxis->setTicker(xTicker);
    ui.customPlot->xAxis2->setTicker(xTicker);
    yTicker->setTickCount(5);
    ui.customPlot->yAxis->setTicker(yTicker);
    ui.customPlot->yAxis2->setTicker(yTicker);
    // ��ʾС����
    ui.customPlot->xAxis->grid()->setSubGridVisible(true);
    ui.customPlot->yAxis->grid()->setSubGridVisible(true);
}

// ��ʼ��ͨ��
void Oscilloscope::setupChannel()
{
    // ��ʼ��ͨ��
    for (int i = 0; i < CH_NUM; ++i) {
        count[i] = 0.0;
        ui.customPlot->addGraph();
    }

    ui.customPlot->axisRect()->setupFullAxesBox(true);
    ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom); // ������ק������
    ui.customPlot->yAxis->setRange(0, 2, Qt::AlignCenter);
}

// ͨ���б��ʼ��
void Oscilloscope::listViewInit()
{
    ui.channelList->setModelColumn(2); // ����
    for (int i = 0; i < CH_NUM; ++i) {
        QListWidgetItem *item = new QListWidgetItem;
        ui.channelList->addItem(item);
        ChannelItem *chItem = new ChannelItem("CH" + QString::number(i + 1));
        ui.channelList->setItemWidget(item, chItem);
        chItem->setChannel(i);
        channelStyleChanged(chItem);
        connect(chItem, &ChannelItem::changelChanged, this, &Oscilloscope::channelStyleChanged);
    }
    ui.channelList->editItem(ui.channelList->item(0));
}

// ����Y��ƫ��
double Oscilloscope::yOffset()
{
    return ui.yOffsetBox->value();
}

// ����Y�᷶Χ
double Oscilloscope::yRange()
{
    return ui.yRangeBox->value();
}

// ����X�᷶Χ
double Oscilloscope::xRange()
{
    return ui.customPlot->xAxis->range().size();
}

// ����Y��ƫ��
void Oscilloscope::setYOffset(double offset)
{
    ui.yOffsetBox->setValue(offset);
}

// ����Y�᷶Χ
void Oscilloscope::setYRange(double range)
{
    ui.yRangeBox->setValue(range);
}

// ����ͨ���Ƿ�ɼ�
bool Oscilloscope::channelVisible(int channel)
{
    ChannelItem *item = (ChannelItem *)(
        ui.channelList->itemWidget(ui.channelList->item(channel)));

    return item->isChecked();
}

// ����ͨ����ɫ
QColor Oscilloscope::channelColor(int channel)
{
    ChannelItem *item = (ChannelItem *)(
        ui.channelList->itemWidget(ui.channelList->item(channel)));

    return item->color();
}

// ��ʼ����
void Oscilloscope::start()
{
    updataTimer.start();
}

// ��������
void Oscilloscope::stop()
{
    updataTimer.stop();
}

// ���ر��ֽ���״̬
bool Oscilloscope::holdReceive()
{
    return ui.holdReceiveBox->isChecked();
}

// ����X�����
void Oscilloscope::setXRange(double range)
{
    setXRange(QString::number(range));
}

// ����X�����
void Oscilloscope::setXRange(const QString &str)
{
    _xRange = str.toDouble();
    ui.xRangeBox->setCurrentText(str);
}

// ���ò��λ��ƿ����
void Oscilloscope::setPlotAntialiased(bool status)
{
    ui.customPlot->setNotAntialiasedElement(QCP::aePlottables, !status);
}

// �������񿹾��
void Oscilloscope::setGridAntialiased(bool status)
{
    ui.customPlot->setAntialiasedElement(QCP::aeGrid, status);
    ui.customPlot->setAntialiasedElement(QCP::aeAxes, status);
}

// ���ñ�����ɫ
void Oscilloscope::setBackground(QColor color)
{
    ui.customPlot->setBackground(QBrush(color));
}

// ���������ͼ����ɫ
void Oscilloscope::setGridColor(QColor color)
{
    QPen pen(color);

    ui.customPlot->xAxis->setBasePen(pen);
    ui.customPlot->xAxis->setTickPen(pen);
    ui.customPlot->xAxis->grid()->setPen(pen);
    ui.customPlot->xAxis->setSubTickPen(pen);
    ui.customPlot->xAxis->setTickLabelColor(color);
    ui.customPlot->xAxis->setLabelColor(color);
    ui.customPlot->xAxis->grid()->setZeroLinePen(pen); // ��㻭��
    ui.customPlot->yAxis->setBasePen(pen);
    ui.customPlot->yAxis->setTickPen(pen);
    ui.customPlot->yAxis->grid()->setPen(pen);
    ui.customPlot->yAxis->setSubTickPen(pen);
    ui.customPlot->yAxis->setTickLabelColor(color);
    ui.customPlot->yAxis->setLabelColor(color);
    ui.customPlot->yAxis->grid()->setZeroLinePen(pen); // ��㻭��
    ui.customPlot->xAxis2->setBasePen(pen);
    ui.customPlot->xAxis2->setTickPen(pen);
    ui.customPlot->xAxis2->setSubTickPen(pen);
    ui.customPlot->yAxis2->setBasePen(pen);
    ui.customPlot->yAxis2->setTickPen(pen);
    ui.customPlot->yAxis2->setSubTickPen(pen);

    // ������ɫ
    color.setAlpha(0x30);
    pen.setColor(color);
    ui.customPlot->xAxis->grid()->setSubGridPen(pen);
    ui.customPlot->yAxis->grid()->setSubGridPen(pen);
}

// ����ͨ����ɫ
void Oscilloscope::setChannelColor(int chanel, const QColor &color)
{
    ChannelItem *item = (ChannelItem *)(
        ui.channelList->itemWidget(ui.channelList->item(chanel)));
    item->setColor(color);
    ui.customPlot->graph(chanel)->setPen(QPen(item->color()));
}

// ����ͨ���Ƿ�ɼ�
void Oscilloscope::setChannelVisible(int chanel, bool visible)
{
    ChannelItem *item = (ChannelItem *)(
        ui.channelList->itemWidget(ui.channelList->item(chanel)));
    item->setChecked(visible);
    ui.customPlot->graph(chanel)->setVisible(visible);
}

// �������
void Oscilloscope::addData(int channel, double value)
{
    ui.customPlot->graph(channel)->addData(count[channel], value);
    count[channel] += 1.0;
}

// �������
void Oscilloscope::clear()
{
    for (int i = 0; i < CH_NUM; ++i) {
        ui.customPlot->graph(i)->data()->clear();
        count[i] = 0.0;
    }
    ui.horizontalScrollBar->setValue(0);
    ui.horizontalScrollBar->setRange(0, 0);
    ui.customPlot->xAxis->setRange(0, _xRange, Qt::AlignLeft);
    ui.customPlot->replot();
}

// ����PNG�ļ�
void Oscilloscope::savePng(const QString &fileName)
{
    ui.customPlot->savePng(fileName);
}

// ����BMP�ļ�
void Oscilloscope::saveBmp(const QString &fileName)
{
    ui.customPlot->saveBmp(fileName);
}

// ����PDF�ļ�
void Oscilloscope::savePdf(const QString &fileName)
{
    ui.customPlot->savePdf(fileName);
}

// ͨ����ʾ���ı�
void Oscilloscope::channelStyleChanged(ChannelItem *item)
{
    int ch = item->channel();
    ui.customPlot->graph(ch)->setVisible(item->isChecked());
    ui.customPlot->graph(ch)->setPen(QPen(item->color()));
    ui.customPlot->replot();
}

// x�ᷢ���仯
void Oscilloscope::xAxisChanged(QCPRange range)
{
    // ����x�᷶Χ
    if (_xRange != range.size()) {
        _xRange = range.size();
        ui.xRangeBox->setEditText(QString::number(_xRange));
    }
    // ���ù�����
    if (qAbs(range.lower * SCALE - ui.horizontalScrollBar->value()) > 1.0 / SCALE) {
        int key = qRound(range.lower * SCALE);
        ui.horizontalScrollBar->setValue(key);
        replotFlag = false;
    }
}

// y�ᷢ���仯
void Oscilloscope::yAxisChanged(QCPRange range)
{
    // ֻ�е��û���קcustomPlot�ؼ�ʱ�Ż�����ƫ��ֵ
    if (range.center() != ui.yOffsetBox->value()) {
        ui.yOffsetBox->setValue(range.center());
    }
    // ֻ�е��û���קcustomPlot�ؼ�ʱ�Ż�����ƫ��ֵ
    if (range.size() != ui.yRangeBox->value()) {
        ui.yRangeBox->setValue(range.size());
    }
}

// �����������ƶ�ʱ����
void Oscilloscope::horzScrollBarChanged(int value)
{
    if (ui.horizontalScrollBar->maximum() == value) {
        replotFlag = true;
    } else {
        replotFlag = false;
    }
    if (ui.customPlot->xAxis->range().lower < value / SCALE || ui.horizontalScrollBar->maximum() != ui.horizontalScrollBar->value()) {
        ui.customPlot->xAxis->setRange(value / SCALE,
            _xRange, Qt::AlignLeft);
    }
}

// Y��ƫ�øı�
void Oscilloscope::yOffsetChanged(double offset)
{
    double range = ui.customPlot->yAxis->range().size();
    ui.customPlot->yAxis->setRange(offset, range, Qt::AlignCenter);
    ui.customPlot->replot();
}

// Y�᷶Χ�ı�
void Oscilloscope::yRangeChanged(double range)
{
    double offset = ui.customPlot->yAxis->range().center();
    ui.customPlot->yAxis->setRange(offset, range, Qt::AlignCenter);
    ui.customPlot->replot();
}

// X�᷶Χ�ı�
void Oscilloscope::xRangeChanged(const QString &str)
{
    double upper = ui.customPlot->xAxis->range().upper;

    _xRange = str.toDouble();
    if (upper < _xRange) {
        ui.horizontalScrollBar->setRange(0, 0);
        ui.horizontalScrollBar->setValue(0);
        ui.customPlot->xAxis->setRange(0, _xRange);
    } else {
        ui.horizontalScrollBar->setRange(0, (int)((upper - _xRange) * SCALE));
        ui.horizontalScrollBar->setValue((int)((upper - _xRange) * SCALE));
        ui.customPlot->xAxis->setRange(upper, _xRange, Qt::AlignRight);
    }
    ui.horizontalScrollBar->setPageStep(_xRange * SCALE);
    ui.customPlot->replot();
}

// ���¶�ʱ������
void Oscilloscope::timeUpdata()
{
    // ��ʾ����
    double key = count[0];
    for (int i = 0; i < CH_NUM; ++i) {
        key = key > count[i] ? key : count[i];
    }
    if (key > _xRange) {
        ui.horizontalScrollBar->setRange(0, (int)((key - _xRange) * SCALE));
        if (replotFlag || key <= ui.customPlot->xAxis->range().upper) {
            ui.horizontalScrollBar->setValue((int)((key - _xRange) * SCALE));
        }
    }
    ui.customPlot->replot();
}

// ����txt�ļ�
void Oscilloscope::saveText(const QString &fname)
{
    QFile file(fname);
    int graphCount = ui.customPlot->graphCount();
    int dataCount[CH_NUM], dataCountMax = -1;

    file.open(QFile::WriteOnly);
    QTextStream out(&file);
    for (int i = 0; i < CH_NUM; ++i) {
        dataCount[i] = ui.customPlot->graph(i)->dataCount();
        if (dataCount[i] > dataCountMax) {
            dataCountMax = dataCount[i];
        }
    }
    out.setRealNumberPrecision(8);
    for (int i = 0; i < dataCountMax; ++i) {
        out << i << ", ";
        for (int j = 0; j < graphCount; ++j) {
            if (i < dataCount[j]) {
                double value = ui.customPlot->graph(j)->dataMainValue(i);
                out << value << ", ";
            }
        }
        out << endl;
    }
    file.close();
}
