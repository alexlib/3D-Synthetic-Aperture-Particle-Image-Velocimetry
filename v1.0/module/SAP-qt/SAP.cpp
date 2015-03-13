// SAP.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include "ProjectImage.h"
using namespace std;

/*#pragma comment(lib,"cxcore200.lib")
#pragma comment(lib,"cv200.lib")
#pragma comment(lib,"highgui200.lib")*/

/*int cameranumber;

void SAP(IplImage* PIC[], IplImage* new_pic, int reference_camera, int WorldorCamera,double distance)  //�����Ѿ���ͶӰ��ͼ������ۼӺ;�ֵ����
{
        int pic_num=cameranumber;                //����ϳ�ͼ���֡��=�������Ŀ
        int pic_w=PIC[0]->width;                 //����ͼ����
        int pic_h=PIC[0]->height;
        CvScalar pixel;

        CvMat *ImagePixel_total[3];//�洢����ֵ(3��ͨ��)�ܺ�[752][576]
        for(int k=0;k<3;k++)
                ImagePixel_total[k]=cvCreateMat(pic_h,pic_w,CV_64FC1);
        CvMat *ImagePixel_num=cvCreateMat(pic_h,pic_w,CV_32SC1);//�洢�������ظ���
        for(int k=0;k<3;k++) //��ʼ������
             cvZero(ImagePixel_total[k]);
        cvZero(ImagePixel_num);
        CvMemStorage* memstorage = cvCreateMemStorage(0);
        CvFileStorage* Read_Param = cvOpenFileStorage( "calibration_result_x.xml", memstorage, CV_STORAGE_READ );//��������������ļ��Թ���ȡ
        CvMat* IntrinsicMatrix=cvCreateMat(3,3,CV_64FC1);//����ǲο�������ڲ�
        CvMat* RotationMatrix=cvCreateMat( 3, 3 ,CV_64FC1);   //����ǲο��������ת����
        CvMat* TranslationMatrix=cvCreateMat( 3, 1 ,CV_64FC1);    //����ǲο������ƽ������
        CvMat* Refer_Intrinsic=cvCreateMat(3,3,CV_64FC1);//����ο�������ڲ�
        CvMat* Refer_Rotation=cvCreateMat(3,3,CV_64FC1);//������������
        CvMat* Refer_Translation=cvCreateMat(3,1,CV_64FC1);//�����������ƽ������
        char num[3];
        num[2]='\0';
        sprintf(num,"%d",reference_camera); //���ο������תΪ�ַ����ͣ����¼��д���Ϊ��ȡ�ο��������
        string paramname="cam";
        paramname=paramname.append(num);
        string paramname_I,paramname_R,paramname_T;
        paramname_I=paramname+"intrinsic_matrix";
        paramname_R=paramname+"rotation_matrix";
        paramname_T=paramname+"translation_vectors";
        char *paramname1=(char *)paramname_I.c_str();
        Refer_Intrinsic=(CvMat*)cvReadByName(Read_Param, NULL, paramname1, NULL);
        paramname1=(char*)paramname_R.c_str();
        Refer_Rotation=(CvMat*)cvReadByName(Read_Param, NULL, paramname1, NULL);
        paramname1=(char*)paramname_T.c_str();
        Refer_Translation=(CvMat*)cvReadByName(Read_Param, NULL, paramname1, NULL);
        CvMat* PlaneNormalVector=cvCreateMat(3,1,CV_64FC1);//������������ϵ�µľ۽�ƽ��ķ�����
        double P[3][1]={0,0,-1.0/3.4184e+003};				//�۽�ƽ�淨����
        P[2][0]=P[2][0]*3418.4/distance;
        cvInitMatHeader(PlaneNormalVector,3,1,CV_64FC1,P);//��ʼ���۽�ƽ�淨����
        CvMat* PlaneNormalVectorC=cvCreateMat(3,1,CV_64FC1);//����ο����������ϵ�¾۽�ƽ��ķ�����
        if(WorldorCamera==0)//�����û���ͬ��ѡ��ת������ϵ
        {
            cvMatMul(Refer_Rotation,PlaneNormalVector,PlaneNormalVectorC);
                cvCopy(PlaneNormalVectorC,PlaneNormalVector,0);
        }
        for(int l=0;l<pic_num;l++)
        {
                //ProjectImage(IplImage* Image,CvMat* C,CvMat* R,CvMat* T,CvMat* Refer_C,CvMat* Refer_R, CvMat* Refer_T, CvMat* PlaneNormalVector,int WorldorCamera,int Imagetype);//CvMat* C�ǲο�������ڲξ���,�Դ�����,�ԻҶ�ͼ����ͶӰ�任
                sprintf(num,"%d",l+1);
                paramname="cam";
                paramname=paramname.append(num);
                paramname_I=paramname+"intrinsic_matrix";
                paramname_R=paramname+"rotation_matrix";
                paramname_T=paramname+"translation_vectors";
                if((l+1)!=reference_camera)//��ͼ��Ϊ�ǲο�ͼ���������ͶӰ�任
                {
                        paramname1=(char *)paramname_I.c_str();
                        IntrinsicMatrix=(CvMat*)cvReadByName(Read_Param, NULL, paramname1, NULL);
                        paramname1=(char*)paramname_R.c_str();
                        RotationMatrix=(CvMat*)cvReadByName( Read_Param, NULL, paramname1, NULL);
                    paramname1=(char *)paramname_T.c_str();
                    TranslationMatrix=(CvMat*)cvReadByName( Read_Param, NULL, paramname1, NULL);//��ȡ"calibration_result.xml"�е�R,T�����
                        //��ͼ�����ͶӰ
            ProjectImage(PIC[l],IntrinsicMatrix,RotationMatrix,TranslationMatrix,Refer_Intrinsic,Refer_Rotation,Refer_Translation,PlaneNormalVector,GRAYIMAGE);
                }
        }

        for(int l=0;l<pic_num;l++)
        {
                for(int i=0;i<pic_h;i++)
                {
                        for(int j=0;j<pic_w;j++)
                        {

                                        pixel=cvGet2D(PIC[l],i,j);	//���صĵ���
                                        for(int k=0;k<3;k++)
                                        {
                                             double mm=CV_MAT_ELEM(*ImagePixel_total[k],double,i,j);
                                             mm+=pixel.val[k];
                                             CV_MAT_ELEM(*ImagePixel_total[k],double,i,j)=mm;
                                        }
                                if((pixel.val[0]||pixel.val[1]||pixel.val[2])!=0)//��������ͳ��
                                        {
                                                int nn=CV_MAT_ELEM(*ImagePixel_num,int,i,j);
                                                nn++;
                                                CV_MAT_ELEM(*ImagePixel_num,int,i,j)=nn;
                                        }
                        }
                }
        }
        for(int i=0;i<pic_h;i++)//��SAPͼ����и�ֵ
        {
                for(int j=0;j<pic_w;j++)
                {
                        if(CV_MAT_ELEM(*ImagePixel_num,int,i,j)!=0)
                        {
                                for(int k=0;k<3;k++)
                                   pixel.val[k]=CV_MAT_ELEM(*ImagePixel_total[k],double,i,j)/CV_MAT_ELEM(*ImagePixel_num,int,i,j);
                            cvSet2D(new_pic,i,j,pixel);
                        }
                }

        }
        for(int k=0;k<3;k++)
        cvReleaseMat(&ImagePixel_total[k]);
        cvReleaseMat(&ImagePixel_num);
        cvReleaseFileStorage(&Read_Param);
        cvReleaseMemStorage(&memstorage);
        cvReleaseMat(&Refer_Intrinsic);
        cvReleaseMat(&Refer_Rotation);
        cvReleaseMat(&Refer_Translation);
        cvReleaseMat(&IntrinsicMatrix);
        cvReleaseMat(&RotationMatrix);
        cvReleaseMat(&TranslationMatrix);
        cvReleaseMat(&PlaneNormalVector);
        cvReleaseMat(&PlaneNormalVectorC);
}

*/





