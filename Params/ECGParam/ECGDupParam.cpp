/**
 ** This file is part of the nPM project.
 ** Copyright(C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by ZhongHuan Duan duanzhonghuan@blmed.cn, 2018/9/29
 **/



#include "ECGDupParam.h"
#include "BaseDefine.h"
#include "IConfig.h"
#include "ECGTrendWidget.h"
#include "Debug.h"
#include "ECGParam.h"
#include "SoundManager.h"
#include "SPO2Param.h"
#include "IBPParam.h"

ECGDupParam *ECGDupParam::_selfObj = NULL;

/**************************************************************************************************
 * 初始化参数。
 *************************************************************************************************/
void ECGDupParam::initParam(void)
{
}

/**************************************************************************************************
 * 处理DEMO数据。
 *************************************************************************************************/
void ECGDupParam::handDemoWaveform(WaveformID /*id*/, short /*data*/)
{
}

/**************************************************************************************************
 * 产生DEMO数据。
 *************************************************************************************************/
void ECGDupParam::handDemoTrendData(void)
{
}

/**************************************************************************************************
 * 功能： 获取子参数值。
 *************************************************************************************************/
void ECGDupParam::getAvailableWaveforms(QStringList &/*waveforms*/,
                                        QStringList &/*waveformShowName*/, int /*flag*/)
{
}

/**************************************************************************************************
 * 功能： 获取子参数值。
 *************************************************************************************************/
short ECGDupParam::getSubParamValue(SubParamID id)
{
    switch (id)
    {
    case SUB_PARAM_HR_PR:
        return getHR();

    default:
        return InvData();
    }
}

/**************************************************************************************************
 * 功能： 显示子参数值。
 *************************************************************************************************/
void ECGDupParam::showSubParamValue()
{
    if (NULL != _trendWidget)
    {
        _trendWidget->showValue();
    }
}

/**************************************************************************************************
 * 设置数据提供对象。
 *************************************************************************************************/
void ECGDupParam::setProvider(ECGProviderIFace *provider)
{
    if (provider == NULL)
    {
        return;
    }
    _provider = provider;
}


/**************************************************************************************************
 * 获取当前的单位。
 *************************************************************************************************/
UnitType ECGDupParam::getCurrentUnit(SubParamID id)
{
    switch (id)
    {
    case SUB_PARAM_ST_I:
    case SUB_PARAM_ST_II:
    case SUB_PARAM_ST_III:
    case SUB_PARAM_ST_aVR:
    case SUB_PARAM_ST_aVL:
    case SUB_PARAM_ST_aVF:
    case SUB_PARAM_ST_V1:
    case SUB_PARAM_ST_V2:
    case SUB_PARAM_ST_V3:
    case SUB_PARAM_ST_V4:
    case SUB_PARAM_ST_V5:
    case SUB_PARAM_ST_V6:
        return UNIT_MV;
    default:
        break;
    }

    return UNIT_BPM;
}

/**************************************************************************************************
 * 设置趋势窗体。
 *************************************************************************************************/
void ECGDupParam::setTrendWidget(ECGTrendWidget *trendWidget)
{
    if (trendWidget == NULL)
    {
        return;
    }
    _trendWidget = trendWidget;
}

void ECGDupParam::updatePR(short pr, PRSourceType type)
{
    if (type == PR_SOURCE_SPO2)
    {
        _prValueFromSPO2 = pr;
    }
    else if (type == PR_SOURCE_IBP)
    {
        _prValueFromIBP = pr;
    }

    bool isIBP1LeadOff = ibpParam.isIBPLeadOff(IBP_INPUT_1);
    bool isIBP2LeadOff = ibpParam.isIBPLeadOff(IBP_INPUT_2);
    bool isSPO2Valid = spo2Param.isValid();

    // 更新pr值
    switch (_prSource)
    {
        case PR_SOURCE_AUTO:
        {
            if (isSPO2Valid && _prValueFromSPO2 != InvData())
            {
                _prValue = _prValueFromSPO2;
            }
            else if ((!isIBP1LeadOff || !isIBP2LeadOff)
                 && (_prValueFromIBP != InvData()))
            {
                _prValue = _prValueFromIBP;
            }
            else
            {
                _prValue = InvData();
            }
        }
        break;
        case PR_SOURCE_SPO2:
        {
            if (isSPO2Valid)
            {
                _prValue = _prValueFromSPO2;
            }
            else
            {
                _prValue = InvData();
            }
        }
        break;
        case PR_SOURCE_IBP:
        {
            if (!isIBP1LeadOff || !isIBP2LeadOff)
            {
                _prValue = _prValueFromIBP;
            }
            else
            {
                _prValue = InvData();
            }
        }
        break;
    }

    if (_trendWidget == NULL)
    {
        return;
    }

    // 参数界面上更新pr值
    switch (_hrSource)
    {
        case HR_SOURCE_AUTO:
        {
            if (_hrValue != InvData())
            {
                return;
            }
            if (_prValue != InvData())
            {
                _trendWidget->setHRValue(_prValue, false);
            }
            else
            {
                _trendWidget->setHRValue(_prValue, true);
            }
            _hrBeatFlag = false;
        }
        break;
        case HR_SOURCE_IBP:
        case HR_SOURCE_SPO2:
        {
            _hrBeatFlag = false;
            _trendWidget->setHRValue(_prValue, false);
        }
        break;
        case HR_SOURCE_ECG:
        case HR_SOURCE_NR:
        break;
    }
}

/**************************************************************************************************
 * 更新VFVT数值。
 *************************************************************************************************/
void ECGDupParam::updateVFVT(bool onoff)
{
    Q_UNUSED(onoff)
    return;
}

/**************************************************************************************************
 * 更新HR心跳图标。
 *************************************************************************************************/
void ECGDupParam::updateHRBeatIcon()
{
    if (_hrBeatFlag)
    {
        if (NULL != _trendWidget && _hrValue != InvData())
        {
            _trendWidget->blinkBeatPixmap();
        }
    }
}

/**************************************************************************************************
 * 更新PR心跳图标。
 *************************************************************************************************/
void ECGDupParam::updatePRBeatIcon()
{
    if (!_hrBeatFlag)
    {
        if (NULL != _trendWidget && _prValue != InvData())
        {
            _trendWidget->blinkBeatPixmap();
        }
    }
}

/**************************************************************************************************
 * 更新HR的值。
 *************************************************************************************************/
void ECGDupParam::updateHR(short hr)
{
    _hrValue = hr;

    if (_trendWidget == NULL)
    {
        return;
    }

    switch (_hrSource)
    {
        case HR_SOURCE_AUTO:
        {
            // HR不为无效时即显示。
            if (_hrValue != InvData())
            {
                _hrBeatFlag = true;
                _trendWidget->setHRValue(_hrValue, true);
            }
            else if (_prValue != InvData())
            {
                _hrBeatFlag = false;
                _trendWidget->setHRValue(_prValue, false);
            }
            else  // HR和PR都为无效时。
            {
                _hrBeatFlag = true;
                _trendWidget->setHRValue(_hrValue, true);
            }
        }
        break;
        case HR_SOURCE_IBP:
        case HR_SOURCE_SPO2:
        {
            _hrBeatFlag = false;
            _trendWidget->setHRValue(_prValue, false);
        }
        break;
        case HR_SOURCE_ECG:
        {
            if (_hrValue != InvData())
            {
                _hrBeatFlag = true;
            }
            else
            {
                _hrBeatFlag = false;
            }
            _trendWidget->setHRValue(_hrValue, true);
        }
        case HR_SOURCE_NR:
        break;
    }
}

/**************************************************************************************************
 * 获取HR的值。
 *************************************************************************************************/
short ECGDupParam::getHR(void) const
{
    if (InvData() != _hrValue)
    {
        return _hrValue;
    }

    if (InvData() != _prValue)
    {
        return _prValue;
    }

    return InvData();
}

/**************************************************************************************************
 * 是否为HR有效。
 *************************************************************************************************/
bool ECGDupParam::isHRValid(void)
{
    if (_hrValue != InvData())
    {
        return true;
    }

    if (_prValue == InvData())
    {
        return true;
    }

    return false;
}

/**************************************************************************************************
 * 是否报警。
 *************************************************************************************************/
void ECGDupParam::isAlarm(bool isAlarm, bool isLimit)
{
    _isAlarm |= isAlarm;
    if (isLimit)
    {
        return;
    }

    if (NULL != _trendWidget)
    {
        _trendWidget->isAlarm(_isAlarm);
        _isAlarm = false;
    }
}

/***************************************************************************************************
 * get the hr source
 **************************************************************************************************/
HRSourceType ECGDupParam::getCurHRSource() const
{
    if (_hrSource == HR_SOURCE_AUTO)
    {
        HRSourceType type = HR_SOURCE_AUTO;
        if (_hrValue == InvData() && _prValue != InvData())
        {
            if (_prSource == PR_SOURCE_SPO2)
            {
                type = HR_SOURCE_SPO2;
            }
            else if (_prSource == PR_SOURCE_IBP)
            {
                type = HR_SOURCE_IBP;
            }
        }
        else
        {
            type = HR_SOURCE_ECG;
        }
        return type;
    }

    return _hrSource;
}

void ECGDupParam::onPaletteChanged(ParamID id)
{
    if (id != PARAM_ECG)
    {
        return;
    }
    QPalette pal = colorManager.getPalette(paramInfo.getParamName(PARAM_ECG));
    _trendWidget->updatePalette(pal);
}
void ECGDupParam::setHrSource(HRSourceType type)
{
    _hrSource = type;

    // 更新pr来源选择
    if (type == HR_SOURCE_SPO2)
    {
        _prSource = PR_SOURCE_SPO2;
    }
    else if (type == HR_SOURCE_IBP)
    {
        _prSource = PR_SOURCE_IBP;
    }
    else
    {
        _prSource = PR_SOURCE_AUTO;
    }
    currentConfig.setNumValue("ECG|HRSource", static_cast<int>(type));
}

bool ECGDupParam::isAutoTypeHrSouce() const
{
    if (_hrSource == HR_SOURCE_AUTO)
    {
        return true;
    }
    else
    {
        return false;
    }
}

QList<SubParamID> ECGDupParam::getShortTrendList()
{
    return _trendWidget->getShortTrendSubParams();
}

void ECGDupParam::updateSubParamLimit(SubParamID id)
{
    if (id == SUB_PARAM_HR_PR)
    {
        _trendWidget->updateLimit();
    }
}

/**************************************************************************************************
 * 构造。
 *************************************************************************************************/
ECGDupParam::ECGDupParam()
    : Param(PARAM_DUP_ECG),
      _provider(NULL),
      _trendWidget(NULL),
      _hrValue(InvData()),
      _prValue(InvData()),
      _prValueFromSPO2(InvData()),
      _prValueFromIBP(InvData()),
      _hrBeatFlag(true),
      _isAlarm(false),
      _hrSource(HR_SOURCE_ECG),
      _prSource(PR_SOURCE_AUTO)
{
}

/**************************************************************************************************
 * 析构。
 *************************************************************************************************/
ECGDupParam::~ECGDupParam()
{
}
