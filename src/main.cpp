#include <OpenNI.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
//general c/c++ includes
#include <vector>
#include <stdio.h>
#include <math.h>
#include <string.h>
//sockets include
#include <sys/socket.h>
#include <unistd.h>
#include<arpa/inet.h>
#include <errno.h>

using namespace std;
using namespace cv;
using namespace openni;





//code to get the depth data
uint findDepth(vector<Vec3f> circles, cv::Mat depth) {
	puts("trying depth");
	 uint16_t pixelData = depth.at<uint16_t>( (int)circles[0][1], (int)circles[0][0]);
	return pixelData;
}

//code to get the yaw error
float findYaw(vector<Vec3f> circles, Mat *image, int distance){
	if (distance < 800 || distance > 2500){
			return -100;
		}
	float x = circles[0][0];
	float y = circles[0][1];

	int lookAtY = (int) ((30.0 * ( 1000.0 / (float)distance)) +y ); // calculating where to start looking for motor reflector
	int linesToCheck =(int) ( 30.0 * (1000.0 / (float)distance)); //calculating how many rows to search for motor reflector

	printf("lookAtY %d, linesToCheck %d \n", lookAtY, linesToCheck);
	int sum = 0;
	int maxSum = 0;
	float xPos = -100;
	int yPos;

	if((lookAtY+linesToCheck) > 450 || (lookAtY+linesToCheck) < 5 || x < 30 || x > 610 ){
		return -100;
	}

	for(int j = 0; j < (linesToCheck-6); j++){ // these four for loops checks for maximum sum of intensity in a sliding 6x4 matrix, sliding linesToCheck number of lines and around 40 pixels on x axis
		for(int i = -24; i <= 20; i++){


			//calculating sum of 4x6 rectangle at position j,i
			for(int k = 0; k < 4 ; k++){
				for(int l = 0; l < 6 ; l++){
					//printf("trying y : %d, x %d \n", (int)lookAtY+j+l, (int)x+i+k);
					sum += (*image).at<uint8_t>((int)(lookAtY+j+l), (int)(x+i+k));
					if((lookAtY+j+l) > 600){
						return -100;
					}
				}
			}

			if(sum > maxSum){
				maxSum = sum;
				xPos = i+2;
				yPos = lookAtY + j;
			}
			sum = 0;

		}
	}

   	Point center(cvRound((int)xPos+x), cvRound(yPos));
    circle( *image, center, 3, Scalar(255,0,0), -1, 8, 0 );



	return xPos;
}




int main()
{

    try {

        	openni::OpenNI::initialize();
        	openni::Device device;
        	int ret = device.open(openni::ANY_DEVICE);
        	if ( ret != openni::STATUS_OK ) {

        		puts("device error\n");
        		return 0;
        }


        	openni::VideoStream depthStream;
        	openni::VideoStream videoStream;

        	videoStream.create(device, openni::SENSOR_IR);
        	depthStream.create(device, openni::SENSOR_DEPTH);

        	//setting videomodes
        	openni::VideoMode vmode = videoStream.getVideoMode();
        	openni::VideoMode pmode = depthStream.getVideoMode();
        	vmode.setResolution(640, 480);
        	pmode.setResolution(640, 480);
        	vmode.setFps(30);
        	pmode.setFps(30);
        	videoStream.setVideoMode(vmode);
        	depthStream.setVideoMode(pmode);

        	//starting streams
        	videoStream.start();
        	depthStream.start();

        	//creating windows
        	cvNamedWindow("Image", CV_WINDOW_AUTOSIZE);
        	cvResizeWindow("Image", 640, 480);
        	cvNamedWindow("Depth", CV_WINDOW_AUTOSIZE);
        	cvResizeWindow("Depth", 640, 480);

        	//checking streams
        	if(!depthStream.isValid() || !videoStream.isValid()){
        		puts("Depth stream not initialized");
        	}
        	//Image containers for openCv
        	cv::Mat image;
        	cv::Mat depth;
#ifdef USE_SOCKET
        	//initializing socket for communication;
        	int socket_desc;
        	struct sockaddr_in server;
            //char message[2 + 3*sizeof(int)];
        	char message[1024];
            //Create socket
            socket_desc = socket(AF_INET , SOCK_STREAM , 0);
            if (socket_desc == -1)
            {
                puts("Could not create socket");
            }

            server.sin_addr.s_addr = inet_addr("192.168.10.49");
            server.sin_family = AF_INET;
            server.sin_port = htons(50007);


            //Connect to remote server
            if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) )
            {
                printf("Error when connecting! %s\n",strerror(errno));
                puts("connect error");
                return 1;
            }

            puts("Connected\n");
#endif
            bool conIsAlive = true;
            float yaw = 0;
            float x = 0;
            float y = 0;
            float z = 0;

            while (conIsAlive) {
        		//getting new image data, blocking functions
                openni::VideoFrameRef colorFrame;
                openni::VideoFrameRef depthFrame;
                videoStream.readFrame( &colorFrame );
                depthStream.readFrame( &depthFrame );


                if ( colorFrame.isValid() && depthFrame.isValid() ) {
                	//going from frame to openCv image container
                    image = cv::Mat(videoStream.getVideoMode().getResolutionY(), videoStream.getVideoMode().getResolutionX(), CV_16U, (char*)colorFrame.getData() );
                    depth = cv::Mat(depthStream.getVideoMode().getResolutionY(), depthStream.getVideoMode().getResolutionX(), CV_16U, (char*)depthFrame.getData() );

                    //converting from 16 to 8 bit images
                    image.convertTo( image, CV_8U );


                    //image processing
                    cv::threshold(image, image, 200, 255, 3);
                    cv::GaussianBlur(image,image, Size(7,7), 10,10);

                    //finding quad
                    vector<Vec3f> circles;
                    cv::HoughCircles( image, circles, CV_HOUGH_GRADIENT, 1 ,640, 200, 10, 10, 40 );

                    x= -100;
                    y= -100;
                    z= -100;
                    if(circles.size() > 0){
                    	//if statement below checks if the circle have jumped position since last frame, this would indicate bad recognition
                    	if(! ( (((x-circles[0][1]) > 20 && circles[0][1]-x > 20) && x != -100 )  ||  ((y-circles[0][0] > 20 && circles[0][0] -y > 20) && y != -100  ))){

                    		//painting found circle and setting x,y,z

                   	    	Point center(cvRound(circles[0][0]), cvRound(circles[0][1]));
                   	         int radius = cvRound(circles[0][2]);
                   	         // circle center
                   	         circle( image, center, 3, Scalar(255,0,0), -1, 8, 0 );
                   	         // circle outline
                   	         circle( image, center, radius, Scalar(0,0,127), 3, 8, 0 );
                   	         x = circles[0][0];
                   	         y = circles[0][1];
                   	         z = findDepth(circles, depth);
                   	         float newYaw = findYaw(circles, &image, z); //lp filtering yaw
                   	         if(newYaw > -20){
                   	        	 yaw = 0.9*yaw + 0.1*newYaw;
                   	         }

                   	         printf("z : %f x: %f y: %f yaw: %f\n ", z,x,y, yaw);
                   	    } else {
                   	    	puts("found erroneous circle");
                   	    }
                    }
#ifdef USE_SOCKET

                   	sprintf(message, " %f %f %f\n",x,y,z);
                   	    if( send(socket_desc , message , strlen(message) , 0) < 0)
                   	    {
                   	        puts("Send failed");
                   	        conIsAlive = false;

                   	    }
#endif

                   	//showing images
                   	    depth = depth*8;
                   	cv::imshow( "Depth", depth );
                	cv::imshow( "Image", image );


                }

                //q for quit
            int key = cv::waitKey( 10 );
            if ( key == 'q' ) {
                break;
            }
        }

        //cleaning up
        videoStream.stop();
        depthStream.stop();
        device.close();

    }
    catch ( std::exception& ) {
    	printf("error");
    }

    return 0;
}



