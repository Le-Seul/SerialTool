#include "wavedecode.h"

enum {
    Frame_Head = 0xA3,          //֡ͷʶ����
    Frame_PointMode = 0xA8,     // ��ģʽʶ����
    Frame_SyncMode = 0xA9,      // ͬ��ģʽʶ����
    Frame_InfoMode  = 0xAA      // ��Ϣ֡ʶ����
};

enum {
    STA_None = 0, // ����״̬
    STA_Head,     // ���յ�֡ͷ
    STA_Point,    // ��ģʽ
    STA_Sync,     // ͬ��ģʽ
    STA_Info,     // ��Ϣģʽ
    STA_SyncData  // ͬ��ģʽ����
};

enum Result {
    Ok = 0,
    Error,
    Done,
};

WaveDecode::WaveDecode()
{
    status = STA_None;
    dataCount = 0;
    frameCount = 0;
    dataLength = 0;
    frameLength = 0;
}

double WaveDecode::data2Double(uint32_t value, int type)
{
    double d;
    union {
        uint32_t i;
        float f;
    } temp;

    switch (type) {
    case 0: // float
        temp.i = value;
        d = temp.f;
        break;
    case 1: // int8
        d = (int8_t)value;
        break;
    case 2: // int16
        d = (int16_t)value;
        break;
    case 3: // int32
        d = (int32_t)value;
        break;
    default:
        d = 0.0;
    }
    return d;
}

// ����һ��������, ����������
int WaveDecode::pointData(WaveDataType &dst, uint8_t byte)
{
    static const int bytes[] = { 4, 1, 2, 4 }; // �������͵��ֽ���

    if (dataCount == 0) { // ��һ���ֽ����������ͺ�ͨ����Ϣ
        channel = byte & 0x0F; // ͨ��ֵ
        // tpye: 0: float, 1: int8, 2: int16, 3: int32
        type = byte >> 4;
        if (type > 3) { // �������ʹ���
            dataCount = 0;
            return Error;
        }
        dataLength = bytes[type];
    } else { // ���漸���ֽ�������
        data = (data << 8) | byte;
        if (dataCount >= dataLength) { // �������
            dst.channel = channel;
            dst.mode = WaveValueMode;
            dst.value = data2Double(data, type);
            dataCount = 0;
            data = 0;
            return Done;
        }
    }
    ++dataCount;
    return Ok;
}

// ת��ʱ���
void WaveDecode::timeStamp(WaveDataType &dst, uint8_t* buffer)
{
    dst.mode = WaveTimeStampMode;
    dst.year = (buffer[0] >> 1) & 0x7F;
    dst.month = ((buffer[0] << 3) & 0x80) | ((buffer[1] >> 5) & 0x07);
    dst.day = buffer[1] & 0x1F;
    dst.hour = (buffer[2] >> 3) & 0x1F;
    dst.min = ((buffer[2] << 3) & 0x38) | ((buffer[3] >> 5) & 0x07);
    dst.sec = ((buffer[3] << 1) & 0x3E) | ((buffer[4] >> 7) & 0x01);
    dst.msec = (((uint16_t)buffer[4] << 3) & 0x03F8) | (((uint16_t)buffer[5] >> 5) & 0x0007);
    dst.sampleRate = (((uint32_t)buffer[5] << 16) & 0x1F0000)
        | (((uint32_t)buffer[6] << 8) & 0x00FF00) | (uint32_t)buffer[7];
}

// ��������֡����, ��ʶ��֡ͷ
bool WaveDecode::frameDecode(WaveDataType &data, uint8_t byte)
{
    int res;

    // ����֡ͷ״̬��
    switch (status) {
    case STA_None:
        status = byte == Frame_Head ? STA_Head : STA_None;
        break;
    case STA_Head:
        /* byte == Frame_PointMode -> status = STA_Point
         * byte == Frame_SyncMode -> status = STA_Sync
         * byte == Frame_InfoMode -> status = STA_Info
         * else -> status = STA_None
         */
        switch (byte) {
        case Frame_PointMode:
            status = STA_Point;
            break;
        case Frame_SyncMode:
            status = STA_Sync;
            break;
        case Frame_InfoMode:
            status = STA_Info;
            frameCount = 0;
            break;
        default:
            status = STA_None;
            break;
        }

        break;
    case STA_Point:
        res = pointData(data, byte);
        switch (res) {
        case Ok: // ���ڽ�������
            break;
        case Error: // ���������¿�ʼ����
            status = STA_None;
            break;
        case Done: // ������ʼ��״̬������true
            status = STA_None;
            return true;
        }
        break;
    case STA_Sync:
        frameCount = 0;
        frameLength = byte;
        // ���len > 64��֡���ȴ���, ������ƥ��֡, ����ת��STA_SyncData״̬
        status = frameLength <= 64 ? STA_SyncData : STA_None;
        break;
    case STA_SyncData:
        if (++frameCount >= frameLength) { // �����ﵽ֡����˵��֡����, ����״̬
            status = STA_None;
        }
        res = pointData(data, byte);
        switch (res) {
        case Ok: // ���ڽ�������
            break;
        case Error: // ���������¿�ʼ����
            status = STA_None;
            break;
        case Done: // ��������true
            return true;
        }
        break;
    case STA_Info:
        infoFrame[frameCount++] = byte;
        if (frameCount >= 8) {
            timeStamp(data, infoFrame);
            frameCount = 0;
            status = STA_None;
            return true;
        }
        break;
    default: // �쳣�����λ״̬
        status = STA_None;
    }
    return false;
}
