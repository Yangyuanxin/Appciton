/**
 ** This file is part of the nPM project.
 ** Copyright (C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by Bingyun Chen <chenbingyun@blmed.cn>, 2018/8/17
 **/

#include "AlarmPauseState.h"
#include "AlarmIndicator.h"
#include "AlarmStateMachine.h"
#include "LightManager.h"
#include <QTimerEvent>
#include "IConfig.h"

class AlarmPauseStatePrivate
{
public:
    AlarmPauseStatePrivate()
        :leftPauseTime(-1)
    {}

    /**
     * @brief getAlarmPausetime convert the Alarm Pause Time to seconds
     * @param time time
     * @return the corresponse seconds
     */
    int getAlarmPausetime(AlarmPauseTime time)
    {
        switch (time)
        {
        case ALARM_PAUSE_TIME_60S:
            return 60;
        case ALARM_PAUSE_TIME_90S:
            return 90;
        case ALARM_PAUSE_TIME_120S:
            return 120;
        case ALARM_PAUSE_TIME_150S:
            return 150;
        case ALARM_PAUSE_TIME_180S:
            return 180;
        case ALARM_PAUSE_TIME_210S:
            return 210;
        case ALARM_PAUSE_TIME_240S:
            return 240;
        default:
            break;
        }
        return 120;
    }

    int leftPauseTime;
};

/**************************************************************************************************
 * 构造。
 *************************************************************************************************/
AlarmPauseState::AlarmPauseState()
    : AlarmState(ALARM_PAUSE_STATE),
      d_ptr(new AlarmPauseStatePrivate())
{
}

/**************************************************************************************************
 * 析构。
 *************************************************************************************************/
AlarmPauseState::~AlarmPauseState()
{
    delete d_ptr;
}

/**************************************************************************************************
 * 状态进入。
 *************************************************************************************************/
void AlarmPauseState::enter()
{
    alarmIndicator.setAlarmStatus(ALARM_STATUS_PAUSE);
    alarmIndicator.delAllPhyAlarm();
    lightManager.enableAlarmAudioMute(true);
    beginTimer(1000);
    int index = ALARM_PAUSE_TIME_120S;
    systemConfig.getNumValue("Alarms|AlarmPauseTime", index);
    d_ptr->leftPauseTime = d_ptr->getAlarmPausetime(static_cast<AlarmPauseTime>(index));
    alarmIndicator.updateAlarmPauseTime(d_ptr->leftPauseTime);
}

void AlarmPauseState::exit()
{
    endTimer();
    d_ptr->leftPauseTime = -1;
    alarmIndicator.updateAlarmPauseTime(d_ptr->leftPauseTime);
}

/**************************************************************************************************
 * 事件处理。
 *************************************************************************************************/
void AlarmPauseState::handAlarmEvent(AlarmStateEvent event, unsigned char */*data*/, unsigned /*len*/)
{
    switch (event)
    {
#if 1
    case ALARM_STATE_EVENT_RESET_BTN_PRESSED:
    {
        // 有栓锁的报警和新的技术报警
        alarmIndicator.techAlarmPauseStatusHandle();
        if (alarmIndicator.hasLatchPhyAlarm())
        {
            alarmIndicator.delLatchPhyAlarm();
        }

        // 有处于未暂停的报警
        if (alarmIndicator.hasNonPausePhyAlarm())
        {
            alarmIndicator.phyAlarmPauseStatusHandle();
        }

        break;
    }

    case ALARM_STATE_EVENT_MUTE_BTN_PRESSED:
    {
        alarmIndicator.phyAlarmPauseStatusHandle();
        alarmStateMachine.switchState(ALARM_NORMAL_STATE);
        break;
    }
#else
    case ALARM_STATE_EVENT_MUTE_BTN_PRESSED:
    {
        // 有栓锁的报警和新的技术报警
        bool ret = alarmIndicator.techAlarmPauseStatusHandle();
        if (alarmIndicator.hasLatchPhyAlarm())
        {
            alarmIndicator.delLatchPhyAlarm();
            ret |= true;
        }

        // 有处于未暂停的报警
        if (alarmIndicator.hasNonPausePhyAlarm())
        {
            alarmIndicator.phyAlarmPauseStatusHandle();
            ret |= true;
        }

        if (ret)
        {
            return;
        }

        alarmIndicator.phyAlarmPauseStatusHandle();
        alarmStateMachine.switchState(ALARM_NORMAL_STATE);
        break;
    }
#endif

    case ALARM_STATE_EVENT_MUTE_BTN_PRESSED_SHORT_TIME:
        if (alarmStateMachine.isEnableAlarmAudioOff())
        {
            alarmStateMachine.switchState(ALARM_AUDIO_OFF_STATE);
        }
        break;

#if 0
    case ALARM_STATE_EVENT_ALL_PHY_ALARM_LATCHED:
    case ALARM_STATE_EVENT_NO_PAUSED_PHY_ALARM:
        alarmStateMachine.switchState(ALARM_NORMAL_STATE);
        break;
#endif

    case ALARM_STATE_EVENT_MUTE_BTN_PRESSED_LONG_TIME:
        if (alarmStateMachine.isEnableAlarmOff())
        {
            alarmStateMachine.switchState(ALARM_OFF_STATE);
        }
        break;

    default:
        break;
    }
}

void AlarmPauseState::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == getTimerID())
    {
        d_ptr->leftPauseTime--;
        alarmIndicator.updateAlarmPauseTime(d_ptr->leftPauseTime);
        if (d_ptr->leftPauseTime <= 0)
        {
            alarmStateMachine.switchState(ALARM_NORMAL_STATE);
        }
    }
}
