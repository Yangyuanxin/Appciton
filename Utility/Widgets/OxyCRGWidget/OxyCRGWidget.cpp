#include "OxyCRGWidget.h"
#include "FontManager.h"
#include "LanguageManager.h"
#include "WindowManager.h"
#include "ColorManager.h"
#include "OxyCRGWidgetLabel.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QMap>
#include "IWidget.h"
#include "IConfig.h"
#include <QPainter>
#include "ComboListPopup.h"
#include "ParamInfo.h"
#include "SystemManager.h"

/**************************************************************************************************
 * 析构。
 *************************************************************************************************/
OxyCRGWidget::OxyCRGWidget():IWidget("OxyCRGWidget"),
    _isShowGrid(true),_isShowFrame(true), _isShowScale(true)
{
    _pixelWPitch = systemManager.getScreenPixelWPitch();
    _pixelHPitch = systemManager.getScreenPixelHPitch();

    QPalette palette;
    palette.setColor(QPalette::WindowText, QColor(255, 255, 0));
    palette.setColor(QPalette::Window, Qt::black);
//    palette.setColor(QPalette::Foreground, Qt::black);
    setPalette(palette);

    _mainLayout = new QVBoxLayout();
    _mainLayout->setMargin(0);
    _mainLayout->setSpacing(0);

    _hLayoutWave = new QVBoxLayout();
    _hLayoutWave->setMargin(1);
    _hLayoutWave->setSpacing(0);

    // 标题栏。
    _titleLabel = new QLabel(trs("UserFaceOxyCRG"));
    _titleLabel->setAlignment(Qt::AlignCenter);
    _titleLabel->setFixedHeight(_titleBarHeight);
    int fontSize = fontManager.getFontSize(7);
    _titleLabel->setFont(fontManager.textFont(fontSize));
    _titleLabel->setWordWrap(true);
    QPalette p;
    p.setColor(QPalette::WindowText, Qt::black);
    p.setColor(QPalette::Window, QColor(255, 255, 0));
    _titleLabel->setPalette(p);

    bottomLayout = new QHBoxLayout();
    bottomLayout->setMargin(2);
    bottomLayout->setSpacing(1);

    _interval = new OxyCRGWidgetLabel("", Qt::AlignLeft | Qt::AlignVCenter, this);
    _interval->setFont(fontManager.textFont(fontSize));
    _interval->setFixedSize(80, _labelHeight);
    _interval->setText("");
    connect(_interval, SIGNAL(released(IWidget*)), this, SLOT(_intervalSlot(IWidget*)));

    _changeTrend = new OxyCRGWidgetLabel("", Qt::AlignLeft | Qt::AlignVCenter, this);
    _changeTrend->setFont(fontManager.textFont(fontSize));
    _changeTrend->setFixedSize(80, _labelHeight);
    _changeTrend->setText("");
    connect(_changeTrend, SIGNAL(released(IWidget*)), this, SLOT(_changeTrendSlot(IWidget*)));

    bottomLayout->addWidget(_interval);
    bottomLayout->addWidget(_changeTrend);

    _mainLayout->addWidget(_titleLabel,0,Qt::AlignCenter);
    _mainLayout->addLayout(_hLayoutWave);
    _mainLayout->addLayout(bottomLayout);

    // 设置布局。
    setLayout(_mainLayout);

    setFocusPolicy(Qt::NoFocus);
}

/**************************************************************************************************
 * 析构。
 *************************************************************************************************/
OxyCRGWidget::~OxyCRGWidget()
{
}

/**************************************************************************************************
 * 获取配置列表
 * 布局
 *************************************************************************************************/
void OxyCRGWidget::_trendLayout(void)
{
    QStringList nodeWidgets;
    QString widgets;
    systemConfig.getStrValue("PrimaryCfg|UILayout|WidgetsOrder|OxyCRGWidget", widgets);
    nodeWidgets = widgets.split(",");

    IWidget *w = NULL;
    for (int i = 0; i < nodeWidgets.size(); i++)
    {
        w = windowManager.getWidget(nodeWidgets[i]);
        if (w == NULL)
        {
            continue;
        }
        w->setParent(this);            // 设置父窗体。
        w->setVisible(true);           // 可见。

        _hLayoutWave->addWidget(w,1);
    }
}

/**************************************************************************************************
 * 清除显示控件
 *************************************************************************************************/
void OxyCRGWidget::_clearLayout()
{
    // 移除_hLayoutWave之前的窗体。
    int trendcount = _hLayoutWave->count();
    for (int i = 0; i < trendcount; i++)
    {
        QLayoutItem *item = _hLayoutWave->takeAt(0);
        IWidget *widget = (IWidget *)item->widget();
        if (widget != NULL)
        {
            widget->setVisible(false);
            widget->setParent(NULL);
        }
    }
}

/**************************************************************************************************
 * 设置时间间隔。
 *************************************************************************************************/
void OxyCRGWidget::_setInterval(OxyCRGInterval index)
{
    superRunConfig.setNumValue("RESP|Interval", (int)index);

    _interval->setText(OxyCRGSymbol::convert(OxyCRGInterval(index)));
}

/**************************************************************************************************
 * 获取时间间隔。
 *************************************************************************************************/
OxyCRGInterval OxyCRGWidget::_getInterval()
{
    int index = OxyCRG_Interval_1;
    superRunConfig.getNumValue("RESP|Interval", index);

    return (OxyCRGInterval)index;
}

/**************************************************************************************************
 * 设置RESP/RR。
 *************************************************************************************************/
void OxyCRGWidget::_setTrend(OxyCRGTrend index)
{
    superRunConfig.setNumValue("RESP|Trend", (int)index);

    _changeTrend->setText(OxyCRGSymbol::convert(OxyCRGTrend(index)));

    QStringList nodeWidgets;
    QString widgets;
    QString path = "PrimaryCfg|UILayout|WidgetsOrder|OxyCRGWidget";
    systemConfig.getStrValue(path, widgets);
    nodeWidgets = widgets.split(",");

    int i = 0;
    if (index == OxyCRG_Trend_RESP)
    {

        for (; i < nodeWidgets.size(); i++)
        {
            if (nodeWidgets[i] == "OxyCRGRRWidget")
            {
                nodeWidgets[i] = "OxyCRGRESPWidget";

                break;
            }
        }

    }
    else
    {
        for (; i < nodeWidgets.size(); i++)
        {
            if (nodeWidgets[i] == "OxyCRGRESPWidget")
            {
                nodeWidgets[i] = "OxyCRGRRWidget";

                break;
            }
        }
    }
    widgets = nodeWidgets.join(",");
    systemConfig.setStrValue(path, widgets);
}

/**************************************************************************************************
 * 获取RESP/RR。
 *************************************************************************************************/
OxyCRGTrend OxyCRGWidget::_getTrend()
{
    int index = OxyCRG_Trend_RESP;
    superRunConfig.getNumValue("RESP|Trend", index);

    return (OxyCRGTrend)index;
}

/**************************************************************************************************
 * 对于给定的数值value, 计算大于等于value且能整除step的数值
 *************************************************************************************************/
int OxyCRGWidget::_roundUp(int value, int step)
{
    while (value % step)
    {
        value++;
    }
    return value;
}

/**************************************************************************************************
 * 对于给定的数值value, 计算小于等于value且能整除step的数值
 *************************************************************************************************/
int OxyCRGWidget::_roundDown(int value, int step)
{
    while (value % step)
    {
        value--;
    }
    return value;
}

/**************************************************************************************************
 * 布局器内部的Widget删除与显示。
 *************************************************************************************************/
void OxyCRGWidget::setVisible(bool visible)
{
    if (!visible)
    {
        _clearLayout();
    }
    else
    {
        _setInterval(_getInterval());
        _setTrend(_getTrend());

        _trendLayout();
    }

    QWidget::setVisible(visible);
}

/**************************************************************************************************
 * 绘图。
 *************************************************************************************************/
void OxyCRGWidget::paintEvent(QPaintEvent *event)
{
    IWidget::paintEvent(event);

    QPainter barPainter(this);

    // 绘制边框。
    QPen pen;
    pen.setColor(Qt::white);
    pen.setWidth(2);
    barPainter.setPen(pen);
    barPainter.drawRect(rect());

    QRect rectAdjust =rect().adjusted(1, _titleBarHeight-1, -1, -_labelHeight-4);
//    barPainter.drawRect(rectAdjust);
    barPainter.drawLine(rectAdjust.bottomLeft(),rectAdjust.bottomRight());

    //画上x轴刻度线
    barPainter.setPen(QPen(Qt::white, 1, Qt::SolidLine));
    for(int i=0;i<4;i++)//分成10份
    {
        barPainter.setFont(fontManager.textFont(fontManager.getFontSize(0)));
        //选取合适的坐标，绘制一段长度为4的直线，用于表示刻度
//        barPainter.drawLine(rectAdjust.bottomRight().x() - (i * (rectAdjust.bottomRight().x() / 4)),rectAdjust.bottomRight().y(),
//                         rectAdjust.bottomRight().x() - (i * (rectAdjust.bottomRight().x() / 4)),(rectAdjust.bottomRight().y() + 4));

        barPainter.drawText((rectAdjust.bottomRight().x() - (i * (rectAdjust.bottomRight().x() / 4))) + 2,  (rectAdjust.bottomRight().y() + 12)
                         ,QString::number(i));
    }

    QRect r = rect();
    r.setBottom(_titleBarHeight);
    barPainter.fillRect(r, QColor(255, 255, 0));

}

/**************************************************************************************************
 * 调整大小。
 *************************************************************************************************/
void OxyCRGWidget::resizeEvent(QResizeEvent *e)
{
    IWidget::resizeEvent(e);

    int l = ((rect().width() / 4) - _interval->width())/2;
    _interval->move(rect().left() + l, rect().bottom() - _labelHeight - 2);
    _changeTrend->move(rect().left() + (rect().width() / 4) + l, rect().bottom() - _labelHeight - 2);
}

/**************************************************************************************************
 * 时间时间触发。
 *************************************************************************************************/
void OxyCRGWidget::_intervalSlot(IWidget *widget)
{
    if (NULL == _intervalList)
    {
        _intervalList = new ComboListPopup(widget, POPUP_TYPE_USER, OxyCRG_Interval_NR, _getInterval());
        for (int i = 0; i < OxyCRG_Interval_NR; i++)
        {
            _intervalList->addItemText(OxyCRGSymbol::convert(OxyCRGInterval(i)));
        }
        _intervalList->setItemDrawMark(false);
        _intervalList->setFont(fontManager.textFont(fontManager.getFontSize(1)));
        connect(_intervalList, SIGNAL(destroyed()), this, SLOT(_intervalDestroyed()));
    }

    _intervalList->show();
}

/**************************************************************************************************
 * 时间间隔更改操作。
 *************************************************************************************************/
void OxyCRGWidget::_changeTrendSlot(IWidget *widget)
{
    if (NULL == _changeTrendList)
    {
        _changeTrendList = new ComboListPopup(widget, POPUP_TYPE_USER, OxyCRG_Trend_NR, _getTrend());
        for (int i = 0; i < OxyCRG_Trend_NR; i++)
        {
            _changeTrendList->addItemText(OxyCRGSymbol::convert(OxyCRGTrend(i)));
        }
        _changeTrendList->setItemDrawMark(false);
        _changeTrendList->setFont(fontManager.textFont(fontManager.getFontSize(1)));
        connect(_changeTrendList, SIGNAL(destroyed()), this, SLOT(_changeTrendDestroyed()));
    }

    _changeTrendList->show();
}

void OxyCRGWidget::_intervalDestroyed()
{
    int index = _intervalList->getCurIndex();
    if (index == -1)
    {
        _intervalList = NULL;
        return;
    }

    _setInterval((OxyCRGInterval)index);

    _intervalList = NULL;
}

/**************************************************************************************************
 * 切换显示操作。
 *************************************************************************************************/
void OxyCRGWidget::_changeTrendDestroyed()
{
    int index = _changeTrendList->getCurIndex();
    if (index == -1)
    {
        _changeTrendList = NULL;
        return;
    }

    _setTrend((OxyCRGTrend)index);
    _clearLayout();
    _trendLayout();

    _changeTrendList = NULL;
}
