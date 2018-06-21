#pragma once
#include "SubMenu.h"
#include "LabelButton.h"


class IComboList;
class SPO2Menu : public SubMenu
{
    Q_OBJECT

public:
    static SPO2Menu &construction(void)
    {
        if (_selfObj == NULL)
        {
            _selfObj = new SPO2Menu();
        }
        return *_selfObj;
    }
    static SPO2Menu *_selfObj;

public:
    ~SPO2Menu();

protected:
    virtual void layoutExec(void);
    virtual void readyShow(void);

private slots:
    void _sensitivitySlot(int index);
    void _smartPulseToneSlot(int index);
    void _alarmLbtnSlot(void);
    void _onConfigUpdated();

private:
    SPO2Menu();

    IComboList *_sensitivity;         // 灵敏度。
    IComboList *_smartPulseTone;      // 智能脉搏音。
    LabelButton *_alarmLbtn;            //跳到报警项设置按钮
};
#define spo2Menu (SPO2Menu::construction())
