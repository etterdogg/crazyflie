#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>
#include <stdio.h>
#include "libfreenect_cv.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <time.h>
#include <unistd.h>

//#define ADJ_GAMMA
#define GAMMA_EXPONENT 2.0

using namespace cv;
using namespace std;


int main(int argc, char **argv)
{
	//clock_t time;
#ifdef ADJ_GAMMA
	unsigned char gamma[256];
	for(int i = 0; i < 256; i++){

				if(i < 100){
					gamma[i] = 0;
				} else {

				gamma[i] = (char) round( pow((float)(2*(i-127.5))/255.0, GAMMA_EXPONENT)*255);
				}

			}
#endif

	int x = 0;
	int y = 0;
	int z = 0;
	//time = clock();
	//int sleep_time = 1 / CLOCKS_PER_SEC;

	while (cvWaitKey(10) < 0) {
		Mat image = freenect_sync_get_rgb_cv(0);
		//IplImage *depth = freenect_sync_get_depth_cv(0);

		/*
		if (!image) {
		    printf("Error: Kinect not connected?\n");
		    return -1;
		}
		*/

	    GaussianBlur(image,image, Size(9,9), 6,6);
#ifdef ADJ_GAMMA
		unsigned char *a = (unsigned char*) image.data;
		for(int i = 0; i < 307200; i++){

			a[i] = gamma[a[i]];
		}
#endif

	    vector<Vec3f> circles;

	   /// Apply the Hough Transform to find the circles

	    HoughCircles( image, circles, CV_HOUGH_GRADIENT, 1 ,400, 200, 2, 15, 100 );

/*
	    if(circles.size() != 0){
	    x = circles[0][0];
	    y = circles[0][1];
	    z = depth->imageData[x*1280+y*2];
	    }
*/

	    printf("x %d y %d z %d\n", x, y, z);
	   /// Draw the circles detected
	    for( size_t i = 0; i < circles.size(); i++ )
	    {

	         Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
	         int radius = cvRound(circles[i][2]);
	         // circle center
	         circle( image, center, 3, Scalar(255,0,0), -1, 8, 0 );
	         // circle outline
	         circle( image, center, radius, Scalar(0,0,255), 3, 8, 0 );
	    }

		imshow("IR", image);
	}
	return 0;
}
