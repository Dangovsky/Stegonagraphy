#include <QCoreApplication>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include "opencv/cv.h"
#include <iostream>
#include <vector>
#include "steganography.h"

using namespace cv;
using namespace std;

void FirstTest()
{
    Steganography stg;
    Mat pure_image = imread("test.jpg");
    //namedWindow( "Pure image", WINDOW_AUTOSIZE );
    //imshow( "Pure image", pure_image );

    Mat image = Mat(pure_image.rows, pure_image.cols, CV_32FC3);
    for (int i = 0; i < pure_image.rows; i++){
        for (int j = 0; j < pure_image.cols; j++){
            for (int k = 0; k < 3; k++){
                image.at<Vec3f>(i,j).val[k] = (float)(pure_image.at<Vec3b>(i,j).val[k] / 255.0);
           }
        }
    }
    namedWindow( "double_image", WINDOW_AUTOSIZE );
    imshow( "double_image", image);


    Mat haar = stg.DiscreteHaarWaveletTransform(image);
    namedWindow( "Haar", WINDOW_AUTOSIZE );
    imshow( "Haar", haar);

    /*
    Mat ihaar = stg.DiscreteHaarWaveletInverseTransform(haar);
    namedWindow( "haar1", WINDOW_AUTOSIZE );
    imshow( "haar1", ihaar);


    haar = stg.DiscreteHaarWaveletTransform(haar);
    namedWindow( "Haar2", WINDOW_AUTOSIZE );
    imshow( "Haar2", haar);

    waitKey(0);

    haar = stg.DiscreteHaarWaveletInverseTransform(haar);
    namedWindow( "haar2", WINDOW_AUTOSIZE );
    imshow( "haar2", haar);

    waitKey(0);
    */

    Mat double_haar(haar.rows, haar.cols, CV_32FC1);
    int from_to[] = {0,0};

    mixChannels(haar, double_haar, from_to, 1);
    namedWindow( "haarCrop1", WINDOW_AUTOSIZE );
    imshow( "haarCrop1", double_haar);
    double_haar = Mat(double_haar, Rect(0,0,double_haar.rows / 2, double_haar.cols / 2));
    namedWindow( "haarCrop", WINDOW_AUTOSIZE );
    imshow( "haarCrop", double_haar);

    /*
    double_haar.convertTo(double_haar, CV_32F, 1/255.0);
    namedWindow( "double_haar", WINDOW_AUTOSIZE );
    imshow( "double_haar", double_haar);

    waitKey(0);
    */

    Mat s, u, v;
    SVD::compute(double_haar, s,u,v, SVD::FULL_UV);
    namedWindow( "S", WINDOW_AUTOSIZE );
    imshow( "S", s);
    namedWindow( "U", WINDOW_AUTOSIZE );
    imshow( "U", u);
    namedWindow( "V", WINDOW_AUTOSIZE );
    imshow( "V", v);

    double_haar = u * Mat::diag(s) * v;
    namedWindow( "S1", WINDOW_AUTOSIZE );
    imshow( "S1", double_haar);

    double_haar.convertTo(double_haar, CV_8U, 255.0);
    namedWindow( "uchar_haar", WINDOW_AUTOSIZE );
    imshow( "uchar_haar", double_haar);

    Mat ihaar = stg.DiscreteHaarWaveletInverseTransform(haar);
    namedWindow( "haar1", WINDOW_AUTOSIZE );
    imshow( "haar1", ihaar);

    waitKey(0);
}

void SecTest()
{
    Steganography stg;
    Mat pure_image = imread("test.jpg");
    //namedWindow( "Pure image", WINDOW_AUTOSIZE );
    //imshow( "Pure image", pure_image );

    /*
    Mat image = Mat(pure_image.rows, pure_image.cols, CV_32FC3);
    for (int i = 0; i < pure_image.rows; i++){
        for (int j = 0; j < pure_image.cols; j++){
            for (int k = 0; k < 3; k++){
                image.at<Vec3f>(i,j).val[k] = (float)(pure_image.at<Vec3b>(i,j).val[k] / 255.0);
           }
        }
    }
    namedWindow( "double_image", WINDOW_AUTOSIZE );
    imshow( "double_image", image);
    */

    Mat image = Mat(pure_image.rows, pure_image.cols, CV_32FC3);
    pure_image.convertTo(image, CV_32FC3, 1/255.0);

    Mat blue(image.rows, image.cols, CV_32FC1);
    int from_to[] = {0,0};

    mixChannels(image, blue, from_to, 1);
    namedWindow( "blue", WINDOW_AUTOSIZE );
    imshow( "blue", blue);

    Mat haar = stg.DiscreteHaarWaveletTransform(blue);
    namedWindow( "haar", WINDOW_AUTOSIZE );
    imshow( "haar", haar);

    Mat crop = Mat(haar, Rect(0,0,haar.rows / 2, haar.cols / 2));
    namedWindow( "crop", WINDOW_AUTOSIZE );
    imshow( "crop", crop);

    Mat inverse = stg.DiscreteHaarWaveletInverseTransform(haar);
    namedWindow( "inverse", WINDOW_AUTOSIZE );
    imshow( "inverse", inverse);

    waitKey(0);
}

void StegTest()
{
    vector<char>* data = new vector<char>{'d', 'a', 't', 'a'};
    vector<int>* indexes = new vector<int>(data->capacity() * 8);
    for (int i = 0; i < indexes->capacity(); i++)
    {
        indexes->at(i) = i * 3;
    }
    Steganography stg;

    Mat pure_image = imread("test.tif");
    Mat full_image = stg.Hide(pure_image, *data, *indexes);
    vector<char> out_data = stg.Find(full_image, *indexes);

    namedWindow( "pure_image", WINDOW_AUTOSIZE );
    imshow( "pure_image", pure_image);

    waitKey(0);

    namedWindow( "full_image", WINDOW_AUTOSIZE );
    imshow( "full_image", full_image);

    waitKey(0);
}

int main(int argc, char *argv[])
{
    SecTest();
    return 0;
}

