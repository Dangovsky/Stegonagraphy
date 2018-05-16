#ifndef STEGANOGRAPHY_H
#define STEGANOGRAPHY_H

#include <vector>
#include <vector>
#include <cmath>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv/cv.h"
#include "opencv/highgui.h"

using namespace cv;
using namespace std;

class Steganography
{
    // Pseudorandom generator params
    int a = 0;
    int b = 0;
    int c = 0;
public:
    Steganography();
    
    vector<int> PseudoRandom(int a, int b, int c, int max, int size); // initialises parameters
    vector<int> PseudoRandom(int max, int size);

    Mat DiscreteHaarWaveletTransform(Mat image);
    Mat DiscreteHaarWaveletInverseTransform(Mat image);

    Mat Hide(Mat image, vector<char> data, vector<int> indexes);
    vector<char> Find(Mat image, vector<int> indexes);
};

#endif // STEGANOGRAPHY_H
