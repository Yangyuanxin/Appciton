/**
 ** This file is part of the nPM project.
 ** Copyright (C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by WeiJuan Zhu <zhuweijuan@blmed.cn>, 2018/8/27
 **/


#pragma once
#include "WaveWidgetItem.h"

class IBPWaveWidget;

/***************************************************************************************************
 * ECG 波形控件标尺
 **************************************************************************************************/
class IBPWaveRuler : public WaveWidgetItem
{
public:
    virtual void paintItem(QPainter &painter);
    void setRuler(double up, double mid, double low);

    explicit IBPWaveRuler(IBPWaveWidget *wave);
    ~IBPWaveRuler();

private:
    double _up;             // 上标尺数值, 单位%
    double _mid;            // 中标尺数值, 单位%
    double _low;            // 下标尺数值, 单位%
};
