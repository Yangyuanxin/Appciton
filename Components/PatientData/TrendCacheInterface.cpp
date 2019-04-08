/**
 ** This file is part of the nPM project.
 ** Copyright (C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by WeiJuan Zhu <zhuweijuan@blmed.cn>, 2019/3/27
 **/

#include "TrendCacheInterface.h"
#include <stddef.h>

static TrendCacheInterface *trendCache = NULL;

TrendCacheInterface *TrendCacheInterface::registerTrendCache(TrendCacheInterface *instance)
{
    TrendCacheInterface *old = trendCache;
    trendCache = instance;
    return old;
}

TrendCacheInterface *TrendCacheInterface::getTrendCache()
{
    return trendCache;
}
