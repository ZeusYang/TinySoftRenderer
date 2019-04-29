#include "Window.h"
#include "ui_Window.h"

#include <QPainter>
#include <QDebug>

#include "RenderThread.h"

Window::Window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Window)
{
    ui->setupUi(this);
    canvas = nullptr;
    render = new RenderThread(width(),height(),this);
    connect(render,&RenderThread::frameOut,this,&Window::receiveFrame);
    render->start();
}

Window::~Window()
{
    render->stopIt();
    delete ui;
    if(render)
    {
        render->quit();
        render->wait();
        delete render;
    }
    render = nullptr;
    if(canvas)
        delete canvas;
    canvas = nullptr;
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
    //qDebug() << "receiving....";
    if(image) delete canvas;
    canvas = new QImage(image, width(), height(), QImage::Format_RGBA8888);
    repaint();
}
