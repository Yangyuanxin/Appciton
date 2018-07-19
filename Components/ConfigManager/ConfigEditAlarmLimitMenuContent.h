/**
 ** This file is part of the nPM project.
 ** Copyright (C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by ZhongHuan Duan duanzhonghuan@blmed.cn, 2018/7/19
 **/

#pragma once
#include "MenuContent.h"

class ConfigEditAlarmLimitMenuContentPrivate;

class ConfigEditAlarmLimitMenuContent : public MenuContent
{
    Q_OBJECT
public:
    ConfigEditAlarmLimitMenuContent();
    ~ConfigEditAlarmLimitMenuContent();

    /* reimplement */
    void readyShow();

    /* reimplement */
    void layoutExec();

private slots:
    void onbtnClick();
    void onRowClicked(int row);
    void onSelectRowChanged(int row);

private:
    ConfigEditAlarmLimitMenuContentPrivate *const d_ptr;
};

