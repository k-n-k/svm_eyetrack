#include <cv.h>
#include <highgui.h>
#include <ml.h>
#include <stdio.h>
#include "time.h"

#define  TPL_WIDTH       12      /* template width       */
#define  TPL_HEIGHT      12      /* template height      */
#define  WINDOW_WIDTH    24      /* search window width  */
#define  WINDOW_HEIGHT   24      /* search window height */
#define  THRESHOLD       0.3

IplImage *frame, *tpl, *tm, *gray, *eyezone1, *eyezone2, *minieyezone1, *minieyezone2, *output1, *output2;
int      object_x0, object_y0, is_tracking = 0;

void mouseHandler( int event, int x, int y, int flags, void *param );
void trackObject();


//const char *cascade_name = "haarcascade_frontalface_default.xml";
//const char *cascade_name = "haarcascade_eye.xml";

const char *righteye_cascade_name = "haarcascade_righteye_2splits.xml";
const char *lefteye_cascade_name = "haarcascade_lefteye_2splits.xml";

//CvHaarClassifierCascade *cascade = 0;
//CvMemStorage *storage = 0;
//CvSeq *faces;
CvHaarClassifierCascade *righteye_cascade = 0;
CvHaarClassifierCascade *lefteye_cascade = 0;
CvMemStorage *righteye_storage = 0;
CvMemStorage *lefteye_storage = 0;
CvSeq *righteye, *lefteye;
static CvScalar colors[] = {
	{{0, 0, 255}}, {{0, 128, 255}},
	{{0, 255, 255}}, {{0, 255, 0}},
	{{255, 128, 0}}, {{255, 255, 0}},
	{{255, 0, 0}}, {{255, 0, 255}}
	};



/* main code */
int main(int argc, char** argv ){
	CvCapture   *capture;
	int i, key;
	//*struct tm *newtime; 
	time_t second,milsecond;
	char rightfilename[100], leftfilename[100];



	//svm用
	int j;
  	int width = 16, height = 16;  /* サンプル画像サイズ */
  	int image_dim = width * height;
  	CvMat m;
  	float a[image_dim];
  	float n_ret = -1.0;
  	float h_ret = -1.0;
  	float j_ret = -1.0;
  	float k_ret = -1.0;
  	float l_ret = -1.0;
  	float scale;
  	IplImage *src, *src_color, *src_tmp;
  	int sx, sy, tw, th;
  	int stepx = 3, stepy = 3;
  	double steps = 1.2;
  	int iterate;
  	CvSVM n_svm = CvSVM ();
	n_svm.load ("n_svm_image.xml");
  	CvSVM h_svm = CvSVM ();
	h_svm.load ("h_svm_image.xml");
  	CvSVM j_svm = CvSVM ();
	j_svm.load ("j_svm_image.xml");
  	CvSVM k_svm = CvSVM ();
	k_svm.load ("k_svm_image.xml");
  	CvSVM l_svm = CvSVM ();
	l_svm.load ("l_svm_image.xml");





	// ブーストされた分類器のカスケードを読み込む
//	cascade = (CvHaarClassifierCascade *) cvLoad (cascade_name, 0, 0, 0);
	righteye_cascade = (CvHaarClassifierCascade *) cvLoad (righteye_cascade_name, 0, 0, 0);
	lefteye_cascade = (CvHaarClassifierCascade *) cvLoad (lefteye_cascade_name, 0, 0, 0);

	/* initialize camera */
	capture = cvCaptureFromCAM( 0 );

	/* always check */
	if( !capture ) return 1;

	/* get video properties, needed by template image */
	frame = cvQueryFrame( capture );
	if ( !frame ) return 1;
    
	/* create template image */
	tpl = cvCreateImage( cvSize( TPL_WIDTH, TPL_HEIGHT ), 
                         frame->depth, frame->nChannels );
    
	/* create image for template matching result */
	tm = cvCreateImage( cvSize( WINDOW_WIDTH  - TPL_WIDTH  + 1,
                                WINDOW_HEIGHT - TPL_HEIGHT + 1 ),
                        IPL_DEPTH_32F, 1 );

	//eyezone
	eyezone1 = cvCreateImage(cvSize(50,50), IPL_DEPTH_8U, 1);
	minieyezone1 = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 1);
	output1 = cvCreateImage(cvSize(512,512), IPL_DEPTH_8U, 1);
   	eyezone2 = cvCreateImage(cvSize(50,50), IPL_DEPTH_8U, 1);
	minieyezone2 = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 1);
	output2 = cvCreateImage(cvSize(512,512), IPL_DEPTH_8U, 1);

	/* create a window and install mouse handler */
	cvNamedWindow( "video", CV_WINDOW_AUTOSIZE );
	cvSetMouseCallback( "video", mouseHandler, NULL );
	cvNamedWindow("output1", CV_WINDOW_AUTOSIZE); 
	cvNamedWindow("output2", CV_WINDOW_AUTOSIZE);

	gray = cvCreateImage (cvGetSize (frame), IPL_DEPTH_8U, 1);
	righteye_storage = cvCreateMemStorage (0);
	lefteye_storage = cvCreateMemStorage (0);
	CvPoint righteye_center, lefteye_center;
    
	// eye candidate
	CvRect righteye_cand1, righteye_cand2, lefteye_cand1, lefteye_cand2, right, left;
	int eye_candidate_num = 0;	



	while( key != 'q' ) {
		eye_candidate_num = 0;
		/* get a frame */
		frame = cvQueryFrame( capture );

		/* always check */
		if( !frame ) break;

		/* 'fix' frame */
		/*   cvFlip( frame, frame, -1 ); */
		frame->origin = 0;
        
		/* perform tracking if template is available */
		if( is_tracking ) trackObject();
        

		cvClearMemStorage (righteye_storage);
		cvClearMemStorage (lefteye_storage);
		cvCvtColor (frame, gray, CV_BGR2GRAY);
		cvEqualizeHist (gray, gray);
		righteye = cvHaarDetectObjects (gray, righteye_cascade, righteye_storage, 1.11, 4, 0, cvSize (40, 40), cvSize(40,40));
		lefteye = cvHaarDetectObjects (gray, lefteye_cascade, lefteye_storage, 1.11, 4, 0, cvSize (40, 40), cvSize(40,40));


		//右目を円で描画
		for (i = 0; i < (righteye ? righteye->total : 0); i++) {
			CvRect *r = (CvRect *) cvGetSeqElem (righteye, i);
			CvPoint center;
			int radius;
			center.x = cvRound (r->x + r->width * 0.5);
			center.y = cvRound (r->y + r->height * 0.5);
			radius = cvRound ((r->width + r->height) * 0.25);
			cvCircle (frame, center, radius, colors[i % 8], 3, 8, 0);
		//右目候補
			if(i == 0){
				righteye_cand1 = *r;
				}
			if(i == 1){
				righteye_cand2 = *r;
				}
			}
		//左目を死角で描画
		for (i = 0; i < (lefteye ? lefteye->total : 0); i++) {
			CvRect *r = (CvRect *) cvGetSeqElem (lefteye, i);
			CvPoint apex1, apex2;
			apex1 = cvPoint(r->x, r->y);
			apex2.x = cvRound(r->x + r->width);
			apex2.y = cvRound(r->y + r->height);
			cvRectangle (frame,apex1, apex2, colors[i % 8], 3, 8, 0);
			
		//左目候補
			if(i == 0){
				lefteye_cand1 = *r;
				}
			if(i == 1){
				lefteye_cand2 = *r;
				}
			}
		//候補しぼり
			if(righteye->total >= 1){
				if(righteye->total >= 2){
					if(righteye_cand1.x <= righteye_cand2.x){
						right = righteye_cand1;
						righteye_center.x = cvRound(right.x + right.width*0.5);
						righteye_center.y = cvRound(right.y + right.height*0.5);
						}			
					else{
						right = righteye_cand2;
						righteye_center.x = cvRound(right.x + right.width*0.5);
						righteye_center.y = cvRound(right.y + right.height*0.5);
						}
					}
				else{
					right = righteye_cand1;
					righteye_center.x = cvRound(right.x + right.width*0.5);
					righteye_center.y = cvRound(right.y + right.height*0.5);
					}
				eyezone1 = cvCreateImage(cvSize(right.width, right.height), IPL_DEPTH_8U, 1);
				cvGetRectSubPix(gray, eyezone1, cvPointTo32f(righteye_center));
				cvEqualizeHist(eyezone1, eyezone1);
				cvResize(eyezone1, minieyezone1, CV_INTER_LINEAR);
				cvResize(minieyezone1, output1, CV_INTER_NN);
			}



			if(lefteye->total >= 1){
				if(lefteye->total >= 2){
					if(lefteye_cand1.x >= lefteye_cand2.x){
						left = lefteye_cand1;
						lefteye_center.x = cvRound(left.x + left.width*0.5);
						lefteye_center.y = cvRound(left.y + left.height*0.5);
						}			
					else{
						left = lefteye_cand2;
						lefteye_center.x = cvRound(left.x + left.width*0.5);
						lefteye_center.y = cvRound(right.y + left.height*0.5);
						}
					}
				else{
					left = lefteye_cand1;
					lefteye_center.x = cvRound(left.x + left.width*0.5);
					lefteye_center.y = cvRound(left.y + left.height*0.5);
					}
				eyezone2 = cvCreateImage(cvSize(left.width, left.height), IPL_DEPTH_8U, 1);
				cvGetRectSubPix(gray, eyezone2, cvPointTo32f(lefteye_center));
				cvEqualizeHist(eyezone2, eyezone2);
				cvResize(eyezone2, minieyezone2, CV_INTER_LINEAR);
				cvResize(minieyezone2, output2, CV_INTER_NN);
			}
			printf("righteye width = %d, height = %d\n", right.width, right.height); 
			printf("lefteye width = %d, height = %d\n", left.width, left.height);
	//		printf("righteye x = %d\n", right.x);
	//		printf("lefteye x = %d\n", left.x);


cvInitMatHeader (&m, 1, image_dim, CV_32FC1, NULL);

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			a[i * width + j] =
			float ((int) ((uchar) (minieyezone1->imageData[i * minieyezone1->widthStep + j])) / 255.0);
          }
        }
        cvSetData(&m, a, sizeof(float)*image_dim);

        // (4)SVMによる判定と結果の描画
        n_ret = n_svm.predict (&m);
		printf("center %f\n", n_ret);
        h_ret = h_svm.predict (&m);
		printf("left %f\n", h_ret);
        j_ret = j_svm.predict (&m);
		printf("down %f\n", j_ret);
        k_ret = k_svm.predict (&m);
		printf("up %f\n", k_ret);
        l_ret = l_svm.predict (&m);
		printf("right %f\n", l_ret);



		/* display frame */
		cvShowImage( "video", frame);
		//cvShowImage( "eyezone1", eyezone1);
		//cvShowImage( "eyezone2", eyezone2);
		cvShowImage( "output1", output1);
		cvShowImage( "output2", output2);

		//ファイル出力,時間計測
		time(&second);
		milsecond = clock();
	//	printf("時間[sec] = %ld\n", second);
		printf("経過時間[usec] = %ld\n", milsecond);
		//sprintf(filename, "%ld.bmp",second);
		//printf("sprintf = %s\n", filename);
		//cvSaveImage(filename, frame,0); 
	   






		
		/* exit if user press 'q' */
		key = cvWaitKey( 1 );
		}

	/* free memory */
	cvDestroyWindow( "video" );
	cvDestroyWindow( "output1");
	cvDestroyWindow( "output2");
	cvReleaseCapture( &capture );
	cvReleaseImage( &tpl );
	cvReleaseImage( &tm );
	cvReleaseImage( &gray);
   	cvReleaseImage( &eyezone1);
	cvReleaseImage( &eyezone2);
	cvReleaseImage( &minieyezone1);
	cvReleaseImage( &minieyezone2);
	cvReleaseImage( &output1);
	cvReleaseImage( &output2);
	return 0;
	}

/* mouse handler */
void mouseHandler( int event, int x, int y, int flags, void *param )
{
	/* user clicked the image, save subimage as template */
    if( event == CV_EVENT_LBUTTONUP ) {
        object_x0 = x - ( TPL_WIDTH  / 2 );
        object_y0 = y - ( TPL_HEIGHT / 2 ); 
        
		cvSetImageROI( frame, 
                       cvRect( object_x0, 
                               object_y0, 
                               TPL_WIDTH, 
                               TPL_HEIGHT ) );
        cvCopy( frame, tpl, NULL );
        cvResetImageROI( frame );

        /* template is available, start tracking! */
        fprintf( stdout, "Template selected. Start tracking... \n" );
        is_tracking = 1;
    }
}

/* track object */
void trackObject()
{
    CvPoint minloc, maxloc;
    double  minval, maxval;

    /* setup position of search window */
    int win_x0 = object_x0 - ( ( WINDOW_WIDTH  - TPL_WIDTH  ) / 2 );
    int win_y0 = object_y0 - ( ( WINDOW_HEIGHT - TPL_HEIGHT ) / 2 );
    
	/*
	 * Ooops, some bugs here.
	 * If the search window exceed the frame boundaries,
	 * it will trigger errors.
	 *
	 * Add some code to make sure that the search window 
	 * is still within the frame.
	 */
	
    /* search object in search window */
    cvSetImageROI( frame, 
                   cvRect( win_x0, 
                           win_y0, 
                           WINDOW_WIDTH, 
                           WINDOW_HEIGHT ) );
    cvMatchTemplate( frame, tpl, tm, CV_TM_SQDIFF_NORMED );
    cvMinMaxLoc( tm, &minval, &maxval, &minloc, &maxloc, 0 );
    cvResetImageROI( frame );
    
    /* if object found... */
    if( minval <= THRESHOLD ) {
        /* save object's current location */
        object_x0 = win_x0 + minloc.x;
        object_y0 = win_y0 + minloc.y;

        /* and draw a box there */
        cvRectangle( frame,
                     cvPoint( object_x0, object_y0 ),
                     cvPoint( object_x0 + TPL_WIDTH, 
					          object_y0 + TPL_HEIGHT ),
                     cvScalar( 0, 0, 255, 0 ), 1, 0, 0 );
    } else {
        /* if not found... */
        fprintf( stdout, "Lost object.\n" );
        is_tracking = 0;
    }
}
