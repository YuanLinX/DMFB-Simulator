#include "drawwidget.h"
#include "ui_drawwidget.h"
#include "dmfb.h"
#include <QDebug>

const QColor DrawWidget::background = QColor(255, 255, 255);

DrawWidget::DrawWidget(QWidget *parent) :
    QWidget(parent),
    manager(nullptr)
{

}

DrawWidget::~DrawWidget()
{
}

auto DrawWidget::getDrawPos(int x, int y, int row, int col)
{
    if(!x)
    {
        x--;
    }
    else if(x + 1 == col)
    {
        x++;
    }
    else if(!y)
    {
        y--;
    }
    else if(y + 1 == row)
    {
        y++;
    }
    else
    {
        qDebug() << "error input position";
    }
    return QPair<int, int>(x, y);
}

void DrawWidget::updateSize(QSize size)
{
    auto current = this->size();
    if(size == QSize())
        size = this->size();
    else
    {
        this->resize(size);
        // resize failed
        if(this->size() == current && current != size)
            return;
    }
    auto row = manager->row + 2;
    auto col = manager->col + 2;
    auto height = size.height();
    auto width = size.width();
    edge_x = width / 10;
    edge_y = height / 10;
    len_x = (width - edge_x * 2) / col;
    len_y = (height - edge_y * 2) / row;
    backimage = QPixmap(width, height);
    QPainter p(&backimage);
    p.setBrush(background);
    p.drawRect(0, 0, this->width() - 1, this->height()- 1);
    p.translate(edge_x + len_x, edge_y + len_y);

    row -= 2;
    col -= 2;
    // draw grid
    for(int r = 0;r <= row; r++)
    {
        p.drawLine(0, len_y * r, len_x * col, len_y * r);
    }
    for (int c = 0;c <= col; c++)
    {
        p.drawLine(len_x * c, 0, len_x * c, len_y * row);
    }
    // draw inputs
    for(auto input:manager->inputs)
    {
        int x= input % col;
        int y = input / col;
        auto pos = getDrawPos(x, y, row, col);
        x = pos.first;
        y = pos.second;
        drawInput(p, x, y);
    }
    // draw output
    {
        int output = manager->output;
        auto pos = getDrawPos(output % col, output / col, row, col);
        drawOutput(p, pos.first, pos.second);
    }
    p.end();

    updateImage();
}

void DrawWidget::drawInput(QPainter & p, int x, int y)
{
    p.save();
    p.setBrush(QColor("red"));
    QRect r(x * len_x, y * len_y, len_x, len_y);
    p.drawRect(r);

    auto font = p.font();
    auto a = p.fontMetrics().size(Qt::TextSingleLine, "Input").width();
    qreal b = qreal(r.width()) / a * 0.9;
    font.setPointSizeF(b * font.pointSizeF());
    font.setFamily("Consolas");
    p.setFont(font);
    p.drawText(r, Qt::AlignCenter, "Input");
    p.restore();
}

void DrawWidget::drawOutput(QPainter & p, int x, int y)
{
    p.save();
    p.setBrush(QColor("blue"));
    QString str = "Output";
    QRect r(x * len_x, y * len_y, len_x, len_y);
    p.drawRect(r);

    auto font = p.font();
    auto a = p.fontMetrics().size(Qt::TextSingleLine, "Output").width();
    qreal b = qreal(r.width()) / a * 0.9;
    font.setPointSizeF(b * font.pointSizeF());
    font.setFamily("Consolas");
    p.setFont(font);
    p.drawText(r, Qt::AlignCenter, "Output");
    p.restore();
}

QSize DrawWidget::getPreferredSize()
{
    const int edge_x = 50;
    const int edge_y = 50;
    const int len_x = 50;
    const int len_y = 50;
    return QSize(edge_x * 2 + len_x * manager->col,
                 edge_y * 2 + len_y * manager->row);
}

void DrawWidget::resizeEvent(QResizeEvent * e)
{
    updateSize();
    QWidget::resizeEvent(e);
}

void DrawWidget::paintEvent(QPaintEvent * e)
{
    if(!manager)
        return;
    QPainter p(this);
    p.drawPixmap(0, 0, image);

    QWidget::paintEvent(e);
}

void DrawWidget::updateImage()
{
    if(!manager)
        return;
    image = backimage.copy();
    QPainter p(&image);
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.translate(edge_x + len_x, edge_y + len_y);
    auto col = manager->col;
    auto row = manager->row;


    for(int pos = 0; pos < col * row; pos++)
    {
        auto drop = manager->getDrop(pos);
        QRect r(pos % col * len_x, pos / col * len_y, len_x, len_y);
        if(drop)
        {
            p.setBrush(drop->color);
            if(drop->isLargeDrop())
            {
                auto large = static_cast<LargeDrop*>(drop);
                auto pos1 = large->pos1;
                auto pos2 = large->pos2;
                // judge it's horizontal or vertical
                if(pos2 - pos1 == 2)
                {
                    // horizontal
                    r = QRect(pos1 % col * len_x, pos1 / col * len_y, len_x * 3, len_y);
                    p.drawEllipse(r);
                }
                else
                {
                    // vertical
                    r = QRect(pos1 % col * len_x, pos1 / col * len_y, len_x, len_y * 3);
                    p.drawEllipse(r);
                }
            }
            else
            {
                if(drop->visible())
                    p.drawEllipse(r);

            }
        }
    }
    update();
    return;
}
