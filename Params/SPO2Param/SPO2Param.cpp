/**
 ** This file is part of the nPM project.
 ** Copyright (C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by Bingyun Chen <chenbingyun@blmed.cn>, 2018/8/27
 **/

#include "SPO2Param.h"
#include "SPO2Alarm.h"
#include "SPO2TrendWidget.h"
#include "SPO2WaveWidget.h"
#include "SPO2ProviderIFace.h"
#include "IConfig.h"
#include "ConfigManager.h"
#include "WaveformCache.h"
#include "ECGDupParam.h"
#include "SystemManager.h"
#include "Framework/ErrorLog/ErrorLog.h"
#include "Framework/ErrorLog/ErrorLogItem.h"
#include "Framework/TimeDate/TimeDate.h"
#include <QTimer>
#include "OxyCRGSPO2TrendWidget.h"
#include "NIBPParam.h"
#include "AlarmSourceManager.h"
#include "O2ParamInterface.h"
#include "RunningStatusBar.h"
#include "UpgradeManager.h"
#include "TEMPParam.h"
#include "PITrendWidget.h"
#include "PVITrendWidget.h"
#include "SPHBTrendWidget.h"
#include "SPOCTrendWidget.h"
#include "SPMETTrendWidget.h"
#include "SPCOTrendWidget.h"
#include "LayoutManager.h"
#include "MessageBox.h"
#include "WindowManager.h"
#include <QPointer>
#include "Components/DataUploader/BLMMessageDefine.h"

class SPO2ParamPrivate
{
public:
    SPO2ParamPrivate();
    ~SPO2ParamPrivate(){}

    void setWaveformSpeed(SPO2WaveVelocity speed, bool isPlugin = false);

    SPO2ProviderIFace *provider;
    SPO2ProviderIFace *plugInProvider;
    SPO2TrendWidget *trendWidget;
    SPO2WaveWidget *waveWidget;
    SPO2WaveWidget *plugInWaveWidget;
    PITrendWidget *piTrendWidget;
    PVITrendWidget *pviTrendWidget;
    SPHBTrendWidget *sphbTrendWidget;
    SPOCTrendWidget *spocTrendWidget;
    SPMETTrendWidget *spmetTrendWidget;
    SPCOTrendWidget *spcoTrendWidget;

    bool isEverCheckFinger;  // use to decide prompt sensor off
    bool plugInIsEverCheckFinger;  // use to decide prompt sensor off

    short spo2Value;
    short plugInSpo2Value;
    short spo2DValue;
    short prValue;
    short barValue;
    short piValue;
    short pluginPIValue;
    short pviValue;
    short sphbValue;
    short spocValue;
    short spmetValue;
    short spcoValue;

    bool isValid;
    bool plugInIsValid;

    int recPackageInPowerOn2sec;  // if receve 5 packages, selftest success, or selftest failed

    OxyCRGSPO2TrendWidget *oxyCRGSPO2Trend;
    bool connectedProvider;
    bool connectedPluginProvider;
    SPO2ModuleType moduleType;
    SPO2RainbowSensor sensorType;   // spo2 rainbow sensor type

    QList<cchdData> cchdDataList;
    int repeatTimes;

    bool isLowPerfusion;
    bool isForceUpdatingPR;      // When the low perfusion state of SpO2 changed, we forced to update the PR value.
    bool isForceUpdatingSPO2;    // When the low perfusion state of SpO2 changed, we forced to update the SpO2 value.
    bool plugInIsLowPerfusion;
    bool plugInIsForceUpdating;  // 当spo2的弱灌注状态发生变化时，该状态位为true

    bool isT5ModuleUpgradeCompleted;
    bool isShowSignalIQ;

    QPointer<MessageBox> curProgramMsgBox;    /* keep the current avaliabe message box window */
    bool bPlugConnected;
};

void SPO2Param::setAverageTime(AverageTime index)
{
    currentConfig.setNumValue("SPO2|AverageTime", static_cast<int>(index));
    if (NULL != d_ptr->provider)
    {
        d_ptr->provider->setAverageTime(index);
    }
    if (NULL != d_ptr->plugInProvider)
    {
        d_ptr->plugInProvider->setAverageTime(index);
    }
}

AverageTime SPO2Param::getAverageTime()
{
    int time = SPO2_AVER_TIME_8SEC;
    currentConfig.getNumValue("SPO2|AverageTime", time);
    return (AverageTime)time;
}

void SPO2Param::enableRawDataSend(bool onOff)
{
    if (d_ptr->provider != NULL)
    {
        d_ptr->provider->enableRawDataSend(onOff);
    }
}

/**************************************************************************************************
 * 设置波形速度。
 *************************************************************************************************/
SPO2ParamPrivate::SPO2ParamPrivate()
    : provider(NULL)
    , plugInProvider(NULL)
    , trendWidget(NULL)
    , waveWidget(NULL)
    , plugInWaveWidget(NULL)
    , piTrendWidget(NULL)
    , pviTrendWidget(NULL)
    , sphbTrendWidget(NULL)
    , spocTrendWidget(NULL)
    , spmetTrendWidget(NULL)
    , spcoTrendWidget(NULL)
    , isEverCheckFinger(false)
    , plugInIsEverCheckFinger(false)
    , spo2Value(InvData())
    , plugInSpo2Value(InvData())
    , spo2DValue(InvData())
    , prValue(InvData())
    , barValue(InvData())
    , piValue(InvData())
    , pluginPIValue(InvData())
    , pviValue(InvData())
    , sphbValue(InvData())
    , spocValue(InvData())
    , spmetValue(InvData())
    , spcoValue(InvData())
    , isValid(false)
    , plugInIsValid(false)
    , recPackageInPowerOn2sec(0)
    , oxyCRGSPO2Trend(NULL)
    , connectedProvider(false)
    , connectedPluginProvider(false)
    , moduleType(MODULE_SPO2_NR)
    , sensorType(SPO2_RAINBOW_SENSOR_M_LNCS)
    , repeatTimes(0)
    , isLowPerfusion(false)
    , isForceUpdatingPR(false)
    , isForceUpdatingSPO2(false)
    , plugInIsLowPerfusion(false)
    , plugInIsForceUpdating(false)
    , isT5ModuleUpgradeCompleted(false)
    , isShowSignalIQ(true)
    , bPlugConnected(false)
{
}

void SPO2ParamPrivate::setWaveformSpeed(SPO2WaveVelocity speed, bool isPlugin)
{
    if (waveWidget == NULL && plugInWaveWidget == NULL)
    {
        return;
    }

    SPO2WaveWidget *w = NULL;

    if (!isPlugin)
    {
        w = waveWidget;
    }
    else
    {
        w = plugInWaveWidget;
    }

    switch (speed)
    {
    case SPO2_WAVE_VELOCITY_62D5:
        w->setWaveSpeed(6.25);
        break;

    case SPO2_WAVE_VELOCITY_125:
        w->setWaveSpeed(12.5);
        break;

    case SPO2_WAVE_VELOCITY_250:
        w->setWaveSpeed(25.0);
        break;

    case SPO2_WAVE_VELOCITY_500:
        w->setWaveSpeed(50.0);
        break;

    default:
        break;
    }
}

/**************************************************************************************************
 * 初始化参数。
 *************************************************************************************************/
void SPO2Param::initParam(void)
{
}

/**************************************************************************************************
 * 处理DEMO数据。
 *************************************************************************************************/
void SPO2Param::handDemoWaveform(WaveformID id, short data)
{
    if (id != WAVE_SPO2 && id != WAVE_SPO2_2)
    {
        return;
    }

    unsigned char waveFlag = 0;
    if (data >= 93)
    {
        waveFlag = 255;
        waveFlag  = waveFlag | SPO2_IQ_FLAG_BIT;
    }

    if (id == WAVE_SPO2)
    {
        addWaveformData(data, waveFlag);
    }
    else if (id == WAVE_SPO2_2)
    {
        addWaveformData(data, waveFlag, true);
    }
}

/**************************************************************************************************
 * 处理DEMO数据。
 *************************************************************************************************/
void SPO2Param::handDemoTrendData(void)
{
    d_ptr->spo2Value = 98;
    if (isConnected(true))
    {
        d_ptr->plugInSpo2Value = 96;
        d_ptr->spo2DValue = abs(d_ptr->spo2Value - d_ptr->plugInSpo2Value);
    }
    else
    {
        d_ptr->plugInSpo2Value = InvData();
        d_ptr->spo2DValue = InvData();
    }
    d_ptr->piValue = 210;
    d_ptr->pluginPIValue = 210;
    d_ptr->pviValue = 23;
    d_ptr->sphbValue = 180;
    d_ptr->spocValue = 20;
    d_ptr->spmetValue = 3;
    d_ptr->spcoValue = 20;
    if (NULL != d_ptr->trendWidget)
    {
        d_ptr->trendWidget->setSPO2Value(d_ptr->spo2Value);
        d_ptr->trendWidget->setPluginSPO2Value(d_ptr->plugInSpo2Value);
        d_ptr->trendWidget->setSPO2DeltaValue(d_ptr->spo2DValue);
        d_ptr->trendWidget->setPIValue(d_ptr->piValue);
        d_ptr->piTrendWidget->setPIValue(d_ptr->piValue);
        d_ptr->piTrendWidget->setPIValue(d_ptr->pluginPIValue, true);
        d_ptr->pviTrendWidget->setPVIValue(d_ptr->pviValue);
        d_ptr->sphbTrendWidget->setSPHBValue(d_ptr->sphbValue);
        d_ptr->spocTrendWidget->setSPOCValue(d_ptr->spocValue);
        d_ptr->spmetTrendWidget->setSpMetValue(d_ptr->spmetValue);
        d_ptr->spcoTrendWidget->setSPCOValue(d_ptr->spcoValue);
    }

    if (NULL != d_ptr->oxyCRGSPO2Trend)
    {
        d_ptr->oxyCRGSPO2Trend->addTrendData(d_ptr->spo2Value);
    }

    int prValue = 60;
    setPR(prValue);
    setPluginPR(prValue);
}

void SPO2Param::exitDemo()
{
    d_ptr->spo2Value = InvData();
    d_ptr->plugInSpo2Value = InvData();
    d_ptr->spo2DValue = InvData();
    d_ptr->piValue = InvData();
    d_ptr->pluginPIValue = InvData();
    d_ptr->pviValue = InvData();
    d_ptr->sphbValue = InvData();
    d_ptr->spocValue = InvData();
    d_ptr->spmetValue = InvData();
    d_ptr->spcoValue = InvData();
    if (NULL != d_ptr->trendWidget)
    {
        d_ptr->trendWidget->setSPO2Value(InvData());
        d_ptr->trendWidget->setPluginSPO2Value(InvData());
        d_ptr->trendWidget->setSPO2DeltaValue(InvData());
        d_ptr->trendWidget->setPIValue(InvData());
        d_ptr->piTrendWidget->setPIValue(InvData());
        d_ptr->piTrendWidget->setPIValue(InvData(), true);
        d_ptr->pviTrendWidget->setPVIValue(InvData());
        d_ptr->sphbTrendWidget->setSPHBValue(InvData());
        d_ptr->spocTrendWidget->setSPOCValue(InvData());
        d_ptr->spmetTrendWidget->setSpMetValue(InvData());
        d_ptr->spcoTrendWidget->setSPCOValue(InvData());
        d_ptr->trendWidget->setBarValue(InvData());
    }

    setPR(InvData());
    d_ptr->waveWidget->resetWave();
    d_ptr->plugInWaveWidget->resetWave();
}

/**************************************************************************************************
 * 获取可得的波形控件集。
 *************************************************************************************************/
void SPO2Param::getAvailableWaveforms(QStringList *waveforms,
                                      QStringList *waveformShowName, int /*flag*/)
{
    waveforms->clear();
    waveformShowName->clear();

    if (NULL != d_ptr->waveWidget)
    {
        waveforms->append(d_ptr->waveWidget->name());
    }
    waveformShowName->append(trs("PLETH"));
}

QString SPO2Param::getWaveWindow(bool isPlugin)
{
    if (isPlugin)
    {
        return d_ptr->plugInWaveWidget->name();
    }
    else
    {
        return d_ptr->waveWidget->name();
    }
}

/**************************************************************************************************
 * 功能： 获取子参数值。
 *************************************************************************************************/
short SPO2Param::getSubParamValue(SubParamID id)
{
    switch (id)
    {
    case SUB_PARAM_SPO2:
        return getSPO2();
    case SUB_PARAM_SPO2_2:
        return getSPO2(true);
    case SUB_PARAM_SPO2_D:
        return getSPO2D();
    case SUB_PARAM_SPHB:
        return getSpHb();
    case SUB_PARAM_SPMET:
        return getSpMet();
    case SUB_PARAM_SPOC:
        return getSpOC();
    case SUB_PARAM_PVI:
        return getPVI();
    case SUB_PARAM_PI:
        return getPI();
    case SUB_PARAM_PLUGIN_PI:
        return getPI(true);
    case SUB_PARAM_SPCO:
        return getSpCO();
    default:
        return InvData();
    }
}

/**************************************************************************************************
 * 功能： 显示子参数值。
 *************************************************************************************************/
void SPO2Param::showSubParamValue()
{
    if (NULL != d_ptr->trendWidget)
    {
        d_ptr->trendWidget->showValue();
    }
    if (d_ptr->piTrendWidget)
    {
        d_ptr->piTrendWidget->showValue();
    }
    if (d_ptr->pviTrendWidget)
    {
        d_ptr->pviTrendWidget->showValue();
    }
    if (d_ptr->sphbTrendWidget)
    {
        d_ptr->sphbTrendWidget->showValue();
    }
    if (d_ptr->spocTrendWidget)
    {
        d_ptr->spocTrendWidget->showValue();
    }
    if (d_ptr->spmetTrendWidget)
    {
        d_ptr->spmetTrendWidget->showValue();
    }
    if (d_ptr->spcoTrendWidget)
    {
        d_ptr->spcoTrendWidget->showValue();
    }
}

/**************************************************************************************************
 * 获取当前的单位。
 *************************************************************************************************/
UnitType SPO2Param::getCurrentUnit(SubParamID id)
{
    switch (id)
    {
    case SUB_PARAM_SPHB:
    {
        if (getSpHbUnit() == SPHB_UNIT_G_DL)
        {
            return UNIT_GDL;
        }

        return UNIT_MMOL_L;
    }
    case SUB_PARAM_SPOC:
        return UNIT_MLDL;
    default:
        return UNIT_PERCENT;
    }
}

QVariantMap SPO2Param::getTrendVariant(int id)
{
    QVariantMap map;

    if (id == BLM_TREND_PARAM_SPO2)
    {
        map["ParamID"] = BLM_PARAM_SPO2;
        map["TrendID"] = BLM_TREND_PARAM_SPO2;
        map["Value"] = d_ptr->spo2Value;
        map["Status"] = d_ptr->spo2Value == InvData() ? 1 : 0;
    }
    else if (id == BLM_TREND_PARAM_PI)
    {
        map["ParamID"] = BLM_PARAM_SPO2;
        map["TrendID"] = BLM_TREND_PARAM_PI;
        map["Value"] = d_ptr->piValue;
        map["Status"] = d_ptr->piValue == InvData() ? 1 : 0;
    }
    return map;
}

/**************************************************************************************************
 * 设置数据提供对象。
 *************************************************************************************************/
void SPO2Param::setProvider(SPO2ProviderIFace *provider, bool isPlugin)
{
    if (provider == NULL)
    {
        return;
    }
    if (d_ptr->waveWidget == NULL)
    {
        return;
    }

    SPO2ProviderIFace *p = NULL;
    SPO2WaveWidget *w = NULL;
    if (isPlugin)
    {
        d_ptr->plugInProvider = provider;
        p = d_ptr->plugInProvider;
        w = d_ptr->plugInWaveWidget;
    }
    else
    {
        d_ptr->provider = provider;
        p = d_ptr->provider;
        w = d_ptr->waveWidget;
    }

    w->setDataRate(p->getSPO2WaveformSample());

    QString str;
    machineConfig.getStrValue("SPO2", str);
    if (str == "MASIMO_SPO2" || str.contains("RAINBOW_SPO2") || isPlugin)
    {
        /*
         * According to the MX-5 preV&V communication protocol checklist/Data Tables,
         * 1.9.3 Ensure the host does not implement additional logic using Masimo exception events.
         * We set the isEverCheckFinger to true here to enable the check sensor alarm at at startup
         */
        if (isPlugin)
        {
            d_ptr->plugInIsEverCheckFinger = true;
        }
        else
        {
            d_ptr->isEverCheckFinger = true;
        }

        p->setSensitivityFastSat(static_cast<SensitivityMode>(getSensitivity()), getFastSat());
        p->setAverageTime(getAverageTime());
        p->setLineFrequency(getLineFrequency());

        SPO2SMARTPLUSETONE pulseTone = getSmartPulseTone();
        if (pulseTone == SPO2_SMART_PLUSE_TONE_ON)
        {
            p->setSmartTone(true);
        }
        else if (pulseTone == SPO2_SMART_PLUSE_TONE_OFF)
        {
            p->setSmartTone(false);
        }
    }
    else if (str == "BLM_S5"  && systemManager.getCurWorkMode() != WORK_MODE_DEMO)
    {
        // Turn on the raw data storage function.
        int recordStatus = 0;
        machineConfig.getNumValue("Record|SPO2", recordStatus);
        enableRawDataSend(recordStatus);
    }

    if (systemManager.getCurWorkMode() == WORK_MODE_DEMO)
    {
        w->setNotify(false);
    }

    //查询状态
    p->sendStatus();

    QString tile = w->getTitle();
    // 请求波形缓冲区。
    if (isPlugin)
    {
        waveformCache.registerSource(WAVE_SPO2_2, p->getSPO2WaveformSample(), 0, p->getSPO2MaxValue(),
                                     tile, p->getSPO2BaseLine());
    }
    else
    {
        waveformCache.registerSource(WAVE_SPO2, p->getSPO2WaveformSample(), 0, p->getSPO2MaxValue(),
                                     tile, p->getSPO2BaseLine());
    }

    // update spo2 value range
    w->setValueRange(0, p->getSPO2MaxValue());
}

/**************************************************************************************************
 * 模块复位。
 *************************************************************************************************/
void SPO2Param::reset()
{
    if (NULL == d_ptr->provider && NULL == d_ptr->plugInProvider)
    {
        return;
    }

    if (d_ptr->provider)
    {
        //设置灵敏度
        d_ptr->provider->setSensitivityFastSat(static_cast<SensitivityMode>(getSensitivity()), getFastSat());

        //查询状态
        d_ptr->provider->sendStatus();
    }
    if (d_ptr->plugInProvider)
    {
        //设置灵敏度
        d_ptr->plugInProvider->setSensitivityFastSat(static_cast<SensitivityMode>(getSensitivity()), getFastSat());

        //查询状态
        d_ptr->plugInProvider->sendStatus();
    }
}

/**************************************************************************************************
 * 设置界面对象。
 *************************************************************************************************/
void SPO2Param::setTrendWidget(SPO2TrendWidget *trendWidget)
{
    if (trendWidget == NULL)
    {
        return;
    }
    d_ptr->trendWidget = trendWidget;
}

void SPO2Param::setTrendWidget(SPHBTrendWidget *trendWidget)
{
    if (trendWidget == NULL)
    {
        return;
    }
    d_ptr->sphbTrendWidget = trendWidget;
}

void SPO2Param::setTrendWidget(SPOCTrendWidget *trendWidget)
{
    if (trendWidget == NULL)
    {
        return;
    }
    d_ptr->spocTrendWidget = trendWidget;
}

void SPO2Param::setTrendWidget(SPMETTrendWidget *trendWidget)
{
    if (trendWidget == NULL)
    {
        return;
    }
    d_ptr->spmetTrendWidget = trendWidget;
}

void SPO2Param::setTrendWidget(PVITrendWidget *trendWidget)
{
    if (trendWidget == NULL)
    {
        return;
    }
    d_ptr->pviTrendWidget = trendWidget;
}

void SPO2Param::setTrendWidget(PITrendWidget *trendWidget)
{
    if (trendWidget == NULL)
    {
        return;
    }
    d_ptr->piTrendWidget = trendWidget;
}

void SPO2Param::setTrendWidget(SPCOTrendWidget *trendWidget)
{
    if (trendWidget == NULL)
    {
        return;
    }
    d_ptr->spcoTrendWidget = trendWidget;
}

/**************************************************************************************************
 * 设置界面对象。
 *************************************************************************************************/
void SPO2Param::setWaveWidget(SPO2WaveWidget *waveWidget, bool isPlugin)
{
    if (waveWidget == NULL)
    {
        return;
    }
    if (!isPlugin)
    {
        d_ptr->waveWidget = waveWidget;
    }
    else
    {
        d_ptr->plugInWaveWidget = waveWidget;
    }
    d_ptr->setWaveformSpeed((SPO2WaveVelocity)getSweepSpeed(), isPlugin);
}

void SPO2Param::setOxyCRGSPO2Trend(OxyCRGSPO2TrendWidget *trendWidget)
{
    if (trendWidget == NULL)
    {
        return;
    }
    d_ptr->oxyCRGSPO2Trend = trendWidget;
}

/**************************************************************************************************
 * 获取QRS/PR音量。
 *************************************************************************************************/
SoundManager::VolumeLevel SPO2Param::getPluseToneVolume(void)
{
    int vol = SoundManager::VOLUME_LEV_3;
    currentConfig.getNumValue("ECG|QRSToneVolume", vol);
    return (SoundManager::VolumeLevel)vol;
}

/**************************************************************************************************
 * 设置SPO2的值。
 *************************************************************************************************/
void SPO2Param::setSPO2(short spo2Value)
{
    paramUpdateTimer->start(PARAM_UPDATE_TIMEOUT);
    if (d_ptr->spo2Value == spo2Value && !d_ptr->isForceUpdatingSPO2)
    {
        return;
    }
    d_ptr->isForceUpdatingSPO2 = false;

    d_ptr->spo2Value = spo2Value;

#ifdef ENABLE_O2_APNEASTIMULATION
    // 窒息唤醒
    O2ParamInterface *o2Param = O2ParamInterface::getO2ParamInterface();
    if (o2Param)
    {
        int apneaStimulationSPO2 = 85;
        int motorSta = false;
        currentConfig.getNumValue("ApneaStimulation|SPO2", apneaStimulationSPO2);
        if (d_ptr->spo2Value < apneaStimulationSPO2 && d_ptr->spo2Value != InvData())
        {
            motorSta = true;
        }
        else
        {
            motorSta = false;
        }
        o2Param->setVibrationReason(APNEASTIMULATION_REASON_SPO2, motorSta);
    }
#endif

    if (NULL != d_ptr->trendWidget)
    {
        d_ptr->trendWidget->setSPO2Value(d_ptr->spo2Value);
        if (d_ptr->spo2Value != InvData() && d_ptr->plugInSpo2Value != InvData())
        {
            d_ptr->spo2DValue = abs(d_ptr->spo2Value - d_ptr->plugInSpo2Value);
        }
        else
        {
            d_ptr->spo2DValue = InvData();
        }
        d_ptr->trendWidget->setSPO2DeltaValue(d_ptr->spo2DValue);
    }

    if (NULL != d_ptr->oxyCRGSPO2Trend)
    {
        d_ptr->oxyCRGSPO2Trend->addTrendData(d_ptr->spo2Value);
    }
}

void SPO2Param::setPluginSPO2(short spo2Value)
{
    if (d_ptr->plugInSpo2Value == spo2Value && !d_ptr->plugInIsForceUpdating)
    {
        return;
    }
    d_ptr->plugInIsForceUpdating = false;
    d_ptr->plugInSpo2Value = spo2Value;

    if (NULL != d_ptr->trendWidget)
    {
        d_ptr->trendWidget->setPluginSPO2Value(d_ptr->plugInSpo2Value);
        if (d_ptr->spo2Value != InvData() && d_ptr->plugInSpo2Value != InvData())
        {
            d_ptr->spo2DValue = abs(d_ptr->spo2Value - d_ptr->plugInSpo2Value);
        }
        else
        {
            d_ptr->spo2DValue = InvData();
        }
        d_ptr->trendWidget->setSPO2DeltaValue(d_ptr->spo2DValue);
    }
}

/**************************************************************************************************
 * 获取SPO2的值。
 *************************************************************************************************/
short SPO2Param::getSPO2(bool isPlugin)
{
    if (!isPlugin)
    {
        return d_ptr->spo2Value;
    }
    else
    {
        return d_ptr->plugInSpo2Value;
    }
}

short SPO2Param::getSPO2D()
{
    return d_ptr->spo2DValue;
}

void SPO2Param::setSpHb(short value)
{
    if (d_ptr->sphbValue == value)
    {
        return;
    }
    d_ptr->sphbValue = value;
    if (NULL != d_ptr->sphbTrendWidget)
    {
        d_ptr->sphbTrendWidget->setSPHBValue(d_ptr->sphbValue);
    }
}

short SPO2Param::getSpHb()
{
    return d_ptr->sphbValue;
}

void SPO2Param::setSpOC(short value)
{
    if (d_ptr->spocValue == value)
    {
        return;
    }
    d_ptr->spocValue = value;
    if (NULL != d_ptr->spocTrendWidget)
    {
        d_ptr->spocTrendWidget->setSPOCValue(d_ptr->spocValue);
    }
}

short SPO2Param::getSpOC()
{
    return d_ptr->spocValue;
}

void SPO2Param::setPVI(short value)
{
    if (d_ptr->pviValue == value)
    {
        return;
    }
    d_ptr->pviValue = value;
    if (NULL != d_ptr->pviTrendWidget)
    {
        d_ptr->pviTrendWidget->setPVIValue(d_ptr->pviValue);
    }
}

short SPO2Param::getPVI()
{
    return d_ptr->pviValue;
}

void SPO2Param::setSpMet(short value)
{
    // According to the development document R-CSD-1105, SpMet display range is 0 ~ 100.0%
    if (value > 1000)
    {
        value = 1000;
    }

    if (d_ptr->spmetValue == value)
    {
        return;
    }
    d_ptr->spmetValue = value;
    if (NULL != d_ptr->spmetTrendWidget)
    {
        d_ptr->spmetTrendWidget->setSpMetValue(d_ptr->spmetValue);
    }
}

short SPO2Param::getSpMet()
{
    return d_ptr->spmetValue;
}

/**************************************************************************************************
 * 设置PR的值。
 *************************************************************************************************/
void SPO2Param::setPR(short prValue)
{
    ecgDupParam.restartParamUpdateTime();
    if (d_ptr->prValue == prValue && !d_ptr->isForceUpdatingPR)
    {
        return;
    }
    d_ptr->isForceUpdatingPR = false;
    d_ptr->prValue = prValue;
    ecgDupParam.updatePR(prValue);

    qDebug() << "-------------PR: " << prValue;
    if (NULL != d_ptr->piTrendWidget)
    {
        d_ptr->trendWidget->setPrValue(prValue);
    }

}

void SPO2Param::setPluginPR(short prValue)
{
    ecgDupParam.updatePluginPR(prValue);
}

void SPO2Param::setPI(short piValue, bool isPlugin)
{
    if (!isPlugin)
    {
        if (d_ptr->piValue == piValue)
        {
            return;
        }
        d_ptr->piValue = piValue;
        if (NULL != d_ptr->trendWidget)
        {
            d_ptr->trendWidget->setPIValue(piValue);
        }
    }
    else
    {
        if (d_ptr->pluginPIValue == piValue)
        {
            return;
        }
        d_ptr->pluginPIValue = piValue;
    }
    if (NULL != d_ptr->piTrendWidget)
    {
        d_ptr->piTrendWidget->setPIValue(piValue, isPlugin);
    }
}

short SPO2Param::getPI(bool isPlugin)
{
    if (isPlugin)
    {
        return d_ptr->pluginPIValue;
    }

    return d_ptr->piValue;
}

void SPO2Param::setSpCO(short spcoValue)
{
    // According to the development document R-CSD-1105, SpCO display range is 0 ~ 100.0%
    if (spcoValue > 100)
    {
        spcoValue = 100;
    }

    if (d_ptr->spcoValue == spcoValue)
    {
        return;
    }
    d_ptr->spcoValue = spcoValue;
    if (NULL != d_ptr->spcoTrendWidget)
    {
        d_ptr->spcoTrendWidget->setSPCOValue(d_ptr->spcoValue);
    }
}

short SPO2Param::getSpCO()
{
    return d_ptr->spcoValue;
}

/**************************************************************************************************
 * 设置波形值。
 *************************************************************************************************/
void SPO2Param::addWaveformData(short wave, unsigned char waveFlag, bool isPlugin)
{
    int flag = 0;
    // record signal IQ flag and value
    flag = flag | waveFlag;
    if (!isPlugin)
    {
        if (!isValid())
        {
            flag = flag | 0x4000;
            if (NULL != d_ptr->trendWidget)
            {
                d_ptr->trendWidget->setBarValue(InvData());
            }
        }
        else
        {
            if (NULL != d_ptr->trendWidget)
            {
                d_ptr->trendWidget->setBarValue(wave * 15 / 255);
            }
        }

        if (preProcessor)
        {
            int waveInt = wave;
            preProcessor->preProcessWave(WAVE_SPO2, &waveInt, &flag);
            wave = waveInt;
        }

        if (d_ptr->waveWidget != NULL)
        {
            d_ptr->waveWidget->addData(wave, flag);
        }
        waveformCache.addData(WAVE_SPO2, (flag << 16) | wave);
    }
    else
    {
        if (!isValid(true))
        {
            flag = flag | 0x4000;
        }
        if (d_ptr->plugInWaveWidget != NULL)
        {
            d_ptr->plugInWaveWidget->addData(wave, flag);
        }


        waveformCache.addData(WAVE_SPO2_2, (flag << 16) | wave);
    }
}

void SPO2Param::clearWaveformData(bool isPlugin)
{
    if (!isPlugin)
    {
        if (d_ptr->waveWidget != NULL)
        {
            d_ptr->waveWidget->resetWave();
        }
    }
    else
    {
        if (d_ptr->plugInWaveWidget != NULL)
        {
            d_ptr->plugInWaveWidget->resetWave();
        }
    }
}

/**************************************************************************************************
 * 设置棒图值。
 *************************************************************************************************/
void SPO2Param::addBarData(short data)
{
    if (d_ptr->barValue == data)
    {
        return;
    }
    d_ptr->barValue = data;
    if (NULL != d_ptr->trendWidget)
    {
        d_ptr->trendWidget->setBarValue(data);
    }
}

/**************************************************************************************************
 * 设置脉搏音标志。
 *************************************************************************************************/
void SPO2Param::setPulseAudio(bool pulse)
{
    HRSourceType type = ecgDupParam.getCurHRSource();
    if (pulse && (type == HR_SOURCE_SPO2  || (type == HR_SOURCE_AUTO && ecgDupParam.getHR(true) == InvData())))
    {
        soundManager.pulseTone(getSmartPulseTone() == SPO2_SMART_PLUSE_TONE_ON
                               ? getSPO2()
                               : -1);
    }
}

void SPO2Param::setBeatVol(SoundManager::VolumeLevel vol)
{
    // 将脉搏音与心跳音绑定在一起，形成联动
    currentConfig.setNumValue("ECG|QRSVolume", static_cast<int>(vol));
    soundManager.setVolume(SoundManager::SOUND_TYPE_PULSE, vol);
    soundManager.setVolume(SoundManager::SOUND_TYPE_HEARTBEAT, vol);
}

SoundManager::VolumeLevel SPO2Param::getBeatVol() const
{
    int vol = SoundManager::VOLUME_LEV_2;
    currentConfig.getNumValue("ECG|QRSVolume", vol);
    return static_cast<SoundManager::VolumeLevel>(vol);
}

void SPO2Param::setNotify(bool enable, QString str, bool isPlugin)
{
    if (!isPlugin)
    {
        if (NULL != d_ptr->waveWidget)
        {
            if (d_ptr->isEverCheckFinger)
            {
                d_ptr->waveWidget->setNotify(enable, str);
            }
        }
    }
    else
    {
        if (NULL != d_ptr->plugInWaveWidget)
        {
            if (d_ptr->plugInIsEverCheckFinger)
            {
                d_ptr->plugInWaveWidget->setNotify(enable, str);
            }
        }
    }
}

/**************************************************************************************************
 * 设置搜索脉搏标志。
 *************************************************************************************************/
void SPO2Param::setSearchForPulse(bool isSearching, bool isPlugin)
{
    if (!isPlugin)
    {
        if (NULL != d_ptr->waveWidget)
        {
            d_ptr->waveWidget->setNotify(isSearching, trs("SPO2PulseSearch"));
        }
        if (isSearching && !d_ptr->isEverCheckFinger)
        {
            d_ptr->isEverCheckFinger = true;
            systemConfig.setNumValue("PrimaryCfg|SPO2|EverCheckFinger", true);
        }
    }
    else
    {
        if (NULL != d_ptr->plugInWaveWidget)
        {
            d_ptr->plugInWaveWidget->setNotify(isSearching, trs("SPO2PulseSearch"));
        }
        if (isSearching && !d_ptr->plugInIsEverCheckFinger)
        {
            d_ptr->plugInIsEverCheckFinger = true;
            systemConfig.setNumValue("PrimaryCfg|SPO2|EverCheckFinger", true);
        }
    }
}

/**************************************************************************************************
 * 设置报警。
 *************************************************************************************************/
void SPO2Param::setOneShotAlarm(SPO2OneShotType t, bool f, bool isPlugin)
{
    AlarmOneShotIFace *alarmSource = NULL;
    if (isPlugin)
    {
        alarmSource = alarmSourceManager.getOneShotAlarmSource(ONESHOT_ALARMSOURCE_SPO2_2);
    }
    else
    {
        alarmSource = alarmSourceManager.getOneShotAlarmSource(ONESHOT_ALARMSOURCE_SPO2);
    }
    if (alarmSource)
    {
        if (t == SPO2_ONESHOT_ALARM_CABLE_OFF && f == true)
        {
            alarmSource->clear();
        }
        alarmSource->setOneShotAlarm(t, f);
    }
}

/**************************************************************************************************
 * 设置报警。
 *************************************************************************************************/
void SPO2Param::noticeLimitAlarm(SubParamID id, bool isAlarm)
{
    if (isNibpSameSide() && nibpParam.isMeasuring())
    {
        // 如果打开同侧功能，且nibp正在测量，则不设置报警
        return;
    }
    switch (id)
    {
    case SUB_PARAM_SPO2:
        if (NULL != d_ptr->trendWidget)
        {
            d_ptr->trendWidget->isAlarmSPO2(isAlarm);
        }
        break;
    case SUB_PARAM_SPO2_2:
        if (NULL != d_ptr->trendWidget)
        {
            d_ptr->trendWidget->isAlarmSPO22(isAlarm);
        }
        break;
    case SUB_PARAM_SPO2_D:
        if (NULL != d_ptr->trendWidget)
        {
            d_ptr->trendWidget->isAlarmSPO2D(isAlarm);
        }
        break;
    case SUB_PARAM_PI:
        if (NULL != d_ptr->piTrendWidget)
        {
            d_ptr->piTrendWidget->isAlarm(isAlarm);
        }
        break;
    case SUB_PARAM_PLUGIN_PI:
        if (NULL != d_ptr->piTrendWidget)
        {
            d_ptr->piTrendWidget->isPluginPIAlarm(isAlarm);
        }
        break;
    case SUB_PARAM_PVI:
        if (NULL != d_ptr->pviTrendWidget)
        {
            d_ptr->pviTrendWidget->isAlarm(isAlarm);
        }
        break;
    case SUB_PARAM_SPHB:
        if (NULL != d_ptr->sphbTrendWidget)
        {
            d_ptr->sphbTrendWidget->isAlarm(isAlarm);
        }
        break;
    case SUB_PARAM_SPOC:
        if (NULL != d_ptr->spocTrendWidget)
        {
            d_ptr->spocTrendWidget->isAlarm(isAlarm);
        }
        break;
    case SUB_PARAM_SPMET:
        if (NULL != d_ptr->spmetTrendWidget)
        {
            d_ptr->spmetTrendWidget->isAlarm(isAlarm);
        }
        break;
    case SUB_PARAM_SPCO:
        if (NULL != d_ptr->spcoTrendWidget)
        {
            d_ptr->spcoTrendWidget->isAlarm(isAlarm);
        }
        break;
    default:
        break;
    }
}

/**************************************************************************************************
 * 状态0x42。
 *************************************************************************************************/
void SPO2Param::setValidStatus(bool isValid, bool isPlugin)
{
    if (!isPlugin)
    {
        d_ptr->isValid = isValid;
    }
    else
    {
        d_ptr->plugInIsValid = isValid;
    }
}

/**************************************************************************************************
 * 状态是否有效。
 *************************************************************************************************/
bool SPO2Param::isValid(bool isPlugin)
{
    if (systemManager.getCurWorkMode() == WORK_MODE_DEMO)
    {
        // 演示模式下，状态强制置为有效
        return true;
    }
    if (!isPlugin)
    {
        return d_ptr->isValid;
    }
    else
    {
        return d_ptr->plugInIsValid;
    }
}

/**************************************************************************************************
 * 是否连接。
 *************************************************************************************************/
bool SPO2Param::isConnected(bool isPlugin)
{
    if (isPlugin)
    {
        return d_ptr->connectedPluginProvider;
    }
    else
    {
        return d_ptr->connectedProvider;
    }
}

void SPO2Param::setConnected(bool isConnected, bool isPlugin)
{
    if (!isPlugin)
    {
        d_ptr->connectedProvider = isConnected;
    }
    else
    {
        d_ptr->connectedPluginProvider = isConnected;

        // Update plugin connection status
        d_ptr->bPlugConnected = isConnected;
        emit updatePluginConnected(isConnected);
    }
    QString wave = getWaveWindow(isPlugin);

    int needUpdate = 0;
    if (isConnected)
    {
        if (isPlugin && layoutManager.getUFaceType() == UFACE_MONITOR_SPO2)
        {
            // update to show SpO2 info
            needUpdate |= layoutManager.setWidgetLayoutable(wave, true);
            if (needUpdate)
            {
                layoutManager.updateLayout();
            }
        }
    }
    else
    {
        AlarmOneShotIFace *alarmSource = alarmSourceManager.getOneShotAlarmSource(ONESHOT_ALARMSOURCE_SPO2_2);
        if (alarmSource)
        {
            alarmSource->clear();
        }

        if (isPlugin && layoutManager.getUFaceType() == UFACE_MONITOR_SPO2)
        {
            // update to show SpO2 info
            needUpdate |= layoutManager.setWidgetLayoutable(wave, false);
            if (needUpdate)
            {
                layoutManager.updateLayout();
            }
        }

        // SPO2 param is disconnected, we set value to invalid.
        if (!isPlugin)
        {
            d_ptr->spo2Value = InvData();
            d_ptr->piValue = InvData();
            d_ptr->pviValue = InvData();
            d_ptr->sphbValue = InvData();
            d_ptr->spocValue = InvData();
            d_ptr->spmetValue = InvData();
            d_ptr->spcoValue = InvData();

            d_ptr->trendWidget->setSPO2Value(InvData());
            d_ptr->trendWidget->setPIValue(InvData());
            d_ptr->trendWidget->setBarValue(InvData());
            d_ptr->waveWidget->resetWave();

            d_ptr->piTrendWidget->setPIValue(InvData());
            d_ptr->pviTrendWidget->setPVIValue(InvData());
            d_ptr->sphbTrendWidget->setSPHBValue(InvData());
            d_ptr->spocTrendWidget->setSPOCValue(InvData());
            d_ptr->spmetTrendWidget->setSpMetValue(InvData());
            d_ptr->spcoTrendWidget->setSPCOValue(InvData());
        }
        else
        {
            d_ptr->plugInSpo2Value = InvData();
            d_ptr->pluginPIValue = InvData();
            d_ptr->trendWidget->setPluginSPO2Value(InvData());
            d_ptr->piTrendWidget->setPIValue(InvData(), true);
            setPluginPR(InvData());
        }
        setPR(InvData());
    }
}

/**************************************************************************************************
 * receive package
 *************************************************************************************************/
void SPO2Param::receivePackage()
{
    if (d_ptr->recPackageInPowerOn2sec < 5)
    {
        ++(d_ptr->recPackageInPowerOn2sec);
    }
}

/**************************************************************************************************
 * check selftest
 *************************************************************************************************/
void SPO2Param::checkSelftest()
{
    if (!systemManager.isSupport(CONFIG_SPO2))
    {
        return;
    }

    QString str;
    machineConfig.getStrValue("SPO2", str);
    if (str == "BLM_TS3")
    {
        if (d_ptr->recPackageInPowerOn2sec == 5)
        {
            systemManager.setPoweronTestResult(TS3_MODULE_SELFTEST_RESULT, SELFTEST_SUCCESS);
        }
        else
        {
            systemManager.setPoweronTestResult(TS3_MODULE_SELFTEST_RESULT, SELFTEST_FAILED);
            ErrorLogItem *item = new CriticalFaultLogItem();
            item->setName("SpO2 Module Self Test Failed");
            errorLog.append(item);
        }
    }
    else if (str == "BLM_S5")
    {
        if (d_ptr->recPackageInPowerOn2sec == 5)
        {
            systemManager.setPoweronTestResult(S5_MODULE_SELFTEST_RESULT, SELFTEST_SUCCESS);
        }
        else
        {
            systemManager.setPoweronTestResult(S5_MODULE_SELFTEST_RESULT, SELFTEST_FAILED);
            ErrorLogItem *item = new CriticalFaultLogItem();
            item->setName("SpO2 Module Self Test Failed");
            errorLog.append(item);
        }
    }
}

void SPO2Param::onPaletteChanged(ParamID id)
{
    if (id != PARAM_SPO2 || !systemManager.isSupport(CONFIG_SPO2))
    {
        return;
    }
    QPalette pal = colorManager.getPalette(paramInfo.getParamName(PARAM_SPO2));
    d_ptr->waveWidget->updatePalette(pal);
    d_ptr->plugInWaveWidget->updatePalette(pal);
    d_ptr->trendWidget->updatePalette(pal);
    d_ptr->piTrendWidget->updatePalette(pal);
}

void SPO2Param::handleRainbowProgram(int result)
{
    MessageBox *msgBox = qobject_cast<MessageBox *>(sender());
    if (msgBox)
    {
        msgBox->deleteLater();
        bool isPlugin = msgBox->property("isPlugin").toBool();

        SPO2ProviderIFace *p = isPlugin ? d_ptr->plugInProvider : d_ptr->provider;
        if (p)
        {
            p->setProgramResponse(result);
        }

        if (result)
        {
            MessageBox *msgBox = new MessageBox(trs("SPO2Program"),
                                                trs("Programming"),
                                                QStringList(trs("EnglishYESChineseSURE")));
            d_ptr->curProgramMsgBox = msgBox;
            WindowManager::getInstance().showWindow(msgBox, WindowManager::ShowBehaviorCloseOthers);
        }
    }
}

void SPO2Param::onUpgradeT5ModuleCompleted()
{
    d_ptr->isT5ModuleUpgradeCompleted = true;
}

void SPO2Param::onTempReset()
{
    if (d_ptr->isT5ModuleUpgradeCompleted)
    {
        d_ptr->isT5ModuleUpgradeCompleted = false;
        // 手动刷新血氧模块，更新板卡数据包转发端口波特率。
        // 目前血氧模块与体温模块共用一个串口转发板，当体温模块升级后，
        // 共用的串口转发板的串口波特率恢复成9600，与部分血氧模块不匹配，
        // 需要在体温模块升级成功后，重新初始化血氧模块，
        // 更新血氧模块的数据端口转发波特率。
        initModule(false);
    }
}

void SPO2Param::setSensitivity(int sens)
{
    currentConfig.setNumValue("SPO2|Sensitivity", static_cast<int>(sens));
    if (NULL != d_ptr->provider)
    {
        if (d_ptr->moduleType == MODULE_RAINBOW_SPO2)
        {
            d_ptr->provider->setSensitivityFastSat(static_cast<SensitivityMode>(sens), getFastSat());
        }
        else if (d_ptr->moduleType != MODULE_SPO2_NR)
        {
            d_ptr->provider->setSensitive(static_cast<SPO2Sensitive>(sens));
        }
    }
    if (NULL != d_ptr->plugInProvider)
    {
        if (d_ptr->moduleType == MODULE_RAINBOW_SPO2)
        {
            d_ptr->plugInProvider->setSensitivityFastSat(static_cast<SensitivityMode>(sens), getFastSat());
        }
        else if (d_ptr->moduleType != MODULE_SPO2_NR)
        {
            d_ptr->plugInProvider->setSensitive(static_cast<SPO2Sensitive>(sens));
        }
    }
}

int SPO2Param::getSensitivity(void)
{
    int sens = 0;
    currentConfig.getNumValue("SPO2|Sensitivity", sens);
    return sens;
}

void SPO2Param::setFastSat(bool isFast)
{
    currentConfig.setNumValue("SPO2|FastSat", static_cast<int>(isFast));
    if (NULL != d_ptr->provider)
    {
        d_ptr->provider->setSensitivityFastSat(static_cast<SensitivityMode>(getSensitivity()), isFast);
    }
    if (NULL != d_ptr->plugInProvider)
    {
        d_ptr->plugInProvider->setSensitivityFastSat(static_cast<SensitivityMode>(getSensitivity()), isFast);
    }
}

bool SPO2Param::getFastSat()
{
    int fastSat = false;
    currentConfig.getNumValue("SPO2|FastSat", fastSat);
    return static_cast<bool>(fastSat);
}

void SPO2Param::setSpHbAveragingMode(SpHbAveragingMode mode)
{
    d_ptr->provider->setSphbAveragingMode(mode);
    currentConfig.setNumValue("SPO2|SpHbAveragingMode", static_cast<int>(mode));
}

SpHbAveragingMode SPO2Param::getSpHbAveragingMode()
{
    int sphbAveragingMode = false;
    currentConfig.getNumValue("SPO2|SpHbAveragingMode", sphbAveragingMode);
    return static_cast<SpHbAveragingMode>(sphbAveragingMode);
}

void SPO2Param::setSpHbUnit(SpHbUnitType unit)
{
    currentConfig.setNumValue("SPO2|SpHbUnit", static_cast<int>(unit));
    if (NULL != d_ptr->sphbTrendWidget)
    {
        UnitType u = unit == SPHB_UNIT_G_DL ? UNIT_GDL : UNIT_MMOL_L;
        d_ptr->sphbTrendWidget->updateUnit(u);
    }

    IWidget *w = layoutManager.getLayoutWidget("SpHbTrendWave");
    if (w)
    {
        /* update widget config will refresh the display unit of the trend wave widget */
        w->updateWidgetConfig();
    }
}

SpHbUnitType SPO2Param::getSpHbUnit()
{
    int  unit = SPHB_UNIT_G_DL;
    currentConfig.getNumValue("SPO2|SpHbUnit", unit);
    return static_cast<SpHbUnitType>(unit);
}

void SPO2Param::setSpHbBloodVessel(SpHbBloodVesselMode mode)
{
    d_ptr->provider->setSpHbBloodVesselMode(mode);
    currentConfig.setNumValue("SPO2|SpHbBloodVessel", static_cast<int>(mode));
}

SpHbBloodVesselMode SPO2Param::getSpHbBloodVessel()
{
    int spHbBloodVessel = false;
    currentConfig.getNumValue("SPO2|SpHbBloodVessel", spHbBloodVessel);
    return static_cast<SpHbBloodVesselMode>(spHbBloodVessel);
}

void SPO2Param::setSpHbPrecision(SpHbPrecisionMode mode)
{
    d_ptr->provider->setSpHbPrecisionMode(mode);
    currentConfig.setNumValue("SPO2|SpHbPrecision", static_cast<int>(mode));
}

SpHbPrecisionMode SPO2Param::getSpHbPrecision()
{
    int sphbPrecision = false;
    currentConfig.getNumValue("SPO2|SpHbPrecision", sphbPrecision);
    return static_cast<SpHbPrecisionMode>(sphbPrecision);
}

void SPO2Param::setPviAveragingMode(AveragingMode mode)
{
    d_ptr->provider->setPVIAveragingMode(mode);
    currentConfig.setNumValue("SPO2|PviAveragingMode", static_cast<int>(mode));
}

AveragingMode SPO2Param::getPviAveragingMode()
{
    int pviAveragingMode = false;
    currentConfig.getNumValue("SPO2|PviAveragingMode", pviAveragingMode);
    return static_cast<AveragingMode>(pviAveragingMode);
}

/**************************************************************************************************
 * 设置智能音。
 *************************************************************************************************/
void SPO2Param::setSmartPulseTone(SPO2SMARTPLUSETONE sens)
{
    if (d_ptr->provider)
    {
        if (sens == SPO2_SMART_PLUSE_TONE_ON)
        {
            d_ptr->provider->setSmartTone(true);
        }
        else if (sens == SPO2_SMART_PLUSE_TONE_OFF)
        {
            d_ptr->provider->setSmartTone(false);
        }
    }
    if (d_ptr->plugInProvider)
    {
        if (sens == SPO2_SMART_PLUSE_TONE_ON)
        {
            d_ptr->plugInProvider->setSmartTone(true);
        }
        else if (sens == SPO2_SMART_PLUSE_TONE_OFF)
        {
            d_ptr->plugInProvider->setSmartTone(false);
        }
    }
    currentConfig.setNumValue("SPO2|SmartPluseTone", static_cast<int>(sens));
}

/**************************************************************************************************
 * 获取智能音。
 *************************************************************************************************/
SPO2SMARTPLUSETONE SPO2Param::getSmartPulseTone(void)
{
    int sens = SPO2_SMART_PLUSE_TONE_OFF;
    currentConfig.getNumValue("SPO2|SmartPluseTone", sens);
    return (SPO2SMARTPLUSETONE)sens;
}

void SPO2Param::setLineFrequency(SPO2LineFrequencyType freq)
{
    int v = freq;

    if (d_ptr->provider)
    {
        d_ptr->provider->setLineFrequency(freq);
    }

    if (d_ptr->plugInProvider)
    {
        d_ptr->plugInProvider->setLineFrequency(freq);
    }

    currentConfig.setNumValue("SPO2|LineFrequency", v);
}

SPO2LineFrequencyType SPO2Param::getLineFrequency()
{
    int freq = SPO2_LINE_FREQ_50HZ;
    currentConfig.getNumValue("SPO2|LineFrequency", freq);
    return static_cast<SPO2LineFrequencyType>(freq);
}

/**************************************************************************************************
 * 设置波形速度。
 *************************************************************************************************/
void SPO2Param::setSweepSpeed(int speed)
{
    currentConfig.setNumValue("SPO2|SweepSpeed", speed);
    d_ptr->setWaveformSpeed((SPO2WaveVelocity)speed);
    d_ptr->setWaveformSpeed((SPO2WaveVelocity)speed, SPO2_RAINBOW_FACTORY_ID_BLM);
}

/**************************************************************************************************
 * 获取波形速度。
 *************************************************************************************************/
int SPO2Param::getSweepSpeed(void)
{
    int speed = SPO2_WAVE_VELOCITY_250;
    currentConfig.getNumValue("SPO2|SweepSpeed", speed);
    return speed;
}

bool SPO2Param::getEverCheckFinger(bool isPlugin)
{
    if (isPlugin)
    {
        return d_ptr->plugInIsEverCheckFinger;
    }
    else
    {
        return d_ptr->isEverCheckFinger;
    }
}

void SPO2Param::updateSubParamLimit(SubParamID id)
{
    switch (id)
    {
    case SUB_PARAM_SPO2:
    case SUB_PARAM_SPO2_2:
    case SUB_PARAM_SPO2_D:
        if (d_ptr->trendWidget)
        {
            d_ptr->trendWidget->updateLimit();
        }
        break;
    case SUB_PARAM_PI:
        if (d_ptr->piTrendWidget)
        {
            d_ptr->piTrendWidget->updateLimit();
        }
        break;
    case SUB_PARAM_PVI:
        if (d_ptr->pviTrendWidget)
        {
            d_ptr->pviTrendWidget->updateLimit();
        }
        break;
    case SUB_PARAM_SPHB:
        if (d_ptr->sphbTrendWidget)
        {
            d_ptr->sphbTrendWidget->updateLimit();
        }
        break;
    case SUB_PARAM_SPOC:
        if (d_ptr->spocTrendWidget)
        {
            d_ptr->spocTrendWidget->updateLimit();
        }
        break;
    case SUB_PARAM_SPMET:
        if (d_ptr->spmetTrendWidget)
        {
            d_ptr->spmetTrendWidget->updateLimit();
        }
        break;
    case SUB_PARAM_SPCO:
        if (d_ptr->spcoTrendWidget)
        {
            d_ptr->spcoTrendWidget->updateLimit();
        }
        break;
    default:
        break;
    }
}

void SPO2Param::setModuleType(SPO2ModuleType type)
{
    d_ptr->moduleType = type;
    if (type == MODULE_RAINBOW_SPO2)
    {
        /*
         * According to the MX-5 Prev &V communication protocol Checklist/Data Table 16.1,
         * ensure Maximum sensitivity is not allowed as a default. The device must use Normal
         * or APOD instead upon power cycle or reset.
         */

        int sens = 0;
        currentConfig.getNumValue("SPO2|Sensitivity", sens);
        if (sens == SPO2_MASIMO_SENS_MAX)
        {
            sens = SPO2_MASIMO_SENS_NORMAL;
            currentConfig.setNumValue("SPO2|Sensitivity", sens);
        }
    }
}

SPO2ModuleType SPO2Param::getModuleType() const
{
    return d_ptr->moduleType;
}

void SPO2Param::setNibpSameSide(bool flag)
{
    int index = flag;
    currentConfig.setNumValue("SPO2|NIBPSameSide", index);
}

bool SPO2Param::isNibpSameSide(void)
{
    int flag;
    currentConfig.getNumValue("SPO2|NIBPSameSide", flag);
    return flag;
}

void SPO2Param::setCCHDData(short value, bool isHand)
{
    cchdData data;
    if (isHand)
    {
        if (d_ptr->cchdDataList.count() != 0 && d_ptr->cchdDataList.last().handValue == InvData())
        {
            d_ptr->cchdDataList.last().handValue = value;
            return;
        }
        data.handValue = value;
    }
    else
    {
        if (d_ptr->cchdDataList.count() != 0 && d_ptr->cchdDataList.last().footValue == InvData())
        {
            d_ptr->cchdDataList.last().footValue = value;
            return;
        }
        data.footValue = value;
    }
    d_ptr->cchdDataList.append(data);
}

CCHDResult SPO2Param::updateCCHDResult()
{
    if (d_ptr->cchdDataList.count() == 0)
    {
        return CCHD_NR;
    }
    short handValue = d_ptr->cchdDataList.last().handValue;
    short footValue = d_ptr->cchdDataList.last().footValue;
    if (handValue == InvData() || footValue == InvData())
    {
        // 不完全数据返回无效值
        return CCHD_NR;
    }
    CCHDResult result = CCHD_NR;

    if ((handValue >= 95 && abs(footValue - handValue) <= 3) ||
            (footValue >= 95 && abs(footValue - handValue) <= 3))
    {
        // 阴性
        d_ptr->repeatTimes = 0;
        result = Negative;
    }
    else if (((handValue >= 90 && handValue <= 94) && (footValue >= 90 && footValue <= 94))
             || (abs(handValue - footValue) > 3))
    {
        // 重复测试判断是否为阳性
        d_ptr->repeatTimes++;
        if (d_ptr->repeatTimes > 2)
        {
            // 重复测量3次则返回阳性
            d_ptr->repeatTimes = 0;
            result = Positive;
        }
        else
        {
            result = RepeatCheck;
        }
    }
    else
    {
        // 阳性
        d_ptr->repeatTimes = 0;
        result = Positive;
    }
    d_ptr->cchdDataList.last().result = result;
    d_ptr->cchdDataList.last().time = timeDate->time();
    return result;
}


QList<cchdData> SPO2Param::getCCHDDataList()
{
    return d_ptr->cchdDataList;
}

void SPO2Param::clearCCHDData(bool isCleanup)
{
    if (!(d_ptr->cchdDataList.isEmpty()))
    {
        if (!isCleanup)
        {
            if (d_ptr->cchdDataList.count() > 3
                    || d_ptr->cchdDataList.last().result == Positive
                    || d_ptr->cchdDataList.last().result == Negative)
            {
                d_ptr->cchdDataList.clear();
            }
            else if (d_ptr->cchdDataList.last().result == CCHD_NR)
            {
                d_ptr->cchdDataList.removeLast();
            }
        }
        else
        {
            d_ptr->cchdDataList.clear();
            d_ptr->repeatTimes = 0;
        }
    }
}

void SPO2Param::setPerfusionStatus(bool isLow, bool isPlugin)
{
    if (!isPlugin)
    {
        if (isLow != d_ptr->isLowPerfusion)
        {
            d_ptr->isForceUpdatingPR = true;
            d_ptr->isForceUpdatingSPO2 = true;
            d_ptr->isLowPerfusion = isLow;
        }
    }
    else
    {
        if (isLow != d_ptr->plugInIsLowPerfusion)
        {
            d_ptr->plugInIsForceUpdating = true;
            d_ptr->plugInIsLowPerfusion = isLow;
        }
    }
}

bool SPO2Param::getPerfusionStatus(bool isPlugin) const
{
    if (isPlugin)
    {
        return d_ptr->plugInIsLowPerfusion;
    }
    return d_ptr->isLowPerfusion;
}

void SPO2Param::initModule(bool plugin)
{
    if (plugin)
    {
        if (d_ptr->plugInProvider)
        {
            d_ptr->plugInProvider->initModule();
        }
    }
    else
    {
        if (d_ptr->provider)
        {
            d_ptr->provider->initModule();
        }
    }
}

void SPO2Param::setSPO2AlarmDelay(SPO2AlarmAudioDelay delay)
{
    currentConfig.setNumValue("SPO2|AlarmDelay", static_cast<int>(delay));
}

SPO2AlarmAudioDelay SPO2Param::getSPO2AlarmDelay()
{
    int delay = SPO2_ALARM_AUDIO_DELAY_0S;
    currentConfig.getNumValue("SPO2|AlarmDelay", delay);
    return static_cast<SPO2AlarmAudioDelay> (delay);
}

int SPO2Param::getAlarmAudioDelay()
{
    int alarmAudioDelay = 0;
    switch (getSPO2AlarmDelay())
    {
    case SPO2_ALARM_AUDIO_DELAY_0S:
        alarmAudioDelay = 0;
        break;
    case SPO2_ALARM_AUDIO_DELAY_5S:
        alarmAudioDelay = 5;
        break;
    case SPO2_ALARM_AUDIO_DELAY_10S:
        alarmAudioDelay = 10;
        break;
    case SPO2_ALARM_AUDIO_DELAY_15S:
        alarmAudioDelay = 15;
        break;
    default:
        break;
    }

    return alarmAudioDelay;
}

void SPO2Param::clearTrendWaveData()
{
    emit clearTrendData();
}

void SPO2Param::showSignalIQ(bool show)
{
    d_ptr->isShowSignalIQ = show;
    int index = show ? 1 : 0;
    currentConfig.setNumValue("SPO2|SignalIQ", index);
}

bool SPO2Param::isShowSignalIQ()
{
    return d_ptr->isShowSignalIQ;
}

void SPO2Param::setSensor(SPO2RainbowSensor sensor)
{
    if (d_ptr->sensorType == sensor)
    {
        return;
    }
    d_ptr->sensorType = sensor;

    layoutManager.updateLayout();
}

SPO2RainbowSensor SPO2Param::getSensor()
{
    return d_ptr->sensorType;
}

void SPO2Param::showRainbowProgramMessage(SPO2RainbowProgramMessageType msg, bool isPlugin)
{
    SPO2ProviderIFace *p = isPlugin ? d_ptr->plugInProvider : d_ptr->provider;
    if (!p)
    {
        return;
    }

    switch (msg)
    {
    case SPO2_RAINBOW_PROG_MSG_REQUEST_ENTER_PROGRAM_MODE:
    {
        if (d_ptr->curProgramMsgBox)
        {
            d_ptr->curProgramMsgBox->disconnect();
            d_ptr->curProgramMsgBox->deleteLater();
        }

        MessageBox *msgBox = new MessageBox(trs("SPO2Program"), trs("RequestEnterProgramMode"));
        connect(msgBox, SIGNAL(finished(int)), this, SLOT(handleRainbowProgram(int)));
        msgBox->setProperty("isPlugin", isPlugin);
        d_ptr->curProgramMsgBox = msgBox;
        WindowManager::getInstance().showWindow(msgBox, WindowManager::ShowBehaviorCloseOthers
                                  |WindowManager::ShowBehaviorNoAutoClose);
    }
        break;
    case SPO2_RAINBOW_PROG_MSG_PROGRAM_COMPLETE:
    {
        if (d_ptr->curProgramMsgBox)
        {
            d_ptr->curProgramMsgBox->disconnect();
            d_ptr->curProgramMsgBox->deleteLater();
        }

        MessageBox *msgBox = new MessageBox(trs("SPO2Program"), trs("ProgramComplete"),
                                            QStringList(trs("EnglishYESChineseSURE")));
        d_ptr->curProgramMsgBox = msgBox;
        WindowManager::getInstance().showWindow(msgBox, WindowManager::ShowBehaviorCloseOthers);
    }
        break;
    case SPO2_RAINBOW_PROG_MSG_COMMUNICATION_ERROR:
    {
        if (d_ptr->curProgramMsgBox)
        {
            d_ptr->curProgramMsgBox->disconnect();
            d_ptr->curProgramMsgBox->deleteLater();
        }

        MessageBox *msgBox = new MessageBox(trs("SPO2Program"),
                                            trs("CommunicationError"),
                                            QStringList(trs("EnglishYESChineseSURE")));
        d_ptr->curProgramMsgBox = msgBox;
        WindowManager::getInstance().showWindow(msgBox, WindowManager::ShowBehaviorCloseOthers);
    }
        break;
    case SPO2_RAINBOW_PROG_MSG_FLASH_ERROR:
    {
        if (d_ptr->curProgramMsgBox)
        {
            d_ptr->curProgramMsgBox->disconnect();
            d_ptr->curProgramMsgBox->deleteLater();
        }

        MessageBox *msgBox = new MessageBox(trs("SPO2Program"),
                                            trs("FlashError"),
                                            QStringList(trs("EnglishYESChineseSURE")));
        d_ptr->curProgramMsgBox = msgBox;
        WindowManager::getInstance().showWindow(msgBox, WindowManager::ShowBehaviorCloseOthers);
    }
        break;
    case SPO2_RAINBOW_PROG_MSG_INVALID_UPGRADE_APPLICATION:
    {
        if (d_ptr->curProgramMsgBox)
        {
            d_ptr->curProgramMsgBox->disconnect();
            d_ptr->curProgramMsgBox->deleteLater();
        }

        MessageBox *msgBox = new MessageBox(trs("SPO2Program"),
                                            trs("InvalidUpgradeApplication"),
                                            QStringList(trs("EnglishYESChineseSURE")));
        d_ptr->curProgramMsgBox = msgBox;
        WindowManager::getInstance().showWindow(msgBox, WindowManager::ShowBehaviorCloseOthers);
    }
        break;
    case SPO2_RAINBOW_PROG_MSG_INVALID_TOOL_CONFIGURATION:
    {
        if (d_ptr->curProgramMsgBox)
        {
            d_ptr->curProgramMsgBox->disconnect();
            d_ptr->curProgramMsgBox->deleteLater();
        }

        MessageBox *msgBox = new MessageBox(trs("SPO2Program"),
                                            trs("InvalidToolConfiguration"),
                                            QStringList(trs("EnglishYESChineseSURE")));
        d_ptr->curProgramMsgBox = msgBox;
        WindowManager::getInstance().showWindow(msgBox, WindowManager::ShowBehaviorCloseOthers);
    }
        break;
    default:
        break;
    }
}

/**************************************************************************************************
 * 构造。
 *************************************************************************************************/
SPO2Param::SPO2Param()
         : Param(PARAM_SPO2)
         , d_ptr(new SPO2ParamPrivate())
{
    systemConfig.getNumValue("PrimaryCfg|SPO2|EverCheckFinger", d_ptr->isEverCheckFinger);
    int index = 0;
    currentConfig.getNumValue("SPO2|SignalIQ", index);
    d_ptr->isShowSignalIQ = static_cast<bool>(index);

    QTimer::singleShot(2000, this, SLOT(checkSelftest()));

    connect(UpgradeManager::getInstance(), SIGNAL(upgradeT5ModuleCompleted()),
            this, SLOT(onUpgradeT5ModuleCompleted()));
    connect(&tempParam, SIGNAL(tempReset()), this, SLOT(onTempReset()));
}

/**************************************************************************************************
 * 析构。
 *************************************************************************************************/
SPO2Param &SPO2Param::getInstance()
{
    static SPO2Param *instance = NULL;
    if (instance == NULL)
    {
        instance = new SPO2Param();
        SPO2ParamInterface *old = registerSPO2Param(instance);
        if (old)
        {
            delete old;
        }
    }
    return *instance;
}

SPO2Param::~SPO2Param()
{
    delete d_ptr;
}

bool SPO2Param::havePlugin()
{
    return d_ptr->bPlugConnected;
}
