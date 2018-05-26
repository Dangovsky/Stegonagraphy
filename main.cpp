#include <QCoreApplication>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <vector>
#include "steganography.h"

using namespace cv;
using namespace std;

void FirstTest()
{
    Steganography stg;
    Mat pure_image = imread("test.jpg", cv::IMREAD_COLOR);
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


    Mat haar = stg.dwtHaar(image);
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

    Mat ihaar = stg.idwtHaar(haar);
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

    Mat haar = stg.dwtHaar(blue); // stg.DiscreteHaarWaveletTransform(blue);
    namedWindow( "haar", WINDOW_AUTOSIZE );
    imshow( "haar", haar);

    Mat crop = Mat(haar, Rect(0,0,haar.rows / 2, haar.cols / 2));
    namedWindow( "crop", WINDOW_AUTOSIZE );
    imshow( "crop", crop);

    Mat inverse = stg.idwtHaar(haar);  // stg.DiscreteHaarWaveletInverseTransform(haar);
    namedWindow( "inverse", WINDOW_AUTOSIZE );
    imshow( "inverse", inverse);

    waitKey(0);
}

void ThTest(){
    Steganography stg;
    Mat pure_image = imread("test.tif"),
        full_image;
    stg.Hide(pure_image,vector<char>(0),vector<int>(0)).convertTo(full_image, CV_8UC3, 255.0);

    namedWindow( "pure_image", WINDOW_AUTOSIZE );
    imshow( "pure_image", pure_image);

    namedWindow( "full_image", WINDOW_AUTOSIZE );
    imshow( "full_image", full_image);

    waitKey(0);
}

void StegTest()
{
    vector<char> data = vector<char>{'h', 'e', 'l', 'l', 'o'};
    vector<int> indexes = vector<int>(data.capacity() * 8);
    for (int i = 0; i < indexes.capacity(); i++)
    {
        indexes.at(i) = 128 - i;
    }
    Steganography stg;

    Mat pure_image = imread("test.tif"),
        full_image = stg.Hide(pure_image, data, indexes),
        converted,
        saved;

    full_image.convertTo(converted, CV_8UC3, 255.0);

    imwrite( "test1.tif", converted);

    saved = imread("test1.tif");

    vector<char> out_data = stg.Find(saved, indexes);

    namedWindow( "converted", WINDOW_AUTOSIZE );
    imshow( "converted", converted);

    namedWindow( "saved", WINDOW_AUTOSIZE );
    imshow( "saved", saved);

    namedWindow( "full_image", WINDOW_AUTOSIZE );
    imshow( "full_image", full_image);

    namedWindow( "pure_image", WINDOW_AUTOSIZE );
    imshow( "pure_image", pure_image);

    waitKey(0);
}

void FloatTest(){
    float f = 109 / 255.0,
            x, xn, x1, f1, f2, fn;
    int exp, exp1;
    char ch;
    const float STEG_W = 100.0;

    x = frexp(f, &exp);
    fn = ldexp(x, exp);
    cout << exp << " . " << x << " | ";

    x = int(x * STEG_W) / STEG_W;
    cout << x << " | ";

    xn = x + 0.25345 / STEG_W;
    cout << xn << " | ";

    f1 = ldexp(xn, exp);
    cout << f1 << " \ ";

    ch = (char)(f1*255.0);
    f2 = ch / 255.0;

    x1 = frexp(f2, &exp1);
    cout << exp1 << " . " << x1 << " | ";

    cin >> f;
}

void StegImTest(){
    Steganography stg;
    Mat     pure_image = imread("test.tif"),
            data = imread("data.png", IMREAD_GRAYSCALE);

    data.convertTo(data, CV_32FC1, 1/255.0);

    Mat     full_image = stg.HideIm(pure_image, data),
            out_data(data.rows, data.cols, data.type()),
            converted(data.rows, data.cols, CV_8UC1);

    stg.FindIm(full_image, out_data.rows, out_data.cols);
    out_data.convertTo(converted, CV_8UC1, 255.0);

    namedWindow( "pure_image", WINDOW_AUTOSIZE );
    imshow( "pure_image", pure_image);

    namedWindow( "data", WINDOW_AUTOSIZE );
    imshow( "data", data);

    namedWindow( "full_image", WINDOW_AUTOSIZE );
    imshow( "full_image", full_image);

    namedWindow( "out_data", WINDOW_AUTOSIZE );
    imshow( "out_data", out_data);

    namedWindow( "converted", WINDOW_AUTOSIZE );
    imshow( "converted", converted);

    waitKey(0);
    waitKey(0);
}

int main(int argc, char *argv[])
{
    StegImTest();
    return 0;
}

