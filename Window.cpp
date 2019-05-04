#include "Window.h"
#include "ui_Window.h"

#include <QTimer>
#include <QThread>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

#include "RenderLoop.h"

Window::Window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Window),
    firstMouseMove(true)
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

    setMouseTracking(true);
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

void Window::receiveFrame(unsigned char *image, const unsigned int &num_triangles,
                          const unsigned int &num_vertices)
{
    if(canvas) delete canvas;
    canvas = new QImage(image, width(), height(), QImage::Format_RGBA8888);
    this->num_trangles = num_triangles;
    this->num_vertices = num_vertices;
    update();
}

void Window::fpsTimeOut()
{
    int fps = loop->getFps();
    loop->setFpsZero();
    this->setWindowTitle("Soft Renderer By YangWC "
                         + QString(" fps: %1 triangles: %2 vertices: %3")
                         .arg(fps).arg(num_trangles).arg(num_vertices));
}

void Window::mouseMoveEvent(QMouseEvent *event)
{
    //    if(!(event->buttons() & Qt::LeftButton))
    //        firstMouseMove = true;
    //    if(firstMouseMove)
    //    {
    //        firstMouseMove = false;
    //        preMousePos = event->pos();
    //    }
    //    else
    //    {
    //        QPoint delta = event->pos() - preMousePos;
    //        preMousePos = event->pos();
    //        loop->receiveMouseEvent(delta.x(), delta.y(), 'L');
    //    }
    if(!(event->buttons() & Qt::LeftButton || event->buttons() & Qt::RightButton))
        firstMouseMove = true;
    if(firstMouseMove)
    {
        firstMouseMove = false;
        preMousePos = event->pos();
    }
    else
    {
        QPoint delta = event->pos() - preMousePos;
        preMousePos = event->pos();
        if(event->buttons() & Qt::LeftButton)
            loop->receiveMouseEvent(delta.x(), delta.y(), "LEFT");
        else if(event->buttons() & Qt::RightButton)
            loop->receiveMouseEvent(delta.x(), delta.y(), "RIGHT");
    }

}

void Window::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_W:
        loop->receiveKeyEvent('W');break;
    case Qt::Key_S:
        loop->receiveKeyEvent('S');break;
    case Qt::Key_A:
        loop->receiveKeyEvent('A');break;
    case Qt::Key_D:
        loop->receiveKeyEvent('D');break;
    case Qt::Key_Q:
        loop->receiveKeyEvent('Q');break;
    case Qt::Key_E:
        loop->receiveKeyEvent('E');break;
    }
}

void Window::wheelEvent(QWheelEvent *event)
{
    double delta = event->delta();
    loop->receiveMouseWheelEvent(delta);
}
