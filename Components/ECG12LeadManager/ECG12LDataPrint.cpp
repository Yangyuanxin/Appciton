#include <QHBoxLayout>
#include "DataStorageDirManager.h"
#include "ECG12LDataPrint.h"
#include "PrintManager.h"
#include "LanguageManager.h"


#define ECG12L_RESCUR_ITEM_HEIGHT (30)

ECG12LDataPrint *ECG12LDataPrint::_selfObj = NULL;

/**********************************************************************************************************************
 * 构造。
 **********************************************************************************************************************/
ECG12LDataPrint::ECG12LDataPrint() : QObject()
{
    _isRealTime = false;
    _isPrint = false;
}

/**********************************************************************************************************************
 * 析构。
 **********************************************************************************************************************/
ECG12LDataPrint::~ECG12LDataPrint()
{

}

/**************************************************************************************************
 * 12导快照数据打印
 *************************************************************************************************/
void ECG12LDataPrint::PrintECG12LData(ECG12LDataStorage::ECG12LeadData *data)
{
    if (NULL != data)
    {
        printManager.requestPrint(PRINT_TYPE_TRIGGER_12L_ECG, PRINT_LAYOUY_ID_12L_ECG,data,
                sizeof(ECG12LDataStorage::ECG12LeadData));
    }

    return ;
}

/**********************************************************************************************************************
 * 绘制打印页
 **********************************************************************************************************************/
void ECG12LDataPrint::drawPrintPage()
{

}

/**********************************************************************************************************************
 * 是否正在打印。
 **********************************************************************************************************************/
bool ECG12LDataPrint::isPrint()
{
    return _isPrint;
}

/**********************************************************************************************************************
//设置是否为实时打印
 **********************************************************************************************************************/
void ECG12LDataPrint::setPrintRealTime(bool isRealTime)
{
    _isRealTime = isRealTime;
    return ;
}

/**********************************************************************************************************************
//获取是否为实时打印
 **********************************************************************************************************************/
bool ECG12LDataPrint::getPrintRealTime()
{
    return _isRealTime;
}
