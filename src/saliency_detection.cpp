#include "saliency_detection.h"
/*
void Saliency_Extraction::init_param(int image_width, int image_height) {
	width = image_width;
	height = image_height;
}
*/
void Saliency_Extraction::saliency_extraction(const cv::Mat& image, cv::Mat& saliency_map) {
	//log spectral
	//cv::Mat result = image.clone();
	//创建Mat数组planes，两个元素，一个是扩展后并转为float型的图像，另一个是同样大小，赋值为0的图像。
	cv::Mat planes[] = { cv::Mat_<float>(image.clone()), cv::Mat::zeros(image.size(), CV_32F) };
	//复数图像
	cv::Mat complexImg;
	//通道合并,双通道
	cv::merge(planes, 2, complexImg);
	//离散傅里叶变换	
	cv::dft(complexImg, complexImg);
	//复数图像中实部虚部分开，分离complexI的两个通道到数组 planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
	cv::split(complexImg, planes);
	cv::Mat mag, logmag, smooth, spectralResidual;
	//计算幅值，mag为输出赋值
	cv::magnitude(planes[0], planes[1], mag);
	// compute the magnitude and switch to logarithmic scale
	// => log(sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
	//对数
	cv::log(mag, logmag);
	//线性滤波，logmag输入、smooth输出、-1为原深度、滤波核size=7
	cv::boxFilter(logmag, smooth, -1, cv::Size(average_filter_size, average_filter_size));
	//图像像素值相减，spectral为输出
	cv::subtract(logmag, smooth, spectralResidual);
	//还原到对数计算前
	cv::exp(spectralResidual, spectralResidual);

	// real part 
	planes[0] = planes[0].mul(spectralResidual) / mag;
	// imaginary part 
	planes[1] = planes[1].mul(spectralResidual) / mag;
	//再次合并成两通道的复数图像
	cv::merge(planes, 2, complexImg);
	cv::dft(complexImg, complexImg, cv::DFT_INVERSE | cv::DFT_SCALE);
	cv::split(complexImg, planes);
	// get magnitude
	cv::magnitude(planes[0], planes[1], mag);
	// get square of magnitude
	//平方
	cv::multiply(mag, mag, mag);
	// Gaussian kernel size. ksize.width and ksize.height can differ but they both must be positive and odd
	//cv::GaussianBlur(mag, mag, cv::Size(5, 5), 8, 8);
	//阈值为1.3×mag第一个通道均值（实域）
	float threshhold = extraction_treshold * cv::mean(mag).val[0];
	//std::cout << mag.at<float>(383, 1249) << std::endl;
	//std::cout << mag.type() << std::endl;
	for (int i = 0;i < image.rows;i++) {
		for (int j = 0;j < image.cols;j++) {
			if (mag.at<float>(i, j) < threshhold)
				//小于阈值是黑色
				saliency_map.at<unsigned char>(i, j) = 0;
			else
				//大于为白色
				saliency_map.at<unsigned char>(i, j) = 255;
		}
	}
	//中值滤波法是一种非线性平滑技术，它将每一像素点的灰度值设置为该点某邻域窗口内的所有像素点灰度值的中值.
	medianBlur(saliency_map, saliency_map, median_filter_size);
	//输出saliencymap（像素级）
}

 void Saliency_Extraction::salience_filtering(const cv::Mat& saliency_map, cv::Mat& image_float, std::vector<SR>& result_arr) {
	int connectivity = 8;
	cv::Mat labels, stats, centroids;
	//输入map、输出label map、stats输出（xywhs）N×5矩阵、Nx2 CV_64F matrix of centroids:
    // [ cx0, cy0; ... ; cx(N-1), cy(N-1
	int obj_num = cv::connectedComponentsWithStats(saliency_map, labels, stats, centroids, connectivity);
	//std::cout << labels.type() << std::endl;
	//display_float_img(labels);
	//display_salience_map(saliency_map.clone(), image_float.clone());
	for (int i = 0; i < stats.rows; i++)
	{
		//check width and height
		int x = stats.at<int>(i, 0);
		int y = stats.at<int>(i, 1);
		int w = stats.at<int>(i, 2);
		int h = stats.at<int>(i, 3);
		//std::cout <<x <<" "<<y<<"   "<< w << " " << h << std::endl;
		//筛选最小宽高
		if (w < min_width || h < min_height)
			continue;
		//处理越界
		if (x == 0 || y == 0 || x + w >= image_float.cols - 1 || y + h >= image_float.rows - 1)
			continue;

		int area = stats.at<int>(i, 4);
		//小于最小占比或者大于最小占比
		if (area < w*h*min_fill_percentage || w * h > image_float.cols*image_float.rows*max_area_percentage)
			continue;
		
		cv::Mat object;
		//讲原图的目标框部分复制给object
		image_float(cv::Rect(x, y, w, h)).copyTo(object);
		//cv::rectangle(image_float, cv::Rect(x, y, w, h), cv::Scalar(255), 1);
		//计算目标框均值
		float mean = cv::sum(object).val[0] / (w*h);
		//std::cout << object.type() << std::endl;
		//计算协方差
		float cov = 0.0;
		for (int p = 0;p < h;p++) {
			for (int q = 0;q < w;q++) {
				//std::cout << x << " " << y << "   " << w << " " << h <<"  "<<p<<","<<q<< "="<<object.at<float>(p, q)<<std::endl;
				//std::cout << object.at<float>(p, q) << std::endl;
				cov += fabs(object.at<float>(p, q) - mean);
			}
		}
		cov = cov / (w*h);
		//std::cout << cov << std::endl;
		//筛选框的协方差
		if (cov < min_cov)
			continue;
		cv::Mat edge;
		//拉普拉斯算法。输出edge
		cv::Laplacian(object, edge, CV_32F, 3);
		float edge_cov = 0;
		for (int p = 0;p < h;p++) {
			for (int q = 0;q < w;q++) {
				//std::cout << edge.at<float>(p, q) << std::endl;
				if(abs(edge.at<float>(p, q))>50)
					edge_cov += 1;
			}
		}
		edge_cov = edge_cov / (w*h);
		//std::cout << edge_cov << std::endl;
		if (edge_cov < min_edge_cov)
			continue;
		Eigen::Map<Eigen::ArrayXXf> image_eigen(&object.at<float>(0, 0), object.cols, object.rows);
		//SR sr(fft(image_eigen.transpose()/255),cov,edge_cov);
		SR sr(image_eigen.transpose()/255, cov, edge_cov);
		sr.set_xy(x, y, w, h);
		result_arr.push_back(sr);
		
	}

	
}
 /*
 void Saliency_Extraction::salience_filtering_eigen(Eigen::ArrayXXf saliency_map, Eigen::ArrayXXf image_float, std::vector<SR>& result_arr) {
	
 }
  */
 void Saliency_Extraction::saliency_extraction_eigen(Eigen::ArrayXXf image, Eigen::ArrayXXf saliency_map) {
	Eigen::ArrayXXcf image_fft = fft(image);
	Eigen::ArrayXXf A = image_fft.abs2();
	Eigen::ArrayXXf L = log(A);
	Eigen::ArrayXXf R = ArrayXXf::Zero(image.rows(), image.cols());
	//find convolution
	for (int i = 0;i < image.rows();i++) {
		for (int j = 0;j < image.cols();j++) {
			float sum = 0.0;
			int count=0;
			for (int p = -average_filter_size/2;p <= average_filter_size / 2;p++) {
				for (int q = -average_filter_size / 2;q <= average_filter_size / 2;q++) {
					int x = i + p;
					int y = j + q;
					if (x >= 0 && x < image.rows() && y >= 0 && y < image.cols()) {
						sum = sum + L(x, y);
						count++;
					}
					
				}
			}
			R(i, j) -= sum / count;
		}
	}

	Eigen::ArrayXXf S = ifft(R.exp()*image_fft/A.sqrt()).square();
	saliency_map = S - extraction_treshold*S.mean();
 }
