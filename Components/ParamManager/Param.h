#pragma once
#include <QString>
#include <QStringList>
#include "ParamInfo.h"
#include "BaseDefine.h"

class Param : public QObject
{
    Q_OBJECT

public:
    // 获取参数的ID。
    ParamID getParamID(void);
    const QString &getName(void);

    // 使能与禁用。
    virtual void enable(void);
    virtual void disable(void);
    bool isEnabled(void);

    virtual bool isConnected(void);

    // 初始化参数。
    virtual void initParam(void);

    // 处理DEMO数据。
    virtual void handDemoWaveform(WaveformID id, short data);
    virtual void handDemoTrendData(void);

    // 获取当前的波形控件名称。
    virtual void getAvailableWaveforms(QStringList &waveforms, QStringList &waveformShowName,
            int flag = 0);
    virtual void getTrendWindow(QString &trendWin);
    virtual void getWaveWindow(QString &waveWin);

    // 获取子参数值
    virtual short getSubParamValue(SubParamID id);

    // check whether a subparam is avaliable, always true in default implement
    virtual bool isSubParamAvaliable(SubParamID id);

    // 显示子参数值
    virtual void showSubParamValue();

    // 获取当前的单位。
    virtual UnitType getCurrentUnit(SubParamID id);

    // 构造与析构。
    Param(ParamID id);
    virtual ~Param();

private:
    ParamID _paramID;
    QString _paramName;
    bool _isEnable;
};
