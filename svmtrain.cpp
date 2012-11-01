#include <cv.h>
#include <highgui.h>
#include <ml.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>


int is_str_bmp(char *filename, char *topchar){
	int n = 0;
	while(*(filename+n) != '\0'){
		n++;
	}
	if((strcmp(".bmp", filename+(n-4)) == 0)&&(strncmp(topchar, filename,6) == 0)){
		return 1;
		
	}
	else{
		return 0;
	}
}

int main (int argc, char **argv){
	int i, j, ii, jj;
	int width = 16, height = 16;  /* サンプル画像サイズ */
	int image_dim = width * height;
	IplImage *img_org;
	IplImage *sample_img;
	CvMat data_mat, res_mat;
	CvTermCriteria criteria;
	CvSVM svm = CvSVM ();
	CvSVMParams param;
	//char filename[64];

	DIR *dir;
	struct dirent *dp;
	char **n_filenames = NULL, 
		 **h_filenames = NULL, 
		 **j_filenames = NULL, 
		 **k_filenames = NULL, 
		 **l_filenames = NULL;
	int nn = 0, nh=0, nj=0, nk=0, nl=0;
	dir = opendir("./");
	char n_char[256] = "n_right", 
		 h_char[256] = "h_right", 
		 j_char[256] = "j_right", 
		 k_char[256] = "k_right", 
		 l_char[256] = "l_right";
	
	
	while((dp = readdir(dir)) != NULL){
		if(dp->d_type == DT_REG){
			if(is_str_bmp(dp->d_name, n_char)){
					n_filenames = (char **)realloc(n_filenames, sizeof(char *)*(nn+1));
					*(n_filenames+nn) = (char *)malloc(sizeof(char) * 256);
					strcpy(*(n_filenames+nn), dp->d_name);
					nn++;
					}
				}
			}
	closedir(dir);
	dir = opendir("./");
	while((dp = readdir(dir)) != NULL){
		if(dp->d_type == DT_REG){
			if(is_str_bmp(dp->d_name, h_char)){
					h_filenames = (char **)realloc(h_filenames, sizeof(char *)*(nh+1));
					*(h_filenames+nh) = (char *)malloc(sizeof(char) * 256);
					strcpy(*(h_filenames+nh), dp->d_name);
					nh++;
					}
				}
			}
	closedir(dir);
	dir = opendir("./");
	while((dp = readdir(dir)) != NULL){
		if(dp->d_type == DT_REG){
			if(is_str_bmp(dp->d_name, j_char)){
					j_filenames = (char **)realloc(j_filenames, sizeof(char *)*(nj+1));
					*(j_filenames+nj) = (char *)malloc(sizeof(char) * 256);
					strcpy(*(j_filenames+nj), dp->d_name);
					nj++;
					}
				}
			}
	closedir(dir);
	dir = opendir("./");
	while((dp = readdir(dir)) != NULL){
		if(dp->d_type == DT_REG){
			if(is_str_bmp(dp->d_name, k_char)){
					k_filenames = (char **)realloc(k_filenames, sizeof(char *)*(nk+1));
					*(k_filenames+nk) = (char *)malloc(sizeof(char) * 256);
					strcpy(*(k_filenames+nk), dp->d_name);
					nk++;
					}
				}
			}
	closedir(dir);
	dir = opendir("./");
	while((dp = readdir(dir)) != NULL){
		if(dp->d_type == DT_REG){
			if(is_str_bmp(dp->d_name, l_char)){
					l_filenames = (char **)realloc(l_filenames, sizeof(char *)*(nl+1));
					*(l_filenames+nl) = (char *)malloc(sizeof(char) * 256);
					strcpy(*(l_filenames+nl), dp->d_name);
					nl++;
					}
				}
			}
	closedir(dir);
	dir = opendir("./");

	printf( "n = %d, h = %d, j = %d, k = %d, l = %d\n", nn, nh, nj, nk, nl);

	
	
	// n vs hjkl	
	int pimage_num = nn;         /* ポジティブサンプル数 */
	int nimage_num = nh + nj + nk + nl;        /* ネガティブサンプル数 */
	int all_image_num = pimage_num + nimage_num;
	char *pimage_char[pimage_num];
	char *nimage_char[nimage_num];
	int res[all_image_num];
	float data[all_image_num * image_dim];



	for(i=0; i < pimage_num; i++){
		pimage_char[i] = n_filenames[i];
		printf("%d, %s\n", i, pimage_char[i]);
	}

	for(i=0; i < nh; i++){
		nimage_char[i] = h_filenames[i];
		printf("%d, %s\n", i, nimage_char[i]);
	}
	for(; i < nh + nj; i++){
		nimage_char[i] = j_filenames[i-nh];
		printf("%d, %s\n", i, nimage_char[i]);
	}
	for(; i < nh + nj + nk; i++){
		nimage_char[i] = k_filenames[i-nh-nj];
		printf("%d, %s\n", i, nimage_char[i]);
	}
	for(; i < nh + nj + nk + nl; i++){
		nimage_char[i] = l_filenames[i-nh-nj-nk];
		printf("%d, %s\n", i, nimage_char[i]);
	}




	
// (1)ポジティブサンプルの読み込み
	for (i = 0; i < pimage_num; i++) {
	//	sprintf (filename, "positive/%03d.png", i);
		img_org = cvLoadImage (pimage_char[i], CV_LOAD_IMAGE_GRAYSCALE);
		sample_img = cvCreateImage (cvSize (width, height), IPL_DEPTH_8U, 1);
		cvResize (img_org, sample_img, CV_INTER_NN);
		cvSmooth (sample_img, sample_img, CV_GAUSSIAN, 3, 0, 0, 0);
		for (ii = 0; ii < height; ii++) {
			for (jj = 0; jj < width; jj++) {
				data[i * image_dim + (ii * width) + jj] = float((int) ((uchar) (sample_img->imageData[ii * sample_img->widthStep + jj])) / 255.0);
			}
		}
		res[i] = 1;
	}

  // (2)ネガティブサンプルの読み込み
  j = i;
  for (i = j; i < j + nimage_num; i++) {
  //  sprintf (filename, "negative/%03d.jpg", i - j);
    img_org = cvLoadImage (nimage_char[i-j], CV_LOAD_IMAGE_GRAYSCALE);
    sample_img = cvCreateImage (cvSize (width, height), IPL_DEPTH_8U, 1);
    cvResize (img_org, sample_img, CV_INTER_NN);
    cvSmooth (sample_img, sample_img, CV_GAUSSIAN, 3, 0, 0, 0);
    for (ii = 0; ii < height; ii++) {
      for (jj = 0; jj < width; jj++) {
        data[i * image_dim + (ii * width) + jj] =float ((int) ((uchar) (sample_img->imageData[ii * sample_img->widthStep + jj])) / 255.0);
      }
    }
    res[i] = 0;
  }

  // (3)SVM学習データとパラメータの初期化
  cvInitMatHeader (&data_mat, all_image_num, image_dim, CV_32FC1, data, CV_AUTOSTEP);
  cvInitMatHeader (&res_mat, all_image_num, 1, CV_32SC1, res, CV_AUTOSTEP);
  criteria = cvTermCriteria (CV_TERMCRIT_EPS, 1000, FLT_EPSILON);
  param = CvSVMParams (CvSVM::C_SVC, CvSVM::RBF, 10.0, 0.09, 1.0, 10.0, 0.5, 1.0, NULL, criteria);

  // (4)SVMの学習とデータの保存
  svm.train (&data_mat, &res_mat, NULL, NULL, param);
  svm.save ("n_svm_image.xml");
  // n vs hjkl ここまで





	// h vs njkl	
	int h_pimage_num = nh;         /* ポジティブサンプル数 */
	int h_nimage_num = nn + nj + nk + nl;        /* ネガティブサンプル数 */
	int h_all_image_num = h_pimage_num + h_nimage_num;
	char *h_pimage_char[h_pimage_num];
	char *h_nimage_char[h_nimage_num];
	int h_res[h_all_image_num];
	float h_data[h_all_image_num * image_dim];



	for(i=0; i < h_pimage_num; i++){
		h_pimage_char[i] = h_filenames[i];
		printf("%d, %s\n", i, h_pimage_char[i]);
	}

	for(i=0; i < nn; i++){
		h_nimage_char[i] = n_filenames[i];
		printf("%d, %s\n", i, h_nimage_char[i]);
	}
	for(; i < nn + nj; i++){
		h_nimage_char[i] = j_filenames[i-nn];
		printf("%d, %s\n", i, h_nimage_char[i]);
	}
	for(; i < nn + nj + nk; i++){
		h_nimage_char[i] = k_filenames[i-nn-nj];
		printf("%d, %s\n", i, h_nimage_char[i]);
	}
	for(; i < nn + nj + nk + nl; i++){
		h_nimage_char[i] = l_filenames[i-nn-nj-nk];
		printf("%d, %s\n", i, h_nimage_char[i]);
	}




	
// (1)ポジティブサンプルの読み込み
	for (i = 0; i < h_pimage_num; i++) {
	//	sprintf (filename, "positive/%03d.png", i);
		img_org = cvLoadImage (h_pimage_char[i], CV_LOAD_IMAGE_GRAYSCALE);
		sample_img = cvCreateImage (cvSize (width, height), IPL_DEPTH_8U, 1);
		cvResize (img_org, sample_img, CV_INTER_NN);
		cvSmooth (sample_img, sample_img, CV_GAUSSIAN, 3, 0, 0, 0);
		for (ii = 0; ii < height; ii++) {
			for (jj = 0; jj < width; jj++) {
				h_data[i * image_dim + (ii * width) + jj] = float((int) ((uchar) (sample_img->imageData[ii * sample_img->widthStep + jj])) / 255.0);
			}
		}
		h_res[i] = 1;
	}
  // (2)ネガティブサンプルの読み込み
  j = i;
  for (i = j; i < j + h_nimage_num; i++) {
  //  sprintf (filename, "negative/%03d.jpg", i - j);
  	
		printf("%d\n", i);
    img_org = cvLoadImage (h_nimage_char[i-j], CV_LOAD_IMAGE_GRAYSCALE);
    sample_img = cvCreateImage (cvSize (width, height), IPL_DEPTH_8U, 1);
    cvResize (img_org, sample_img, CV_INTER_NN);
    cvSmooth (sample_img, sample_img, CV_GAUSSIAN, 3, 0, 0, 0);
    for (ii = 0; ii < height; ii++) {
      for (jj = 0; jj < width; jj++) {
        h_data[i * image_dim + (ii * width) + jj] =float ((int) ((uchar) (sample_img->imageData[ii * sample_img->widthStep + jj])) / 255.0);
      }
    }
    h_res[i] = 0;
  }
  // (3)SVM学習データとパラメータの初期化
  cvInitMatHeader (&data_mat, h_all_image_num, image_dim, CV_32FC1, h_data, CV_AUTOSTEP);
  cvInitMatHeader (&res_mat, h_all_image_num, 1, CV_32SC1, h_res, CV_AUTOSTEP);
  criteria = cvTermCriteria (CV_TERMCRIT_EPS, 1000, FLT_EPSILON);
  param = CvSVMParams (CvSVM::C_SVC, CvSVM::RBF, 10.0, 0.09, 1.0, 10.0, 0.5, 1.0, NULL, criteria);

  // (4)SVMの学習とデータの保存
  svm.train (&data_mat, &res_mat, NULL, NULL, param);
  svm.save ("h_svm_image.xml");
  // h vs njkl ここまで

	// j vs hnkl	
	int j_pimage_num = nj;         /* ポジティブサンプル数 */
	int j_nimage_num = nh + nn + nk + nl;        /* ネガティブサンプル数 */
	int j_all_image_num = j_pimage_num + j_nimage_num;
	char *j_pimage_char[j_pimage_num];
	char *j_nimage_char[j_nimage_num];
	int j_res[j_all_image_num];
	float j_data[j_all_image_num * image_dim];



	for(i=0; i < j_pimage_num; i++){
		j_pimage_char[i] = j_filenames[i];
		printf("%d, %s\n", i, j_pimage_char[i]);
	}

	for(i=0; i < nh; i++){
		j_nimage_char[i] = h_filenames[i];
		printf("%d, %s\n", i, j_nimage_char[i]);
	}
	for(; i < nh + nn; i++){
		j_nimage_char[i] = n_filenames[i-nh];
		printf("%d, %s\n", i, j_nimage_char[i]);
	}
	for(; i < nh + nn + nk; i++){
		j_nimage_char[i] = k_filenames[i-nh-nn];
		printf("%d, %s\n", i, j_nimage_char[i]);
	}
	for(; i < nh + nn + nk + nl; i++){
		j_nimage_char[i] = l_filenames[i-nh-nn-nk];
		printf("%d, %s\n", i, j_nimage_char[i]);
	}




	
// (1)ポジティブサンプルの読み込み
	for (i = 0; i < j_pimage_num; i++) {
	//	sprintf (filename, "positive/%03d.png", i);
		img_org = cvLoadImage (j_pimage_char[i], CV_LOAD_IMAGE_GRAYSCALE);
		sample_img = cvCreateImage (cvSize (width, height), IPL_DEPTH_8U, 1);
		cvResize (img_org, sample_img, CV_INTER_NN);
		cvSmooth (sample_img, sample_img, CV_GAUSSIAN, 3, 0, 0, 0);
		for (ii = 0; ii < height; ii++) {
			for (jj = 0; jj < width; jj++) {
				j_data[i * image_dim + (ii * width) + jj] = float((int) ((uchar) (sample_img->imageData[ii * sample_img->widthStep + jj])) / 255.0);
			}
		}
		j_res[i] = 1;
	}

  // (2)ネガティブサンプルの読み込み
  j = i;
  for (i = j; i < j + j_nimage_num; i++) {
  //  sprintf (filename, "negative/%03d.jpg", i - j);
    img_org = cvLoadImage (j_nimage_char[i-j], CV_LOAD_IMAGE_GRAYSCALE);
    sample_img = cvCreateImage (cvSize (width, height), IPL_DEPTH_8U, 1);
    cvResize (img_org, sample_img, CV_INTER_NN);
    cvSmooth (sample_img, sample_img, CV_GAUSSIAN, 3, 0, 0, 0);
    for (ii = 0; ii < height; ii++) {
      for (jj = 0; jj < width; jj++) {
        j_data[i * image_dim + (ii * width) + jj] =float ((int) ((uchar) (sample_img->imageData[ii * sample_img->widthStep + jj])) / 255.0);
      }
    }
    j_res[i] = 0;
  }

  // (3)SVM学習データとパラメータの初期化
  cvInitMatHeader (&data_mat, j_all_image_num, image_dim, CV_32FC1, j_data, CV_AUTOSTEP);
  cvInitMatHeader (&res_mat, j_all_image_num, 1, CV_32SC1, j_res, CV_AUTOSTEP);
  criteria = cvTermCriteria (CV_TERMCRIT_EPS, 1000, FLT_EPSILON);
  param = CvSVMParams (CvSVM::C_SVC, CvSVM::RBF, 10.0, 0.09, 1.0, 10.0, 0.5, 1.0, NULL, criteria);

  // (4)SVMの学習とデータの保存
  svm.train (&data_mat, &res_mat, NULL, NULL, param);
  svm.save ("j_svm_image.xml");
  // j vs hnkl ここまで

	// k vs hjnl	
	int k_pimage_num = nk;         /* ポジティブサンプル数 */
	int k_nimage_num = nh + nj + nn + nl;        /* ネガティブサンプル数 */
	int k_all_image_num = k_pimage_num + k_nimage_num;
	char *k_pimage_char[k_pimage_num];
	char *k_nimage_char[k_nimage_num];
	int k_res[k_all_image_num];
	float k_data[k_all_image_num * image_dim];



	for(i=0; i < k_pimage_num; i++){
		k_pimage_char[i] = k_filenames[i];
		printf("%d, %s\n", i, k_pimage_char[i]);
	}

	for(i=0; i < nh; i++){
		k_nimage_char[i] = h_filenames[i];
		printf("%d, %s\n", i, k_nimage_char[i]);
	}
	for(; i < nh + nj; i++){
		k_nimage_char[i] = j_filenames[i-nh];
		printf("%d, %s\n", i, k_nimage_char[i]);
	}
	for(; i < nh + nj + nn; i++){
		k_nimage_char[i] = n_filenames[i-nh-nj];
		printf("%d, %s\n", i, k_nimage_char[i]);
	}
	for(; i < nh + nj + nn + nl; i++){
		k_nimage_char[i] = l_filenames[i-nh-nj-nn];
		printf("%d, %s\n", i, k_nimage_char[i]);
	}




	
// (1)ポジティブサンプルの読み込み
	for (i = 0; i < k_pimage_num; i++) {
	//	sprintf (filename, "positive/%03d.png", i);
		img_org = cvLoadImage (k_pimage_char[i], CV_LOAD_IMAGE_GRAYSCALE);
		sample_img = cvCreateImage (cvSize (width, height), IPL_DEPTH_8U, 1);
		cvResize (img_org, sample_img, CV_INTER_NN);
		cvSmooth (sample_img, sample_img, CV_GAUSSIAN, 3, 0, 0, 0);
		for (ii = 0; ii < height; ii++) {
			for (jj = 0; jj < width; jj++) {
				k_data[i * image_dim + (ii * width) + jj] = float((int) ((uchar) (sample_img->imageData[ii * sample_img->widthStep + jj])) / 255.0);
			}
		}
		k_res[i] = 1;
	}

  // (2)ネガティブサンプルの読み込み
  j = i;
  for (i = j; i < j + k_nimage_num; i++) {
  //  sprintf (filename, "negative/%03d.jpg", i - j);
    img_org = cvLoadImage (k_nimage_char[i-j], CV_LOAD_IMAGE_GRAYSCALE);
    sample_img = cvCreateImage (cvSize (width, height), IPL_DEPTH_8U, 1);
    cvResize (img_org, sample_img, CV_INTER_NN);
    cvSmooth (sample_img, sample_img, CV_GAUSSIAN, 3, 0, 0, 0);
    for (ii = 0; ii < height; ii++) {
      for (jj = 0; jj < width; jj++) {
        k_data[i * image_dim + (ii * width) + jj] =float ((int) ((uchar) (sample_img->imageData[ii * sample_img->widthStep + jj])) / 255.0);
      }
    }
    k_res[i] = 0;
  }

  // (3)SVM学習データとパラメータの初期化
  cvInitMatHeader (&data_mat, k_all_image_num, image_dim, CV_32FC1, k_data, CV_AUTOSTEP);
  cvInitMatHeader (&res_mat, k_all_image_num, 1, CV_32SC1, k_res, CV_AUTOSTEP);
  criteria = cvTermCriteria (CV_TERMCRIT_EPS, 1000, FLT_EPSILON);
  param = CvSVMParams (CvSVM::C_SVC, CvSVM::RBF, 10.0, 0.09, 1.0, 10.0, 0.5, 1.0, NULL, criteria);

  // (4)SVMの学習とデータの保存
  svm.train (&data_mat, &res_mat, NULL, NULL, param);
  svm.save ("k_svm_image.xml");
  // k vs hjnl ここまで

printf("lkaishi\n");
	// l vs hjkn	
	int l_pimage_num = nl;         /* ポジティブサンプル数 */
	int l_nimage_num = nh + nj + nk + nn;        /* ネガティブサンプル数 */
	int l_all_image_num = l_pimage_num + l_nimage_num;
	char *l_pimage_char[l_pimage_num];
	char *l_nimage_char[l_nimage_num];
	int l_res[l_all_image_num];
	float l_data[l_all_image_num * image_dim];



	for(i=0; i < l_pimage_num; i++){
		l_pimage_char[i] = l_filenames[i];
		printf("%d, %s\n", i, l_pimage_char[i]);
	}

	for(i=0; i < nh; i++){
		l_nimage_char[i] = h_filenames[i];
		printf("%d, %s\n", i, l_nimage_char[i]);
	}
	for(; i < nh + nj; i++){
		l_nimage_char[i] = j_filenames[i-nh];
		printf("%d, %s\n", i, l_nimage_char[i]);
	}
	for(; i < nh + nj + nk; i++){
		l_nimage_char[i] = k_filenames[i-nh-nj];
		printf("%d, %s\n", i, l_nimage_char[i]);
	}
	for(; i < nh + nj + nk + nn; i++){
		l_nimage_char[i] = n_filenames[i-nh-nj-nk];
		printf("%d, %s\n", i, l_nimage_char[i]);
	}




	
// (1)ポジティブサンプルの読み込み
	for (i = 0; i < l_pimage_num; i++) {
	//	sprintf (filename, "positive/%03d.png", i);
		img_org = cvLoadImage (l_pimage_char[i], CV_LOAD_IMAGE_GRAYSCALE);
		sample_img = cvCreateImage (cvSize (width, height), IPL_DEPTH_8U, 1);
		cvResize (img_org, sample_img, CV_INTER_NN);
		cvSmooth (sample_img, sample_img, CV_GAUSSIAN, 3, 0, 0, 0);
		for (ii = 0; ii < height; ii++) {
			for (jj = 0; jj < width; jj++) {
				l_data[i * image_dim + (ii * width) + jj] = float((int) ((uchar) (sample_img->imageData[ii * sample_img->widthStep + jj])) / 255.0);
			}
		}
		l_res[i] = 1;
	}

  // (2)ネガティブサンプルの読み込み
  j = i;
  for (i = j; i < j + l_nimage_num; i++) {
  //  sprintf (filename, "negative/%03d.jpg", i - j);
    img_org = cvLoadImage (l_nimage_char[i-j], CV_LOAD_IMAGE_GRAYSCALE);
    sample_img = cvCreateImage (cvSize (width, height), IPL_DEPTH_8U, 1);
    cvResize (img_org, sample_img, CV_INTER_NN);
    cvSmooth (sample_img, sample_img, CV_GAUSSIAN, 3, 0, 0, 0);
    for (ii = 0; ii < height; ii++) {
      for (jj = 0; jj < width; jj++) {
        l_data[i * image_dim + (ii * width) + jj] =float ((int) ((uchar) (sample_img->imageData[ii * sample_img->widthStep + jj])) / 255.0);
      }
    }
    l_res[i] = 0;
  }

  // (3)SVM学習データとパラメータの初期化
  cvInitMatHeader (&data_mat, all_image_num, image_dim, CV_32FC1, l_data, CV_AUTOSTEP);
  cvInitMatHeader (&res_mat, all_image_num, 1, CV_32SC1, l_res, CV_AUTOSTEP);
  criteria = cvTermCriteria (CV_TERMCRIT_EPS, 1000, FLT_EPSILON);
  param = CvSVMParams (CvSVM::C_SVC, CvSVM::RBF, 10.0, 0.09, 1.0, 10.0, 0.5, 1.0, NULL, criteria);

  // (4)SVMの学習とデータの保存
  svm.train (&data_mat, &res_mat, NULL, NULL, param);
  svm.save ("l_svm_image.xml");
  // l vs hjkn ここまで









  cvReleaseImage (&img_org);
  cvReleaseImage (&sample_img);

  return 0;
}

