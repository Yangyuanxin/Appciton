/**
 ** This file is part of the nPM project.
 ** Copyright (C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by Bingyun Chen <chenbingyun@blmed.cn>, 2018/8/27
 **/

#pragma once
#include <QMap>
#include <QString>
#include "ParamDefine.h"
#include "UnitManager.h"

class Provider;
class BLMProvider;
class Param;
class ParamManager
{
public:
    static ParamManager &construction(void)
    {
        if (_selfObj == NULL)
        {
            _selfObj = new ParamManager();
        }
        return *_selfObj;
    }
    static ParamManager *_selfObj;

    // 将Provider和参数对象加入进来。
    void addProvider(Provider &provider);
    void addParam(Param &param);
    /**
     * @brief getProvider get a provider base on the provider name
     * @param name provider name
     * @return pointer to the provider or NULL if not found
     */
    Provider *getProvider(const QString &name);

    // 获取系统的Params。
    void getParams(QList<Param*> &params);
    void getParams(QList<ParamID> &paramID);

    // 将数据生产者和参数对象关联起来。
    void connectParamProvider(void);

    // connect the paramters to the demo provider
    void connectDemoParamProvider(void);

    // 获取子参数值。
    short getSubParamValue(const QString &paramName, SubParamID id);
    short getSubParamValue(ParamID paramID, SubParamID id);

    // check whether a param is avaliable
    bool isSubParamAvaliable(ParamID paramID, SubParamID id);

    // 显示子参数值
    void showSubParamValue();

    // 获取子参数的当前单位。
    UnitType getSubParamUnit(ParamID, SubParamID id);

    // 参数是否有效。
    bool isParamEnable(ParamID id);

    // check whether the param is stopped
    bool isParamStopped(ParamID id);

    // 检查Proider的连接状态。
    void checkProviderConnection(void);

    // 获取版本号。Notify the providers to get version info
    void getVersion(void);

    // get the BLMProvider by paramID
    BLMProvider *getBLMProvider(ParamID paramId);


    // 析构。
    ~ParamManager();

private:
    ParamManager();

    typedef QMap<QString, Provider*> ProviderMap;
    typedef QMap<QString, Param*> ParamMap;
    typedef QMap<ParamID, Param*> ParamWithIDMap;
    typedef QMap<ParamID, BLMProvider*> BLMProviderMap;
    ProviderMap _providers;
    ParamMap _params;
    ParamWithIDMap _paramWithID;
    BLMProviderMap _blmProviders;
};
#define paramManager (ParamManager::construction())
#define deleteParamManager() (delete ParamManager::_selfObj)
