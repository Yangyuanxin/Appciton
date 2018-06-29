/**
 ** This file is part of the nPM project.
 ** Copyright (C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by Bingyun Chen <chenbingyun@blmed.cn>, 2018/6/29
 **/


#pragma once
#include <QWidget>
#include <QScrollArea>

class MenuSidebarPrivate;
class MenuSidebar : public QScrollArea
{
    Q_OBJECT
public:
    explicit MenuSidebar(QWidget *parent = NULL);
    ~MenuSidebar();

    /* add a string item */
    void addItem(const QString &text);

    /* reimplement */
    QSize sizeHint() const;


signals:
    void selectItemChanged(const QString &text);

protected:
    void resizeEvent(QResizeEvent *ev);
    void showEvent(QShowEvent *ev);
    void scrollContentsBy(int dx, int dy);

private slots:
    void onItemClicked();
    void onItemFocusChanged(bool in);

private:
    QScopedPointer<MenuSidebarPrivate> d_ptr;
};

