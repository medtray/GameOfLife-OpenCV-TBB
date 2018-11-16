/**
 
 *
 * Conway's Game of Life is a simulation, which takes as its initial state a
 * two-dimensional grid of zeroes and ones.  The state of a cell in step k+1 is
 * determined exclusively by the state of that cell and its neighbors in step k.
 *
 * The rules are as follows:
 * - If a "1" cell has fewer than two live neighbors, then it becomes "0" in
 *   the next round.  This is "underpopulation"
 * - If a "1" cell has two or three live neighbors, then it remains "1" in the
 *   next round.  This is "healthy population"
 * - If a "1" cell has more than three live neighbors, then it becomes "0" in
 *   the next round.  This is "overpopulation".
 * - If a "0" cell has exactly three live neighbors, then it becomes "1" in the
 *   next round.  This is "reproduction".
 *
 * More details, and some examples, can be found at
 * https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life.
 *
 * The input to your program will be a text file of N lines, each of which
 * contains M columns.  The values in this file will be 1s and 0s, and they will
 * represent the initial state of the game.  This input should be given by the
 * "-i" parameter.
 *
 * The output of your program will be a video file, generated with OpenCV.  The
 * file name will be determined by the "-o" parameter.  The frame rate of the
 * video will be determined by the "-f" parameter.  The number of pixels per
 * cell of the game will be determined by the "-p" parameter.  Your program
 * should run the simulation for a number of rounds determined by the "-r"
 * parameter.  Your output video should include a text watermark,
 * determined by the "-w" parameter.  Lastly, the "-s" parameter should
 * determine which frames of the game should be output to image files.  For
 * example, "-s1,4,16,256" would result in frames 1, 4, 16, and 256 being output
 * as images named frame1.png, frame4.png, frame16.png, and frame256.png
 *
 * You should consider using tbb or custom threads to both (a) parallelize the
 * creation of frames, and (b) pass frames to the video output generator.  Be
 * sure to use appropriate smart pointers to avoid memory leaks.
 *
 */

#include <iostream>
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <fstream>   // for ifstream
#include <iostream>  // for cout
#include <vector>    // for vector
#include <iomanip>   // for setw
#include <string>    // for string
#include <sstream>   // for stringstream
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv/ml.h"
#include <vector>
#include <iostream>
#include <limits>
#include <math.h>       /* sqrt */
#include <thread>
#include <tbb/pipeline.h>


 #include "state.h"
using namespace std;
using namespace cv;

//read a comma seperated matrix
void ReadMatrix(const char * fname , std::vector< std::vector<double> > & matrix)
{
    string line;
    ifstream in(fname);
    vector<double> v;
    while(getline(in,line))
{
    std::stringstream   linestream(line);
    std::string         value;
    v.clear();

    /* while(getline(linestream,value)) //we can specify other types of delimiter seperated values
    {
      
        v.push_back(atof(value.c_str()));
    } */

     for(char c : line) {
        v.push_back(atof(&c));
    }
    matrix.push_back(v);

}
}
 
 //prepare bounding box matrix for text watermark
cv::Mat TextMat(cv::Mat img,std::string text,cv::Rect& roi)
{
    
    int baseline = 0;
    cv::Size textSize = cv::getTextSize(text, FONT_HERSHEY_COMPLEX_SMALL, 3, 6, &baseline);

    baseline += 6;

    cv::Mat textImg(textSize.height + baseline, textSize.width, img.type());

    cv::Point textOrg((textImg.cols - textSize.width) / 2, (textImg.rows + textSize.height - baseline) / 2);

    cv::putText(textImg, text, textOrg, FONT_HERSHEY_COMPLEX_SMALL, 3, Scalar(150), 6);

    // Resizing according to the ROI
    cv::resize(textImg, textImg, roi.size());

    return  textImg;

}
 
 
/** Store the arguments for our program */
struct arg_t {
  /** The input image */
  std::string in_img = "";


  /** The output video */
  std::string out_vid = "";

  int number_of_rounds=100;
 
  int number_of_pixels_per_cell=100;
  int frame_rate=16;
  std::string text_watermark="";
  std::string images_to_output="";
   /** Should we show the usage */
  bool usage = false;
};



void parse_args(int argc, char **argv, arg_t &args) {
  using namespace std;
  long opt;
  while ((opt = getopt(argc, argv, "i:o:f:p:r:w:s:h")) != -1) {
    switch (opt) {
    case 'i':
      args.in_img = string(optarg);
      break;
    case 'o':
      args.out_vid = string(optarg);
      break;
    case 'f':
      args.frame_rate = atoi(optarg);
      break;
    case 'p':
      args.number_of_pixels_per_cell = atoi(optarg);
      break;
    case 'r':
      args.number_of_rounds = atoi(optarg);
      break;
    case 'w':
      args.text_watermark = string(optarg);
      break;
    case 's':
      args.images_to_output = string(optarg);
      break;
    case 'h':
      args.usage = true;
      break;
    }
  }
}



int main(int argc, char *argv[]) {
    // By now, you should be able to write your own argument parsing code :)
    

    // Parse command line arguments, and if usage was requested, print it and exit
    arg_t args;
    parse_args(argc, argv, args);
    if (args.usage) {
    cout << "-i: in-image, -o: out-video, -f: frame_rate, -p: number_of_pixels_per_cell, -r: number_of_rounds, -w: text_watermark, -s: images_to_output, -h: usage" << endl;
    exit(0);
    }

    std::string file =args.in_img;
    

    //std::cout << " path: " << path << '\n';
    std::string output_video =args.out_vid;
    std::size_t found = output_video.find_last_of("/\\");
    string path=output_video.substr(0,found);

    int number_of_rounds=args.number_of_rounds;
    int number_of_pixels_per_cell=args.number_of_pixels_per_cell;
    int frame_rate=args.frame_rate;
    string text_watermark=args.text_watermark;
    VideoWriter out;
    std::vector<int> images_to_output;

    std::stringstream ss(args.images_to_output);

    int i;

    //read list of images to save

    while (ss >> i)
    {
    images_to_output.push_back(i);

    if (ss.peek() == ',')
    ss.ignore();
    }



    std::vector< std::vector<double> > imatrix;

    //read input text file
    ReadMatrix(file.c_str(),imatrix);
   

    Mat frame0(imatrix.size(), imatrix.at(0).size(), CV_64F);
    //transform vector of vectors to opencv matrix
    for(int i=0; i<frame0.rows; ++i)
        for(int j=0; j<frame0.cols; ++j)
            frame0.at<double>(i, j) = imatrix.at(i).at(j);

    //calculate the length per dimension given the number of pixels per cell
    int size_per_dimension=(int) round(sqrt(number_of_pixels_per_cell)) ;

    //calculate new dimension for resized frame
    int w = frame0.cols*size_per_dimension, h = frame0.rows *size_per_dimension;

    //open a video
    out.open(output_video, out.fourcc('a', 'v', 'c', '1'), frame_rate, Size(w, h), 0);

    //initialize initial state with first frame
    State initial_state(frame0.rows,frame0.cols,frame0);
    Mat frame1(imatrix.size(), imatrix.at(0).size(), CV_64F);
    State next_state(frame0.rows,frame0.cols,frame1);
    Mat current_frame;
    Mat resized_frame;

    //handle first frame 
    current_frame=initial_state._state;
    cv::resize(current_frame, resized_frame, Size(w, h), 0, 0, INTER_NEAREST);

    //resize frame
    resized_frame.convertTo(resized_frame,CV_8UC3);
    resized_frame= 255.0*resized_frame;
    cv::Rect roi(1, h/2, resized_frame.cols/5, resized_frame.rows/6);

    //matrix for text watermark
    cv::Mat TextMatrix= TextMat(resized_frame,text_watermark,roi);
    
    // Put into frame (this is done to avoid putting the text everytime we have new frame)
    cv::Mat destRoi = resized_frame(roi);
    TextMatrix.copyTo(destRoi,TextMatrix);
    
    //add to video
    out << resized_frame;
    //check if we need to save the first state
    auto it = std::find(images_to_output.begin(), images_to_output.end(), 1);
    if (it == images_to_output.end())
    {
    }
    else{
        std::string file2 =path+"/frame"+to_string(1)+".png";
        imwrite(file2,resized_frame);
    }

    //define first filter in the tbb pipeline
    class PrepareFrame: public tbb::filter {
    private:
    State next_state;
    State initial_state;
    int iterations;
    int counter=2;

    public:
    PrepareFrame(State& next_state,State& initial_state,int iterations): filter(tbb::filter::mode::serial_in_order), next_state{next_state}, initial_state{initial_state}, iterations{iterations} {};
    void* operator()(void*){
    if (counter<=iterations)
    {
      //std::cout<<"processing"+to_string(counter)+" frame";
      //update next state
        next_state.UpdateState(initial_state);
        auto state_mat = new cv::Mat{};
        *state_mat=next_state._state;
        //swap pointers 
        std::swap(initial_state, next_state);
        counter++;
        //sleep(1);
        return state_mat;
    }
    return nullptr;

    };
    };

    //define second filter in the tbb pipeline

    class WriteToVideo: public tbb::filter {
    private:
    int w;
    int h;
    int j=2;
    VideoWriter video1;
    std::vector<int> images_to_output;
    cv::Mat TextMatrix;
    string path;
    cv::Rect roi;
    

    public:
    WriteToVideo(int w, int h,VideoWriter video1,std::vector<int> images_to_output,cv::Mat TextMatrix,string path,cv::Rect roi
    ): filter(tbb::filter::mode::serial_in_order),w{w}, h{h}, video1{video1}, images_to_output{images_to_output}, TextMatrix{TextMatrix},path{path},roi{roi} {};
    void* operator()(void* ptr){
     // std::cout<<"receiving"+to_string(j)+" frame";
    auto src = static_cast<cv::Mat*>(ptr);
    Mat resized_frame;
    //resize frame
    cv::resize(*src, resized_frame, Size(w, h), 0, 0, INTER_NEAREST);
    resized_frame.convertTo(resized_frame,CV_8UC3);
    resized_frame= 255.0*resized_frame;
    
    // add text to frame
    cv::Mat destRoi = resized_frame(roi);
    TextMatrix.copyTo(destRoi, TextMatrix);

    //Inside this filter, we define 2 threads that work in parallel: one thread will be responsible
    //for video writer and the other thread will be responsible for image saving.
     
    //define a function for writer to video thread
    auto write_to_video = [](VideoWriter video1,Mat resized_frame) { 
      //std::cout<<"video";
        video1 << resized_frame; 
    };

    //define a function for save image thread
    auto save_frame = [](std::vector<int> images_to_output,string path,int j,Mat resized_frame) { 
     // std::cout<<"frame";
        auto it = std::find(images_to_output.begin(), images_to_output.end(), j);
        if (it == images_to_output.end())
        {
        }
        else{
                std::string file2 =path+"/frame"+to_string(j)+".png";

                imwrite(file2,resized_frame);
            
        }
    };

    std::thread thread_video(write_to_video, video1,resized_frame); 
    std::thread thread_saveimage(save_frame, images_to_output,path,j,resized_frame); 

    thread_video.join(); 
    thread_saveimage.join();


    /* std::cout<<"done";
    cout << "M = "<< endl << " "  << *src << endl << endl; */ 
    //sleep(0.5);
    
    j++;
    };
    };

    tbb::task_scheduler_init init;
    tbb::pipeline pipe;
    auto frame_worker = PrepareFrame{next_state,initial_state,number_of_rounds};
    auto video_worker = WriteToVideo{w,h,out,images_to_output,TextMatrix,path,roi};

    //add filters to the tbb pipeline
    pipe.add_filter(frame_worker);
    pipe.add_filter(video_worker);

    pipe.run(10); 
    out.release();




  

}
