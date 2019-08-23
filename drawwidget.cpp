#include "drawwidget.h"
#include "ui_drawwidget.h"
#include "dmfb.h"
#include <QDebug>

const QColor DrawWidget::background = QColor(255, 255, 255);

DrawWidget::DrawWidget(QWidget *parent) :
    QWidget(parent),
    manager(nullptr),
    drawPollution(false)
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

    updateBackground();

}

void DrawWidget::updateBackground()
{
    backimage = QPixmap(size());
    QPainter p(&backimage);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setBrush(background);
    p.drawRect(0, 0, this->width() - 1, this->height()- 1);
    p.translate(edge_x + len_x, edge_y + len_y);

    auto row = manager->row;
    auto col = manager->col;
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
        drawText(p, x, y, "Input", QColor("red"));
    }
    // draw output
    {
        int output = manager->output;
        auto pos = getDrawPos(output % col, output / col, row, col);
        drawText(p, pos.first, pos.second, "Output", QColor("blue"));
    }
    // draw cleaner
    if(manager->cleaner)
    {
        auto wash = getDrawPos(0, 0, row, col);
        auto waste = getDrawPos(col - 1, row - 1, row, col);
        drawText(p, wash.first, wash.second, "Wash", QColor("cyan"));
        drawText(p, waste.first, waste.second, "Waste", QColor("green"));
        // draw obstacle
        p.setPen(QPen(QColor("indianred"), 3));
        for (int pos = 0; pos < row * col; pos++)
        {
            if(manager->getObstacle(pos))
            {
                auto x = pos % col;
                auto y = pos / col;
                p.drawLine(x * len_x, y * len_y, (x + 1) * len_x, (y + 1) * len_y);
                p.drawLine((x + 1) * len_x, y * len_y, x * len_x, (y + 1) * len_y);
            }
        }
    }

    p.end();
    updateImage();
}

inline double getScale(const QPainter & p, const QRect & r, const QString & str)
{
    auto size = p.fontMetrics().size(Qt::TextSingleLine, str);
    auto scals_x = double(r.width()) / size.width();
    auto scals_y = double(r.height()) / size.height();
    return scals_x > scals_y ? scals_y : scals_x;
}

void DrawWidget::drawText(QPainter &p, int x, int y, QString s, const QBrush & brush)
{
    p.save();
    QRect r(x * len_x, y * len_y, len_x, len_y);
    if(brush.style() != Qt::NoBrush)
    {
        p.setBrush(brush);
        p.drawRect(r);
    }

    auto font = p.font();
    font.setPointSizeF(getScale(p, r, s) * 0.8 * font.pointSizeF());
    font.setFamily("Consolas");
    p.setFont(font);
    p.drawText(r, Qt::AlignCenter, s);
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

void DrawWidget::mousePressEvent(QMouseEvent * e)
{
    auto _pos = e->pos();
    auto col = manager->col;
    auto row = manager->row;
    auto x = (_pos.x() - edge_x) / len_x - 1;
    auto y = (_pos.y() - edge_y) / len_y - 1;
    auto pos = y * col + x;
    if(manager->cleaner && 0 <= pos && pos < row * col)
    {
        // click some rect
        manager->setObstacle(pos);
        updateBackground();

    }
    QWidget::mousePressEvent(e);
}

void DrawWidget::updateImage()
{
    if(!manager)
        return;
    image = backimage.copy();
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.translate(edge_x + len_x, edge_y + len_y);
    auto col = manager->col;
    auto row = manager->row;

    if(drawPollution)
    {
        p.setPen(QColor("black"));
        for(int pos = 0; pos < col * row; pos++)
        {
            auto s = QString("%1").arg(manager->pollute[pos].size());
            drawText(p, pos % col, pos / col, s, Qt::NoBrush);
        }
    }
    else
    {
        // else draw drops
        p.setPen(Qt::NoPen);
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
                else if(drop->isMark())
                {
                    p.save();
                    p.setBrush(Qt::NoBrush);
                    p.setPen(QPen(drop->color, 3));
                    r = QRect(pos % col * len_x + len_x / 4,pos / col * len_y + len_y / 4, len_x / 2, len_y / 2);
                    p.drawEllipse(r);
                    p.restore();
                }
                else
                {
                    if(drop->visible())
                        p.drawEllipse(r);

                }
            }
        }
    }

    update();
    return;
}
