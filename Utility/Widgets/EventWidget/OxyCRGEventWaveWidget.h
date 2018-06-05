#pragma once
#include "IWidget.h"
#include "EventDataDefine.h"

struct WaveformDesc
{
    WaveformDesc()
    {
        waveRangeMax = 0;
        waveRangeMin = 0;
        startY = 0;
        endY = 0;
        offsetX = 0;
        waveID = WAVE_NONE;
    }
    int waveRangeMax;
    int waveRangeMin;
    double startY;
    double endY;
    double offsetX;
    WaveformID waveID;
};

enum OxyCRGEventStorageDuration
{
    OxyCRG_EVENT_DURATION_1_3MIN = 0,           // before:1, after:3
    OxyCRG_EVENT_DURATION_2_2MIN,               // before:2, after:2
    OxyCRG_EVENT_DURATION_3_1MIN                // before:3, after:1
};

class OxyCRGEventWaveWidgetPrivate;
class OxyCRGEventWaveWidget : public IWidget
{
    Q_OBJECT
public:
    OxyCRGEventWaveWidget();
    ~OxyCRGEventWaveWidget();

    void setWaveTrendSegments(const QVector<WaveformDataSegment *> waveSegments, const QVector<TrendDataSegment *> trendSegments);
    void setWaveWidgetCompressed(WaveformID id);
    void setWaveWidgetTrend1(bool isRR);

public slots:
    void leftMoveCursor(void);
    void rightMoveCursor(void);
    void leftMoveCoordinate(void);
    void rightMoveCoordinate(void);

protected:
    void paintEvent(QPaintEvent *e);

private:
    void _drawTrend(QPainter &painter);
    void _drawWave(QPainter &painter);
    double _mapWaveValue(WaveformDesc &waveDesc, int wave);
    void _loadTrendData(void);
    void _drawDottedLine(QPainter &painter, qreal x1, qreal y1, qreal x2, qreal y2);

private:
    QScopedPointer<OxyCRGEventWaveWidgetPrivate> d_ptr;

};
