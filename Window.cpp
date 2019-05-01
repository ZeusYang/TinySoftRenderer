#include "Window.h"
#include "ui_Window.h"

#include <QTimer>
#include <QThread>
#include <QPainter>
#include <QDebug>

#include "RenderLoop.h"

Window::Window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Window)
{
    ui->setupUi(this);
    canvas = nullptr;
    loop = new RenderLoop(width(), height(), nullptr);
    loopThread = new QThread(this);

    // fps counting.
    timer = new QTimer();
    connect(timer,&QTimer::timeout,this,&Window::fpsTimeOut);

    // render thread.
    loop->moveToThread(loopThread);
    connect(loopThread,&QThread::finished,loop, &RenderLoop::deleteLater);
    connect(loopThread,&QThread::started,loop,&RenderLoop::loop);
    connect(loop,&RenderLoop::frameOut,this,&Window::receiveFrame);

    // begin the thread.
    loopThread->start();
    timer->start(1000);
}

Window::~Window()
{
    delete ui;
    loop->stopIt();
    loopThread->quit();
    loopThread->wait();
    //if(loop)delete loop;
    if(canvas)delete canvas;
    if(loopThread)delete loopThread;
    loop = nullptr;
    canvas = nullptr;
    loopThread = nullptr;
}

void Window::paintEvent(QPaintEvent *event)
{
    if(canvas)
    {
        QPainter painter(this);
        painter.drawImage(0, 0, *canvas);
    }
    QWidget::paintEvent(event);
}

void Window::receiveFrame(unsigned char *image)
{
    if(canvas) delete canvas;
    canvas = new QImage(image, width(), height(), QImage::Format_RGBA8888);
    repaint();
}

void Window::fpsTimeOut()
{
    int fps = loop->getFps();
    loop->setFpsZero();
    this->setWindowTitle("Soft Renderer By YangWC " + QString(" fps: %1").arg(fps));
}
