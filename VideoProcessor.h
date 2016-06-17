#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <QObject>
#include <QDateTime>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "SpatialFilter.h"

enum spatialFilterType {LAPLACIAN, GAUSSIAN};
enum temporalFilterType {IIR, IDEAL};

class VideoProcessor : public QObject {

    Q_OBJECT

    friend class MagnifyDialog;

public:

    explicit VideoProcessor(QObject *parent = 0);

    bool isStop();

    bool isModified();

    bool isOpened();

    void setDelay(int d);

    void setFileName(QString n);

    long getNumberOfProcessedFrames();

    long getNumberOfPlayedFrames();

    cv::VideoCapture getCapture();

    cv::Size getFrameSize();

    long getFrameNumber();

    double getPositionMS();

    double getFrameRate();

    long getLength();

    double getLengthMS();

    int getCodec(char codec[4]);

    bool setRelativePosition(double pos);

    bool setInput(const std::string &fileName);

    bool setOutput(const std::string &filename, int codec=0, double framerate=0.0, bool isColor=true);

    void setSpatialFilter(spatialFilterType type);

    void setTemporalFilter(temporalFilterType type);

    void playIt();

    void pauseIt();

    void stopIt();

    void prevFrame();

    void nextFrame();

    bool jumpTo(long index);

    void close();

    void motionMagnify();

    void writeOutput();

private slots:
    void revertVideo();

signals:
    void showFrame(cv::Mat frame);
    void revert();
    void sleep(int msecs);
    void updateBtn();
    void updateProgressBar();
    void reload(const std::string &);
    void updateProcessProgress(const std::string &message, int percent);
    void closeProgressDialog();

private:    
    cv::VideoCapture capture;
    int delay;
    double rate;
    long fnumber;
    long length;
    bool stop;
    bool modify;
    long curPos;
    int curIndex;
    int curLevel;
    int digits;    
    std::string extension;
    int levels;
    float alpha;
    float lambda_c;    
    float fl;
    float fh;
    float chromAttenuation;
    float delta;
    float exaggeration_factor;
    float lambda;
    QString fileName;
    cv::VideoWriter writer;
    cv::VideoWriter tempWriter;
    std::string outputFile;
    std::string tempFile;
    std::vector<std::string> tempFileList;
    std::vector<cv::Mat> lowpass1;
    std::vector<cv::Mat> lowpass2;

    bool getNextFrame(cv::Mat& frame);

    void writeNextFrame(cv::Mat& frame);

    bool createTemp(double framerate=0.0, bool isColor=true);

    bool spatialFilter(const cv::Mat &src, std::vector<cv::Mat> &pyramid);

    void temporalFilter(const cv::Mat &src, cv::Mat &dst);

    void temporalIIRFilter(const cv::Mat &src, cv::Mat &dst);

    void amplify(const cv::Mat &src, cv::Mat &dst);

    void attenuate(cv::Mat &src, cv::Mat &dst);
};

#endif // VIDEOPROCESSOR_H
