
#include "sc.h"

using namespace cv;
using namespace std;


bool seam_carving(Mat& in_image, int new_width, int new_height, Mat& out_image){

    // some sanity checks
    // Check 1 -> new_width <= in_image.cols
    if(new_width>in_image.cols){
        cout<<"Invalid request!!! new_width has to be smaller than the current size!"<<endl;
        return false;
    }
    if(new_height>in_image.rows){
        cout<<"Invalid request!!! ne_height has to be smaller than the current size!"<<endl;
        return false;
    }

    if(new_width<=0){
        cout<<"Invalid request!!! new_width has to be positive!"<<endl;
        return false;

    }

    if(new_height<=0){
        cout<<"Invalid request!!! new_height has to be positive!"<<endl;
        return false;

    }


    return seam_carving_trivial(in_image, new_width, new_height, out_image);
}


void rotateImage(Mat &matImage, int angle){
    if (angle == -90){
        transpose(matImage, matImage);
        flip(matImage, matImage,0);
    } else if (angle == 90){
        transpose(matImage, matImage);
        flip(matImage, matImage,1);
    }
}
// seam carves by removing trivial seams
bool seam_carving_trivial(Mat& in_image, int new_width, int new_height, Mat& out_image){

    Mat iimage = in_image.clone();
    Mat oimage = in_image.clone();
    Mat bluredImage = in_image.clone();
    GaussianBlur(bluredImage, bluredImage,Size(3,3),0,0, BORDER_DEFAULT);

    for(int i=0; i<in_image.cols-new_width;i++){
        reduce_seam_trivial(iimage,oimage,'v',bluredImage);
        iimage=oimage.clone();
    }

    for(int i=0; i<in_image.rows-new_height;i++){
        reduce_seam_trivial(iimage,oimage,'h',bluredImage);
        iimage=oimage.clone();
    }

    out_image = oimage.clone();
    return true;
}

bool reduce_seam_trivial(Mat& iimage, Mat& oimage, char align, Mat& bluredImage){

    if(align=='h') {
        rotateImage(iimage, 90);
        rotateImage(bluredImage,90);
    }

    Mat gradX, gradY, absGradX, absGradY,gray,sobelGradient;
    cvtColor(iimage,gray,CV_BGR2GRAY);
    Sobel(gray, gradX, CV_32F, 1, 0,3);
    convertScaleAbs( gradX, absGradX );
    Sobel(gray, gradY, CV_32F, 0, 1,3);
    convertScaleAbs( gradY, absGradY );
    addWeighted( absGradX, 0.5, absGradY, 0.5, 0, sobelGradient);
    absGradX.release();
    absGradY.release();
    gradX.release();
    gradY.release();
    gray.release();

    int rows = sobelGradient.rows;
    int cols = sobelGradient.cols;
    int minEnergy;
    int minValueCol= 0;
    vector<int> seam(rows);
    int r=0,c=0;

    int energyMatrix[rows][cols];

    for(int j=0;j<cols;j++){
        energyMatrix[0][j]= (int)sobelGradient.at<uchar>(0,j);
    }

    for(int i = 1; i < rows; i++){
        for(int j = 0; j < cols; j++){
            if (j == 0)
                energyMatrix[i][j] = min(energyMatrix[i-1][j+1], energyMatrix[i-1][j]);
            else if (j == cols-1)
                energyMatrix[i][j] = min(energyMatrix[i-1][j-1], energyMatrix[i-1][j]);
            else
                energyMatrix[i][j] = min(min(energyMatrix[i-1][j-1], energyMatrix[i-1][j]), energyMatrix[i-1][j+1]);
            energyMatrix[i][j] += (int)sobelGradient.at<uchar>(i,j);
        }
    }

    minEnergy = energyMatrix[rows-1][0];

    for(int j=1;j<cols;j++){
        if(minEnergy > energyMatrix[rows-1][j]){
            minEnergy = energyMatrix[rows-1][j];
            minValueCol = j;
        }
    }

    r = rows-1;
    c = minValueCol;
    seam[r]=c;
    while(r!=0){
        int val = energyMatrix[r][c] - (int)sobelGradient.at<uchar>(r,c);
        if(c==cols-1){
            if(val==energyMatrix[r-1][c-1])
                c=c-1;
        }
        else if(c==0){
            if(val==energyMatrix[r-1][c+1])
                c=c+1;
        }
        else{
            if(val == energyMatrix[r-1][c-1])
                c=c-1;
            else if(val == energyMatrix[r-1][c+1])
                c=c+1;
        }
        r--;
        seam[r]=c;
    }

    Mat updatedImage = Mat(iimage.rows, iimage.cols-1, CV_8UC3);
    Mat updatedBlurImage = Mat(bluredImage.rows, bluredImage.cols-1, CV_8UC3);

    int rs= iimage.rows;
    int cs= iimage.cols;
    for(int row=0; row<rs;row++){
        int a=0;
        for(int col=0;col<cs;col++){
            if(col!=seam[row]){
                updatedImage.at<Vec3b>(row,a)=iimage.at<Vec3b>(row,col);
                updatedBlurImage.at<Vec3b>(row,a)=iimage.at<Vec3b>(row,col);
                a++;
            }
        }
    }

    oimage=updatedImage.clone();
    bluredImage=updatedBlurImage.clone();
    updatedImage.release();
    updatedBlurImage.release();

    if(align=='h'){
        rotateImage(oimage,-90);
        rotateImage(bluredImage,-90);
    }

    return true;
}

