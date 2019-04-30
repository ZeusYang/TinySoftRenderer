#include "Window.h"
#include "ui_Window.h"

#include <QPainter>
#include <QDebug>

#include "RenderLoop.h"
#include "RenderThread.h"

Window::Window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Window)
{
    ui->setupUi(this);
    canvas = nullptr;
    loop = new RenderLoop(width(), height(), nullptr);
    loopThread = new QThread(this);
    loop->moveToThread(loopThread);
    connect(loopThread,&QThread::finished,loop, &RenderLoop::deleteLater);
    connect(loopThread,&QThread::started,loop,&RenderLoop::loop);
    connect(loop,&RenderLoop::frameOut,this,&Window::receiveFrame);
    loopThread->start();
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
