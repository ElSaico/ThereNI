#include <lo/lo.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>
#include <stdio.h>

#include "libfreenect_cv.h"

#define DEBUG 1

#define min(x,y) ((x) < (y) ? (x) : (y))
#define X_BY_DEPTH(x,z) (((x)-320)*((z)-10)*0.0021*(640.0/480.0))
#define Y_BY_DEPTH(y,z) (((y)-240)*((z)-10)*0.0021)
//#define DEPTH_TO_CM(x) (100.0 / ((x) * -0.0030711016 + 3.3309495161))
#define DEPTH_TO_CM(x) (123.6 * tan((x)/2842.5 + 1.1863))

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

void set_upper_image(uint16_t* depth, IplImage* dest) {
	cvSet(dest, cvScalar(0,0,0,0), NULL);
	int i, x, z;
	for (i = 0; i < 640*480; ++i) {
		x = i % 640;
		z = depth[x] / 2;
		dest->imageData[640*z+x]= 255;
	}
	dest->imageData[320*pitch_z+pitch_x] = 192;
}

void set_antennas(int event, int x, int y, int flags, void* parms) {
	uint16_t* mouse_status = ((uint16_t**)parms)[0];
	uint16_t* depth = ((uint16_t**)parms)[1];
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
		case CV_EVENT_MBUTTONUP:
			pitch_z = depth[640*y+x];
			volume_z = pitch_z;
			break;
	}
}

void calculate_distances(uint16_t* depth, float* pitch, float* volume) {
	int i;
	double diff_x, diff_y, diff_zp, diff_zv, dp = 999, dv = 999, dpz, dvz;
	for (i = 0; i < 640*480; i++) {
		dpz = DEPTH_TO_CM(pitch_z);
		diff_x = X_BY_DEPTH(pitch_x,dpz) - (X_BY_DEPTH(i%640,dpz));
		diff_zp = dpz - DEPTH_TO_CM(depth[i]);
		dp = min(dp, sqrt(diff_x*diff_x+diff_zp*diff_zp));
		
		dvz = DEPTH_TO_CM(pitch_z);
		diff_y = Y_BY_DEPTH(volume_y,dvz) - (Y_BY_DEPTH(i/640,dvz));
		diff_zv = dvz - DEPTH_TO_CM(depth[i]);
		dv = min(dv, sqrt(diff_y*diff_y+diff_zv*diff_zv));
	}
	//printf("%lf %lf\n", dp, dv);
	*pitch = 50000 / dp;
	*volume = 1;
}

int main(int argc, char **argv) {
	float pitch, volume;
	lo_address chuck = lo_address_new(NULL, "8765");
	int k;
	uint16_t mouse_status[2] = {0, 0};
	uint16_t* depth_buf = (uint16_t*) malloc(640*480*2);
	uint16_t* params[] = {mouse_status, depth_buf};
#ifdef DEBUG
	IplImage* upper = cvCreateImage(cvSize(640,2048),8,1);
	IplImage* side = cvCreateImage(cvSize(1024,480),8,1);
#endif

	cvNamedWindow("RGB", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Depth", CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback("Depth", set_antennas, params);
	while ((k = cvWaitKey(10)) != '\n') {
		IplImage *image = freenect_sync_get_rgb_cv(0);
		if (!image) {
			fprintf(stderr, "Error: Kinect not connected?\n");
			return -1;
		}
		cvCvtColor(image, image, CV_RGB2BGR);
		IplImage *depth = freenect_sync_get_depth_cv(&depth_buf, 0);
		if (!depth) {
			fprintf(stderr, "Error: Kinect not connected?\n");
			return -1;
		}
		switch (k) {
			case  -1: break;
			case 'w': pitch_z += 10; break;
			case 's': pitch_z -= 10; break;
			case 'a': volume_z += 10; break;
			case 'd': volume_z -= 10; break;
			default: printf("%d\n", k);
		}
		cvLine(depth, cvPoint(pitch_x, pitch_y0),
		       cvPoint(pitch_x, pitch_y1), cvRealScalar(pitch_z), 5, 8, 0);
		cvLine(depth, cvPoint(volume_x0, volume_y),
		       cvPoint(volume_x1, volume_y), cvRealScalar(volume_z), 5, 8, 0);
		calculate_distances(depth_buf, &pitch, &volume);
		//printf("%f %f\n", pitch, volume);
		if (pitch >= 10 && pitch <= 1000)
			lo_send(chuck, "/update", "ff", pitch, volume);
		else
			lo_send(chuck, "/update", "ff", 0.0, 0.0);
#ifdef DEBUG
		set_upper_image(depth_buf, upper);
#endif
		cvShowImage("RGB", image);
		cvShowImage("Depth", GlViewColor(depth));
#ifdef DEBUG
		cvShowImage("Depth (up)", upper);
#endif
	}
	return 0;
}
