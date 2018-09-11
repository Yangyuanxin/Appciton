/**
 ** This file is part of the nPM project.
 ** Copyright (C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by Bingyun Chen <chenbingyun@blmed.cn>, 2018/9/7
 **/

#include "RecordPageProcessor.h"
#include <QList>
#include <QApplication>
#include <QTimer>
#include "Utility.h"
#include "Debug.h"
#include "unistd.h"
#include <QTimerEvent>

#define PAGE_QUEUE_SIZE 32

class RecordPageProcessorPrivate
{
public:
    RecordPageProcessorPrivate(RecordPageProcessor *const q_ptr, PrintProviderIFace *iface)
        : q_ptr(q_ptr), iface(iface), processing(false), pause(false),
          updateSpeed(false), queueIsFull(false), curSpeed(PRINT_SPEED_NR),
          timerID(-1), curProcessingPage(NULL), curPageXPos(0)
    {}

    void stopProcessing();

    RecordPageProcessor *const q_ptr;
    PrintProviderIFace *iface;
    QList<RecordPage *> pages;
    bool processing;
    bool pause;
    bool updateSpeed;
    bool queueIsFull;
    PrintSpeed curSpeed;
    int timerID;
    RecordPage *curProcessingPage;
    int curPageXPos;
};

RecordPageProcessor::RecordPageProcessor(PrintProviderIFace *iface, QObject *parent)
    : QObject(parent), d_ptr(new RecordPageProcessorPrivate(this, iface))
{
}

RecordPageProcessor::~RecordPageProcessor()
{
    qDeleteAll(d_ptr->pages);
}

bool RecordPageProcessor::isProcessing() const
{
    return d_ptr->processing;
}

void RecordPageProcessor::updatePrintSpeed(PrintSpeed speed)
{
    if (d_ptr->curSpeed == speed || speed > PRINT_SPEED_NR)
    {
        return;
    }

    d_ptr->curSpeed = speed;
    if (d_ptr->processing)
    {
        d_ptr->updateSpeed = true;
    }
    else
    {
        // set speed imediately
        d_ptr->iface->setPrintSpeed(d_ptr->curSpeed);
        qDebug() << "Set Print Speed " << speed;
    }
}

void RecordPageProcessor::addPage(RecordPage *page)
{
    if (!page)
    {
        return;
    }

#if 1
#ifdef Q_WS_QWS
    QString path("/mnt/nfs/tmp/");
#else
    QString path("/srv/nfs/tmp/");
#endif
    // QString path("/usr/local/nPM/bin/");
    path += page->getID();
    path += ".png";
    page->save(path);
#endif

    d_ptr->pages.append(page);

    if (d_ptr->pages.size() >= PAGE_QUEUE_SIZE)
    {
        d_ptr->queueIsFull = true;
        emit pageQueueFull(d_ptr->queueIsFull);
    }

    if (!d_ptr->processing)
    {
        // process the page in the next event loop
        d_ptr->processing = true;
        d_ptr->timerID = startTimer(2);
    }
}

void RecordPageProcessor::stopProcess()
{
    qDeleteAll(d_ptr->pages);
    d_ptr->pages.clear();


    if (d_ptr->queueIsFull)
    {
        d_ptr->queueIsFull = false;
        emit pageQueueFull(d_ptr->queueIsFull);
    }

    d_ptr->stopProcessing();
    emit processFinished();
}

void RecordPageProcessor::pauseProcessing(bool pause)
{
    if (!d_ptr->processing)
    {
        qDebug() << "Not start yet, not need to pause";
    }

    d_ptr->pause = pause;
}

void RecordPageProcessor::timerEvent(QTimerEvent *ev)
{
    if (d_ptr->timerID == ev->timerId())
    {
        // check whether speed need to update
        if (d_ptr->updateSpeed)
        {
            d_ptr->updateSpeed = false;
            d_ptr->iface->setPrintSpeed(d_ptr->curSpeed);
        }

        // update page queue status
        if (d_ptr->queueIsFull && d_ptr->pages.size() < PAGE_QUEUE_SIZE)
        {
            d_ptr->queueIsFull = false;
            emit pageQueueFull(d_ptr->queueIsFull);
        }

        // try to fetch a page from the page list
        if (d_ptr->curProcessingPage == NULL)
        {
            if (!d_ptr->pages.isEmpty())
            {
                d_ptr->curProcessingPage = d_ptr->pages.takeFirst();
            }
        }

        if (d_ptr->curProcessingPage == NULL)
        {
            // no more page to process
            d_ptr->stopProcessing();
            emit processFinished();
            return;
        }

        if (d_ptr->pause)
        {
            // already pause, do nothing
            return;
        }

        // send page data
        int dataLen = d_ptr->curProcessingPage->height() / 8;
        unsigned char data[dataLen] = {0};
        d_ptr->curProcessingPage->getColumnData(d_ptr->curPageXPos++, data);
        if (!d_ptr->iface->sendBitmapData(data, dataLen))
        {
#if 0
            // send failed, uart buffer might be full
            qdebug("send bitmap data failed, send again after 20 ms.");
            ::usleep(20 * 1000);

            // send again, and don't care the result
            d_ptr->iface->sendBitmapData(data, dataLen);
#endif
        }

        if (d_ptr->curPageXPos >= d_ptr->curProcessingPage->width())
        {
            qDebug() << "Process Page" << d_ptr->curProcessingPage->getID()
                     << "left pages:" << d_ptr->pages.size();
            // finished processing this page
            delete d_ptr->curProcessingPage;
            d_ptr->curProcessingPage = NULL;
            d_ptr->curPageXPos = 0;
        }
    }
}

void RecordPageProcessorPrivate::stopProcessing()
{
    processing = false;

    if (curProcessingPage)
    {
        delete curProcessingPage;
        curProcessingPage = NULL;
        curPageXPos = 0;
    }

    iface->flush();
    iface->stop();

    if (timerID != -1)
    {
        q_ptr->killTimer(timerID);
    }
    timerID = -1;
}
