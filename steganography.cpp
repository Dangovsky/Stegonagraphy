#include "steganography.h"
#include <iostream>
#include <vector>
#include <cmath>
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

Mat Steganography::DiscreteHaarWaveletTransform(Mat image)
{
    int i, j;
    Mat* output = new Mat(image.rows, image.cols, image.type());
    for (i = 0; i < image.rows; i++){
        for (j = 0; j < image.cols / 2; j++){
             output->at<float>(i,j) = (image.at<float>(i,2 * j)  + image.at<float>(i,2 * j + 1)) / (float)sqrt(2);
             output->at<float>(i,j + output->cols / 2) = abs(image.at<float>(i,2 * j) - image.at<float>(i,2 * j + 1)) / (float)sqrt(2);
        }
    }

    output->copyTo(image);

    for (i = 0; i < image.rows / 2; i++){
        for (j = 0; j < image.cols; j++){
             output->at<float>(i,j) = (image.at<float>(2 * i,j) + image.at<float>(2 * i + 1,j)) / (float)sqrt(2);
             output->at<float>(i + output->rows / 2,j) = abs(image.at<float>(2 * i,j) - image.at<float>(2 * i + 1,j)) / (float)sqrt(2);
        }
    }
    return *output;
}

Mat  Steganography::DiscreteHaarWaveletInverseTransform(Mat image)
{
    int i, j;
    Mat* output = new Mat(image.rows, image.cols, image.type());
    for (i = 0; i < image.rows / 2; i++){
        for (j = 0; j < image.cols; j++){
             output->at<float>(2 * i + 1,j) = (image.at<float>(i,j) * (float)sqrt(2) + image.at<float>(i + image.rows / 2,j) * (float)sqrt(2)) / 2.0;
             output->at<float>(2 * i,j) = abs(image.at<float>(i,j) * (float)sqrt(2) - image.at<float>(i + image.rows / 2,j) * (float)sqrt(2)) / 2.0;
        }
    }

    output->copyTo(image);

    for (i = 0; i < image.rows; i++){
        for (j = 0; j < image.cols / 2; j++){
             output->at<float>(i,2 * j + 1) = (image.at<float>(i,j) * (float)sqrt(2) + image.at<float>(i,j + image.cols/ 2) * (float)sqrt(2)) / 2.0;
             output->at<float>(i,2 * j)  = abs(image.at<float>(i,j) * (float)sqrt(2) - image.at<float>(i,j + image.cols/ 2) * (float)sqrt(2)) / 2.0;
        }
    }
    return *output;
}

Mat Steganography::Hide(Mat pure_image, vector<char> data, vector<int> indexes)
{
   Mat image(pure_image.rows, pure_image.cols, CV_32FC3);
   if (pure_image.type() != CV_32FC3)
   {
       pure_image.convertTo(image,CV_32FC3, 1/255.0);
   }

   Mat imageBlue(image.rows, image.cols, CV_32FC1);

   int from_to[] = {0,0};
   mixChannels(image, imageBlue, from_to, 1);

   Mat s, u, v,
           haarOutput1Lvl = DiscreteHaarWaveletTransform(pure_image),
           haarOutput2Lvl = DiscreteHaarWaveletTransform(Mat(haarOutput1Lvl, Rect(0,0,haarOutput1Lvl.cols / 2, haarOutput1Lvl.rows / 2)));

   namedWindow( "haarOutputLvl", WINDOW_AUTOSIZE );
   imshow( "haarOutputLvl", haarOutput2Lvl);

   Mat blue(haarOutput2Lvl, Rect(0, 0, haarOutput2Lvl.rows / 2, haarOutput2Lvl.cols / 2));

   SVD::compute(Mat(blue, Rect(0, 0, blue.rows / 2, blue.cols / 2)), s, u, v, SVD::FULL_UV);

   namedWindow( "s", WINDOW_AUTOSIZE );
   imshow( "s", s);

   int j = 0;
   for (int i : indexes) {
       //s.at<Vec3f>(s.rows / 2, indexes[i]).val[0] = (s.at<Vec3f>(indexes[i], indexes[i]).val[0] & 0xFFFE) | ((ch >> (i % 8)) & 1);
       //s.at<Vec3f>(s.rows / 2, indexes[i]).val[0] += alfa * ((ch >> (i % 8)) & 1);
       cout <<   s.at<float>(i, 0) << "/";
       if ((data.at(j / 8) >> (j % 8)) & 1)
       {    //1
           s.at<float>(i, 0) = (float)((char)(s.at<float>(i, 0) * 255.0) | 0x07) / 255.0;
       }
       else
       {    //0
           //s.at<float>(i, 0) = (float)((char)(s.at<float>(i, 0) * 255.0) ^ 0x07) / 255.0;
       }
       cout <<  s.at<float>(i, 0) << " ";
       j++;
   }
   cout <<  endl;

   namedWindow( "s1", WINDOW_AUTOSIZE );
   imshow( "s1", s);

   s = u * Mat::diag(s) * v;

   s.copyTo(blue(Rect(0, 0, blue.rows / 2, blue.cols / 2)));
   mixChannels(blue, haarOutput2Lvl, from_to, 1);

   namedWindow( "haarOutput2Lvl", WINDOW_AUTOSIZE );
   imshow( "haarOutput2Lvl", haarOutput2Lvl);

   haarOutput2Lvl = DiscreteHaarWaveletInverseTransform(haarOutput2Lvl);

   haarOutput2Lvl.copyTo(haarOutput1Lvl(Rect(0,0,haarOutput1Lvl.cols / 2, haarOutput1Lvl.rows / 2)));
   haarOutput1Lvl = DiscreteHaarWaveletInverseTransform(haarOutput1Lvl);

   return haarOutput1Lvl;
}

vector<char> Steganography::Find(Mat full_image, vector<int> indexes)
{

    Mat image;
    if (image.type() == CV_32FC3)
    {
        full_image.copyTo(image);
    }
    else
    {
        image = Mat(full_image.rows, full_image.cols, CV_32FC3);
        for (int i = 0; i < full_image.rows; i++){
            for (int j = 0; j < full_image.cols; j++){
                for (int k = 0; k < 3; k++){
                    image.at<Vec3f>(i,j).val[k] = (float)(full_image.at<Vec3b>(i,j).val[k] / 255.0);
               }
            }
        }
    }

    vector<char>* data = new vector<char>(indexes.capacity() / 8, 0);

    Mat s,
            haarOutput1Lvl = DiscreteHaarWaveletTransform(image),
            haarOutput2Lvl = DiscreteHaarWaveletTransform(Mat(haarOutput1Lvl, Rect(0,0,haarOutput1Lvl.cols / 2, haarOutput1Lvl.rows / 2)));

    Mat blue(haarOutput2Lvl.rows, haarOutput2Lvl.cols, CV_32FC1);

    int from_to[] = {0,0};
    mixChannels(haarOutput2Lvl, blue, from_to, 1);

    SVD::compute(Mat(blue, Rect(0, 0, blue.rows / 2, blue.cols / 2)), s, SVD::NO_UV);

    int k, j = 0;
    char ch, count;
    for (int i : indexes) {
        //data->at(j) = ((data->at(j) >> (j % 8)) & 0xFE) | (s.at<Vec3f>(s.rows / 2, i).val[0] & 1);
        cout <<   s.at<float>(i, 0) << " ";
        ch = (char)(s.at<float>(i, 0) * 255.0);

        count = 0;
        for (k = 0; k < 4; k++)
        {
            count += (ch >> k) & 1;
        }

        if(count > 1)
        {//1
            data->at(j / 8) |=  1 << (j % 8);
        }
        //0
            //data->at(j / 8) |= ((data->at(j / 8) >> (j % 8)) & 0xFF) << (j % 8);


        j++;
    }

    return *data;
}
