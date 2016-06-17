#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QDateTime>
#include <QTime>
#include <QFileDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QProgressDialog>
#include <QLabel>
#include <queue>
#include "VideoProcessor.h"
#include "MagnifyDialog.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class VideoProcessor;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();    

    // File functions
	bool maybeSave();   // whether needs save
    bool save();        // save
    void play();        // save
    bool saveAs();      // save as
    bool saveFile(const QString &fileName);     // save file
    bool LoadFile(const QString &fileName);    // load file

private slots:
    void on_actionOpen_triggered();

    void on_actionQuit_triggered();

    void on_actionClose_triggered();

    void on_actionSave_as_triggered();

    void on_btnPlay_clicked();

    void on_btnStop_clicked();

    void on_btnPause_clicked();

    void on_btnNext_clicked();

    void on_btnLast_clicked();  

    void showFrame(cv::Mat frame);

    void revert();

    void sleep(int msecs);

    void updateBtn();

    void updateProgressBar();

    void updateProcessProgress(const std::string &message, int value);

    void on_progressSlider_sliderMoved(int position);

    void closeProgressDialog();

    void on_motion_triggered();

protected:
    void closeEvent(QCloseEvent *);
    
private:
    Ui::MainWindow *ui;

    QProgressDialog *progressDialog;

    MagnifyDialog *magnifyDialog;

    void updateStatus(bool vi);
    void updateTimeLabel();

    //ko ni videa
    QString inputTip;

    //FPS.je kaze
    QLabel *rateLabel;

    //lokacija videa
    QString curFile;

    VideoProcessor *video;

};

#endif // MAINWINDOW_H
