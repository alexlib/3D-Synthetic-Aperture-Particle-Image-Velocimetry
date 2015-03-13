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
#pragma comment(lib,"highgui200.lib")
*/

//CvMat* C�ǲο�������ڲξ���,�Դ�����
void ProjectImage(IplImage* Image, CvMat* C, CvMat* R, CvMat* T, CvMat* Refer_C,
                                        CvMat* Refer_R, CvMat* Refer_T,
                            CvMat* PlaneNormalVector, int Imagetype)
{
    //�������ͼ��Ͳο�ͼ��֮����ڿռ���ĳ��ƽ��ĵ�Ӧ����

    /******************************************************/
    /*  �ⲿ��Ϊ����ο�ͼ��������ͼ����ڳ�����һ��ƽ��ĵ�Ӧ����    */
    /******************************************************/

    CvMat* Product[8];
    for(int i=0;i<8;i++)
            Product[i]=cvCreateMat(3,3,CV_64FC1);

    CvMat* InvofRefer_R=cvCreateMat(3,3,CV_64FC1);

    cvT(Refer_R,InvofRefer_R);

    //Product[0] = R*Rf^T
    cvMatMul(R,InvofRefer_R,Product[0]);

    CvMat* TPlaneNormalVector=cvCreateMat(1,3,CV_64FC1);
    cvT(PlaneNormalVector,TPlaneNormalVector);

    //Product[1] = Tf*N^T
    cvMatMul(Refer_T,TPlaneNormalVector,Product[1]);

    CvMat* I=cvCreateMat(3,3,CV_64FC1);
    CvScalar ratio;
    ratio=cvRealScalar(1);

    //��ʼ����λ��
    cvSetIdentity(I,ratio);

    cvSub(I,Product[1],Product[2],0);
    //Product[2] = I - Tf*N^T
    //Product[3] = (R * Rf^T) * (I - Tf*N^T)
    cvMatMul(Product[0],Product[2],Product[3]);
    //Product[4] = T * N^T
    cvMatMul(T,TPlaneNormalVector,Product[4]);
    //Product[5] = (R * Rf^T) * (I - Tf*N^T) + T * N^T
    cvAdd(Product[3],Product[4],Product[5],0);
    //Product[6] = C * [(R * Rf^T) * (I - Tf*N^T) + T * N^T]
    cvMatMul(C,Product[5],Product[6]);

    CvMat* InvofRefer_C=cvCreateMat(3,3,CV_64FC1);

    cvInv(Refer_C,InvofRefer_C,CV_LU);
    //Product[7] = C * [(R * Rf^T) * (I - Tf*N^T) + T * N^T] *Cf^-1
    cvMatMul(Product[6],InvofRefer_C,Product[7]);

    /*������ĵ�Ӧ����ֵ������homography[3][3]*/

    double homography[3][3],*Homographydata;
    for(int i=0;i<3;i++)
    {
            for(int j=0;j<3;j++)
                    homography[i][j]=0;
    }

    Homographydata=Product[7]->data.db;

    //������õ��ĵ�Ӧ����ֵ��homography[][];
    for(int i=0;i<3;i++)
    {
         for(int j=0;j<3;j++)
              homography[i][j]=Homographydata[i*3+j];
    };

    //���ڴ洢�����任��ͼ��
    IplImage* ProjectedImage=cvCreateImage(cvSize(Image->width,Image->height),IPL_DEPTH_8U,3);
    cvZero(ProjectedImage);
    double point[3];
    for(int i=0;i<3;i++)
            point[i]=0;
    CvScalar s;

    for(int height=0;height<Image->height;height++)  //����任��ͼ������ص��ڱ任ǰͼ���е�λ�ò���ֵ
        {
            for(int width=0;width<Image->width;width++)
            {
                for(int k=0;k<3;k++)
                        point[k]=homography[k][0]*width+homography[k][1]*height+homography[k][2];
                point[0]=point[0]/point[2];
                point[1]=point[1]/point[2];        //�������ת��Ϊ���������
                int a,b;
                a=point[0];
                b=point[1];                       //���ص�����ȡ������doubleתΪint

                if(a>=0&&a<Image->width&&b>=0&&b<Image->height)
                {
                        s=cvGet2D(Image,b,a);
                        cvSet2D(ProjectedImage,height,width,s);
                }
            };
        };

    //���任���ͼ���Ƶ�ԭͼ��Ĵ洢�ռ���
    cvCopy(ProjectedImage,Image,0);

    //���¼������ڲ���֡�任�Ƿ���ȷ
    //��ʾ�任���ͼ��
    //cvNamedWindow( "ProjectImage",CV_WINDOW_AUTOSIZE);
    //cvShowImage( "ProjectImage", Image);
    //cvResizeWindow("ProjectImage",Image->width,Image->height);
    //cvWaitKey(0);
    //cvDestroyWindow( "ProjectImage" );

    cvReleaseImage(&ProjectedImage);
    cvReleaseMat(&InvofRefer_R);
    cvReleaseMat(&TPlaneNormalVector);
    cvReleaseMat(&I);
    cvReleaseMat(&InvofRefer_C);

    for(int i=0;i<8;i++)
        cvReleaseMat(&Product[i]);

}
