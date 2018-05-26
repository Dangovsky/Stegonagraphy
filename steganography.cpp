#include "steganography.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv/cv.h"
#include "opencv/highgui.h"

using namespace std;
using namespace cv;

Steganography::Steganography()
{
}

vector<int> Steganography::PseudoRandom(int max, int size)
{
    vector<int>* arr = new vector<int>(size,0);

    arr->at(0) = a;
    for (int i = 0; i < size - 1; i++)
    {
        arr->at(i + 1) = (b * arr->at(i) + a) % c;
    }

    for (int i = 0; i < size; i++)
    {
            arr->at(i) %= max;
    }
    return *arr;
}

vector<int> Steganography::PseudoRandom(int a, int b, int c, int max, int size)
{
    this->a = a;
    this->b = b;
    this->c = c;

    return PseudoRandom(max, size);
}

Mat Steganography::dwtHaar(const Mat &imgSrc)
{
    int w = imgSrc.cols,
        h = imgSrc.rows;

    Mat imgLL   = Mat::zeros(h/2, w/2, CV_32F),
        imgHL   = Mat::zeros(h/2, w/2, CV_32F),
        imgLH   = Mat::zeros(h/2, w/2, CV_32F),
        imgHH   = Mat::zeros(h/2, w/2, CV_32F),
        imgTmp1 = Mat::zeros(h/2, w,   CV_32F),
        imgTmp2 = Mat::zeros(h/2, w,   CV_32F);

    for(int y=0; y < h; y+=2)
    {
        for(int x=0; x < w; x++)
        {
            imgTmp1.at<float>(y/2,x) = (imgSrc.at<float>(y,x) + imgSrc.at<float>(y+1,x))*ISQ2;
            imgTmp2.at<float>(y/2,x) = (imgSrc.at<float>(y,x) - imgSrc.at<float>(y+1,x))*ISQ2;
        }
    }

    for(int y=0; y < h/2; y++)
    {
        for(int x=0; x < w; x+=2)
        {
            imgLL.at<float>(y,x/2) = (imgTmp1.at<float>(y,x) + imgTmp1.at<float>(y,x+1))*ISQ2;
            imgLH.at<float>(y,x/2) = (imgTmp1.at<float>(y,x) - imgTmp1.at<float>(y,x+1))*ISQ2;
        }
    }

    for(int y=0; y < h/2; y++)
    {
        for(int x=0; x < w; x+=2)
        {
            imgHL.at<float>(y,x/2) = (imgTmp2.at<float>(y,x) + imgTmp2.at<float>(y,x+1))*ISQ2;
            imgHH.at<float>(y,x/2) = (imgTmp2.at<float>(y,x) - imgTmp2.at<float>(y,x+1))*ISQ2;
        }
    }

    Mat imgDst(imgSrc.size(), imgSrc.type());
    // Копируем четверти на свои места
    imgLL.copyTo(imgDst(Rect(0,     0,     w/2, h/2)));
    imgHL.copyTo(imgDst(Rect(w/2,   0,     w/2, h/2)));
    imgLH.copyTo(imgDst(Rect(0,     h/2,   w/2, h/2)));
    imgHH.copyTo(imgDst(Rect(w/2,   h/2,   w/2, h/2)));

    return imgDst;
}

Mat Steganography::idwtHaar(const Mat &imgSrc)
{
    int w = imgSrc.cols,
        h = imgSrc.rows;

    Mat imgLL(imgSrc, Rect(0,   0,   w/2, h/2)),
        imgHL(imgSrc, Rect(w/2, 0,   w/2, h/2)),
        imgLH(imgSrc, Rect(0,   h/2, w/2, h/2)),
        imgHH(imgSrc, Rect(w/2, h/2, w/2, h/2)),
        imgTmp1 = Mat::zeros(h/2, w, CV_32F),
        imgTmp2 = Mat::zeros(h/2, w, CV_32F),
        imgTmp3 = Mat::zeros(h/2, w, CV_32F),
        imgTmp4 = Mat::zeros(h/2, w, CV_32F),
        imgTmp  = Mat::zeros(h,   w, CV_32F);

    for(int y=0; y < h/2; y++)
    {
        for(int x=0; x < w/2; x++)
        {
            imgTmp1.at<float>(y,x*2) = imgLL.at<float>(y,x);
            imgTmp2.at<float>(y,x*2) = imgLH.at<float>(y,x);
            imgTmp3.at<float>(y,x*2) = imgHL.at<float>(y,x);
            imgTmp4.at<float>(y,x*2) = imgHH.at<float>(y,x);
        }
    }

    for(int y=0; y < h/2; y++)
    {
        for(int x=0; x < w; x+=2)
        {
            float v1 = imgTmp1.at<float>(y,x),
                  v2 = imgTmp2.at<float>(y,x);
            imgTmp1.at<float>(y,x)   = (v1 + v2)*ISQ2;
            imgTmp1.at<float>(y,x+1) = (v1 - v2)*ISQ2;

            float v3 = imgTmp3.at<float>(y,x),
                  v4 = imgTmp4.at<float>(y,x);
            imgTmp3.at<float>(y,x)   = (v3 + v4)*ISQ2;
            imgTmp3.at<float>(y,x+1) = (v3 - v4)*ISQ2;
        }
    }

    Mat imgDst(imgSrc.size(), imgSrc.type());

    for(int y=0; y < h/2; y++)
    {
        for(int x=0; x < w; x++)
        {
            imgDst.at<float>(y*2,x) = imgTmp1.at<float>(y,x);
            imgTmp.at<float>(y*2,x) = imgTmp3.at<float>(y,x);
        }
    }

    for(int y=0; y < h; y+=2)
    {
        for(int x=0; x < w; x++)
        {
            float v1 = imgDst.at<float>(y,x),
                  v2 = imgTmp.at<float>(y,x);
            imgDst.at<float>(y,x)   = (v1 + v2)*ISQ2;
            imgDst.at<float>(y+1,x) = (v1 - v2)*ISQ2;
        }
    }
    return imgDst;
}

Mat Steganography::Hide(Mat pure_image, vector<char> data, vector<int> indexes)
{   
   Mat  image(pure_image.rows, pure_image.cols, CV_32FC3),
        imageBlue(image.rows, image.cols, CV_32FC1);

   if(pure_image.type() == 21){
       pure_image.copyTo(image);
   }else{
       pure_image.convertTo(image,CV_32FC3, 1/255.0);
   }

   int from_to[] = {0,0};
   mixChannels(image, imageBlue, from_to, 1);

   Mat s, u, v,
           haar1Lvl = dwtHaar(imageBlue),
           haar2Lvl = dwtHaar(Mat(haar1Lvl, Rect(haar1Lvl.cols / 2, haar1Lvl.rows / 2,haar1Lvl.cols / 2, haar1Lvl.rows / 2))),
           crop(haar2Lvl, Rect(0, 0, haar2Lvl.rows / 2, haar2Lvl.cols / 2));

   SVD::compute(crop, s, u, v, SVD::FULL_UV);

   int j = 0,
       exp;

   float x;
   for (int i = 0; i < indexes.size(); i++) {
       cout <<   s.at<float>(indexes.at(i), 0) << "\\ ";

       x = frexp(s.at<float>(indexes.at(i), 0), &exp);
       cout << x << "| ";

       x = int(x * STEG_W) / STEG_W;

       if (!((data.at(j / 8) >> (j % 8)) & 1)){ // 0
            x < 0 ?
                   x -= 0.25345 / STEG_W :
                   x += 0.25345 / STEG_W;
            cout << x << " . " <<  exp << "\\ 0 \\ ";
            s.at<float>(indexes.at(i), 0) = ldexp(x, exp);
       } else { // 1
           x < 0 ?
                  x -= 0.75345 / STEG_W :
                  x += 0.75345 / STEG_W;
           cout << x << " . " <<  exp << "\\ 1 \\ ";
           s.at<float>(indexes.at(i), 0) = ldexp(x, exp);
       }
       cout <<  s.at<float>(indexes.at(i), 0) << endl;
       j++;
   }
   cout <<  endl;

   crop = u * Mat::diag(s) * v;

   crop.copyTo(haar2Lvl(Rect(0, 0, haar2Lvl.rows / 2, haar2Lvl.cols / 2)));

   haar2Lvl = idwtHaar(haar2Lvl);

   haar2Lvl.copyTo(haar1Lvl(Rect(haar1Lvl.cols / 2, haar1Lvl.rows / 2, haar1Lvl.cols / 2, haar1Lvl.rows / 2)));
   imageBlue = idwtHaar(haar1Lvl);

   mixChannels(imageBlue, image, from_to, 1);

   return image;
}

vector<char> Steganography::Find(Mat full_image, vector<int> indexes)
{
    Mat image(full_image.rows, full_image.cols, CV_32FC3),
        imageBlue(image.rows, image.cols, CV_32FC1);

    if(full_image.type() == 21){
        full_image.copyTo(image);
    }else{
        full_image.convertTo(image,CV_32FC3, 1/255.0);
    }

    int from_to[] = {0,0};
    mixChannels(image, imageBlue, from_to, 1);

    Mat     s,
            haar1Lvl = dwtHaar(imageBlue),
            haar2Lvl = dwtHaar(Mat(haar1Lvl, Rect(haar1Lvl.cols / 2, haar1Lvl.rows / 2,haar1Lvl.cols / 2, haar1Lvl.rows / 2))),
            crop(haar2Lvl, Rect(0, 0, haar2Lvl.rows / 2, haar2Lvl.cols / 2));

    SVD::compute(crop, s, SVD::NO_UV);

    vector<char> data(indexes.capacity() / 8,0);
    int j = 0,
        exp;
    float x;
    vector<float>::iterator it;

    for (int i = 0; i < indexes.size(); i++){
        cout <<  s.at<float>(indexes.at(i), 0);
        x = frexp(s.at<float>(indexes.at(i), 0), &exp);

        cout << "/ " <<  x;
        x = int((x * STEG_W - (int)(x * STEG_W)) * 100.0);
        cout << "/ " <<  x << " . " << exp << endl;

        if (x < -50.0 | x > 50.0){
            data.at(j / 8) = data.at(j / 8) | (1 << (j % 8));
        }
        j++;
    }
    return data;
}

Mat Steganography::HideIm(Mat pure_image, Mat data)
{
   Mat  image(pure_image.rows, pure_image.cols, CV_32FC3),
        imageBlue(image.rows, image.cols, CV_32FC1);

   if(pure_image.type() == 21){
       pure_image.copyTo(image);
   }else{
       pure_image.convertTo(image,CV_32FC3, 1/255.0);
   }

   int from_to[] = {0,0};
   mixChannels(image, imageBlue, from_to, 1);

   Mat s, u, v, sd, ud, vd,
           haar1Lvl = dwtHaar(imageBlue),
           haar2Lvl = dwtHaar(Mat(haar1Lvl, Rect(haar1Lvl.cols / 2, haar1Lvl.rows / 2,haar1Lvl.cols / 2, haar1Lvl.rows / 2))),
           crop(haar2Lvl, Rect(0, 0, haar2Lvl.rows / 2, haar2Lvl.cols / 2));

   SVD::compute(crop, s, u, v, SVD::FULL_UV);
   SVD::compute(data, sd, ud, vd, SVD::FULL_UV);

   s.copyTo(ss) ; u.copyTo(su); v.copyTo(sv);

   addWeighted(s, 1 - STEG_W, sd, STEG_W, 0 ,s , -1);
   addWeighted(u, 1 - STEG_W, ud, STEG_W, 0 ,u , -1);
   addWeighted(v, 1 - STEG_W, vd, STEG_W, 0 ,v , -1);

   crop = u * Mat::diag(s) * v;

   crop.copyTo(haar2Lvl(Rect(0, 0, haar2Lvl.rows / 2, haar2Lvl.cols / 2)));

   haar2Lvl = idwtHaar(haar2Lvl);

   haar2Lvl.copyTo(haar1Lvl(Rect(haar1Lvl.cols / 2, haar1Lvl.rows / 2, haar1Lvl.cols / 2, haar1Lvl.rows / 2)));
   imageBlue = idwtHaar(haar1Lvl);

   mixChannels(imageBlue, image, from_to, 1);

   return image;
}

Mat Steganography::FindIm(Mat full_image, int w, int h)
{
    Mat     data(h, w, CV_32FC1),
            ds(h, 1, CV_32FC1),
            dv(h, h, CV_32FC1),
            du(w, w, CV_32FC1),
            image(full_image.rows, full_image.cols, CV_32FC3),
            imageBlue(image.rows, image.cols, CV_32FC1);

    if(full_image.type() == 21){
        full_image.copyTo(image);
    }else{
        full_image.convertTo(image,CV_32FC3, 1/255.0);
    }

    int from_to[] = {0,0};
    mixChannels(image, imageBlue, from_to, 1);

    Mat     s, u, v,
            haar1Lvl = dwtHaar(imageBlue),
            haar2Lvl = dwtHaar(Mat(haar1Lvl, Rect(haar1Lvl.cols / 2, haar1Lvl.rows / 2,haar1Lvl.cols / 2, haar1Lvl.rows / 2))),
            crop(haar2Lvl, Rect(0, 0, haar2Lvl.rows / 2, haar2Lvl.cols / 2));

    SVD::compute(crop, s, u, v, SVD::FULL_UV);

    //subtract(s, ss, ds);
    //subtract(u, su, du);
    //subtract(v, sv, dv);

    addWeighted(s, 1, ss, -1 + STEG_W, 0 ,ds , -1);
    addWeighted(u, 1, su, -1 + STEG_W, 0 ,du , -1);
    addWeighted(v, 1, sv, -1 + STEG_W, 0 ,dv , -1);


    data = du * Mat::diag(ds) * dv / STEG_W;

    return data;
}
