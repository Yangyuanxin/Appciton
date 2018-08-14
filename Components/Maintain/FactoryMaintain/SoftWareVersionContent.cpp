/**
 ** This file is part of the nPM project.
 ** Copyright (C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by ZhongHuan Duan duanzhonghuan@blmed.cn, 2018/7/20
 **/
#include "SoftWareVersionContent.h"
#include <QGridLayout>
#include <QLabel>
#include <QMap>
#include "IConfig.h"
#include <QProcess>

class SoftWareVersionContentPrivate
{
public:
    enum MenuItem
    {
        ITEM_LAB_SYS_VER = 0,
        ITEM_LAB_PMG_VER,
        ITEM_LAB_U_BOOT,
        ITEM_LAB_KERNEL,
        ITEM_LAB_KEYBD_MOD,
        ITEM_LAB_RECOD_MOD,
        ITEM_LAB_ECG_ALGHTP,
        ITEM_LAB_BOOTSTD_LOGOVER,
        ITEM_LAB_MAX,
    };

    SoftWareVersionContentPrivate();
    /**
     * @brief loadOptions
     */
    void loadOptions();

    QMap <MenuItem, QLabel *> labs;
};

SoftWareVersionContentPrivate::SoftWareVersionContentPrivate()
{
    labs.clear();
}

void SoftWareVersionContentPrivate::loadOptions()
{

    QString outPut;
    QProcess process;
    outPut.clear();
    process.start("cat /proc/version");

    if (process.waitForFinished())
    {
        QByteArray byteArray = process.readAll();
        outPut = byteArray;
    }
    QStringList versionList = outPut.split(" ");
    if (versionList.count() >= 3)
    {
        labs[ITEM_LAB_KERNEL]->setText(trs(versionList.at(2)));
    }

    QString tmpStr;

    tmpStr.clear();
    systemConfig.getStrValue("SoftWareVersion|SystemSoftwareVersion", tmpStr);
    labs[ITEM_LAB_SYS_VER]->setText(trs(tmpStr));

    tmpStr.clear();
    systemConfig.getStrValue("SoftWareVersion|PowerManagerSoftwareVersion", tmpStr);
    labs[ITEM_LAB_PMG_VER]->setText(trs(tmpStr));

    tmpStr.clear();
    systemConfig.getStrValue("SoftWareVersion|Uboot", tmpStr);
    labs[ITEM_LAB_U_BOOT]->setText(trs(tmpStr));

    tmpStr.clear();
    systemConfig.getStrValue("SoftWareVersion|KeyboardModule", tmpStr);
    labs[ITEM_LAB_KEYBD_MOD]->setText(trs(tmpStr));

    tmpStr.clear();
    systemConfig.getStrValue("SoftWareVersion|RecorderModule", tmpStr);
    labs[ITEM_LAB_RECOD_MOD]->setText(trs(tmpStr));

    tmpStr.clear();
    systemConfig.getStrValue("SoftWareVersion|ECGAlgorithmType", tmpStr);
    labs[ITEM_LAB_ECG_ALGHTP]->setText(trs(tmpStr));

    tmpStr.clear();
    systemConfig.getStrValue("SoftWareVersion|BootAndStandbyLogoVersion", tmpStr);
    labs[ITEM_LAB_BOOTSTD_LOGOVER]->setText(trs(tmpStr));
}

SoftWareVersionContent::SoftWareVersionContent():
    MenuContent(trs("SoftWareVersionMenu"),
                trs("SoftWareVersionMenuDesc")),
    d_ptr(new SoftWareVersionContentPrivate)
{
}

SoftWareVersionContent::~SoftWareVersionContent()
{
    delete d_ptr;
}

void SoftWareVersionContent::readyShow()
{
    d_ptr->loadOptions();
}

void SoftWareVersionContent::layoutExec()
{
    if (layout())
    {
        return;
    }

    QGridLayout *layout = new QGridLayout(this);
    layout->setMargin(10);

    QLabel *labelLeft;
    QLabel *labelRight;

    labelLeft = new QLabel(trs("SystemSoftwareVersion"));
    layout->addWidget(labelLeft, d_ptr->labs.count(), 0);
    labelRight = new QLabel("");
    layout->addWidget(labelRight, d_ptr->labs.count(), 1);
    d_ptr->labs.insert(SoftWareVersionContentPrivate
                       ::ITEM_LAB_SYS_VER, labelRight);

    labelLeft = new QLabel(trs("PowerManagerSoftwareVersion"));
    layout->addWidget(labelLeft, d_ptr->labs.count(), 0);
    labelRight = new QLabel("");
    layout->addWidget(labelRight, d_ptr->labs.count(), 1);
    d_ptr->labs.insert(SoftWareVersionContentPrivate
                       ::ITEM_LAB_PMG_VER, labelRight);

    labelLeft = new QLabel(trs("Uboot"));
    layout->addWidget(labelLeft, d_ptr->labs.count(), 0);
    labelRight = new QLabel("");
    layout->addWidget(labelRight, d_ptr->labs.count(), 1);
    d_ptr->labs.insert(SoftWareVersionContentPrivate
                       ::ITEM_LAB_U_BOOT, labelRight);

    labelLeft = new QLabel(trs("Kernel"));
    layout->addWidget(labelLeft, d_ptr->labs.count(), 0);
    labelRight = new QLabel("");
    layout->addWidget(labelRight, d_ptr->labs.count(), 1);
    d_ptr->labs.insert(SoftWareVersionContentPrivate
                       ::ITEM_LAB_KERNEL, labelRight);

    labelLeft = new QLabel(trs("KeyboardModule"));
    layout->addWidget(labelLeft, d_ptr->labs.count(), 0);
    labelRight = new QLabel("");
    layout->addWidget(labelRight, d_ptr->labs.count(), 1);
    d_ptr->labs.insert(SoftWareVersionContentPrivate
                       ::ITEM_LAB_KEYBD_MOD, labelRight);

    labelLeft = new QLabel(trs("RecorderModule"));
    layout->addWidget(labelLeft, d_ptr->labs.count(), 0);
    labelRight = new QLabel("");
    layout->addWidget(labelRight, d_ptr->labs.count(), 1);
    d_ptr->labs.insert(SoftWareVersionContentPrivate
                       ::ITEM_LAB_RECOD_MOD, labelRight);

    labelLeft = new QLabel(trs("ECGAlgorithmType"));
    layout->addWidget(labelLeft, d_ptr->labs.count(), 0);
    labelRight = new QLabel("");
    layout->addWidget(labelRight, d_ptr->labs.count(), 1);
    d_ptr->labs.insert(SoftWareVersionContentPrivate
                       ::ITEM_LAB_ECG_ALGHTP, labelRight);

    labelLeft = new QLabel(trs("BootAndStandbyLogoVersion"));
    layout->addWidget(labelLeft, d_ptr->labs.count(), 0);
    labelRight = new QLabel("");
    layout->addWidget(labelRight, d_ptr->labs.count(), 1);
    d_ptr->labs.insert(SoftWareVersionContentPrivate
                       ::ITEM_LAB_BOOTSTD_LOGOVER, labelRight);

    layout->setRowStretch(d_ptr->labs.count(), 1);
}









































