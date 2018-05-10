#pragma once
#include "SubMenu.h"

class IComboList;
class LabelButton;
class ISpinBox;
class ConfigCO2Menu : public SubMenu
{
    Q_OBJECT

public:
    ConfigCO2Menu();
    static ConfigCO2Menu &construction(void)
    {
        if (_selfObj == NULL)
        {
            _selfObj = new ConfigCO2Menu();
        }
        return *_selfObj;
    }
    static ConfigCO2Menu *_selfObj;

public:
    ~ConfigCO2Menu();

protected:
    virtual void layoutExec(void);
    virtual void readyShow(void);

private slots:
//    void _apneaTimeSlot(int index);
    void _speedSlot(int index);
    void _fico2DisplaySlot(int index);
    void _o2CompenReleased(QString strValue);
    void _n2oCompenReleased(QString strValue);
    void _zeroCalibReleased(void);

private:
    //CO2Menu();
    void _loadOptions(void);

//    IComboList *_apneaTime;              // 窒息时间。
    IComboList *_speed;                  // 波形速度。
    IComboList *_fico2Display;           // FiCO2显示开关。
    ISpinBox *_o2Compen;              // 氧气补偿。
    ISpinBox *_n2oCompen;             // 笑气补偿。
    LabelButton *_zeroCalib;             // 校零。
};