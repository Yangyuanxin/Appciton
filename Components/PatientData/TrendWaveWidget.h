#pragma once
#include "IWidget.h"
#include "TrendSubWaveWidget.h"
#include "TrendDataDefine.h"
#include <QScrollArea>

class QHBoxLayout;
class QVBoxLayout;
class TrendWaveWidget : public IWidget
{
    Q_OBJECT
public:
    TrendWaveWidget();
    ~TrendWaveWidget();

public:
    void changeTrendDisplay();
    void setWidgetSize(int w, int h);
    void leftMoveCursor();
    void rightMoveCursor();

protected:
    void paintEvent(QPaintEvent *event);


private:
    void _trendLayout(void);
    void _clearLayout(void);

private:
    QVBoxLayout *_mainLayout;
    QVBoxLayout *_hLayoutTrend;

    ResolutionRatio _timeInterval;
    int _waveRegionWidth;
    int _oneFrameWidth;

    unsigned _initTime;
    int _cursorPosition;
    unsigned _currentCursorTime;

    IWidget *_subWidget;
    QScrollArea *_subWidgetScrollArea;

};
