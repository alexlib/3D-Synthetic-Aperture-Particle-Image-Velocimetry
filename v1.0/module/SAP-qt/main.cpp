#include <QtCore/QCoreApplication>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

#include "projectimage.h"
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"

#include <QDebug>

int cameranumber = 4;
int cameraArraySize = 4;

using namespace std;

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

    //��������������ļ��Թ���ȡ
    CvFileStorage* Read_Param = cvOpenFileStorage("calibration_result_x.xml", memstorage, CV_STORAGE_READ );

    CvMat* IntrinsicMatrix=cvCreateMat(3,3,CV_64FC1);//����ǲο�������ڲ�
    CvMat* RotationMatrix=cvCreateMat( 3, 3 ,CV_64FC1);   //����ǲο��������ת����
    CvMat* TranslationMatrix=cvCreateMat( 3, 1 ,CV_64FC1);    //����ǲο������ƽ������
    CvMat* Refer_Intrinsic=cvCreateMat(3,3,CV_64FC1);//����ο�������ڲ�
    CvMat* Refer_Rotation=cvCreateMat(3,3,CV_64FC1);//������������
    CvMat* Refer_Translation=cvCreateMat(3,1,CV_64FC1);//�����������ƽ������
    char num[3];
    num[2]='\0';
    sprintf(num,"%d",reference_camera); //���ο������תΪ�ַ����ͣ����¼��д���Ϊ��ȡ�ο��������

    string paramname="";
    string paramname_I,paramname_R,paramname_T;

    paramname="cam";
    //paramname = "camera_";
    paramname=paramname.append(num);
    //paramname=paramname+"_";
    paramname_I=paramname+"intrinsic_matrix";
    paramname_R=paramname+"rotation_matrix";
    paramname_T=paramname+"translation_vectors";
    //paramname_T = paramname+"trasnlation_vector";

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
        //paramname = "camera_";
        paramname=paramname.append(num);
        //paramname=paramname+"_";
        paramname_I=paramname+"intrinsic_matrix";
        paramname_R=paramname+"rotation_matrix";
        paramname_T=paramname+"translation_vectors";
        //paramname_T = paramname+"trasnlation_vector";
/*
        paramname = "camera_";
        paramname=paramname.append(num);
        paramname=paramname+"_";
        paramname_I=paramname+"intrinsic_matrix";
        paramname_R=paramname+"rotation_matrix";
        //paramname_T=paramname+"translation_vectors";
        paramname_T = paramname+"trasnlation_vector";
*/
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

int main( int argc,char** argv)
{
    QCoreApplication a(argc, argv);

    ifstream fpic("pic_p.txt");
    if(fpic.is_open())
        qDebug() << "find file!";
    else
        qDebug() << "can't find file!";
    string picname;
    int Reference_cam = 1;
    int imagenumber = 4;
    double distance = 3148;
    int WorldorCamera = 0;
    /*cout<<"please input the number of cameras for SAP video:\n";
    cin>>cameranumber;
    cout<<"please input the number of images for SAP video:\n";
    cin>>imagenumber;		//��Ƶ��ͼ���֡��
    cout<<"please input the Reference camera for viewpoint selection[1-64]:\n";
    cin>>Reference_cam;   //�ο����
    cout<<"choose the coordination, 0or1, correspond to world and camera coordination:\n";
    cin>>WorldorCamera;
    cout<<"please input the distance between the focal plane and the reference camera(mm)(default 3418):\n";
    cin>>distance;*/
    CvVideoWriter * writer = 0;
    int isColor = 1;
    int fps = 20; // or 30
    int frameW = 752; //  camera CCD resolution
    int frameH = 576;
    //writer=cvCreateVideoWriter("out.avi",-1,fps,cvSize(frameW,frameH),isColor); //д���ɫ֡?��������򲻷�,��������©
    /*����CV_FOURCC(��P��,��I��,��M��,��1��) = MPEG-1 codec
      CV_FOURCC(��M��,��J��,��P��,��G��) = motion-jpeg codec (does not work well)
      CV_FOURCC(��M��, ��P��, ��4��, ��2��) = MPEG-4.2 codec
      CV_FOURCC(��D��, ��I��, ��V��, ��3��) = MPEG-4.3 codec
      CV_FOURCC(��D��, ��I��, ��V��, ��X��) = MPEG-4 codec
      CV_FOURCC(��U��, ��2��, ��6��, ��3��) = H263 codec
      CV_FOURCC(��I��, ��2��, ��6��, ��3��) = H263I codec
      CV_FOURCC(��F��, ��L��, ��V��, ��1��) = FLV1 codec     */
    //IplImage* img1=cvCreateImage(cvSize(752,576),IPL_DEPTH_8U,1);

    //����֡�Ƿ����
    int test=1;
    while(test<=imagenumber)
    {
        std::getline(fpic,picname);
        qDebug() << "1"<<picname.c_str() << "2";
        int i;
        ifstream fdirect("direct_p.txt");
        if(fdirect.is_open())
            qDebug() << "find file!";
        else
            qDebug() << "can't find file!";

        string directname;
        IplImage* new_pic1 = cvCreateImage(cvSize(frameW,frameH),IPL_DEPTH_8U,3);
        IplImage *pImg[cameraArraySize];
        for(i=0;i<cameraArraySize;i++)
        {
             pImg[i]=NULL;
        }
        //pImg=new IplImage*[cameranumber];  //��̬����һ��ͼ������
        i=0;  //���ڼ����������Ƿ񳬳���ȡ��Χ
        while(i<cameranumber)//��ȡͬ֡ͼ��
        {
            std::getline(fdirect,directname);
            qDebug() << "getimage:"<<directname.c_str();

            string filename;
            filename=directname+picname;
            char *filename1=(char*)filename.c_str();
            pImg[i]=cvLoadImage(filename1,1);
            //��ȡ3ͨ��ͼ��

            //���¼��д���Ϊ����֡�Ƿ���ȷ��ȡ
            //cvNamedWindow("Original",CV_WINDOW_AUTOSIZE);
            //cvShowImage("Original",pImg[i]);
            //cvWaitKey(0);
            //cvDestroyWindow("Original");

            i++;
        }

        //��ͼ����кϳɿ׾�����
        SAP(pImg,new_pic1,Reference_cam,WorldorCamera,distance);

/*      cvNamedWindow( "SAP_PIC",CV_WINDOW_AUTOSIZE);*/
/*      */
        // ��ʾ�ϳ�ͼ��
        cvShowImage( "SA_PIV", new_pic1);
        // д����Ƶ
        //cvWriteFrame(writer,new_pic1); //
        // �����ļ�
        cvSaveImage(("D:\\" + picname).c_str(),new_pic1); //

// 		cvWaitKey(0);
// 		cvDestroyWindow( "SAP_PIC" );

        cvReleaseImage( &new_pic1 );
        //�ͷ�ͼ��洢�ռ�
        for(i=0;i<cameraArraySize;i++)
        {
            if (pImg[i])
            {
                cvReleaseImage(&pImg[i]);
            }
            else
                break;
        }
        test++;
    }
    //cvReleaseVideoWriter(&writer); //

    //printf("tttttttttt88888888888888888888888888888888888888888888888888888888888\n");

    cout<<"SAP�������";
    cvWaitKey('q');

    return a.exec();

}
