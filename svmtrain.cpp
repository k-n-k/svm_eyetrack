#include <cv.h>
#include <highgui.h>
#include <ml.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <string>
#include <vector>
using namespace std;

int is_str_bmp(const char *filename, const char *topchar){
	int n = strlen(filename);
	/*
	 * int n = 0;
	while(filename[n]!= '\0'){
		n++;
	}*/
	if((strcmp(".bmp", filename+(n-4)) == 0)&&
		(strncmp(topchar, filename,strlen(topchar)) == 0)){
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
	enum {N, H, J, K, L, DIRS_SIZE} Dirs;
	string DIR_NAMES[] = {"N",
			      "H",
			      "J",
			      "K",
			      "L"};
	vector<string> filenames[DIRS_SIZE];
	dir = opendir("./");
	string topstrs[5] = {"n_right",
			     "h_right",
			     "j_right",
			     "k_right",
			     "l_right"};
	while((dp = readdir(dir)) != NULL){
	  if(dp->d_type == DT_REG){
	    for(int i = 0; i < DIRS_SIZE; ++i){
	      if(is_str_bmp(dp->d_name, topstrs[i].c_str())){
		filenames[i].push_back(dp->d_name);
	      }
	    }
	  }
	}
	closedir(dir);

	// printf( "n = %d, h = %d, j = %d, k = %d, l = %d\n", nn, nh, nj, nk, nl);
	for(int i = 0; i < DIRS_SIZE; ++i){
	  printf("%s = %d, ", DIR_NAMES[i].c_str(), (int)filenames[i].size());
	}
	printf("\n");

	for(int cur = 0; cur < DIRS_SIZE; ++cur){
	  // cur vs others
	  int pimage_num = filenames[cur].size();         /* ポジティブサンプル数 */
	  int nimage_num = 0;        /* ネガティブサンプル数 */
	  for(int c = 0; c < DIRS_SIZE; ++c){
	    if(c == cur){ continue; }
	    nimage_num += filenames[c].size();
	  }
	  int all_image_num = pimage_num + nimage_num;
	  vector<char const *> pimage_char;
	  vector<char const *> nimage_char;
	  vector<int> res;
	  vector<float> data;

	  /* ベクタのメモリを先に確保することで、再確保を未然に防ぐ */
	  pimage_char.reserve(pimage_num);
	  nimage_char.reserve(nimage_num);
	  res.reserve(all_image_num);
	  data.reserve(all_image_num * image_dim);


	  for(int i = 0; i < pimage_num; i++){
	    pimage_char.push_back(filenames[cur][i].c_str());
	    printf("%d, %s\n", i, pimage_char[i]);
	  }
	  
	  for(int c = 0; c < DIRS_SIZE; ++c){
	    if(c == cur){ continue; }
	    for(int cc = 0; cc < filenames[c].size(); ++cc){
	      nimage_char.push_back(filenames[c][cc].c_str());
	      printf("%d, %s\n", (int)nimage_char.size() - 1, nimage_char.back());
	    }
	  }

	
	  // (1)ポジティブサンプルの読み込み
	  for (int i = 0; i < pimage_num; i++) {
	    //	sprintf (filename, "positive/%03d.png", i);
	    img_org = cvLoadImage (pimage_char[i], CV_LOAD_IMAGE_GRAYSCALE);
	    sample_img = cvCreateImage (cvSize (width, height), IPL_DEPTH_8U, 1);
	    cvResize (img_org, sample_img, CV_INTER_NN);
	    cvSmooth (sample_img, sample_img, CV_GAUSSIAN, 3, 0, 0, 0);
	    for (ii = 0; ii < height; ii++) {
	      for (jj = 0; jj < width; jj++) {
		data.push_back((float)((int) ((uchar) (sample_img->imageData[ii * sample_img->widthStep + jj])) / 255.0));
	      }
	    }
	    res.push_back(1);
	  }
	  
	  // (2)ネガティブサンプルの読み込み
	  for (int i = 0; i < nimage_num; i++) {
	    //  sprintf (filename, "negative/%03d.jpg", i - j);
	    img_org = cvLoadImage (nimage_char[i], CV_LOAD_IMAGE_GRAYSCALE);
	    sample_img = cvCreateImage (cvSize (width, height), IPL_DEPTH_8U, 1);
	    cvResize (img_org, sample_img, CV_INTER_NN);
	    cvSmooth (sample_img, sample_img, CV_GAUSSIAN, 3, 0, 0, 0);
	    for (ii = 0; ii < height; ii++) {
	      for (jj = 0; jj < width; jj++) {
		data.push_back((float) ((int) ((uchar) (sample_img->imageData[ii * sample_img->widthStep + jj])) / 255.0));
	      }
	    }
	    res.push_back(0);
	  }
	  
	  // (3)SVM学習データとパラメータの初期化
	  cvInitMatHeader (&data_mat, all_image_num, image_dim, CV_32FC1, &data[0], CV_AUTOSTEP);
	  cvInitMatHeader (&res_mat, all_image_num, 1, CV_32SC1, &res[0], CV_AUTOSTEP);
	  criteria = cvTermCriteria (CV_TERMCRIT_EPS, 1000, FLT_EPSILON);
	  param = CvSVMParams (CvSVM::C_SVC, CvSVM::RBF, 10.0, 0.09, 1.0, 10.0, 0.5, 1.0, NULL, criteria);
	  
	  // (4)SVMの学習とデータの保存
	  svm.train (&data_mat, &res_mat, NULL, NULL, param);
	  svm.save ("n_svm_image.xml");
	  
	}

	cvReleaseImage (&img_org);
	cvReleaseImage (&sample_img);
	return 0;
	/*
	int nn = filenames[N].size();
	int nh = filenames[H].size();
	int nj = filenames[J].size();
	int nk = filenames[K].size();
	int nl = filenames[L].size();
	
	vector<char *> n_filenames;
	for(int i = 0; i < filenames[N].size(); ++i){
	  n_filenames.push_back(new char[256]);
	  strcpy(n_filenames.back(), filenames[N][i].c_str());
	}
	vector<char *> h_filenames;
	for(int i = 0; i < filenames[H].size(); ++i){
	  h_filenames.push_back(new char[256]);
	  strcpy(h_filenames.back(), filenames[H][i].c_str());
	}
	vector<char *> j_filenames;
	for(int i = 0; i < filenames[J].size(); ++i){
	  j_filenames.push_back(new char[256]);
	  strcpy(j_filenames.back(), filenames[J][i].c_str());
	}
	vector<char *> k_filenames;
	for(int i = 0; i < filenames[K].size(); ++i){
	  k_filenames.push_back(new char[256]);
	  strcpy(k_filenames.back(), filenames[K][i].c_str());
	}
	vector<char *> l_filenames;
	for(int i = 0; i < filenames[L].size(); ++i){
	  l_filenames.push_back(new char[256]);
	  strcpy(l_filenames.back(), filenames[L][i].c_str());
	}
	*/
}

