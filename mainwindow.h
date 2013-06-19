#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv.hpp>

#include "colorhistogram.h"

#include <QTimer>
#include <QSlider>

using namespace cv;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void cameraTimerTimeout();
    void on_cameraButton_clicked();
    void on_edgeButton_clicked();
    void on_foregroundButton_clicked();
    void on_cannyThreshold_sliderMoved(int position);
    Mat cannyThreshold();

private:
    Ui::MainWindow *ui;
    VideoCapture capture;
    QTimer cameraTimer;
    bool isCameraRunning;
    bool edgeDetection;
    bool foregroundDetection;
    ColorHistogram ch;
    Mat image, src_gray;
    Mat dst, detected_edges;
    int lowThreshold;
    int ratio;
    int kernel_size;
};

#endif // MAINWINDOW_H
