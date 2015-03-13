#include "normalizedcrosscorrelator.h"
#include <vector>
#include <string>
#include <sstream>
#include <QDebug>

using namespace std;

NormalizedCrossCorrelate::NormalizedCrossCorrelate(Settings *settingsPass, std::list<Grid3DPoint*>* gridPass)
    : PivEngine(settingsPass, gridPass)
{
    // ��ʼ����
    _intLengthX = PivEngine::intLengthX();
    _intLengthY = PivEngine::intLengthY();
    _intLengthZ = PivEngine::intLengthZ();

    // ���ò�����
    settings = settingsPass;

    // ���¼���ͼ��ı�־
    _lastIntZ = -1;

    // �������������ٽ���Դ�г�ͻ����
    mutex.lock();
    //prepareNCC(0,0,0);
    prepareNCC();
    mutex.unlock();
}

NormalizedCrossCorrelate::~NormalizedCrossCorrelate()
{
    // �������������ٽ���Դ�г�ͻ����
    mutex.lock();
    mutex.unlock();

    // ��framePointA��B���������մ�����Ϊ���ڴ����з����ڴ棬ͳһ����


    // ͼƬ�ڴ��ͷ�
    for(int l = 0; l < _intLengthZ; l++)
    {
        if (imgA[l])
        {
            cvReleaseImage(&imgA[l]);
        }
        else
            break;
    }
    for(int l = 0; l < candidateLimit; l++)
    {
        if (imgB[l])
        {
            cvReleaseImage(&imgB[l]);
        }
        else
            break;
    }
}

/**
 * ����׼��
 * @param _intAx
 * @param _intAy
 * @param _intAz
 */
void NormalizedCrossCorrelate::prepareNCC()
//void NormalizedCrossCorrelate::prepareNCC(int _intAxPass, int _intAyPass, int _intAzPass)
{

/*    // A֡���ӵ���ʼ����
    this->_intAx = _intAxPass;
    this->_intAy = _intAyPass;
    this->_intAz = _intAzPass;
*/
    // ����ٶ�����
    candidateLimit = 20;
    //*(IplImage*) a = ;
    //*(IplImage*) b = ;
    imgA = new IplImage*[_intLengthZ];
    imgB = new IplImage*[candidateLimit*2 + _intLengthZ];
    int i = 0;
    for(i = 0; i < _intLengthZ; i++)
    {
         *(imgA+i) = NULL;
    }
    for(i = 0; i < candidateLimit; i++)
    {
         imgB[i] = NULL;
    }
}

/**
 * ָ�ĵڶ�֡B֡��ѡƥ�䴰������Ͻ����꣬������(x,y,z)�ֱ���С�ĵ�����
 */
//bool NormalizedCrossCorrelate::crossCorrelate(int _intBx, int _intBy, int _intBz)
bool NormalizedCrossCorrelate::crossCorrelate(int _intAx, int _intAy, int _intAz, PivPointData* pointDataPass)
{

    // ��¼���λ��
    int _maxBx = -1;
    int _maxBy = -1;
    int _maxBz = -1;
    double _maxNCCCoeffient = 0;
    double _currentNCCCoeffient = 0;

    // ʵ�������ѡ���򴰸�Խ��߶˵�
    actualx_start = 0;
    actualy_start = 0;
    actualz_start = 0;
    actualx_end = this->settings->getROIWidth();
    actualy_end = this->settings->getROIHeight();
    actualz_end = this->settings->getROIDepth();
    // һ��
    if (_intAx - candidateLimit >= 0)
        actualx_start = _intAx - candidateLimit;
    else
        actualx_start = 0;
    if (_intAy - candidateLimit >= 0)
        actualy_start = _intAy - candidateLimit;
    else
        actualy_start = 0;
    if (_intAz - candidateLimit >= 0)
        actualz_start = _intAz - candidateLimit;
    else
        actualz_start = 0;
    // ��һ��
    if (_intAx + _intLengthX + candidateLimit <= this->settings->getROIWidth())
        actualx_end = _intAx + candidateLimit;
    else
        actualx_end = this->settings->getROIWidth() - _intLengthX;
    if (_intAy + _intLengthY + candidateLimit <= this->settings->getROIHeight())
        actualy_end = _intAy + candidateLimit;
    else
        actualy_end = this->settings->getROIHeight() - _intLengthY;
    if (_intAz + _intLengthZ + candidateLimit <= this->settings->getROIDepth())
        actualz_end = _intAz + candidateLimit;
    else
        actualz_end = this->settings->getROIDepth() - _intLengthZ;

    qDebug() << "begin" << _intAx << "," << _intAy << "," << _intAz;

    if (_intAz != _lastIntZ)
    {
        // ͼƬ�ڴ��ͷ�
        for(int l = 0; l < _intLengthZ; l++)
        {
            if (imgA[l])
            {
                cvReleaseImage(&imgA[l]);
            }
            else
                break;
        }
        for(int l = 0; l < candidateLimit; l++)
        {
            if (imgB[l])
            {
                cvReleaseImage(&imgB[l]);
            }
            else
                break;
        }

        // ͼƬ���ص�һ֡
        string pathfileA;
        for (int l = 0; l < _intLengthZ; l++)
        {
            pathfileA = settings->pathA + settings->filesA[_intAz + l];
            imgA[l] = cvLoadImage(pathfileA.data(), 1);//CV_LOAD_IMAGE_GRAYSCALE);
            if (!imgA[l])
            {
                qDebug() << "img of frame A error!" << pathfileA.data();
                return false;
            }
            else
                qDebug() << "success load A image" << pathfileA.data();
        }
        // ͼƬ���صڶ�֡
        string pathfileB;
        for (int l = 0; l < (actualz_end + _intLengthZ - actualz_start + 1); l++)
        {
            pathfileB = settings->pathB + settings->filesB[actualz_start + l];
            imgB[l] = cvLoadImage(pathfileB.data(), 1);//CV_LOAD_IMAGE_GRAYSCALE);
            if (!imgB[l])
            {
                qDebug() << "img of frame B error!";
                return false;
            }
            else
                qDebug() << "success load B image" << pathfileB.data();
        }

        _lastIntZ = _intAz;
    }

    for (ck = actualz_start; ck < actualz_end; ck++)
        for (cj = actualy_start; cj < actualy_end; cj++)
            for (ci = actualx_start; ci < actualx_end; ci++)
                {
                    _currentNCCCoeffient = NccCore(_intAx,_intAy,_intAz,ci,cj,ck);
                    if (_currentNCCCoeffient > _maxNCCCoeffient)
                    {
                        _maxNCCCoeffient = _currentNCCCoeffient;
                        _maxBx = ci;
                        _maxBy = cj;
                        _maxBz = ck;
                    }
                }

    // ��дPivPointData,delta t���������
    double deltat = 1;
    pointDataPass->u = (_maxBx - _intAx) / deltat;
    pointDataPass->v = (_maxBy - _intAy) / deltat;
    pointDataPass->w = (_maxBz - _intAz) / deltat;
    pointDataPass->ncc = _maxNCCCoeffient;

    qDebug()  << "vectory:" << "A(" <<
                 _intAx << "," << _intAy << "," << _intAz << ") - B(" <<
                 _maxBx << "," << _maxBy << "," << _maxBz << ") = vec(" <<
                 _maxBx - _intAx << ", " << _maxBy - _intAy << "," << _maxBz - _intAz << "), NCC" << _maxNCCCoeffient ;


    return true;
}

//void NormalizedCrossCorrelate::NCCCore()
double NormalizedCrossCorrelate::NccCore(int _intAx, int _intAy, int _intAz,
                                         int _intBx, int _intBy, int _intBz)
{

    // ����ؼ���
    // Qncc = ��(framea * frameb) / sqrt(��framea^2 * ��frameb^2)

    double upValSum = 0;
    double downAValSum = 0;
    double downBValSum = 0;

    for (k = 0; k < _intLengthZ; k++)
    {
        for (j = 0; j < _intLengthY; j++)
            for (i = 0; i < _intLengthX; i++)
            {
                sA = cvGet2D(imgA[k], _intAy + j, _intAx + i);
                sB = cvGet2D(imgB[k + _intBz - actualz_start], _intBy + j, _intBx + i);
                // �����
                upValSum += sA.val[0] * sB.val[0];
                // ���ĸ
                downAValSum += sA.val[0] * sA.val[0];
                downBValSum += sB.val[0] * sB.val[0];
            }
    }

    double nccCoefficient = upValSum / sqrt(downAValSum * downBValSum);

//    int d = actualz_start;
//    qDebug() << "\timgA index:" << _intAz << "-" << _intAz + _intLengthZ
//             << "imgB index:" << _intBz - actualz_start << "-" << _intBz - actualz_start + _intLengthZ;
    qDebug() << "NCC:" << nccCoefficient << "A(" <<
                _intAx << "," << _intAy << "," << _intAz << ") B(" <<
                _intBx << "," << _intBy << "," << _intBz << ")";

    return nccCoefficient;
}

/***************************************************
NCC = ��(current_patch_nomalized * mean_patch_nomalized) / sqrt(��current_patch_nomalized^2 * ��mean_patch_nomalized^2)
Ф����  18:47:37
double CDepthFrame::CCNomalizedCrossCorrelation(CSFCIPatch mean_patch,CSFCIPatch reference_patch,ColorSpace color_space)
{
        //NCC�����ϵ�����жϸ�patch��mean_patch��������//
        // ��ʽ NCC = ��(current_patch_nomalized * mean_patch_nomalized) / sqrt(��current_patch_nomalized^2 * ��mean_patch_nomalized^2)
        //�����������൱�ڼ��������������ļн�
        CvScalar mean_patch_pixel_average, mean_patch_sqrted_sum;
        CSFCIPatch mean_patch_nomalized, mean_patch_nomalized_sqrted;
        double result = 0.0;
        CvScalar current_patch_average, temp, temp1;
        CSFCIPatch current_patch_nomalized,current_patch_nomalized_sqrted;
        if (mean_patch.GetPatchValid()&&reference_patch.GetPatchValid())		//�ж�patch��Ч�򲻽��м���
        {
                mean_patch_nomalized = mean_patch;
                current_patch_nomalized = reference_patch;
                mean_patch_nomalized.ChangeColorSpaceTo(spaceGRAY);
                current_patch_nomalized.ChangeColorSpaceTo(spaceGRAY);			//�Ҷȿռ����
                mean_patch_pixel_average = mean_patch_nomalized.GetPixelAverage();	//��ȡmean_patch�����ؾ�ֵ
                current_patch_average = current_patch_nomalized.GetPixelAverage();
                mean_patch_nomalized.Add(-mean_patch_pixel_average.val[0]);		//��һ������,ȥֱ������
                current_patch_nomalized.Add(-current_patch_average.val[0]);
                mean_patch_nomalized_sqrted = mean_patch_nomalized * mean_patch_nomalized;	//ƽ������
                current_patch_nomalized_sqrted = current_patch_nomalized * current_patch_nomalized;
                mean_patch_sqrted_sum = mean_patch_nomalized_sqrted.GetPixelSum();	//���
                current_patch_nomalized = current_patch_nomalized * mean_patch_nomalized;
                temp = current_patch_nomalized.GetPixelSum();
                temp1 = current_patch_nomalized_sqrted.GetPixelSum();
                result = sqrt(temp1.val[0] * mean_patch_sqrted_sum.val[0]);
                result = temp.val[0] / result;
                return result;
        }
        else
        {
                return result;
        }
}
****************************************************/
