/**
 ** This file is part of the nPM project.
 ** Copyright (C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by luoyuchun <luoyuchun@blmed.cn>, 2018/8/27
 **/

#pragma once
#include "AlarmParamIFace.h"
#include "IBPSymbol.h"

// overrun alarm.
class IBPLimitAlarm : public AlarmLimitIFace
{
public:
    static IBPLimitAlarm &construction(void)
    {
        if (_selfObj == NULL)
        {
            _selfObj = new IBPLimitAlarm();
        }
        return *_selfObj;
    }
    static IBPLimitAlarm *_selfObj;

public:
    virtual QString getAlarmSourceName(void);

    virtual ParamID getParamID(void) { return PARAM_IBP; }

    // alarm source number.
    virtual int getAlarmSourceNR(void);

    // alarm id corresponding to waveform ID
    virtual WaveformID getWaveformID(int id);

    // alarm id corresponding to param ID
    virtual SubParamID getSubParamID(int id);

    // alarm id corresponding to alarm priority
    virtual AlarmPriority getAlarmPriority(int id);

    virtual int getValue(int id);

    // alarm enable
    virtual bool isAlarmEnable(int id);

    // alarm upper limit
    virtual int getUpper(int id);

    // alarm lower limit
    virtual int getLower(int id);

    virtual int getCompare(int value, int id);

    // alarm id convert string.
    virtual const char *toString(int id);

    // alarm notify
    virtual void notifyAlarm(int id, bool flag);

    // constructor and destructor.
    IBPLimitAlarm();
    virtual ~IBPLimitAlarm();
private:
    bool _isARTSYSAlarm;
    bool _isARTDIAAlarm;
    bool _isARTMAPAlarm;
    bool _isPASYSAlarm;
    bool _isPADIAAlarm;
    bool _isPAMAPAlarm;
    bool _isCVPMAPAlarm;
    bool _isLAPMAPAlarm;
    bool _isRAPMAPAlarm;
    bool _isICPMAPAlarm;
    bool _isAUXP1SYSAlarm;
    bool _isAUXP1DIAAlarm;
    bool _isAUXP1MAPAlarm;
    bool _isAUXP2SYSAlarm;
    bool _isAUXP2DIAAlarm;
    bool _isAUXP2MAPAlarm;
};
#define ibpLimitAlarm (IBPLimitAlarm::construction())

// one shot alarm
class IBPOneShotAlarm : public AlarmOneShotIFace
{
public:
    static IBPOneShotAlarm &construction(void)
    {
        if (_selfObj == NULL)
        {
            _selfObj = new IBPOneShotAlarm();
        }
        return *_selfObj;
    }
    static IBPOneShotAlarm *_selfObj;

public:
    virtual QString getAlarmSourceName(void);

    virtual ParamID getParamID(void) { return PARAM_IBP; }

    virtual int getAlarmSourceNR(void);

    virtual WaveformID getWaveformID(int id);

    virtual SubParamID getSubParamID(int id);

    virtual AlarmPriority getAlarmPriority(int id);

    virtual AlarmType getAlarmType(int id);

    virtual bool isAlarmed(int id);

    virtual void notifyAlarm(int id, bool isAlarm);

    virtual const char *toString(int id);

    virtual bool isAlarmEnable(int id);

    virtual bool isRemoveAfterLatch(int id);

    // constructor and destructor.
    virtual ~IBPOneShotAlarm();

private:
    IBPOneShotAlarm();
};
#define ibpOneShotAlarm (IBPOneShotAlarm::construction())
