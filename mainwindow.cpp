#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //variable init
    isCameraRunning = false;
    edgeDetection = false;
    foregroundDetection = false;
    ratio = 3; // ratio of lower:upper threshold of 3:1
    kernel_size = 3; // for the Sobel operations to be performed internally by the Canny function
    lowThreshold = 1; // default minimum starting value for canny

    // UI init
    ui->cannyThreshold->setMinimum(1);
    ui->cannyThreshold->setMaximum(100);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_cameraButton_clicked()
{
    // the webcam is not yet started
    if(!isCameraRunning)
    {
        // open camera stream
        capture.open(CV_CAP_ANY); // default: 0

        if(!capture.isOpened())
            return;

        // set the acquired frame size to the size of its container
        capture.set(CV_CAP_PROP_FRAME_WIDTH, ui->video->size().width());
        capture.set(CV_CAP_PROP_FRAME_HEIGHT, ui->video->size().height());

        isCameraRunning = true;

        // start timer for acquiring the video
        cameraTimer.start(33); // 33 ms = 30 fps
        // at the timeout() event, execute the cameraTimerTimeout() method
        connect(&cameraTimer, SIGNAL(timeout()), this, SLOT(cameraTimerTimeout()));

        // update the user interface...
        ui->cameraButton->setText("Stop Camera");
        ui->edgeButton->setEnabled(true);
        ui->foregroundButton->setEnabled(true);
    }
    else
    {
        if(!capture.isOpened())
            return;

        // stop timer
        cameraTimer.stop();
        // release camera stream
        capture.release();

        isCameraRunning = false;

        // restore the user interface to the original status...
        ui->cameraButton->setText("Start Camera");
        ui->histLabel->setText("Hue Histogram");
        ui->edgeButton->setEnabled(false);
        ui->foregroundButton->setEnabled(false);
    }
}

void MainWindow::cameraTimerTimeout() {

    if (isCameraRunning && capture.isOpened())
    {

        QImage videoFrame, histFrame;

        // get the current frame from the video stream
        capture >> image;

        // show histogram (only Hue)
        Mat hist = ch.getHueHistogram(image);
        cvtColor(hist, hist, CV_BGR2RGB);
        histFrame = QImage((uchar*)hist.data, hist.cols, hist.rows, hist.step, QImage::Format_RGB888);
        ui->histLabel->setPixmap(QPixmap::fromImage(histFrame));

        // apply canny
        if (edgeDetection) {
            // grayscale conversion
            cvtColor(image, src_gray, CV_BGR2GRAY );
            image = cannyThreshold();
        }

        // foreground detection
        if (foregroundDetection) {

            // threshold the grayscale image with the histogram average value
            Mat hsv_img, thresholdImg;
            hsv_img.create(image.size(), CV_8U);
            cvtColor(image, hsv_img, CV_BGR2HSV);

            vector<Mat> hsv_planes;
            split(hsv_img, hsv_planes);

            float threshVal = ch.getAverage();

            if (ui->Bg_Fg_Invert->checkState()==Qt::Checked)
                threshold(hsv_planes[0], thresholdImg, threshVal, 179.0, THRESH_BINARY);
            else
                threshold(hsv_planes[0], thresholdImg, threshVal, 179.0, THRESH_BINARY_INV);

            blur(thresholdImg, thresholdImg, Size(5,5));


            // dilate to fill gaps, erode to smooth edges
            dilate(thresholdImg, thresholdImg, Mat(), Point(-1,1), 6);
            erode(thresholdImg, thresholdImg, Mat(), Point(-1,1), 6);

            threshold(thresholdImg, thresholdImg, threshVal, 179.0, THRESH_BINARY);

            // create the new image
            Mat foreground(image.size(), CV_8UC3, Scalar(255,255,255));
            image.copyTo(foreground, thresholdImg);

            cvtColor(foreground, image, CV_BGR2RGB);

        }
        else
        {
            cvtColor(image, image, CV_BGR2RGB);
        }

        videoFrame = QImage((uchar*)image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);

        // show the frame
        ui->video->setPixmap(QPixmap::fromImage(videoFrame));

    }
}

void MainWindow::on_edgeButton_clicked()
{
    // flip edge detection status
    edgeDetection = !edgeDetection;

    // update the UI
    if (edgeDetection)
    {
        ui->edgeButton->setText("Edges: ON");
        ui->cannyThreshold->setEnabled(true);
        ui->foregroundButton->setEnabled(false);
    }
    else
    {
        ui->edgeButton->setText("Edges: OFF");
        ui->cannyThreshold->setEnabled(false);
        ui->foregroundButton->setEnabled(true);
    }
}

void MainWindow::on_foregroundButton_clicked()
{
    // flip foreground detection status
    foregroundDetection = !foregroundDetection;

    // update the UI
    if (foregroundDetection)
    {
        ui->foregroundButton->setText("Foreground: ON");
        ui->Bg_Fg_Invert->setEnabled(true);
        ui->edgeButton->setEnabled(false);
    }
    else
    {
        ui->foregroundButton->setText("Foreground: OFF");
        ui->Bg_Fg_Invert->setEnabled(false);
        ui->edgeButton->setEnabled(true);
    }

}

void MainWindow::on_cannyThreshold_sliderMoved(int position)
{
    // set the canny threshold value
    lowThreshold = position;
}

Mat MainWindow::cannyThreshold()
{
    // reduce noise with a kernel 3x3
    blur(src_gray, detected_edges, Size(3,3));

    // canny detector
    Canny(detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size);

    // using Canny's output as a mask, we display our result
    dst = Scalar::all(0);

    image.copyTo(dst, detected_edges);

    return dst;
}
