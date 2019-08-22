#ifndef DRAWWIDGET_H
#define DRAWWIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include "dmfb.h"
#include <QResizeEvent>
#include <QPixmap>
#include <QColor>



namespace Ui {
class DrawWidget;
}

class DrawWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DrawWidget(QWidget *parent = nullptr);
    ~DrawWidget();
    void paintEvent(QPaintEvent *);
    DMFB * manager;
    QSize getPreferredSize();
    void updateSize(QSize=QSize());
    void updateImage();
    void resizeEvent(QResizeEvent *);

private:
    int edge_x;
    int edge_y;
    int len_x;
    int len_y;
    void drawInput(QPainter &, int x, int y);
    void drawOutput(QPainter &, int x, int y);
    auto getDrawPos(int x, int y, int row, int col);
    QPixmap backimage;
    QPixmap image;
    static const QColor background;

};

#endif // DRAWWIDGET_H
