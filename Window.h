#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

namespace Ui {
class Window;
}

class QImage;
class RenderThread;
class Window : public QWidget
{
    Q_OBJECT

public:
    explicit Window(QWidget *parent = nullptr);
    ~Window();

private:
    void paintEvent(QPaintEvent *) override;

protected slots:
    void receiveFrame(unsigned char *image);

private:
    Ui::Window *ui;
    QImage *canvas;
    RenderThread *render;
};

#endif // WINDOW_H
