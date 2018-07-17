/**
 ** This file is part of the nPM project.
 ** Copyright (C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by ZhongHuan Duan duanzhonghuan@blmed.cn, 2018/7/16
 **/
#include "ConfigEditMenuWindow.h"
#include "ConfigEditAlarmLimitMenu.h"
#include "ConfigEditGeneralMenu.h"
#include "ConfigEditEcgMenu.h"
#include "ConfigEditRespMenu.h"
#include "ConfigEditNIBPMenu.h"
#include "ConfigEditSpO2Menu.h"
#include "ConfigEditCO2Menu.h"
#include "ConfigEditCOMenu.h"
#include "ConfigEditIBPMenu.h"
#include "SupervisorCodeMarker.h"
#include "ConfigEditDisplayMenu.h"
#include "ConfigEditCO2MenuContent.h"
#include "ConfigEditCOMenuContent.h"
#include "ConfigEditEcgMenuContent.h"

class ConfigEditMenuWindowPrivate
{
public:
    ConfigEditMenuWindowPrivate()
        : isSubmenuInitial(false),
          curEditConfig(NULL)
    {
        configName.clear();
        subMenuMap.clear();
    }

    bool isSubmenuInitial;
    Config *curEditConfig;
    QString configName;
    QMap <QString, MenuContent *> subMenuMap;
};

ConfigEditMenuWindow::ConfigEditMenuWindow()
    : MenuWindow(),
      d_ptr(new ConfigEditMenuWindowPrivate())
{
}

ConfigEditMenuWindow *ConfigEditMenuWindow::getInstance()
{
    static ConfigEditMenuWindow *instance = NULL;
    if (instance == NULL)
    {
        instance = new ConfigEditMenuWindow();
    }
    return instance;
}

ConfigEditMenuWindow::~ConfigEditMenuWindow()
{
}

void ConfigEditMenuWindow::initializeSubMenu()
{
    if (d_ptr->isSubmenuInitial)
    {
        return;
    }

    d_ptr->subMenuMap.clear();

    MenuContent *subMenu;

//    SubMenu *subMenu = new ConfigEditGeneralMenu();
//    addSubMenu(subMenu);
//    d_ptr->subMenuMap["ConfigEditGeneralMenu"] = subMenu;

//    subMenu = new ConfigEditAlarmLimitMenu();
//    addSubMenu(subMenu);
//    d_ptr->subMenuMap["ConfigEditAlarmLimitMenu"] = subMenu;

    subMenu = new ConfigEditECGMenuContent();
    addMenuContent(subMenu);
    d_ptr->subMenuMap["ConfigEditEcgMenu"] = subMenu;

//    subMenu = new ConfigEditRespMenu();
//    addSubMenu(subMenu);
//    d_ptr->subMenuMap["ConfigEditRespMenu"] = subMenu;

//    subMenu = new ConfigEditSpO2Menu();
//    addSubMenu(subMenu);
//    d_ptr->subMenuMap["ConfigEditSpO2Menu"] = subMenu;

//    subMenu = new ConfigEditNIBPMenu();
//    addSubMenu(subMenu);
//    d_ptr->subMenuMap["ConfigEditNIBPMenu"] = subMenu;

    subMenu = new ConfigEditCOMenuContent();
    addMenuContent(subMenu);
    d_ptr->subMenuMap["ConfigCOMenu"] = subMenu;

    subMenu = new ConfigEditCO2MenuContent();
    addMenuContent(subMenu);
    d_ptr->subMenuMap["ConfigCO2Menu"] = subMenu;

//    subMenu = new ConfigIBPMenu();
//    addSubMenu(subMenu);
//    d_ptr->subMenuMap["ConfigIBPMenu"] = subMenu;

//    subMenu = new SupervisorCodeMarker();
//    addSubMenu(subMenu);
//    d_ptr->subMenuMap["SupervisorCodeMarker"] = subMenu;

//    subMenu = new ConfigEditDisplayMenu();
//    addSubMenu(subMenu);
//    d_ptr->subMenuMap["ConfigEditDisplayMenu"] = subMenu;


    d_ptr->isSubmenuInitial = true;
}

void ConfigEditMenuWindow::setCurrentEditConfig(Config *config)
{
    d_ptr->curEditConfig = config;
}

Config *ConfigEditMenuWindow::getCurrentEditConfig() const
{
    return d_ptr->curEditConfig;
}

void ConfigEditMenuWindow::setCurrentEditConfigName(const QString &name)
{
    d_ptr->configName = name;
}

QString ConfigEditMenuWindow::getCurrentEditConfigName() const
{
    return d_ptr->configName;
}

QMap <QString, MenuContent *> ConfigEditMenuWindow::getCurrentEditConfigItem() const
{
    return d_ptr->subMenuMap;
}
