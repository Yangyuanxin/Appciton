/**
 ** This file is part of the nPM project.
 ** Copyright (C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by Bingyun Chen <chenbingyun@blmed.cn>, 2018/7/3
 **/


#pragma once
#include <QString>
#include <QList>
#include <QColor>

enum SoftKeyActionType
{
    SOFTKEY_ACTION_NONE = -1,
    SOFTKEY_ACTION_STANDARD,
    SOFTKEY_ACTION_12LEAD,
    SOFTKEY_ACTION_RESCUE_DATA,
    SOFTKEY_ACTION_DEL_RESCUE_DATA,
    SOFTKEY_ACTION_CALCULATE,
    SOFTKEY_ACTION_NR,
};

enum SoftBaseKeyType
{
    SOFT_BASE_KEY_PAT_INFO,
    SOFT_BASE_KEY_PAT_NEW,
    SOFT_BASE_KEY_PREVIOUS_PAGE,
    SOFT_BASE_KEY_ALARM_LIMIT,
    SOFT_BASE_KEY_CODE_MARKER,
    SOFT_BASE_KEY_RESCUE_DATA,
    SOFT_BASE_KEY_WINDOWLAYOUT,
    SOFT_BASE_KEY_CALCULATION,
    SOFT_BASE_KEY_NEXT_PAGE,
    SOFT_BASE_KEY_MAIN_SETUP,
    SOFT_BASE_KEY_NR
};

typedef void (*SoftkeyHook)(bool);  // 参数为bool值，按下为1，弹起为0。
struct KeyActionDesc
{
    KeyActionDesc(const QString &txt = QString(), const QString &path = QString(),
                  SoftkeyHook phook = NULL, SoftBaseKeyType type = SOFT_BASE_KEY_NR,
                  bool focus = true,
                  const QColor &releaseColor = QColor(32, 32, 32),
                  const QColor &color = Qt::white,
                  const QColor &pressColor = QColor(0x1C, 0x86, 0xEE),
                  bool border = true)
        : text(txt), iconPath(path), hook(phook), type(type),
          focus(focus), color(color), pressColor(pressColor),
          releaseColor(releaseColor), border(border)
    {
    }

    bool operator !=(const KeyActionDesc &desc)
    {
        if (this->hook != desc.hook || this->iconPath != desc.iconPath ||
                this->type != desc.type)
        {
            return true;
        }

        return false;
    }

    QString text;
    QString iconPath;
    SoftkeyHook hook;
    SoftBaseKeyType type;
    bool focus;
    QColor color;
    QColor pressColor;
    QColor releaseColor;
    bool border;
};

class SoftkeyWidget;
class SoftkeyActionBase
{
public:    // 一些共有的功能处理。
    static void previousPage(bool isPressed);
    static void nextPage(bool isPressed);
    static void patientInfo(bool isPressed);
    static void patientNew(bool isPressed);
    static void ecgLeadChange(bool isPressed);
    static void codeMarker(bool isPressed);
    static void limitMenu(bool isPressed);
    static void WindowLayout(bool isPressed);
    static void rescueData(bool isPressed);
    static void calculation(bool isPressed);

    static void mainsetup(bool isPressed);

public:
    // 获取动作描述总个数。
    virtual int getActionDescNR(void);

    // 获取动作描述句柄。
    virtual KeyActionDesc *getActionDesc(int index);

    // 获取类型。
    SoftKeyActionType getType(void);

    // 构造与析构。
    explicit SoftkeyActionBase(SoftKeyActionType t);
    virtual ~SoftkeyActionBase();

protected:  // 处理翻页功能，需要派生类设置这些属性。
    // 翻到上一页。
    void previousPage(void);
    // 翻到下一页。
    void nextPage(void);

private:
    SoftKeyActionType _type;
};
