//
//  main.cpp
//  ovtest
//
//  Created by vitrum.zhu on 15/7/21.
//  Copyright (c) 2015年 vitrum.zhu. All rights reserved.
//

// Example showing how to read and write images
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;



cv::Point2f center(0,0);

cv::Point2f computeIntersect(cv::Vec4i a,cv::Vec4i b)
{
    int x1 = a[0],y1 = a[1],x2 = a[2],y2 = a[3],x3 = b[0],y3 = b[1],x4 = b[2],y4 = b[3];
    
    if (float d = ((float)(x1 - x2)*(y3 - y4)-(y1 - y2)*(x3 - x4)))
    {
        cv::Point2f pt;
        pt.x = ((x1*y2 - y1*x2)*(x3 - x4) - (x1 - x2)*(x3*y4 - y3*x4))/d;
        pt.y = ((x1*y2 - y1*x2)*(y3 - y4) - (y1 - y2)*(x3*y4 - y3*x4))/d;
        return pt;
    }
    else
        return cv::Point2f(-1,-1);
}

void sortCorners(std::vector<cv::Point2f>& corners,cv::Point2f center)
{
    std::vector<cv::Point2f> top,bot;
    
    for (unsigned int i =0;i< corners.size();i++)
    {
        if (corners[i].y<center.y)
        {
            top.push_back(corners[i]);
        }
        else
        {
            bot.push_back(corners[i]);
        }
    }
    
    cv::Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
    cv::Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
    cv::Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
    cv::Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];
    
    corners.clear();
    //注意以下存放顺序是顺时针，当时这里出错了，如果想任意顺序下文开辟的四边形矩阵注意对应
    corners.push_back(tl);
    corners.push_back(tr);
    corners.push_back(br);
    corners.push_back(bl);
    
}

// Main

int main(int argc, char** argv)
{
    // Load an image from file - change this based on your image name
    Mat img = imread("myimg.png", CV_LOAD_IMAGE_UNCHANGED);
    Mat img_proc;
    if(img.empty())
    {
        fprintf(stderr, "failed to load input image\n");
        return -1;
    }
    int w = img.size().width, h = img.size().height, min_w = 200;
    double scale = min(10.0, w * 1.0 / min_w);
    int w_proc = w * 1.0 / scale, h_proc = h * 1.0 / scale;
    
    resize(img, img_proc, Size(w_proc, h_proc));
    Mat img_dis = img_proc.clone();
    
    // this is just to show, that you won't have to pre-alloc
    // result-images with c++ any more..
    Mat bw;
    cvtColor(img_proc,bw,CV_BGR2GRAY);
    
    imshow("gray_src", bw);
    blur(bw,bw,cv::Size(3,3));
    
    // Save temp image file.
//    if( ! imwrite("image_gray.png", bw) )
//    {
//        fprintf(stderr, "failed to write image file\n");
//    }
    
    imshow("blur", bw);
    Canny(bw, bw, 100, 100,3);
    
    imshow("cannyblur",bw);

    
    // Save temp image file.
//    if( ! imwrite("image_cannyblur.png", bw) )
//    {
//        fprintf(stderr, "failed to write image file\n");
//    }



    std::vector<cv::Vec4i> lines;
//    std::vector<cv::Line> horizontals, verticals;
//    HoughLinesP(bw, lines, 1, CV_PI / 180, w_proc / 3, w_proc / 3, 20);
    cv::HoughLinesP(bw,lines,1,CV_PI/180,70,30,10);
    std::cout<<"共检测出:"<<lines.size()<<std::endl;

    
    
    //1像素分辨能力  1度的角度分辨能力        >70可以检测成连线       30是最小线长
    //在直线L上的点（且点与点之间距离小于maxLineGap=10的）连成线段，然后这些点全部删除，并且记录该线段的参数，就是起始点和终止点
    
    
    // Save temp image file.
//    if( ! imwrite("image_HoughLinesP.png", bw) )
//    {
//        fprintf(stderr, "failed to write image file\n");
//    }
    
    //needed for visualization only//这里是将检测的线调整到延长至全屏，即射线的效果，其实可以不必这么做
//    for (unsigned int i = 0;i<lines.size();i++)
//    {
//        cv::Vec4i v = lines[i];
//        lines[i][0] = 0;
//        lines[i][1] = ((float)v[1] - v[3])/(v[0] - v[2])* -v[0] + v[1];
//        lines[i][2] = img.cols;
//        lines[i][3] = ((float)v[1] - v[3])/(v[0] - v[2])*(img.cols - v[2]) + v[3];
//        
//    }
    
    std::vector<cv::Point2f> corners;//线的交点存储
    for (unsigned int i = 0;i<lines.size();i++)
    {
        for (unsigned int j=i+1;j<lines.size();j++)
        {
            cv::Point2f pt = computeIntersect(lines[i],lines[j]);
            if (pt.x >= 0 && pt.y >=0)
            {
                corners.push_back(pt);
            }
        }
    }
    std::cout<<"共交叉点出:"<<corners.size()<<std::endl;
    
    
    std::vector<cv::Point2f> approx;
    cv::approxPolyDP(cv::Mat(corners),approx,cv::arcLength(cv::Mat(corners),true)*0.02,true);
    
    if (approx.size()!=4)
    {
        std::cout<<"The object is not quadrilateral（四边形）:"<<approx.size()<<std::endl;
        return -1;
    }
    
    //get mass center
    for (unsigned int i = 0;i < corners.size();i++)
    {
        center += corners[i];
    }
    center *=(1./corners.size());
    
    sortCorners(corners,center);
    
    cv::Mat dst = img.clone();
    
    //Draw Lines
    for (unsigned int i = 0;i<lines.size();i++)
    {
        cv::Vec4i v = lines[i];
        cv::line(dst,cv::Point(v[0],v[1]),cv::Point(v[2],v[3]),CV_RGB(0,255,0));    //目标版块画绿线
    }
    
    //draw corner points
    cv::circle(dst,corners[0],3,CV_RGB(255,0,0),2);
    cv::circle(dst,corners[1],3,CV_RGB(0,255,0),2);
    cv::circle(dst,corners[2],3,CV_RGB(0,0,255),2);
    cv::circle(dst,corners[3],3,CV_RGB(255,255,255),2);
    
    //draw mass center
    cv::circle(dst,center,3,CV_RGB(255,255,0),2);
    
    cv::Mat quad = cv::Mat::zeros(300,220,CV_8UC3);//设定校正过的图片从320*240变为300*220
    
    //corners of the destination image
    std::vector<cv::Point2f> quad_pts;
    quad_pts.push_back(cv::Point2f(0,0));
    quad_pts.push_back(cv::Point2f(quad.cols,0));//(220,0)
    quad_pts.push_back(cv::Point2f(quad.cols,quad.rows));//(220,300)
    quad_pts.push_back(cv::Point2f(0,quad.rows));
    
    // Get transformation matrix
    cv::Mat transmtx = cv::getPerspectiveTransform(corners,quad_pts);   //求源坐标系（已畸变的）与目标坐标系的转换矩阵
    
    // Apply perspective transformation透视转换
    cv::warpPerspective(img,quad,transmtx,quad.size());
    cv::imshow("src",img);
    cv::imshow("image",dst);
    cv::imshow("quadrilateral",quad);
    
    cv::waitKey();  
    return 0;
    
    

}