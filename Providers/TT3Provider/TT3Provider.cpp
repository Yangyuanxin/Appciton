#include "TT3Provider.h"
#include "TEMPParam.h"
#include "Debug.h"
#include <QString>
#include "crc8.h"
#include "SystemManager.h"
#include "BLMEDUpgradeParam.h"
#include "ServiceVersion.h"
#include "ErrorLog.h"
#include "ErrorLogItem.h"
#include "RawDataCollection.h"
#include "IConfig.h"

static const char *tempErrorCode[] = {
    "Unknown mistake.\r\n",
    "The data saved in Flash is reset to the default value.\r\n",
    "\n",
    "\n",
    "\n",
    "\n",
    "\n",
    "\n",
    "Flash wrong.\r\n",
    "AD Sampling failed.\r\n",
    "Check channel of AD Sampling is overrange.\r\n",
    "Calibration is unsuccessful.\r\n",
};

/**************************************************************************************************
 * 模块与参数对接。
 *************************************************************************************************/
bool TT3Provider::attachParam(Param &param)
{
    if (param.getParamID() == PARAM_TEMP)
    {
        tempParam.setProvider(this);
        return true;
    }
    return false;
}

/**************************************************************************************************
 * 读取数据。
 *************************************************************************************************/
void TT3Provider::handlePacket(unsigned char *data, int len)
{
    BLMProvider::handlePacket(data, len);

    int enable = 0;
    switch(data[0])
    {
    // 自检状态
    case TT3_RSP_SELF_STATE:
        _selfTest(data,len);
        break;

    // 获取测量状态
    case TT3_RSP_PROBE_STATE:
        _probeState(data);
        break;

    // 通道校准
    case TT3_RSP_CHANNEL:
        tempParam.getCalibrateData(data);
        break;

    // 启动帧。
    case TT3_NOTIFY_START_UP:
    {
        _sendACK(data[0]);

        ErrorLogItem *item = new CriticalFaultLogItem();
        item->setName("TT3 Start");
        errorLog.append(item);

        tempParam.reset();
        break;
    }

    // 测量数据
    case TT3_NOTIFY_DATA:
        _sendACK(data[0]);
        _result(data);
        break;

    // 测量超界帧
    case TT3_NOTIFY_OVERRANGE:
        _sendACK(data[0]);
        _overRange(data);
        break;

    // 探头脱落帧
    case TT3_NOTIFY_PROBE_OFF:
        _sendACK(data[0]);
        _sensorOff(data);
        break;

    case TT3_CYCLE_ALIVE:
        feed();
        break;

    case TT3_CYCLE_DATA:
        machineConfig.getNumValue("Record|NIBP", enable);
        if (enable)
        {
            rawDataCollection.pushData("BLM_TT3", data,len);
        }
        break;

    case TT3_DATA_ERROR:
        _sendACK(data[0]);
        _errorWarm(data,len);
        break;

    default:
        break;
    }
}

/**************************************************************************************************
 * 与模块的通信中断。
 *************************************************************************************************/
void TT3Provider::disconnected(void)
{
    _disconnected = true;
    _shotAlarm();
    tempParam.setOneShotAlarm(TEMP_ONESHOT_ALARM_COMMUNICATION_STOP, true);
}

/**************************************************************************************************
 * 与模块的通信恢复。
 *************************************************************************************************/
void TT3Provider::reconnected(void)
{
    _disconnected = false;
    tempParam.setOneShotAlarm(TEMP_ONESHOT_ALARM_COMMUNICATION_STOP, false);
}

/**************************************************************************************************
 * ACK应答。
 *************************************************************************************************/
void TT3Provider::_sendACK(unsigned char type)
{
    unsigned char data = type & 0xFF;
    sendCmd(TT3_RESPONSE_ACK, &data, 1);
}

/**************************************************************************************************
 * 发送协议命令。
 *************************************************************************************************/
void TT3Provider::sendCmdData(unsigned char cmdId, const unsigned char *data, unsigned int len)
{
    sendCmd(cmdId, data, len);
}

/**************************************************************************************************
 * 获取版本号
 *************************************************************************************************/
void TT3Provider::sendVersion()
{
    sendCmd(TT3_CMD_GET_VERSION, NULL, 0);
}

/**************************************************************************************************
 * 发送自检指令。
 *************************************************************************************************/
void TT3Provider::sendTEMPSelfTest()
{
    sendCmd(TT3_CMD_SELF_STATE, NULL, 0);
}

/**************************************************************************************************
 * 获取测量状态
 *************************************************************************************************/
void TT3Provider::sendProbeState()
{
    sendCmd(TT3_CMD_PROBE_STATE, NULL, 0);
}

void TT3Provider::sendCalibrateData(int channel, int value)
{
    unsigned char cmd[2];

    cmd[0] = channel & 0xFF;
    cmd[1] = value & 0xFF;

    sendCmd(TT3_CMD_CHANNEL, cmd, 2);
}

/**************************************************************************************************
 * 接收自检状态。
 *************************************************************************************************/
void TT3Provider::_selfTest(unsigned char *packet, int len)
{
    int num = packet[1];
    if (num > 0)
    {
        systemManager.setPoweronTestResult(TT3_MODULE_SELFTEST_RESULT,SELFTEST_FAILED);
        tempParam.setErrorDisable();
        tempParam.setOneShotAlarm(TEMP_ONESHOT_ALARM_MODULE_DISABLE, true);

        QString errorStr("");
        errorStr = "error code = ";
        for(int i = 1; i < len; i++)
        {
            errorStr += QString().sprintf("0x%02x, ", packet[i]);
        }
        errorStr += "\r\n";

        for(int i = 2; i < len; i++)
        {
            switch (packet[i])
            {
            case 0x01:
            case 0x08:
            case 0x09:
            case 0x0A:
            case 0x0B:
                errorStr += tempErrorCode[packet[i]];
                break;
            default:
                errorStr += tempErrorCode[0];
                break;
            }
        }

        ErrorLogItem *item = new CriticalFaultLogItem();
        item->setName("TT3 Selftest Error");
        item->setLog(errorStr);
        item->setSubSystem(ErrorLogItem::SUB_SYS_TT3);
        item->setSystemState(ErrorLogItem::SYS_STAT_SELFTEST);
        item->setSystemResponse(ErrorLogItem::SYS_RSP_REPORT);
        errorLog.append(item);

        _shotAlarm();
    }
    else if (num == 0)
    {
        systemManager.setPoweronTestResult(TT3_MODULE_SELFTEST_RESULT,SELFTEST_SUCCESS);
    }
}

void TT3Provider::_errorWarm(unsigned char *packet, int len)
{
    tempParam.setErrorDisable();
    tempParam.setOneShotAlarm(TEMP_ONESHOT_ALARM_MODULE_DISABLE, true);

    QString errorStr("");
    errorStr = "error code = ";
    for(int i = 1; i < len; i++)
    {
        errorStr += QString().sprintf("0x%02x, ", packet[i]);
    }
    errorStr += "\r\n";

    for(int i = 1; i < len; i++)
    {
        switch (packet[i])
        {
        case 0x01:
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
            errorStr += tempErrorCode[packet[i]];
            break;
        default:
            errorStr += tempErrorCode[0];
            break;
        }
    }

    ErrorLogItem *item = new CriticalFaultLogItem();
    item->setName("TT3 Error");
    item->setLog(errorStr);
    item->setSubSystem(ErrorLogItem::SUB_SYS_TT3);
    item->setSystemState(ErrorLogItem::SYS_STAT_RUNTIME);
    item->setSystemResponse(ErrorLogItem::SYS_RSP_REPORT);
    errorLog.append(item);

    _shotAlarm();
}

/**************************************************************************************************
 * 接收测量状态。
 *************************************************************************************************/
void TT3Provider::_probeState(unsigned char *packet)
{
    if (packet[1] & 0x01)
    {
        _overRang1 = true;
    }
    else
    {
        _overRang1 = false;
    }
    if (packet[1] & 0x10)
    {
        _sensorOff1 = true;
        _overRang1 = false;
    }
    else
    {
        _sensorOff1 = false;
    }

    if (packet[2] & 0x01)
    {
        _overRang2 = true;
    }
    else
    {
        _overRang2 = false;
    }
    if (packet[2] & 0x10)
    {
        _sensorOff2 = true;
        _overRang2 = false;
    }
    else
    {
        _sensorOff2 = false;
    }

    _shotAlarm();
}

/**************************************************************************************************
 * 体温值。
 *************************************************************************************************/
void TT3Provider::_result(unsigned char *packet)
{
    _temp1 = InvData();
    _temp2 = InvData();
    _tempd = InvData();

    bool sensorOff1 = _sensorOff1;
    bool sensorOff2 = _sensorOff2;

    if (!(0xFF == packet[2] && 0xFF == packet[1]))
    {
        _temp1 = (int)((packet[2]<<8) + packet[1]);

        sensorOff1 = false;
    }

    if (!(0xFF == packet[4] && 0xFF == packet[3]))
    {
        _temp2 = (int)((packet[4]<<8) + packet[3]);

        sensorOff2 = false;
    }

//    if (_temp1 != InvData() && _temp2 != InvData())
    if (_temp1 >=0 && _temp1 <= 500 && _temp2 >= 0 && _temp2 <= 500)
    {
        _tempd = abs(_temp1 - _temp2);
    }
    else
    {
        _tempd = InvData();
    }

    if (sensorOff1 != _sensorOff1 || sensorOff2 != _sensorOff2)
    {
        _sensorOff1 = sensorOff1;
        _sensorOff2 = sensorOff2;

        _shotAlarm();
    }
    else
    {
        tempParam.setTEMP(_temp1, _temp2, _tempd);
    }
}

/**************************************************************************************************
 * 超界。
 *************************************************************************************************/
void TT3Provider::_overRange(unsigned char *packet)
{
    if (packet[1] == 0x00)
    {
        _overRang1 = false;
        _overRang2 = false;
    }
    else if (packet[1] == 0x01)
    {
        _overRang1 = true;
        _overRang2 = false;
    }
    else if (packet[1] == 0x02)
    {
        _overRang1 = false;
        _overRang2 = true;
    }
    else if (packet[1] == 0x03)
    {
        _overRang1 = true;
        _overRang2 = true;
    }
    _shotAlarm();
}

/**************************************************************************************************
 * 探头脱落。
 *************************************************************************************************/
void TT3Provider::_sensorOff(unsigned char *packet)
{
    if (packet[1] == 0x00)
    {
        _sensorOff1 = false;
        _sensorOff2 = false;
    }
    else if (packet[1] == 0x01)
    {
        _sensorOff1 = true;
        _sensorOff2 = false;

        _overRang1 = false;
    }
    else if (packet[1] == 0x02)
    {
        _sensorOff1 = false;
        _sensorOff2 = true;

        _overRang2 = false;
    }
    else if (packet[1] == 0x03)
    {
        _sensorOff1 = true;
        _sensorOff2 = true;

        _overRang1 = false;
        _overRang2 = false;
    }
    _shotAlarm();
}

/**************************************************************************************************
 * 报警显示。
 *************************************************************************************************/
void TT3Provider::_shotAlarm()
{
    // 自检失败
    if (_disconnected)
    {
        tempParam.setTEMP(InvData(), InvData(), InvData());
        return;
    }

    // 探头全部脱落报警，取消探头全部超界，
    if (_sensorOff1 && _sensorOff2)
    {
        _temp1 = InvData();
        _temp2 = InvData();
        _tempd = InvData();

        // 取消探头全部超界
        tempParam.setOneShotAlarm(TEMP_OVER_RANGR_ALL, false);
        tempParam.setOneShotAlarm(TEMP_SENSOR_OFF_1, false);
        tempParam.setOneShotAlarm(TEMP_SENSOR_OFF_2, false);
        tempParam.setOneShotAlarm(TEMP_OVER_RANGR_1, false);
        tempParam.setOneShotAlarm(TEMP_OVER_RANGR_2, false);

        // 探头全部脱落报警
        tempParam.setOneShotAlarm(TEMP_SENSOR_OFF_ALL, true);
    }
    else
    {
        // 取消探头全部脱落
        tempParam.setOneShotAlarm(TEMP_SENSOR_OFF_ALL, false);

        // 探头未脱落，探头全部超界
        if (!_sensorOff1 && !_sensorOff2)
        {
            // 取消探头1、探头2脱落报警
            tempParam.setOneShotAlarm(TEMP_SENSOR_OFF_1, false);
            tempParam.setOneShotAlarm(TEMP_SENSOR_OFF_2, false);

            if (_overRang1 && _overRang2)
            {
                tempParam.setOneShotAlarm(TEMP_OVER_RANGR_ALL, true);
                // 取消探头1、探头2超界
                tempParam.setOneShotAlarm(TEMP_OVER_RANGR_1, false);
                tempParam.setOneShotAlarm(TEMP_OVER_RANGR_2, false);


            }
            else
            {
                tempParam.setOneShotAlarm(TEMP_OVER_RANGR_ALL, false);
                if (_overRang1)
                {
                    tempParam.setOneShotAlarm(TEMP_OVER_RANGR_1, true);
                }
                else
                {
                    tempParam.setOneShotAlarm(TEMP_OVER_RANGR_1, false);
                }

                if (_overRang2)
                {
                    tempParam.setOneShotAlarm(TEMP_OVER_RANGR_2, true);
                }
                else
                {
                    tempParam.setOneShotAlarm(TEMP_OVER_RANGR_2, false);
                }
            }
        }
        // 单个探头脱落
        else
        {
            // 取消探头全部超界
            tempParam.setOneShotAlarm(TEMP_OVER_RANGR_ALL, false);
            // 探头1脱落报警，取消探头全部脱落，取消探头全部超界，取消探头1超界
            if (_sensorOff1)
            {
                _temp1 = InvData();
                // 取消探头1超界
                tempParam.setOneShotAlarm(TEMP_OVER_RANGR_1, false);
                // 探头1脱落报警
                tempParam.setOneShotAlarm(TEMP_SENSOR_OFF_1, true);
            }
            // 取消探头全部脱落，取消探头1脱落报警，取消探头1脱落报警，判断探头1是否超界
            else
            {
                // 取消探头1脱落报警
                tempParam.setOneShotAlarm(TEMP_SENSOR_OFF_1, false);

                // 判断探头1是否超界
                if (_overRang1)
                {
                    // 探头1超界报警
                    tempParam.setOneShotAlarm(TEMP_OVER_RANGR_1, true);
                }
                else
                {
                    // 取消探头1超界报警
                    tempParam.setOneShotAlarm(TEMP_OVER_RANGR_1, false);
                }
            }

            // 探头2脱落报警，取消探头全部超界，取消探头2超界
            if (_sensorOff2)
            {
                // 取消探头2超界
                tempParam.setOneShotAlarm(TEMP_OVER_RANGR_2, false);
                // 探头2脱落报警
                tempParam.setOneShotAlarm(TEMP_SENSOR_OFF_2, true);
            }
            // 取消探头全部脱落，取消探头2脱落报警，取消探头2脱落报警，判断探头2是否超界
            else
            {
                // 取消探头2脱落报警
                tempParam.setOneShotAlarm(TEMP_SENSOR_OFF_2, false);

                // 判断探头2是否超界
                if (_overRang2)
                {
                    // 探头2超界报警
                    tempParam.setOneShotAlarm(TEMP_OVER_RANGR_2, true);
                }
                else
                {
                    // 取消探头2超界报警
                    tempParam.setOneShotAlarm(TEMP_OVER_RANGR_2, false);
                }
            }
        }
    }
    if (_temp1 == InvData() || _temp2 == InvData())
    {
        _tempd = InvData();
    }
    tempParam.setTEMP(_temp1, _temp2, _tempd);
}

/**************************************************************************************************
 * 构造。
 *************************************************************************************************/
TT3Provider::TT3Provider() : BLMProvider("BLM_TT3"), TEMPProviderIFace()
{
    UartAttrDesc portAttr(115200, 8, 'N', 1);
    if (!initPort(portAttr))
    {
        systemManager.setPoweronTestResult(TT3_MODULE_SELFTEST_RESULT, SELFTEST_FAILED);
    }

    setDisconnectThreshold(5);

    _disconnected = false;
    _overRang1 = false;
    _overRang2 = false;
    _sensorOff1 = false;
    _sensorOff2 = false;

    _temp1 = InvData();
    _temp2 = InvData();
    _tempd = InvData();
}

/**************************************************************************************************
 * 析构。
 *************************************************************************************************/
TT3Provider::~TT3Provider()
{

}

