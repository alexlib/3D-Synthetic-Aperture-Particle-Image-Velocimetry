#ifndef NORMALIZEDCROSSCORRELATE_H
#define NORMALIZEDCROSSCORRELATE_H

#include <list>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "frame_3dpoint.h"
#include "pivengine.h"
#include "settings.h"

class NormalizedCrossCorrelate : public PivEngine
{
public:
    NormalizedCrossCorrelate(Settings * settingsPass, std::list<Grid3DPoint*>* gridPass);

    virtual ~NormalizedCrossCorrelate();

protected:
    //bool crossCorrelate(int topLeftRow, int topLeftColumn);
    bool crossCorrelate(int _intAx, int _intAy, int _intAz, PivPointData* pointdata);

    void prepareNCC();
    //void prepareNCC(int _intAxPass, int _intAyPass, int _intAzPass);

    //void NCCCore();
    double NccCore(int _intAxPass, int _intAyPass, int _intAzPass,
                 int _intBxPass, int _intByPass, int _intBzPass);

private:
    // ѡȡ��������ĳ����
    int _intLengthX, _intLengthY, _intLengthZ;

    QMutex mutex;

    // A֡���ӵ���ʼ����_intAx, _intAy, _intAz�̳���PivEngine
    // ����Ҫ�ˣ���ԭʼ�������㻥��س���
    //std::list<Frame3DPoint&> * framePoints_A;
    //std::list<Frame3DPoint&> * framePoints_B;

    //IplImage *imgA;
    //IplImage *imgB;

    IplImage **imgA;
    IplImage **imgB;

    CvScalar sA;
    CvScalar sB;

    int i;
    int j;
    int k;//���

    // ��ѡ���������ʱ����
    int ci;
    int cj;
    int ck;

    // ʵ�������ѡ���򴰸�Խ��߶˵�
    int actualx_start;
    int actualy_start;
    int actualz_start;
    int actualx_end;
    int actualy_end;
    int actualz_end;

    // ���¼���ͼ��ı�־
    int _lastIntZ;

    // ��ѡ��������--����ٶ�
    int candidateLimit;

    // ���ò�����
    Settings * settings;
};

#endif // NORMALIZEDCROSSCORRELATE_H
