#include <lo/lo.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>

#include "libfreenect_cv.h"

int pitch_x = 400;
int pitch_y1 = 100;
int pitch_y0 = 300;
int pitch_z = 350;

int volume_x0 = 400;
int volume_x1 = 100;
int volume_y = 300;
int volume_z = 350;

IplImage *GlViewColor(IplImage *depth) {
	static IplImage *image = 0;
	if (!image) image = cvCreateImage(cvSize(640,480), 8, 3);
	unsigned char *depth_mid = (unsigned char*)(image->imageData);
	int i;
	for (i = 0; i < 640*480; i++) {
		int lb = ((short *)depth->imageData)[i] % 256;
		int ub = ((short *)depth->imageData)[i] / 256;
		switch (ub) {
			case 0:
				depth_mid[3*i+2] = 255;
				depth_mid[3*i+1] = 255-lb;
				depth_mid[3*i+0] = 255-lb;
				break;
			case 1:
				depth_mid[3*i+2] = 255;
				depth_mid[3*i+1] = lb;
				depth_mid[3*i+0] = 0;
				break;
			case 2:
				depth_mid[3*i+2] = 255-lb;
				depth_mid[3*i+1] = 255;
				depth_mid[3*i+0] = 0;
				break;
			case 3:
				depth_mid[3*i+2] = 0;
				depth_mid[3*i+1] = 255;
				depth_mid[3*i+0] = lb;
				break;
			case 4:
				depth_mid[3*i+2] = 0;
				depth_mid[3*i+1] = 255-lb;
				depth_mid[3*i+0] = 255;
				break;
			case 5:
				depth_mid[3*i+2] = 0;
				depth_mid[3*i+1] = 0;
				depth_mid[3*i+0] = 255-lb;
				break;
			default:
				depth_mid[3*i+2] = 0;
				depth_mid[3*i+1] = 0;
				depth_mid[3*i+0] = 0;
				break;
		}
	}
	return image;
}

void showAntennas(IplImage* im) {
	cvLine(im, cvPoint(pitch_x, pitch_y0), cvPoint(pitch_x, pitch_y1), cvRealScalar(pitch_z), 5, 8, 0);
	cvLine(im, cvPoint(volume_x0, volume_y), cvPoint(volume_x1, volume_y), cvRealScalar(volume_z), 5, 8, 0);
}

void set_antennas(int event, int x, int y, int flags, void* parms) {
	int* mouse_status = (int*)parms;
	switch (event) {
		case CV_EVENT_LBUTTONUP:
			if (mouse_status[0] == 0) {
				pitch_x = x;
				pitch_y0 = y;
				pitch_y1 = y;
			} else {
				pitch_y1 = y;
			}
			mouse_status[0] = 1 - mouse_status[0];
			break;
		case CV_EVENT_RBUTTONUP:
			if (mouse_status[1] == 0) {
				volume_x0 = x;
				volume_x1 = x;
				volume_y = y;
			} else {
				volume_x1 = x;
			}
			mouse_status[1] = 1 - mouse_status[1];
			break;
	}
}

int main(int argc, char **argv) {
	float pitch = 440.0, volume = 1.0;
	int k, show_antennas = 0;
	lo_address chuck = lo_address_new(NULL, "8765");
	int mouse_status[2] = {0, 0};
	cvNamedWindow("RGB", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Depth", CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback("Depth", set_antennas, &mouse_status);
	while ((k = cvWaitKey(10)) != '\n') {
		IplImage *image = freenect_sync_get_rgb_cv(0);
		if (!image) {
			fprintf(stderr, "Error: Kinect not connected?\n");
			return -1;
		}
		cvCvtColor(image, image, CV_RGB2BGR);
		IplImage *depth = freenect_sync_get_depth_cv(0);
		if (!depth) {
			fprintf(stderr, "Error: Kinect not connected?\n");
			return -1;
		}
		switch (k) {
			case  -1: break;
			case 'w': pitch_z -= 10; break;
			case 's': pitch_z += 10; break;
			case 'a': volume_z += 10; break;
			case 'd': volume_z -= 10; break;
			case 'x': show_antennas = 1 - show_antennas; break;
			default: printf("%d\n", k);
		}
		if (show_antennas) {
			showAntennas(depth);
			//pitch = calculate_pitch();
			//volume = calculate_volume();
			lo_send(chuck, "/update", "ff", pitch, volume);
		}
		cvShowImage("RGB", image);
		cvShowImage("Depth", GlViewColor(depth));
	}
	return 0;
}
