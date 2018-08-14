/**
 ** This file is part of the nPM project.
 ** Copyright (C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by luoyuchun <luoyuchun@blmed.cn>, 2018/7/25
 **/

#include "TrendTableSetWindow.h"
#include "LanguageManager.h"
#include "Button.h"
#include "ComboBox.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "TrendDataSymbol.h"
#include "TrendTableWindow.h"
#include "IConfig.h"
#include "WindowManager.h"

TrendTableSetWindow *TrendTableSetWindow::selfObj = NULL;

class TrendTableSetWindowPrivate
{
public:
    TrendTableSetWindowPrivate()
        : resolutionRatioCbo(NULL), trendGroupCbo(NULL), eventTypeCbo(NULL)
    {}

    ComboBox *resolutionRatioCbo;
    ComboBox *trendGroupCbo;
    ComboBox *eventTypeCbo;
};

TrendTableSetWindow::~TrendTableSetWindow()
{
}

void TrendTableSetWindow::showEvent(QShowEvent *ev)
{
    Window::showEvent(ev);

    QString prefix = "TrendTable|";
    int index = 0;

    QString ratioPrefix = prefix + "ResolutionRatio";
    systemConfig.getNumValue(ratioPrefix, index);
    d_ptr->resolutionRatioCbo->setCurrentIndex(index);

    QString groupPrefix = prefix + "TrendGroup";
    systemConfig.getNumValue(groupPrefix, index);
    d_ptr->trendGroupCbo->setCurrentIndex(index);
}

void TrendTableSetWindow::timeIntervalReleased(int t)
{
    QString prefix = "TrendTable|ResolutionRatio";
    systemConfig.setNumValue(prefix, t);
    TrendTableWindow::getInstance()->setTimeInterval(t);
}

void TrendTableSetWindow::trendGroupReleased(int g)
{
    QString prefix = "TrendTable|TrendGroup";
    systemConfig.setNumValue(prefix, g);
    TrendTableWindow::getInstance()->setTrendGroup(g);
}

void TrendTableSetWindow::incidentReleased(int type)
{
    Q_UNUSED(type)
}

TrendTableSetWindow::TrendTableSetWindow()
    : Window(), d_ptr(new TrendTableSetWindowPrivate())
{
    setWindowTitle(trs("TrendTableSet"));

    QGridLayout *gridLayout = new QGridLayout();
    QLabel *label;

    label = new QLabel(trs("ResolutionRatio"));
    d_ptr->resolutionRatioCbo = new ComboBox();
    for (int i = 0; i < RESOLUTION_RATIO_NR; i ++)
    {
        d_ptr->resolutionRatioCbo->addItem(trs(TrendDataSymbol::convert((ResolutionRatio)i)));
    }
    connect(d_ptr->resolutionRatioCbo, SIGNAL(currentIndexChanged(int)), this, SLOT(timeIntervalReleased(int)));
    gridLayout->addWidget(label, 0, 0);
    gridLayout->addWidget(d_ptr->resolutionRatioCbo, 0, 1);

    label = new QLabel(trs("TrendGroup"));
    d_ptr->trendGroupCbo = new ComboBox();
    d_ptr->trendGroupCbo->addItems(QStringList()
                                   << "Resp"
                                   << "IBP"
                                   << "AG"
                                  );
    connect(d_ptr->trendGroupCbo, SIGNAL(currentIndexChanged(int)), this, SLOT(trendGroupReleased(int)));
    gridLayout->addWidget(label, 1, 0);
    gridLayout->addWidget(d_ptr->trendGroupCbo, 1, 1);

    label = new QLabel(trs("Incident"));
    d_ptr->eventTypeCbo = new ComboBox();
    connect(d_ptr->eventTypeCbo, SIGNAL(currentIndexChanged(int)), this, SLOT(incidentReleased(int)));
    gridLayout->addWidget(label, 2, 0);
    gridLayout->addWidget(d_ptr->eventTypeCbo, 2, 1);

    setWindowLayout(gridLayout);
}