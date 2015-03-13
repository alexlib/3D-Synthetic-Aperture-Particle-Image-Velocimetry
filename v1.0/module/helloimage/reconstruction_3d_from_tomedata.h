/**
 * ��Reconstruction3DFromTomeData
 * �̳���public Reconstruction3DIntensity
 *
 * Tomo-PIV��SAPIVͼ���3ά�ؽ�����
 * �ⲿ�ӿڱ�����ò���
 * a) ����ʵ���� Reconstruction3DFromTomeData * recon = new Reconstruction3DFromTomeData();
 * b) ����ͼ�� recon->loadImageFiles(files, path);
 * c) ��ʼ��������ļ���
 *             recon->initail3dPointsResultFile("D:\\resultpoints.txt");
 * d) ����������ά�㣬������ά������㣬ͬ�������Ѽ����(��Ϊ���ڶ�ά������ɾ��)
 *             recon->RawImagesProcess();
 * e) ��������ʣ���������ļ�
 *             recon->savePointsToFile("D:\\points.txt");
 * @author ����
 * @version 1.0.0
 * @date 2013-05-21
 */

#ifndef RECONSTRUCTION_3D_FROM_TOMEDATA_H
#define RECONSTRUCTION_3D_FROM_TOMEDATA_H

#include "reconstruction_3dintensity.h"
#include <QObject>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <string>

using namespace std;

class Reconstruction3DFromTomeData : public QObject,
        public Reconstruction3DIntensity
{

    Q_OBJECT
private:

    // �൱���ڽ��п����˲�
    static int filterSpatial;
    CvScalar s;
    int i;
    int j;
    int k;//���
    int num;
    int max;
    int currentX, currentY, currentZ;
    int tempPointX;
    int currentPointNum;
    int sumPointVal;


    IplImage *img;

public:
    Reconstruction3DFromTomeData();

    ~Reconstruction3DFromTomeData();

    void RawImagesProcess();

    void loadImageFiles(std::vector<string> *filename, string path);
private:
    void reconstructProgressSignal(int s);

signals:
    void reconstructProgress(int);
};

#endif // RECONSTRUCTION_3D_FROM_TOMEDATA_H
