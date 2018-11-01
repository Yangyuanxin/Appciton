/**
 ** This file is part of the nPM project.
 ** Copyright(C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by ZhongHuan Duan duanzhonghuan@blmed.cn, 2018/9/25
 **/

#pragma once
#include "RingBuff.h"
#include <QColor>
#include <QTimer>
#include "BaseDefine.h"
#include <QPainterPath>

#define X_SHIFT     (2)
#define Y_SHIFT     (2)
#define WX_SHIFT     (50)

class OxyCRGTrendWaveWidgetPrivate
{
public:
    OxyCRGTrendWaveWidgetPrivate()
                : dataBuf(NULL),
                  dataBufIndex(0),
                  dataBufLen(0),
                  name(""),
                  rulerHigh(InvData()),
                  rulerLow(InvData()),
                  waveColor(Qt::green),
                  waveDataRate(1),
                  timer(NULL),
                  isClearWaveData(true)
    {
    }

    virtual ~OxyCRGTrendWaveWidgetPrivate(){}

    RingBuff<short> *dataBuf;    // 波形数据缓存
    int dataBufIndex;            // 波形数据缓存下标
    int dataBufLen;              // 波形数据长度

    QString name;                // 波形名称
    int rulerHigh;               // 标尺高值
    int rulerLow;                // 标尺低值

    QColor waveColor;            // 波形颜色

    int waveDataRate;            // 波形速率

    QTimer *timer;               // 定时器用来更新波形刷新

    bool isClearWaveData;        // 是否清除波形数据

    QPainterPath backgroundRulerPath;  // 背景标尺
};

