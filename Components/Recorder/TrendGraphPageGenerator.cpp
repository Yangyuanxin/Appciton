#include "TrendGraphPageGenerator.h"
#include "RecorderManager.h"
#include "TrendDataStorageManager.h"
#include "PatientManager.h"
#include <QPainter>
#include <QVector>
#include "FontManager.h"
#include "ParamInfo.h"
#include "Utility.h"
#include "AlarmConfig.h"


#define AXIS_X_SECTION_WIDTH (RECORDER_PIXEL_PER_MM * 16)
#define AXIS_Y_SECTION_HEIGHT (RECORDER_PIXEL_PER_MM * 8)

#define AXIS_X_SECTION_NUM 4
#define AXIS_Y_SECTION_NUM 2

#define AXIS_X_WIDTH (AXIS_X_SECTION_WIDTH * AXIS_X_SECTION_NUM)
#define AXIS_Y_HEIGH (AXIS_Y_SECTION_HEIGHT * AXIS_Y_SECTION_NUM + AXIS_Y_SECTION_HEIGHT / 4)

#define TICK_LENGTH (RECORDER_PIXEL_PER_MM)


class TrendGraphPageGeneratorPrivate
{
public:
    TrendGraphPageGeneratorPrivate()
        :curPageType(RecordPageGenerator::TitlePage),
        startTime(0),
        endTime(0),
        deltaT(0),
        curDrawnGraph(0),
        marginLeft(20 * RECORDER_PIXEL_PER_MM) //2 cm
    {}

    RecordPage *createGraphPage();

    /**
     * @brief getAxisInfo get the axis info
     * @note each page can be drawn only two graph at most, one on the top, the other on the bottom
     * @param page the record page
     * @param graphInfo graph info
     * @param onTop graph is on the top region or not
     */
    GraphAxisInfo getAxisInfo(const RecordPage *page, const TrendGraphInfo & graphInfo, bool onTop);

    void drawTrendValue(QPainter *painter, const QPointF &origin, const TrendGraphInfo &graphInfo);

    qreal timestampToX(unsigned t)
    {
        return (t - startTime) * 1.0 * AXIS_X_WIDTH / (endTime - startTime);
    }

    qreal mapTrendYValue(TrendDataType val, const TrendGraphInfo & graphInfo)
    {
        int axisH = AXIS_Y_SECTION_HEIGHT * AXIS_Y_SECTION_NUM;
        qreal mapH = (val -  graphInfo.scale.min) * 1.0 * axisH / (graphInfo.scale.max - graphInfo.scale.min);
        if(mapH > axisH)
        {
            mapH = axisH;
        }
        else if(mapH < 0)
        {
            mapH = 0.0;
        }
        return - mapH;
    }

    QList<QPainterPath> generatePainterPath(const TrendGraphInfo &graphInfo);

    RecordPage *drawGraphPage();

    RecordPageGenerator::PageType curPageType;
    unsigned startTime;
    unsigned endTime;
    unsigned deltaT;
    QList<TrendGraphInfo> trendGraphInfos;
    int curDrawnGraph;
    int marginLeft;
};


#define GRAPH_SPACING 24
GraphAxisInfo TrendGraphPageGeneratorPrivate::getAxisInfo(const RecordPage *page, const TrendGraphInfo &graphInfo, bool onTop)
{
    //draw two graph
    GraphAxisInfo axisInfo;
    SubParamID subParamID = graphInfo.subParamID;
    UnitType unit = graphInfo.unit;
    QString name;

    switch(subParamID)
    {
        case SUB_PARAM_NIBP_SYS:
        name = paramInfo.getParamName(PARAM_NIBP);
        break;
        case SUB_PARAM_ART_SYS:
        case SUB_PARAM_PA_SYS:
        case SUB_PARAM_CVP_MAP:
        case SUB_PARAM_LAP_MAP:
        case SUB_PARAM_RAP_MAP:
        case SUB_PARAM_AUXP1_SYS:
        case SUB_PARAM_AUXP2_SYS:
        name = paramInfo.getIBPPressName(subParamID);
        break;
        default:
        name = paramInfo.getSubParamName(subParamID);
        break;
    }

    axisInfo.caption = QString("%1/%2")
            .arg(trs(name))
            .arg(trs(Unit::getSymbol(unit)));

    axisInfo.height =  AXIS_Y_HEIGH;
    axisInfo.width = AXIS_X_WIDTH;
    axisInfo.marginLeft = marginLeft;
    axisInfo.xSectionWidth = AXIS_X_SECTION_WIDTH;
    axisInfo.xSectionNum = AXIS_X_SECTION_NUM;
    axisInfo.ySectionHeight = AXIS_Y_SECTION_HEIGHT;
    axisInfo.ySectionNum = AXIS_Y_SECTION_NUM;
    axisInfo.tickLength = TICK_LENGTH;
    axisInfo.drawArrow = true;

    if(onTop)
    {
        axisInfo.origin = QPointF(marginLeft, page->height() / 2 - GRAPH_SPACING );
    }
    else
    {
        axisInfo.origin = QPointF(marginLeft, page->height() /2 + AXIS_Y_HEIGH);

        //calcuelate the x labels
        unsigned t = startTime;
        QStringList timeList;
        QList<int> dayList;
        for(int i=0; i < AXIS_X_SECTION_NUM && t < endTime; i++)
        {
            QDateTime dt = QDateTime::fromTime_t(t);
            timeList.append(dt.toString("hh:mm:ss"));
            dayList.append(dt.date().day());
            t += deltaT;
        }

        bool crossTwoDay = false;
        for(int i = 0; i < dayList.size() - 1; i++)
        {
            if(dayList.at(i) != dayList.at(i+1))
            {
                crossTwoDay = true;
                break;
            }
        }

        for(int i = 0; i< timeList.size(); i++)
        {
            if(crossTwoDay)
            {
                axisInfo.xLabels.append(QString("%1%2")
                                        .arg(timeList.at(i))
                                        .arg(dayList.at(i)));
            }
            else
            {
                axisInfo.xLabels.append(timeList.at(i));
            }
        }
    }

    LimitAlarmConfig config = alarmConfig.getLimitAlarmConfig(subParamID, unit);
    axisInfo.yLabels = QStringList()<<Util::convertToString(graphInfo.scale.min, config.scale)
                                   <<QString()
                                  <<Util::convertToString(graphInfo.scale.max, config.scale);
    return axisInfo;
}

#define isEqual(a, b) (qAbs((a)-(b)) < 0.000001)

QList<QPainterPath> TrendGraphPageGeneratorPrivate::generatePainterPath(const TrendGraphInfo &graphInfo)
{
    QList<QPainterPath> paths;

    switch (graphInfo.subParamID) {
    case SUB_PARAM_NIBP_SYS:
    {
        QPainterPath path;

        QVector<TrendGraphDataV3>::ConstIterator iter = graphInfo.trendDataV3.constBegin();
        for(;iter != graphInfo.trendDataV3.constEnd(); iter++)
        {
            if(iter->data[0] == InvData())
            {
                continue;
            }

            qreal x = timestampToX(iter->timestamp);
            qreal sys = mapTrendYValue(iter->data[0], graphInfo);
            qreal dia = mapTrendYValue(iter->data[1], graphInfo);
            qreal map = mapTrendYValue(iter->data[2], graphInfo);

            //draw nibp symbol
            path.moveTo(x - TICK_LENGTH / 2, sys - 0.866 * TICK_LENGTH);
            path.lineTo(x, sys);
            path.lineTo(x + TICK_LENGTH / 2, sys - 0.866 * TICK_LENGTH);

            path.moveTo(x - TICK_LENGTH / 2, dia + 0.866 * TICK_LENGTH);
            path.lineTo(x, dia);
            path.lineTo(x + TICK_LENGTH / 2, dia + 0.866 * TICK_LENGTH);

            path.moveTo(x, sys);
            path.lineTo(x, dia);

            path.moveTo(x - TICK_LENGTH / 2, map);
            path.lineTo(x + TICK_LENGTH / 2, map);
        }
        paths.append(path);
    }
        break;
    case SUB_PARAM_ART_SYS:
    case SUB_PARAM_PA_SYS:
    case SUB_PARAM_AUXP1_SYS:
    case SUB_PARAM_AUXP2_SYS:
    {
        QPainterPath sysPath;
        QPainterPath diaPath;
        QPainterPath mapPath;

        bool lastPointInvalid = true;
        QPointF sysLastPoint;
        QPointF diaLastPoint;
        QPointF mapLastPoint;

        QVector<TrendGraphDataV3>::ConstIterator iter = graphInfo.trendDataV3.constBegin();
        for(;iter != graphInfo.trendDataV3.constEnd(); iter++)
        {
            if(iter->data[0] == InvData())
            {
                if(!lastPointInvalid)
                {
                    sysPath.lineTo(sysLastPoint);
                    diaPath.lineTo(diaLastPoint);
                    mapPath.lineTo(mapLastPoint);
                    lastPointInvalid = true;
                }
                continue;
            }

            qreal x = timestampToX(iter->timestamp);
            qreal sys = mapTrendYValue(iter->data[0], graphInfo);
            qreal dia = mapTrendYValue(iter->data[1], graphInfo);
            qreal map = mapTrendYValue(iter->data[2], graphInfo);

            if(lastPointInvalid)
            {
                sysPath.moveTo(x, sys);
                diaPath.moveTo(x, dia);
                mapPath.moveTo(x, map);
                lastPointInvalid = false;
            }
            else
            {
                if(!isEqual(sysLastPoint.y(), sys))
                {
                    sysPath.lineTo(x, sys);
                }

                if(!isEqual(diaLastPoint.y(), dia))
                {
                    diaPath.lineTo(x, dia);
                }

                if(!isEqual(mapLastPoint.y(), map))
                {
                    mapPath.lineTo(x, map);
                }
            }

            sysLastPoint.rx() = x;
            sysLastPoint.ry() = sys;
            diaLastPoint.rx() = x;
            diaLastPoint.ry() = dia;
            mapLastPoint.rx() = x;
            mapLastPoint.ry() = map;
        }

        if(!lastPointInvalid)
        {
            sysPath.lineTo(sysLastPoint);
            diaPath.lineTo(diaLastPoint);
            mapPath.lineTo(mapLastPoint);
        }

        paths.append(sysPath);
        paths.append(diaPath);
        paths.append(mapPath);

    }
        break;
    default:
    {
        QPainterPath path;

        QPointF lastPoint;
        bool lastPointInvalid = true;
        QVector<TrendGraphData>::ConstIterator iter =  graphInfo.trendData.constBegin();
        for(; iter != graphInfo.trendData.constEnd(); iter++)
        {
            if(iter->data == InvData())
            {
                if(!lastPointInvalid)
                {
                    path.lineTo(lastPoint);
                    lastPointInvalid = true;
                }
                continue;
            }

            qreal x = timestampToX(iter->timestamp);
            qreal y = mapTrendYValue(iter->data, graphInfo);

            if(lastPointInvalid)
            {
                path.moveTo(x, y);
                lastPointInvalid = false;
            }
            else
            {
                if(!isEqual(lastPoint.y(), y))
                {
                    path.lineTo(x, y);
                }
            }

            lastPoint.rx() = x;
            lastPoint.ry() = y;
        }

        if(!lastPointInvalid)
        {
            path.lineTo(lastPoint);
        }

        paths.append(path);
    }
        break;
    }

    return paths;
}

void TrendGraphPageGeneratorPrivate::drawTrendValue(QPainter *painter, const QPointF& origin, const TrendGraphInfo &graphInfo)
{
    painter->save();
    painter->setPen(Qt::white);
    painter->setBrush(Qt::NoBrush);
    QList<QPainterPath> paths = generatePainterPath(graphInfo);
    QList<QPainterPath>::ConstIterator iter;
    for(iter = paths.constBegin(); iter != paths.constEnd(); iter++)
    {
        painter->save();
        painter->translate(origin);
        painter->drawPath(*iter);
        painter->restore();
    }
    painter->restore();
}


RecordPage *TrendGraphPageGeneratorPrivate::drawGraphPage()
{
    if(curDrawnGraph == trendGraphInfos.size())
    {
        //all pages are drawn
        return NULL;
    }

    int pageWidth = marginLeft + AXIS_X_SECTION_NUM * AXIS_X_SECTION_WIDTH;
    RecordPage *page =  new RecordPage(pageWidth);
    QPainter painter(page);

    QFont font = fontManager.recordFont(24);
    painter.setFont(font);
    if(curDrawnGraph + 2 <= trendGraphInfos.size())
    {
        //draw two graph
        GraphAxisInfo axisInfo = getAxisInfo(page, trendGraphInfos.at(curDrawnGraph), true);

        //draw the axis
        RecordPageGenerator::drawGraphAxis(&painter, axisInfo);

        //draw graph
        drawTrendValue(&painter, axisInfo.origin, trendGraphInfos.at(curDrawnGraph));

        curDrawnGraph++;

        axisInfo = getAxisInfo(page, trendGraphInfos.at(curDrawnGraph), false);
        RecordPageGenerator::drawGraphAxis(&painter, axisInfo);
        drawTrendValue(&painter, axisInfo.origin, trendGraphInfos.at(curDrawnGraph));
        curDrawnGraph++;
    }
    else
    {
        //draw only one graph
        GraphAxisInfo axisInfo = getAxisInfo(page, trendGraphInfos.at(curDrawnGraph), false);
        RecordPageGenerator::drawGraphAxis(&painter, axisInfo);
        drawTrendValue(&painter, axisInfo.origin, trendGraphInfos.at(curDrawnGraph));
        curDrawnGraph++;
    }

    return page;
}

TrendGraphPageGenerator::TrendGraphPageGenerator(const QList<TrendGraphInfo> &trendInfos,
                                                 unsigned startTime, unsigned endTime, QObject *parent)
    :RecordPageGenerator(parent), d_ptr(new TrendGraphPageGeneratorPrivate)
{
    d_ptr->startTime = startTime;
    d_ptr->endTime = endTime;
    d_ptr->trendGraphInfos = trendInfos;
    d_ptr->deltaT = (endTime - startTime) /  AXIS_X_SECTION_NUM;
}

TrendGraphPageGenerator::~TrendGraphPageGenerator()
{

}

int TrendGraphPageGenerator::type() const
{
    return Type;
}

RecordPage *TrendGraphPageGenerator::createPage()
{
    switch(d_ptr->curPageType)
    {
    case TitlePage:
        // BUG: patient info of the event might not be the current session patient
        d_ptr->curPageType = TrendGraphPage;
        return createTitlePage(QString(trs("GraphTrendRecording")), patientManager.getPatientInfo());

    case TrendGraphPage:
        if(d_ptr->curDrawnGraph < d_ptr->trendGraphInfos.size())
        {
            return d_ptr->drawGraphPage();
        }

        //fall through
    case EndPage:
        d_ptr->curPageType = NullPage;
        return createEndPage();
    default:
        break;
    }
    return NULL;
}
