#include "reconstruction_3dintensity.h"


#include <QFile>
#include <QTextStream>
#include <QString>

#include <QDebug>

Reconstruction3DIntensity::Reconstruction3DIntensity()
{
    height = 0;
    width = 0;
    depth = 0;

    tempP = NULL;

    temp3DPoints = list<Temp3DPoint*>();
    piterator = temp3DPoints.begin();

}

Reconstruction3DIntensity::~Reconstruction3DIntensity()
{
    temp3DPoints.clear();
}

void Reconstruction3DIntensity::loadImageFiles(std::vector<string> *filename, string path)
{
    if (path.length() == 0 || (*filename).size() == 0)
    {
        return;
    }
    this->path = path;
    for (int nn = 0; nn < (*filename).size(); nn++)
    {
            files.push_back((*filename).at(nn));
    }
}

std::list<Temp3DPoint*> Reconstruction3DIntensity::getCurrent3DPointsList()
{
    return temp3DPoints;
}

long Reconstruction3DIntensity::index(int x, int y, int z)
{
    return x * this->height * this->depth + y * this->depth + z;
}

// �����⣬��������ʱlastZPos��������
/**
 * @return ���ص�����״̬����tempP�ķ���ֵ����������һ��ƽ����
 *         -1 ��ʾʧ�ܣ�û���ҵ����Բ���ڵ�ĵط��������ϰ汾�ĺ�������ֵtrue���������Ƶ�����λ��Ϊ��true��
 *         0  ��ʾ�ҵ����ڵ�ǰzpos����lastZPos+1λ�ô��ҵ�zpos��ͬ��y���ڵĵ㣬��lastZPos��û���������ص�
 *               ��ͬһ��ͼ��Ŀɲ����ά����ڵ�ĵ㣩����������������ڵ�
 *         1  ��ʾ�ҵ����ڵ�ǰzpos����lastZPos�����ҵ��������ص㣬��������������ڵ�
 *         /----ɾ��/2  ��ʾ�ҵ����ڵ�ǰzposû���ҵ�������lastZPoos���ҵ��������ص㣬��������������ڵ�
 */
int Reconstruction3DIntensity::getNextPiteratorLastYpos(
    Temp3DPoint*& tempP, int tempPointX ,int lastYPos, int lastZPos)
{
    static int times = 0;// ��¼�����������Ƿ����

    if (temp3DPoints.end() == piterator)
        return -1;

    while ((*piterator) != NULL && piterator != temp3DPoints.end())
    {
        tempP = NULL;

        tempP = (*piterator);
        // ��bug
        if (tempP->ypos == lastYPos + 1 && tempP->zpos == lastZPos + 1)
        {
            times++;
            piterator++;
        }

        // Ѱ��yPos����
        while (tempP != NULL)
        {
            if(tempP->xpos >= tempPointX - 4)
            {
                // ֻ�ڵ�ǰ���ƽ���������ڵ㣬lastZPos���û�����ڵ�
                if (tempP->zpos > lastZPos && tempP->ypos >= lastYPos)
                {
                    if (times != 0)
                    {
                        qDebug() << "\t[itTp]xpos:" << tempP->xpos << ", ypos:" << tempP->ypos <<
                                     ", zpos:" << tempP->zpos << ", val:" << tempP->val << ", next:" << tempP->next;
                        times = 0;
                        return 0;
                    }
                }
                else if (tempP->zpos == lastZPos && tempP->ypos >= lastYPos - 4 && tempP->ypos < lastYPos + 4)
                {
                    if (times != 0)
                    {
                        qDebug() << "\t[itTp]xpos:" << tempP->xpos << ", ypos:" << tempP->ypos <<
                                     ", zpos:" << tempP->zpos << ", val:" << tempP->val << ", next:" << tempP->next;
                        times = 0;
                        return 1;
                    }
                }
            }
            tempP = tempP->next;
        }

        times++;
        piterator++;

    }

    //if (tempP != NULL)
    qDebug() << "\t[itTp]ypos:" << lastYPos + 1 << ",zpos:" << lastZPos + 1 <<"end()";
    times = 0;
    // ��������
    return -1;

}

bool Reconstruction3DIntensity::insertToList(int x, int y, int z, int val)
{
    Temp3DPoint * p = new Temp3DPoint;
    p->xpos = x;
    p->ypos = y;
    p->zpos = z;
    p->val = val;
    p->next = NULL;

    // �����λ��
    long tempIndex = this->index(x, y, z); // �������
    long cmpindex = 0;

    Temp3DPoint *tempP = NULL;
    while (insertIter != temp3DPoints.end())
    {
        tempP = (*insertIter);
        // Ѱ��yPos����
        // while (tempP->next != NULL)
        //    tempP = tempP->next;

        cmpindex =  this->index(tempP->xpos, tempP->ypos, tempP->zpos); // �Ƚϵ�
        if (cmpindex < tempIndex)
            insertIter++;
        else if (cmpindex > tempIndex)
        {
            temp3DPoints.insert(insertIter, p);
            return true;
        }
    }

    // ���뵽ĩβ
    temp3DPoints.insert(insertIter, p);

    return true;
}

bool Reconstruction3DIntensity::deleteRedundant(std::list<Temp3DPoint*>::iterator deletePoint, bool deleteFromList)
{
    static int deletenum = 0;
    Temp3DPoint * newValP = new Temp3DPoint;
    Temp3DPoint * resetValP = (*deletePoint);
    Temp3DPoint * deleteP;
    int sumVal = resetValP->val;
    int sumYPos = resetValP->ypos;
    int sumXPos = resetValP->xpos;
    int sumZPos = resetValP->zpos;

    int samePynum = 1;
    while (resetValP->next != NULL)
    {
        resetValP = resetValP->next;
        sumVal += resetValP->val;
        sumYPos += resetValP->ypos;
        sumXPos += resetValP->xpos;
        sumZPos += resetValP->zpos;
        samePynum++;
    }
    if (samePynum > 1)
    {
        newValP->val = sumVal / samePynum;
        newValP->ypos = sumYPos / samePynum;
        newValP->xpos = sumXPos / samePynum;
        newValP->zpos = sumZPos / samePynum;
        newValP->next = NULL;
    }
    else
    {
        return false;
    }

    // ɾ��ԭ�ڵ�
    for (int d = 0; d < samePynum - 1; d++)
    {
        deleteP = (*deletePoint)->next;
        (*deletePoint)->next = deleteP->next;
        delete deleteP;
        deletenum++;
        qDebug() << "\t[delete:num]" << deletenum;
    }

    //std::list<Temp3DPoint*>::iterator iter = deletePoint;

    qDebug() << "\t[remove:x,y.z]" << (*deletePoint)->xpos << ","
             << (*deletePoint)->ypos << "," << (*deletePoint)->zpos;

    // Ҫ�������� ��ֹ������ʧЧ���������������β����Ҫ����
    temp3DPoints.remove(*(deletePoint++));

    // ��Z���ʱ��ȷ��Ϊһ���㣬ֱ�Ӵ�list��ɾ����д���ļ�
    if (deleteFromList == true)
    {
        static int num = 0;
        // �����ļ�
        //QFile file(QString(("D:\\"+"3dPoints_1.txt").data()));
        QFile file("D:\\3dPoints_1.txt");

        // ׷��д���������Ϣ
        if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
            return true;

        QTextStream out(&file);

        out << "num:" << num << ":(x,y,z)=("
            << newValP->xpos << "," << newValP->ypos << "," << newValP->zpos << "),\tindex:"
                << index(newValP->xpos,newValP->ypos,newValP->zpos) << "\n";

        file.close();
        delete newValP;

        if (deletePoint == temp3DPoints.begin())
            return true;
        else
        {
            deletePoint--;
            if (deletePoint == temp3DPoints.begin())
                // ������ʼ���ڿ�ʼ������
                return true;
        }
        return true;
    }

    // û��z��ȵ�ɾ���ڵ㣬��д���ļ�

    if (deletePoint == temp3DPoints.begin())
    {
        // ������ʼ���ڿ�ʼ������
        temp3DPoints.insert(temp3DPoints.begin(), newValP);
        qDebug() << "\t[reinsert:x,y.z]" << newValP->xpos << ","
                 << newValP->ypos << "," << newValP->zpos;
        return true;

    }
    else
    {
        deletePoint--;
        if (deletePoint == temp3DPoints.begin())
        {
            // ������ʼ���ڿ�ʼ������
            temp3DPoints.insert(temp3DPoints.begin(), newValP);
            qDebug() << "\t[reinsert:x,y.z]" << newValP->xpos << ","
                     << newValP->ypos << "," << newValP->zpos;
            return true;

        }
        else
        {
            long tempIndex = this->index(newValP->xpos, newValP->ypos, newValP->zpos); // �������
            long cmpindex;

            // ���²���ڵ�
            while (deletePoint != temp3DPoints.begin())
            {
                cmpindex =  this->index((*deletePoint)->xpos, (*deletePoint)->ypos, (*deletePoint)->zpos); // �Ƚϵ�

                if (cmpindex > tempIndex)
                    deletePoint--;
                else
                {
                    //deletePoint--;
                    deletePoint++;
                    temp3DPoints.insert(deletePoint, newValP);
                    qDebug() << "\t[reinsert:x,y.z]" << newValP->xpos << ","
                             << newValP->ypos << "," << newValP->zpos;
                    return true;
                }
            }
        }
    }

    return false;
}

void Reconstruction3DIntensity::initail3dPointsResultFile(string resultFile)
{
    QFile file(QString(resultFile.data()));

    file.remove();
}

void Reconstruction3DIntensity::savePointsToFile(string saveFile)
{
    int num = 0;
    if (temp3DPoints.size() == 0)
    {
        qDebug() << "no points to save";
        return;
    }

    /*QString filename = QFileDialog::getSaveFileName(
       this,
       QObject::tr("����TXT�ļ����"),
       QDir::currentPath(),
       "Document files (*.txt);;All files(*.*)");
    if (filename.isNull()) {

        // �û�ȡ��ѡ��
        QMessageBox::information(this, QObject::tr("����TXT�ļ����"),
                                 QObject::tr("û��ָ��Ҫ������ļ�Ŀ��"), QMessageBox::Ok | QMessageBox::Cancel);
    } else {
        //�û�ѡ�����ļ�
        QMessageBox::information(this, QObject::tr("����TXT�ļ����"),
                                 QObject::tr("�����ļ��ɹ�"), QMessageBox::Ok | QMessageBox::Cancel);
*/
    // �����ļ�
    QFile file(QString(saveFile.data()));

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);

    // �����ʽ
    std::list<Temp3DPoint*>::iterator iter;

    //iter = temp3DPoints.begin();
    //temp3DPoints.remove((*iter));

    qDebug() << "size" << temp3DPoints.size();

    for (iter = temp3DPoints.begin(); iter != temp3DPoints.end(); iter++)
    {
        num++;
        out << "num:" << num << ":(x,y,z)=(" << (*iter)->xpos << "," << (*iter)->ypos << "," << (*iter)->zpos << "),\tindex:"
            << index((*iter)->xpos,(*iter)->ypos,(*iter)->zpos) << "\n";
    }
    file.close();

}

