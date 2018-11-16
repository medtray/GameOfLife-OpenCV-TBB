#pragma once
#include <iostream>
#include <tbb/tbb.h>
#include <opencv2/opencv.hpp>
using namespace cv;

class State  {
public:
    
    int _State_WIDTH; 
    int _State_HEIGHT; 
    Mat _state;
   
public:
State(const int &State_WIDTH, const int &State_HEIGHT,Mat state) :  //constructor
        _State_WIDTH(State_WIDTH), _State_HEIGHT(State_HEIGHT),_state(state)
    {   
    }

    int getPoint(const int x, const int y) const {  //get point in position (x,y)
    return _state.at<double>(x, y);
    }

    int AlivePixelsForPoint(const int x, const int y) const { //calculate number of alive pixels around point (x,y)
    int pixels_alive = 0;

    int neighbor_left = x-1;
    if (neighbor_left < 0)
        neighbor_left = x;

    int neighbor_right = x+1;
    if (neighbor_right >= _State_WIDTH) 
        neighbor_right = x;
    
    int neighbor_up = y-1;
    if (neighbor_up < 0)
        neighbor_up = y;

    int neighbor_down = y+1;
    if (neighbor_down >= _State_HEIGHT)
        neighbor_down = y;

    for (int x1=neighbor_left; x1<=neighbor_right; ++x1) {
        for (int y1=neighbor_up; y1<=neighbor_down; ++y1) {
            if (x1==x && y1==y)
                continue;
            pixels_alive += _state.at<double>(x1, y1);
        }
    }
    return pixels_alive;
    }

   
    int updatePoint(const int current_pixel, const int pixels_alive) const { //update value of pixel
    // using current value and number of pixels alive
    if (current_pixel==1) {
        if (pixels_alive > 3 || pixels_alive < 2)
            return 0;
    } else {
        if (pixels_alive == 3)
            return 1;
    }
    return current_pixel;
    }

    
  void UpdateState(const State &previous_state) { //update current state given previous state
    
  auto update_pixel = [&](double &pixel, const int * position) 
  {
   pixel= updatePoint(previous_state.getPoint(position[0], position[1]), previous_state.AlivePixelsForPoint(position[0], position[1]));

  };

    _state.forEach<double>(update_pixel); // update all pixels in parallel

  }
   
};