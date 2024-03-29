﻿/**
 ** This file is part of the nPM project.
 ** Copyright(C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by Bingyun Chen <chenbingyun@blmed.cn>, 2018/7/3
 **/


#include "SoftkeyActionBase.h"
#include "SoftkeyActionBase.h"
#include <QApplication>
#include "ECGParam.h"
#include "WindowManager.h"
#include "PatientInfoWindow.h"
#include "MessageBox.h"
#include "CO2Param.h"
#include "DoseCalculationManager.h"
#include "Framework/UI/MenuWindow.h"
#include "PatientManager.h"
#include "PatientInfoWindow.h"
#include "CodeMarkerWindow.h"
#include "ScreenLayoutWindow.h"
#include "MainMenuWindow.h"
#include "TrendGraphWindow.h"
#include "EventWindow.h"
#include "IBPParam.h"
#include "IConfig.h"
#include "NightModeManager.h"
#include "CalculateWindow.h"
#include "DischargePatientWindow.h"
#include "TrendTableWindow.h"
#include "Framework/Language/LanguageManager.h"
#include "LayoutManager.h"
#include "BigFontLayoutWindow.h"
#include "SoftKeyManager.h"
#include "NIBPParam.h"
#include "RecorderManager.h"
#include "TrendCache.h"
#include "TrendDataStorageManager.h"
#include "TimeManager.h"
#include "ContinuousPageGenerator.h"
#include "FreezeWindow.h"
#include "IBPZeroWindow.h"
#include "Alarm.h"
#include <QPointer>

/***************************************************************************************************
 * 所有的快捷按键定义。
 **************************************************************************************************/
static KeyActionDesc _baseKeys[] =
{
    KeyActionDesc("", "", "main.png",  SoftkeyActionBase::mainsetup
                    , SOFT_BASE_KEY_NR, true, QColor(27, 79, 147)),
    KeyActionDesc("", "Patient", "PatientInfo.png", SoftkeyActionBase::patientInfo),
    KeyActionDesc("", "Admit", "PatientNew.png", SoftkeyActionBase::patientNew),
    KeyActionDesc("", "Discharge", "PatientDischarge.png", SoftkeyActionBase::patientRelieve),
#ifdef VITAVUE_15_INCHES
    KeyActionDesc("", "AlarmReset", "AlarmResetSoftkey.png", SoftkeyActionBase::alarmReset),
    KeyActionDesc("", "AlarmPause", "AlarmPauseSoftkey.png", SoftkeyActionBase::alarmPause),
    KeyActionDesc("", "Print", "manualTirgger.png", SoftkeyActionBase::manualTirgger),
    KeyActionDesc("", "NIBP", "NIBPMeasure.png", SoftkeyActionBase::nibpMeasure),
    KeyActionDesc("", "Freeze", "freeze.png", SoftkeyActionBase::freeze),
#endif
    KeyActionDesc("", "ECGCalcLead", "LeadSelection.png", SoftkeyActionBase::ecgLeadChange),
    KeyActionDesc("", "AlarmLimitMenu", "limitSet.png", SoftkeyActionBase::limitMenu),
    KeyActionDesc("", "CodeMarker", "CodeMarker.png", SoftkeyActionBase::codeMarker),
    KeyActionDesc("", "TrendGraph", "Summary.png", SoftkeyActionBase::summaryReview),
    KeyActionDesc("", "eventReview", "Summary1.png", SoftkeyActionBase::eventReview),
    KeyActionDesc("", "TrendTable", "trend.png", SoftkeyActionBase::trendTable),
    KeyActionDesc("", "ChooseScreen", "screenSwitch.png", SoftkeyActionBase::switchSystemMode),
    KeyActionDesc("", "ScreenSetup", "interface.png",   SoftkeyActionBase::windowLayout),
#ifndef HIDE_PARAM_SWITCH
    KeyActionDesc("", "ParameterSwitch", "paraSwitch.png"),
#endif
    KeyActionDesc("", "DisableTouchScreen", "banTouch.png", SoftkeyActionBase::banTouchScreen),
#ifndef HIDE_STANDBY_FUNCTION
    KeyActionDesc("", "Standby", "standby.png", SoftkeyActionBase::standby),
#endif
    KeyActionDesc("", "CO2ZeroCalib", "calib.png", SoftkeyActionBase::CO2Zero),
    KeyActionDesc("", co2StandbyHint, co2StandbyIcon, SoftkeyActionBase::CO2Handle),
    KeyActionDesc("", trs("IBPZero"), "calib.png", SoftkeyActionBase::IBPZero),
    KeyActionDesc("", "Calculation", "dosecalculation.png", SoftkeyActionBase::calculation),
    KeyActionDesc("", "ToneVolume", "keyBoard.png", SoftkeyActionBase::keyVolume),
    KeyActionDesc("", "SystemBrightness", "Brightness.png", SoftkeyActionBase::systemBrightness),
    KeyActionDesc("", "NightMode", "nightMode.png", SoftkeyActionBase::nightMode),
    KeyActionDesc("", "PrintSetup", "printSetup.png", SoftkeyActionBase::printSet),
};

/***************************************************************************************************
 * 导联改变回调。
 **************************************************************************************************/
void SoftkeyActionBase::ecgLeadChange(bool isPressed)
{
    if (isPressed)
    {
        return;
    }
    ecgParam.autoSetCalcLead();
}

/***************************************************************************************************
 * CodeMarker回调。
 **************************************************************************************************/
void SoftkeyActionBase::codeMarker(bool isPressed)
{
    if (isPressed)
    {
        return;
    }

    bool isVisible = codeMarkerWindow.isVisible();
    while (NULL != QApplication::activeModalWidget())
    {
        QApplication::activeModalWidget()->hide();
    }

    if (isVisible)
    {
        return;
    }
    windowManager.showWindow(&codeMarkerWindow , WindowManager::ShowBehaviorCloseIfVisiable |
                             WindowManager::ShowBehaviorCloseOthers);
}

/***************************************************************************************************
 * 报警限设置回调。
 **************************************************************************************************/
void SoftkeyActionBase::limitMenu(bool isPressed)
{
    if (isPressed)
    {
        return;
    }

    MainMenuWindow *p = MainMenuWindow::getInstance();
    p->popup(trs("AlarmLimitMenu"));
}

void SoftkeyActionBase::windowLayout(bool isPressed)
{
    if (isPressed)
    {
        return;
    }

    if (layoutManager.getUFaceType() == UFACE_MONITOR_BIGFONT)
    {
        windowManager.showWindow(BigFontLayoutWindow::getInstance(),
                                 WindowManager::ShowBehaviorCloseOthers);
    }
    else
    {
        windowManager.showWindow(ScreenLayoutWindow::getInstance(),
                                 WindowManager::ShowBehaviorCloseOthers);
    }
}

/***************************************************************************************************
 * 数据查看回调。
 **************************************************************************************************/
void SoftkeyActionBase::rescueData(bool isPressed)
{
    if (isPressed)
    {
        return;
    }
    MainMenuWindow *p = MainMenuWindow::getInstance();
    p->popup(trs("DataReviewMenu"));
}

void SoftkeyActionBase::calculation(bool isPressed)
{
    if (isPressed)
    {
        return;
    }
    CalculateWindow *w = CalculateWindow::getInstance();
    windowManager.showWindow(w, WindowManager::ShowBehaviorCloseOthers);
}

/***************************************************************************************************
 * 病人信息。
 **************************************************************************************************/
void SoftkeyActionBase::patientInfo(bool isPressed)
{
    if (isPressed)
    {
        return;
    }

    if (!patientManager.isMonitoring())
    {
        patientManager.newPatient();
    }
    windowManager.showWindow(&patientInfoWindow , WindowManager::ShowBehaviorCloseOthers);
}

void SoftkeyActionBase::patientNew(bool isPressed)
{
    if (isPressed)
    {
        return;
    }
    bool isVisible = patientInfoWindow.isVisible();

    while (NULL != QApplication::activeModalWidget())
    {
        QApplication::activeModalWidget()->hide();
    }

    if (isVisible)
    {
        return;
    }

    // 创建新病人
    // patientMenu.createPatient();
    QStringList slist;
    slist << trs("No") << trs("Yes");
    MessageBox messageBox(trs("Warn"), trs("DischargeAndRecePatient"), slist, true);
    if (messageBox.exec() == 1)
    {
        patientManager.newPatient();
        windowManager.showWindow(&patientInfoWindow , WindowManager::ShowBehaviorCloseOthers);
    }
}

void SoftkeyActionBase::patientRelieve(bool isPressed)
{
    if (isPressed)
    {
        return;
    }
    DischargePatientWindow dischargeWin;
    if (dischargeWin.exec() == QDialog::Accepted)
    {
        patientManager.dischargePatient();
    }
}

void SoftkeyActionBase::mainsetup(bool isPressed)
{
    if (isPressed)
    {
        return;
    }

    MainMenuWindow *w = MainMenuWindow::getInstance();
    windowManager.showWindow(w, WindowManager::ShowBehaviorCloseOthers);

    // 每次打开主界面时，强制聚焦在首个item
    // 需要放在showWindow下面
    w->focusFirstMenuItem();
}

void SoftkeyActionBase::banTouchScreen(bool isPressed)
{
    if (isPressed)
    {
        return;
    }

#ifdef Q_WS_QWS
    bool isOn = systemManager.isTouchScreenOn();
    QString iconPath, hint;
    if (isOn)
    {
        iconPath = QString("banTouch.png");
        hint = "DisableTouchScreen";
    }
    else
    {
        iconPath = QString("touch.png");
        hint = "EnabledTouchScreen";
    }
    _baseKeys[SOFT_BASE_KEY_SCREEN_BAN].iconPath = iconPath;
    _baseKeys[SOFT_BASE_KEY_SCREEN_BAN].hint = hint;
    systemManager.setTouchScreenOnOff(!isOn);
    softkeyManager.refreshPage(false);
#endif
}

void SoftkeyActionBase::trendTable(bool isPressed)
{
    if (isPressed)
    {
        return;
    }

    TrendTableWindow::getInstance()->setHistoryData(false);
    windowManager.showWindow(TrendTableWindow::getInstance(),
                             WindowManager::ShowBehaviorHideOthers |
                             WindowManager::ShowBehaviorNoAutoClose);
}

void SoftkeyActionBase::switchSystemMode(bool isPressed)
{
    if (isPressed)
    {
        return;
    }
    MainMenuWindow *w = MainMenuWindow::getInstance();
    w->popup(trs("ScreenSetting"));
}

/***************************************************************************************************
 * summary回顾回调。
 **************************************************************************************************/
void SoftkeyActionBase::summaryReview(bool isPressed)
{
    if (isPressed)
    {
        return;
    }

    TrendGraphWindow::getInstance()->setHistoryData(false);
    windowManager.showWindow(TrendGraphWindow::getInstance(),
                             WindowManager::ShowBehaviorCloseIfVisiable
                             | WindowManager::ShowBehaviorCloseOthers
                             | WindowManager::ShowBehaviorNoAutoClose);
}

void SoftkeyActionBase::eventReview(bool isPressed)
{
    if (isPressed)
    {
        return;
    }

    EventWindow::getInstance()->setHistoryData(false);
    windowManager.showWindow(EventWindow::getInstance(), WindowManager::ShowBehaviorCloseIfVisiable
                             | WindowManager::ShowBehaviorCloseOthers);
}

void SoftkeyActionBase::standby(bool isPressed)
{
    if (isPressed)
    {
        return;
    }
    systemManager.setWorkMode(WORK_MODE_STANDBY);
}

void SoftkeyActionBase::CO2Zero(bool isPressed)
{
    if (isPressed)
    {
        return;
    }
    co2Param.zeroCalibration();
}

void SoftkeyActionBase::CO2Handle(bool isPressed)
{
    if (isPressed)
    {
        return;
    }

    if (!co2Param.getCO2Switch())
    {
        co2Param.setModuleWorkMode(CO2_WORK_MEASUREMENT);
    }
    else
    {
        co2Param.setModuleWorkMode(C02_WORK_SLEEP);
    }
}

void SoftkeyActionBase::IBPZero(bool isPressed)
{
    if (isPressed)
    {
        return;
    }

    windowManager.closeAllWidows();
    IBPZeroWindow w;
    w.exec();
}

void SoftkeyActionBase::systemBrightness(bool isPressed)
{
    if (isPressed)
    {
        return;
    }
    if (!nightModeManager.nightMode())
    {
        MainMenuWindow *w = MainMenuWindow::getInstance();
        w->popup(trs("NormalFunctionMenu"), qVariantFromValue(QString("SystemBrightness")));
    }
}

void SoftkeyActionBase::keyVolume(bool isPressed)
{
    if (isPressed)
    {
        return;
    }

    if (!nightModeManager.nightMode())
    {
        MainMenuWindow *w = MainMenuWindow::getInstance();
        w->popup(trs("NormalFunctionMenu"), qVariantFromValue(QString("ToneVolume")));
    }
}

void SoftkeyActionBase::nightMode(bool isPressed)
{
    if (isPressed)
    {
        return;
    }

    bool nightMode = nightModeManager.nightMode();

    // 根据夜间模式状态, 设置屏幕亮度和音量按键类型是否可用
    softkeyManager.setKeyTypeAvailable(SOFT_BASE_KEY_KEYBOARD_VOLUMN, nightMode);
    softkeyManager.setKeyTypeAvailable(SOFT_BASE_KEY_SCREEN_BRIGHTNESS, nightMode);

    // 设置焦点在快捷按键接口
    softkeyManager.setFocusBaseKey(SOFT_BASE_KEY_NIGHT_MODE);

    nightModeManager.setNightMode(!nightMode);
}

void SoftkeyActionBase::printSet(bool isPressed)
{
    if (isPressed)
    {
        return;
    }
    MainMenuWindow *w = MainMenuWindow::getInstance();
    w->popup(trs("PrintSettingMenu"));
}

void SoftkeyActionBase::alarmReset(bool isPressed)
{
    if (isPressed)
    {
        alertor.updateResetKeyStatus(true);
    }
    else
    {
        alertor.updateResetKeyStatus(false);
    }
}

void SoftkeyActionBase::alarmPause(bool isPressed)
{
    if (isPressed)
    {
        alertor.updateMuteKeyStatus(true);
    }
    else
    {
        alertor.updateMuteKeyStatus(false);
    }
}

void SoftkeyActionBase::manualTirgger(bool isPressed)
{
    if (isPressed)
    {
        return;
    }

    // print
    if (recorderManager.isPrinting())
    {
        recorderManager.stopPrint();
    }
    else if (!recorderManager.getPrintStatus())
    {
        unsigned t = timeManager.getCurTime();
        recorderManager.addPageGenerator(new ContinuousPageGenerator());
        trendCache.collectTrendData(t);
        trendCache.collectTrendAlarmStatus(t);
        trendDataStorageManager.storeData(t, TrendDataStorageManager::CollectStatusPrint);
    }
}

void SoftkeyActionBase::nibpMeasure(bool isPressed)
{
    if (isPressed)
    {
        // nibp 维护模式和demo模式、开机较零模式不响应nibp测量按钮
        if (systemManager.isSupport(CONFIG_NIBP) && systemManager.getCurWorkMode() != WORK_MODE_DEMO
                && !nibpParam.isMaintain() && !nibpParam.isZeroSelfTestState())
        {
            nibpParam.keyPressed();
        }
    }
    else
    {
        //  维护模式和demo模式不响应nibp测量按钮
        if (systemManager.isSupport(CONFIG_NIBP) && systemManager.getCurWorkMode() != WORK_MODE_DEMO
                && !nibpParam.isMaintain() && !nibpParam.isZeroSelfTestState())
        {
            nibpParam.keyReleased();
        }
    }
}

void SoftkeyActionBase::freeze(bool isPressed)
{
    if (isPressed)
    {
        return;
    }

    static QPointer<FreezeWindow> currentFreezeWindow;

    if (layoutManager.getUFaceType() == UFACE_MONITOR_BIGFONT)
    {
        // should not freeze when in big font interface
        return;
    }

    if (currentFreezeWindow)
    {
        currentFreezeWindow->done(0);
        return;
    }

    while (NULL != QApplication::activeModalWidget())
    {
        QApplication::activeModalWidget()->close();
    }

    windowManager.closeAllWidows();
    FreezeWindow freezeWindow;
    currentFreezeWindow = &freezeWindow;
    freezeWindow.exec();
}

/***************************************************************************************************
 * 获取动作描述总个数。
 **************************************************************************************************/
int SoftkeyActionBase::getActionDescNR(void)
{
    return 0;
}

/***************************************************************************************************
 * 获取动作描述句柄。
 **************************************************************************************************/
KeyActionDesc *SoftkeyActionBase::getActionDesc(int index)
{
    if (index > SOFT_BASE_KEY_NR)
    {
        return NULL;
    }

    return &_baseKeys[index];
}

KeyActionDesc *SoftkeyActionBase::getBaseActionDesc(SoftBaseKeyType baseType)
{
#ifdef Q_WS_QWS
    if (baseType == SOFT_BASE_KEY_SCREEN_BAN)
    {
        bool isOn = systemManager.isTouchScreenOn();
        QString iconPath, hint;
        if (isOn)
        {
            iconPath = QString("touch.png");
            hint = trs("EnabledTouchScreen");
        }
        else
        {
            iconPath = QString("banTouch.png");
            hint = trs("DisableTouchScreen");
        }
        _baseKeys[baseType].iconPath = iconPath;
        _baseKeys[baseType].hint = hint;
    }
#endif
    if (baseType == SOFT_BASE_KEY_CO2_HANDLE)
    {
        // 初始化co2待机测量的图标和指示信息
        QString iconPath, hint;
        if (co2Param.getCO2Switch())
        {
            iconPath = QString(co2MeasureIcon);
            hint = co2MeasureHint;
        }
        else
        {
            iconPath = QString(co2StandbyIcon);
            hint = co2StandbyHint;
        }
        _baseKeys[baseType].iconPath = iconPath;
        _baseKeys[baseType].hint = hint;
    }
    return &_baseKeys[baseType];
}

/***************************************************************************************************
 * 获取类型。
 **************************************************************************************************/
SoftKeyActionType SoftkeyActionBase::getType(void)
{
    return _type;
}

/***************************************************************************************************
 * 构造。
 **************************************************************************************************/
SoftkeyActionBase::SoftkeyActionBase(SoftKeyActionType t)
    :_type(t)
{
}

/***************************************************************************************************
 * 析构。
 **************************************************************************************************/
SoftkeyActionBase::~SoftkeyActionBase()
{
}

