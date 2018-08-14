/**
 ** This file is part of the nPM project.
 ** Copyright (C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by luoyuchun <luoyuchun@blmed.cn>, 2018/7/19
 **/

#pragma once
#include <QAbstractButton>
#include <QScopedPointer>

class SpinBoxPrivate;
class SpinBox : public QAbstractButton
{
    Q_OBJECT
public:
    enum CtrlStatus
    {
        SPIN_BOX_NONE,
        SPIN_BOX_FOCUS_IN,
        SPIN_BOX_FOCUS_OUT,
        SPIN_BOX_EDITABLE,
        SPIN_BOX_DISABLE
    };

    explicit SpinBox(QWidget *parent = NULL);
    ~SpinBox();

    /**
     * @brief setArrow set arrow display
     * @param arrow
     */
    void setArrow(bool arrow);
    bool isArrow(void) const;

    /**
     * @brief setValue set current display value
     * @param value
     */
    void setValue(int value);
    int getValue(void);

    /**
     * @brief setRange  set value range
     * @param min min value
     * @param max max value
     */
    void setRange(int min, int max);
    void getRange(int &min, int &max);

    /**
     * @brief setScale set scale
     * @param scale
     */
    void setScale(int scale);

    /**
     * @brief setStep set step
     * @param step
     */
    void setStep(int step);

    /* reimplement */
    QSize sizeHint() const;
signals:
    void valueChange(int value, int scale);

private slots:
    void onPopupDestroy();
    void onEditValueUpdate(int value);

protected:
    /* reimplement */
    void paintEvent(QPaintEvent *ev);
    /* reimplement */
    void keyPressEvent(QKeyEvent *ev);
    /* reimplement */
    void keyReleaseEvent(QKeyEvent *ev);
    /* reimplement */
    void mouseReleaseEvent(QMouseEvent *ev);
     /* reimplement */
    void focusInEvent(QFocusEvent *ev);
     /* reimplement */
    void focusOutEvent(QFocusEvent *ev);

private:
    QScopedPointer<SpinBoxPrivate> d_ptr;
};