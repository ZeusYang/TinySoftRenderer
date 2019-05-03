#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

namespace Ui {
class Window;
}

class QImage;
class QTimer;
class QThread;
class RenderLoop;
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
    void receiveFrame(unsigned char *image, const unsigned int &num_triangles,
                      const unsigned int &num_vertices);

    void fpsTimeOut();

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::Window *ui;
    bool firstMouseMove;
    QPoint preMousePos;
    QTimer *timer;
    QImage *canvas;
    int num_trangles, num_vertices;
    QThread *loopThread;
    RenderLoop *loop;
};

#endif // WINDOW_H
