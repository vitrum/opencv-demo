//
//  auto.cpp
//  ovtest
//
//  Created by vitrum.zhu on 15/7/23.
//  Copyright (c) 2015年 vitrum.zhu. All rights reserved.
//

#include "auto.h"


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
#include <iostream>
using namespace cv;
using namespace std;



cv::Point2f center(0,0);

cv::Point2f computeIntersect(cv::Vec4i a,
                             cv::Vec4i b)
{
    int x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3], x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];
    //    float denom;
    
    if (float d = ((float)(x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4)))
    {
        cv::Point2f pt;
        pt.x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / d;
        pt.y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / d;
        return pt;
    }
    else
        return cv::Point2f(-1, -1);
}

void sortCorners(std::vector<cv::Point2f>& corners,
                 cv::Point2f center)
{
    std::vector<cv::Point2f> top, bot;
    
    for (int i = 0; i < corners.size(); i++)
    {
        if (corners[i].y < center.y)
            top.push_back(corners[i]);
        else
            bot.push_back(corners[i]);
    }
    corners.clear();
    
    if (top.size() == 2 && bot.size() == 2){
        cv::Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
        cv::Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
        cv::Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
        cv::Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];
        
        
        corners.push_back(tl);
        corners.push_back(tr);
        corners.push_back(br);
        corners.push_back(bl);
    }
}

int main()
{
    cv::Mat src = cv::imread("myimg-3.png");
    Mat img_proc ,img_dis ;
    
    if (src.empty())
        return -1;
    int w = src.size().width, h = src.size().height, min_w = 100;
    double scale = min(30.0, w * 1.0 / min_w);
    int w_proc = w * 1.0 / scale, h_proc = h * 1.0 / scale;
    resize(src, img_proc, Size(w_proc, h_proc));
    img_dis = img_proc.clone();
    
    
    cv::Mat bw;
    cv::cvtColor(img_proc, bw, CV_BGR2GRAY);
    cv::blur(bw, bw, cv::Size(3, 3));
    cv::Canny(bw, bw, 50, 200, 3);
    
    
    
    
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(bw, lines, 1, CV_PI/180, 30, 30, 0);
    
    //    cv::HoughLinesP(bw, lines, 1,CV_PI / 180, w_proc / 3, w_proc / 3, 0);
    
    
    cout<<"w:"<< src.size().width<< ", w_proc:"<<w_proc <<endl;
    
    
    
    
    // Expand the lines
    for (int i = 0; i < lines.size(); i++)
    {
        cv::Vec4i v = lines[i];
        lines[i][0] = 0;
        lines[i][1] = ((float)v[1] - v[3]) / (v[0] - v[2]) * -v[0] + v[1];
        lines[i][2] = src.cols;
        lines[i][3] = ((float)v[1] - v[3]) / (v[0] - v[2]) * (src.cols - v[2]) + v[3];
    }
    
    cv::Mat dst = src.clone();
    
    // Draw lines 把线打印出来看看
    for (int i = 0; i < lines.size(); i++)
    {
        cv::Vec4i v = lines[i];
        cv::line(img_dis, cv::Point(v[0], v[1]), cv::Point(v[2], v[3]), CV_RGB(0,255,0));
    }
    imshow("bwimage", bw);
    cv::imshow("image", img_dis);
    cv::waitKey();
    
    
    
    
    std::vector<cv::Point2f> corners;
    for (int i = 0; i < lines.size(); i++)
    {
        for (int j = i+1; j < lines.size(); j++)
        {
            cv::Point2f pt = computeIntersect(lines[i], lines[j]);
            if (pt.x >= 0 && pt.y >= 0)
                corners.push_back(pt);
        }
    }
    
    std::vector<cv::Point2f> approx;
    cv::approxPolyDP(cv::Mat(corners), approx, cv::arcLength(cv::Mat(corners), true) * 0.02, true);
    
    
    
    
    
    
    if (approx.size() != 4)
    {
        std::cout << "The object is not quadrilateral!" << std::endl;
        std::cout << "Approx size is : " <<approx.size()<< std::endl;
        return -1;
    }
    
    // Get mass center
    for (int i = 0; i < corners.size(); i++)
        center += corners[i];
    center *= (1. / corners.size());
    
    sortCorners(corners, center);
    if (corners.size() == 0){
        std::cout << "The corners were not sorted correctly!" << std::endl;
        //        std::cout << "The corners were not sorted correctly! : " <<corners.size()<< std::endl;
        return -1;
    }
    
    
    // Draw corner points
    cv::circle(img_dis, corners[0], 3, CV_RGB(255,0,0), 2);
    cv::circle(img_dis, corners[1], 3, CV_RGB(0,255,0), 2);
    cv::circle(img_dis, corners[2], 3, CV_RGB(0,0,255), 2);
    cv::circle(img_dis, corners[3], 3, CV_RGB(255,255,255), 2);
    
    // Draw mass center
    cv::circle(img_dis, center, 3, CV_RGB(255,255,0), 2);
    
    cv::Mat quad = cv::Mat::zeros(300, 220, CV_8UC3);
    
    std::vector<cv::Point2f> quad_pts;
    quad_pts.push_back(cv::Point2f(0, 0));
    quad_pts.push_back(cv::Point2f(quad.cols, 0));
    quad_pts.push_back(cv::Point2f(quad.cols, quad.rows));
    quad_pts.push_back(cv::Point2f(0, quad.rows));
    
    cv::Mat transmtx = cv::getPerspectiveTransform(corners, quad_pts);
    cv::warpPerspective(src, quad, transmtx, quad.size());
    
    cv::imshow("image", img_dis);
    cv::imshow("quadrilateral", quad);
    cv::waitKey();
    return 0;
}