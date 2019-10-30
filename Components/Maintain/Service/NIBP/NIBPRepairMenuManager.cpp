/**
 ** This file is part of the nPM project.
 ** Copyright(C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by ZhongHuan Duan duanzhonghuan@blmed.cn, 2018/9/12
 **/



#include "NIBPRepairMenuManager.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "LabelButton.h"
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QPainter>
#include <QBitmap>
#include <QScrollArea>
#include <QScrollBar>
#include "ColorManager.h"
#include "Debug.h"
#include "FontManager.h"

#include "NIBPParam.h"



NIBPRepairMenuManager *NIBPRepairMenuManager::_selfObj = NULL;

/**************************************************************************************************
 * 功能：初始化。
 *************************************************************************************************/
void NIBPRepairMenuManager::init()
{
    _repairError = false;
    _replyFlag = false;

    // 进入服务模式。
    nibpParam.changeMode(NIBP_STATE_MACHINE_SERVICE);
}

/**************************************************************************************************
 * 进入维护模式的应答。
 *************************************************************************************************/
void NIBPRepairMenuManager::unPacket(bool flag)
{
    _replyFlag = flag;
    if (flag == false)
    {
        messageBox();
    }
}

/**************************************************************************************************
 * 返回列表。
 *************************************************************************************************/
void NIBPRepairMenuManager::returnMenu()
{
}

/**************************************************************************************************
 * 提示框显示。
 *************************************************************************************************/
void NIBPRepairMenuManager::messageBox(void)
{
    messageBoxWait->exec();
}

/**************************************************************************************************
 * 警告框显示。
 *************************************************************************************************/
void NIBPRepairMenuManager::warnShow(bool enable)
{
    // 模块硬件出错
    if (nibpParam.isErrorDisable())
    {
        messageBoxError->exec();
    }
    // 模块通信错误
    else
    {
        if (enable)
        {
            if (messageBoxWait != NULL && messageBoxWait->isVisible())
            {
                messageBoxWait->close();
            }
            messageBoxError->exec();
        }
        else
        {
            if (messageBoxError != NULL && messageBoxError->isVisible())
            {
                messageBoxError->close();
            }
        }
    }

    _repairError = enable;
}

void NIBPRepairMenuManager::_closeSlot()
{
    // 转换到状态，发送退出服务模式
    nibpParam.handleNIBPEvent(NIBP_EVENT_SERVICE_REPAIR_RETURN, NULL, 0);
}

bool NIBPRepairMenuManager::getRepairError(void)
{
    return _repairError;
}

void NIBPRepairMenuManager::setMonitorState(NIBPMonitorStateID id)
{
    monitorState = id;
}

NIBPMonitorStateID NIBPRepairMenuManager::getMonitorState()
{
    return monitorState;
}

/**************************************************************************************************
 * 构造。
 *************************************************************************************************/
NIBPRepairMenuManager::NIBPRepairMenuManager()
{
    _repairError = false;
    _replyFlag = false;
    monitorState = NIBP_MONITOR_STANDBY_STATE;
    messageBoxWait = new  MessageBox(trs("Warn"), trs("NIBPServiceWaitAgain"),
                                     QStringList(trs("EnglishYESChineseSURE")));
    messageBoxError = new MessageBox(trs("Warn"), trs("NIBPServiceModuleErrorQuitTryAgain"),
                                     QStringList(trs("EnglishYESChineseSURE")), true);
}

/**************************************************************************************************
 * 析构。
 *************************************************************************************************/
NIBPRepairMenuManager::~NIBPRepairMenuManager()
{
    delete messageBoxWait;
    delete messageBoxError;
}
