#include <opencv2/opencv.hpp>
#include <stdio.h>
using namespace cv;
using namespace std;


/************************************************
函  数  名: getGaussianArray
功       能 : 获取高斯分布数组 
输入参数 :
arr_size   :矩阵大小
sigma      :
输出参数 :
array      :二维数组
/**********************************************/
double **getGaussianArray(int arr_size, double sigma)
{
    int i, j;
    // [1] 初始化权值数组
    double **array = new double*[arr_size];
    for (i = 0; i < arr_size; i++) {
        array[i] = new double[arr_size];
    }
    // [2] 高斯分布计算
    int center_i, center_j;
    center_i = center_j = arr_size / 2;
    double pi = 3.141592653589793;
    double sum = 0.0f;
    // [2-1] 高斯函数
    for (i = 0; i < arr_size; i++ ) {
        for (j = 0; j < arr_size; j++) {
            array[i][j] = 
                //后面进行归一化，这部分可以不用
                //0.5f *pi*(sigma*sigma) * 
                exp( -(1.0f)* ( ((i-center_i)*(i-center_i)+(j-center_j)*(j-center_j)) /
                                                             (2.0f*sigma*sigma) ));
            sum += array[i][j];
        }
    }
    // [2-2] 归一化求权值
    for (i = 0; i < arr_size; i++) {
        for (j = 0; j < arr_size; j++) {
            array[i][j] /= sum;
            //printf(" [%.15f] ", array[i][j]);
        }
        //printf("\n");
    }
    return array;
}


/************************************************
函  数  名: extendEdge
功       能 : 图像边缘扩充(对称-映射)
输入参数 :
_src         :输入图像
w            :输入图像宽
h             :输入图像高
_size        :要扩充边缘大小(两边个 _size / 2)
输出参数 :
_dst :输出图像
/**********************************************/
void extendEdge(const unsigned char *_src, unsigned char *_dst, int w, int h, int _size)
{
    int center = _size / 2;
    int size = _size  - 1;  
    int ww = w + size;
    int hh = h + size;
    //unsigned char src[ww * hh];
    int i, j;

    //center
    for ( i = 0; i < h; i++)
    {
        for ( j = 0; j < w; j++)
        {
            /* code */
            *(_dst + (i + center) * ww + (j + center)) = *(_src + i * w + j);
        }
        /* code */
    }

    //left
    //printf("i : %d i:%d\n", centor, hh - centor);
    for (i = center; i < hh - center; i++)
    {
        int size = _size - 1;
        for ( j = 0; j < center; j++)
        {
            int len = i * ww + j;
            int lenn = len + size;
            //printf("w : %d ww:%d size:%d\n", len, lenn, size);
            *(_dst + len) = *(_dst + lenn);
            size = size - 2;
           
        }
       
    }

    //right
    for (i = center; i < hh - center; i++)
    {
        int size = 2;
        for ( j = ww - center; j < ww; j++)
        {
            *(_dst + i * ww + j) = *(_dst + i * ww + j - size);
            size = size + 2;   
        }
    }

    //up
    size = _size - 1;
    for (i = 0; i < center; i++)
    {
        for ( j = 0; j < ww; j++)
        {
            *(_dst + i * ww + j) = *(_dst + (i + size) * ww + j);   
        }  
        size = size - 2;       
    }

    //down
    size = 2;
    for (i = hh - center; i < hh; i++)
    {
        for ( j = 0; j < ww; j++)
        {
            *(_dst + i * ww + j) = *(_dst + (i - size) * ww + j);
        }      
        size = size + 2; 
    }
    
}


/************************************************
函  数  名: gaussian
功       能 : 获取高斯分布数组 
输入参数 :
_src         :输入图像
w            :输入图像宽
h             :输入图像高
_size        :高斯核大小
sigma      :
输出参数 :
_dst         :输出图像
/**********************************************/
void myGaussian(const unsigned char *_src, unsigned char *_dst, int w, int h, int _size, double sigma)
{
    //获得高斯核
	double **_array;
	_array = getGaussianArray(_size, sigma);

    int ww = w + _size - 1;
    int hh = h + _size - 1;

    //扩充图像边缘,方便计算边缘
    unsigned char dst[ww * hh];
    extendEdge(_src, dst, w, h, _size);

    // [1] 扫描
	int center = _size / 2;
    for (int i = 0; i < hh; i++) 
	{
        for (int j = 0; j < ww; j++) 
		{
            // [2] 忽略边缘
                        if (i > (_size / 2) - 1 && j > (_size / 2) - 1 &&
                i < hh - (_size / 2) && j < ww - (_size / 2))
			{
                // [3] 找到图像输入点f(i,j),以输入点为中心与核中心对齐
                //     核心为中心参考点 卷积算子=>高斯矩阵180度转向计算
                //     x y 代表卷积核的权值坐标   i j 代表图像输入点坐标
                //     卷积算子     (f*g)(i,j) = f(i-k,j-l)g(k,l)          f代表图像输入 g代表核
                //     带入核参考点 (f*g)(i,j) = f(i-(k-ai), j-(l-aj))g(k,l)   ai,aj 核参考点
                //     加权求和  注意：核的坐标以左上0,0起点
                double sum = 0.0;
                for (int k = 0; k < _size; k++) 
				{
                    for (int l = 0; l < _size; l++) 
					{
						sum += *(dst + (i-k+center) * ww + j-l+center) * _array[k][l];
                    }
                }
                // 放入中间结果,计算所得的值与没有计算的值不能混用
                *(_dst + (i - center) * w + j - center) = sum;
            }
        }
    }
}

int main()
{
    int size = 9;
	cv::Mat matSrc, matDst1, matDst2 ,matDst3;
	matSrc = cv::imread("lena.jpg", 0);
	matDst1 = Mat::zeros(matSrc.rows + 0, matSrc.cols + 0, CV_8UC1);

	myGaussian(matSrc.data, matDst1.data, matSrc.cols, matSrc.rows, size, 1.5);
	imwrite("matDst1.bmp",matDst1);

	matDst2 = Mat::zeros(matSrc.rows, matSrc.cols, CV_8UC1);
	GaussianBlur(matSrc, matDst2, Size(size,size),1.5);
	imwrite("matDst2.bmp",matDst2);

	for (int x = 0; x < 1024; x++) 
	{
		printf("x->:%d matDst1 : %d  matDst2:%d\n", x, *(matDst1.data + x), *(matDst2.data + x));
	}
}
