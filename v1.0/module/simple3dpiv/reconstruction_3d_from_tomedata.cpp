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
#include "reconstruction_3d_from_tomedata.h"
#include "mycvimage_widget.h"

#include <QDebug>
#include <fstream>

// �൱���ڽ��п����˲�
int Reconstruction3DFromTomeData::filterSpatial = 200;

Reconstruction3DFromTomeData::Reconstruction3DFromTomeData() : QObject(),
                                                Reconstruction3DIntensity()
{
    i = 0;
    j = 0;
    k = 0;//���
    num = 0;
    max = 0;
    currentX = -1, currentY = -1, currentZ = -1;
    tempPointX = 0;
    currentPointNum = 0;
    sumPointVal = 0;

    files = std::vector<string>();

    img = NULL;
}

Reconstruction3DFromTomeData::~Reconstruction3DFromTomeData()
{
    if (img)
        cvReleaseImage(&img);

    files.clear();
}

void Reconstruction3DFromTomeData::RawImagesProcess()
{

    static bool rowElementEnd = false;
    static int return_nextPIterator = 5;

    static bool initailIter = false;// �����Ѿ���ʼ��

    string pathfile;

    this->depth = files.size();
    if (this->depth == 0)
    {
        qDebug() << "[process]You have not read image data files!";
        return;
    }

    for (k = 0; k < files.size(); k++)
    {

        pathfile = path + files[k];
        if (img)
            cvReleaseImage(&img);
        img = cvLoadImage(pathfile.data(), 1);//CV_LOAD_IMAGE_GRAYSCALE);

        this->width = img->width;
        this->height = img->height;
        qDebug() << "[height,width,depth]:" << width << ", " << height << ", " << depth;

        for (i = 0; i < img->height; i++)
            for (j = 0; j < img->width; j++)
            {
                s = cvGet2D(img, i, j);

                if (max < s.val[0])
                    max = s.val[0];

                // ������У���������Ϊ��ʼ
                if (i != currentY || k != currentZ)
                {
                    initailIter = false;
                }

                if (initailIter == false)
                {
                    insertIter = temp3DPoints.begin();
                    piterator = temp3DPoints.begin();
                    qDebug() <<"in(k,i)("<< k<< ","<<i<< "):"<<(*piterator)->xpos;
                    rowElementEnd = false;
                    initailIter = true;
                    // ��ʼ��tempP
                    if (piterator != temp3DPoints.end())
                        tempP = (*piterator);
                }

                // ��ͬһ�У���������һ�������ص㣬˵������һ����
                if (s.val[0] > filterSpatial && i == currentY)
                {
                    sumPointVal += s.val[0];
                    currentPointNum++;
                }
                // ����ͬһ�У�ǿ�Ƹ��м����ꣻ���ڸ�������������filterSpatial�����ص�
                else if ((k != currentZ && currentPointNum != 0) || (i != currentY && currentPointNum != 0) ||
                         (i == currentY && currentPointNum != 0 && s.val[0] <= filterSpatial))
                {

                    tempPointX = currentX + 1 - (int)currentPointNum / 2;

                    // ��ʼ�������Ƚ�
                    if ((*piterator) != NULL)
                    {
                        // ��Ȼ����һ�еĵ�
                        if (i != currentY)
                        {
                            // ���³�ʼ��
                            initailIter = false;
                            currentY = i - 1;
                        }

                        if (rowElementEnd == false && piterator != temp3DPoints.end()
                                &&  currentX - (*piterator)->xpos > 4)
                        {
                            // ������piterator����
                            return_nextPIterator = this->getNextPiteratorLastYpos(tempP, tempPointX, currentY - 1, k - 1);
                            if (return_nextPIterator < 0)
                                rowElementEnd = true;
                            else
                                rowElementEnd = false;
                        }

                        qDebug() << "\t[curr]xpos:" << tempPointX << ", ypos:" << currentY << ", zpos" << k
                                 << ", val:" << sumPointVal / currentPointNum;

                        // (һ). yPos����, xPos�ӽ�,��Ϊ��һ����, ��ʱ�ӵ��ڵ��next��
                        if (rowElementEnd == false && piterator != temp3DPoints.end() &&
                                (tempPointX - tempP->xpos < 4 && tempPointX - tempP->xpos > -4))
                        {
                            if ((tempP->zpos == k && i - tempP->ypos <= 4 && i - tempP->ypos > -4)
                                || (tempP->zpos + 1 == k && i - tempP->ypos <= 4 && i - tempP->ypos > -4))
                            {
                                Temp3DPoint * p = new Temp3DPoint;
                                p->xpos = tempPointX;
                                p->ypos = i;
                                p->zpos = k;
                                p->val = sumPointVal / currentPointNum;
                                p->next = NULL;

                                while (tempP->next != NULL)
                                    tempP = tempP->next;

                                tempP->next = p;
                                num++;
        \
                                qDebug() << "[n]x,y,z(num):" << tempPointX << "," <<
                                            currentY << "," << k << "(" << num << "), val" << sumPointVal / currentPointNum;

                            }
                        }
                        // (��). ��yPos������, x����һ����ͬ��insert��list�У�����һ�нڵ���ƽ������������
                        else if (rowElementEnd == false && piterator != temp3DPoints.end()
                                 && ((tempP->zpos == k && tempP->ypos + 1 < currentY)
                                        || tempP->zpos + 1 < k)
                                 && (tempPointX - tempP->xpos < 4 && tempPointX - tempP->xpos > -4))
                        {

                            insertToList(tempPointX, i, k, sumPointVal / currentPointNum);

                            num++;

                            qDebug() << "[i]x,y,z(num):" << tempPointX << ", " << currentY << "(" << num << ")" <<
                                  " s.val[0]: " << sumPointVal / currentPointNum;

                            // ���õ�����ָ���val��ypos, ��������
                            deleteRedundant(piterator, false);
                        }
                        // (��). ��xPos����һ�в�ͬ������yPos,zpos��ͬ��񣬲��뵽list��
                        else
                        {
                            // ��Ȼ����һ�еĵ�
                            if (i != currentY || k != currentZ)
                            {
                                // ���³�ʼ��
                                initailIter = false;
                                currentY = i - 1;
                                currentZ = k - 1;
                            }

                            insertToList(tempPointX, i, k, sumPointVal / currentPointNum);

                            num++;

                            qDebug() << "[i]x,y,z(num):" << tempPointX << ", " << i << "," << k
                                     << "(" << num << ")" << " s.val[0]: " << sumPointVal / currentPointNum;
                        }
                    }
                    currentPointNum = 0;
                    sumPointVal = 0;
                }

                // ��ͬһ�У�����û�м������еĵ㣬û�е�
                else if (i == currentY && currentPointNum == 0 && s.val[0] <= filterSpatial)
                {
                    // ��1�п�ʼ�������Ƚ�
                    if (i > 0 && piterator != temp3DPoints.end())
                    {
                        tempP = NULL;
                        tempP = (*piterator);
                        // Ѱ��yPos����
                        while (tempP->next != NULL)
                            tempP = tempP->next;

                        if (rowElementEnd == false && currentX - tempP->xpos > 4)
                        {
                            return_nextPIterator = this->getNextPiteratorLastYpos(tempP, currentX, i - 1, k - 1);

                            if (return_nextPIterator < 0)
                                rowElementEnd = true;
                            else
                                rowElementEnd = false;
                        }

                        // ����һ���м�����Ļ����������࣬��ǰ�Ѿ�û�е���
                        else if (rowElementEnd == false
                                 && ((tempP->zpos == k && tempP->ypos + 1 < currentY) || tempP->zpos < k)
                                && currentX - tempP->xpos <= 4 && currentX - tempP->xpos > -4)
                        {
                            // ���õ�����ָ���val��ypos, ��������
                            deleteRedundant(piterator, false);
                        }
                        /*else if (rowElementEnd == false && tempP->zpos < k
                                 && currentX - tempP->xpos <= 4 && currentX - tempP->xpos > -4)
                        {
                            deleteRedundant(piterator, false);
                        }*/
                    }
                }

                currentY = i;
                currentX = j;
                currentZ = k;
            }
        reconstructProgressSignal(k);
    }

    qDebug() << "max = " << max;


    // ͼ�����
    /*CvScalar setS;
    std::list<Temp3DPoint*>::iterator iter;

    qDebug() << "size" << temp3DPoints.size();

    for (iter = temp3DPoints.begin(); iter != temp3DPoints.end(); iter++)
    {
        setS.val[0] = 0;
        setS.val[1] = 255;
        setS.val[2] = 0;

        if (0 < (*iter)->ypos && (*iter)->ypos < img->height &&
                0 < (*iter)->xpos && (*iter)->xpos < img->width)
            cvSet2D(img, (*iter)->ypos, (*iter)->xpos, setS);
    }
    cvSaveImage("D:\\first.bmp",img);

    MyCVImageWidget *mw = new MyCVImageWidget(img);
    mw->show();
    */
    return;

}

void Reconstruction3DFromTomeData::loadImageFiles(std::vector<string> *filename, string path)
{

    if (path.length() == 0 || (*filename).size() == 0)
    {
        //qDebug() << "no file in loadImageFiles";
        return;
    }
    this->path = path;

    fstream _file;
    string pathfile;

    for (int nn = 0; nn < (*filename).size(); nn++)
    {
        pathfile = path + (*filename).at(nn);
        _file.open(pathfile.data(), ios::in);
        if(_file)
        {
            //qDebug() << "has-file" << pathfile.data();
            files.push_back((*filename).at(nn));
            _file.close();
            continue;
        }
        else
        {
            //qDebug() << "no-file" << pathfile.data();
            continue;
        }
    }
}

void Reconstruction3DFromTomeData::reconstructProgressSignal(int s)
{
    emit reconstructProgress(s);
}
