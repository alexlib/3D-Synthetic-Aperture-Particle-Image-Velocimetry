#include "pivthread.h"

#include <QVector>
#include <QSemaphore>
#include "pivdata.h"
#include <iostream>

// Processing modules
//#include "fftcrosscorrelate.h"
#include "normalizedcrosscorrelator.h"

PivThread::PivThread(QSemaphore *freePass, QSemaphore *usedPass, QMutex *mutexPass, QVector<PivData*> *dataVectorPass, QList<int> listPass, QObject *parent) : QThread(parent)
{
    free = freePass;
    used = usedPass;
    mutex = mutexPass;
    dataVector = dataVectorPass;
    list = listPass;

    //analysisCreated = false;
    settingsSet = false;
    //filedataSet = false;
    //filter = false;

    abort = false;

}

PivThread::~PivThread()
{
    wait();
    switch(_processor)
    {
    case _3DPIV::FFTCorrelator:
       // delete fftCrossCorrelate;
        break;
    case _3DPIV::NCCCorrelator:
        delete nccCrossCorrelate;
    default:
        break;
    }

    //if (analysisCreated) delete analysis;
}

void PivThread::setSettings(Settings *settingsPass)
{
    settings = settingsPass;
    _processor = settings->processor();
    settingsSet = true;

}

/*void PivThread::setFileData(DataContainer *filedataPass)
{
    filedata = filedataPass;
    for (int i = 0; i < list.size(); i++)
    {
        filelist.append(filedata->data(list.value(i)));
    }
    filedataSet = true;
}*/

/**
 *  ��ӻ���ض���ʵ�����Լ��������Ĳ�����ʵ��
 */
void PivThread::initializeProcessor()
{
    if (/*filedataSet && */settingsSet)
    {
        switch(_processor)
        {
        case _3DPIV::FFTCorrelator:
            //fftCrossCorrelate = new FFTCrossCorrelate(settings,*filedata->gridList());
            break;
        case _3DPIV::NCCCorrelator:
            nccCrossCorrelate = new NormalizedCrossCorrelate(settings, settings->gridList());
            break;
        default:
            break;
        }
        //filter = settings->batchFilter();
        //analysis = new Analysis(settings,filedata);
        //filterOptions = settings->filterOptions();
        //analysisCreated = true;
    }
}

int PivThread::process()
{
    mutex->lock();
    initializeProcessor();
    mutex->unlock();
    start();

    return 0;
}

void PivThread::stopProcess()
{
    abort = true;
}

void PivThread::run()
{
    PivData *pivData;

    int i = 0;
    // ���ѭ���Լ�free��used������ɶ��˼����ע�͵�
    //while (i < filelist.size() && !abort)
    {
        free->acquire();

        switch(_processor)
        {
        case _3DPIV::FFTCorrelator:
            //pivData = fftCrossCorrelate->operator()(filelist.value(i));
            break;
        case _3DPIV::NCCCorrelator:
            pivData = nccCrossCorrelate->operator()();
            break;
        default:
            break;
        }
        // pivData��index��name��ָ��ʲô��˼
        //pivData->setIndex(filelist.value(i).index());
        //pivData->setName(filelist.value(i).imageA());
        //if (filter) analysis->filterData(pivData,filterOptions);

        dataVector->append(pivData);

        used->release();
        ++i;
    }
}
