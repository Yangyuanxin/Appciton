/**
 ** This file is part of the nPM project.
 ** Copyright (C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by Bingyun Chen <chenbingyun@blmed.cn>, 2018/9/17
 **/

#pragma once
#include <QObject>

class LayoutManagerPrivate;
class QLayout;
class IWidget;
class LayoutManager : public QObject
{
public:
    /**
     * @brief ScreenLayoutManager::getInstance get the instance
     */
    LayoutManager &getInstance();

    ~LayoutManager();

    /**
     * @brief reloadLayoutConfig reload the layout info from the config file
     */
    void reloadLayoutConfig();

    /**
     * @brief getContentLayout get the content's layout
     * @return
     */
    QLayout *getContentLayout();

    /**
     * @brief addLayoutWidget add widget that need layout
     * @param w the widget
     */
    void addLayoutWidget(IWidget *w);

    /**
     * @brief getLayoutWidget get the layout widget base on the widget's name
     * @param name the layout widget's name
     * @return  the layout widget
     */
    IWidget *getLayoutWidget(const QString &name);

private:
    LayoutManagerPrivate * const d_ptr;
    LayoutManager();
};

#define layoutManger (LayoutManager::getInstance())
