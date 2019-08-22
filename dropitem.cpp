#include "dropitem.h"
#include <QDebug>

/*******************DropItem Class***************************/

// initalize static variable
const DropID DropItem::start_id = 1000;

DropID DropItem::next_id = DropItem::start_id;

QMap<DropID, DropItem *> DropItem::idToItem;

DropItem::DropItem(DropID id, int pos=-1, bool visible=true):id(id), pos(pos), _visible(visible)
{

}

DropItem::~DropItem()
{

}

int DropItem::getPos() const
{
    return pos;
}

void DropItem::move(int pos)
{
    this->pos = pos;
}

void DropItem::setVisible(bool v)
{
    _visible = v;
}

bool DropItem::isMark()
{
    return id < 0;
}

bool DropItem::visible()
{
    return _visible;
}

DropID DropItem::getNewID()
{
    return next_id++;
}

void DropItem::initialize()
{
    for(auto p: idToItem)
    {
        delete p;
        p = nullptr;
    }
    idToItem.clear();
    next_id = start_id;
}

void DropItem::registeItem(DropItem * item)
{
    if(idToItem.contains(item->id))
        qDebug()<<"warning: id "<<item->id<<" has already existed!";
    idToItem[item->id] = item;
}

/*******************Drop Class***************************/

Drop::Drop(int pos, QColor color):DropItem (getNewID(), pos), mark(DropMark::create(id, color))
{
    this->color = color;
}

Drop::~Drop()
{

}

Drop * Drop::create(int pos)
{
    auto color = QColor(rand()%256, rand()%256, rand() % 256);
    auto drop = new Drop(pos, color);
    registeItem(drop);
    return drop;
}

DropMark * Drop::getMark() const
{
    return mark;
}

bool Drop::isLargeDrop()
{
    return  false;
}

/*******************LargeDrop Class***************************/

LargeDrop::LargeDrop(int pos, DropItem * d0, DropItem * d1, DropItem * d2, bool merge=true): DropItem (getNewID(), pos), merge(merge)
{
    drop = static_cast<Drop *>(d0);
    if(d1->getPos() > d2->getPos())
    {
        auto tem = d1;
        d1 = d2;
        d2 = tem;
    }
    // drop1 is at left || up of drop2
    drop1 = static_cast<Drop *>(d1);
    drop2 = static_cast<Drop *>(d2);
    pos1 = drop1->getPos();
    pos2 = drop2->getPos();

    color = QColor(rand()%256, rand()%256, rand() % 256);
}

LargeDrop * LargeDrop::create(int pos, DropItem *d0, DropItem *d1, DropItem *d2, bool merge)
{
    auto large = new LargeDrop(pos, d0, d1, d2, merge);
    registeItem(large);
    return large;
}

bool LargeDrop::toMerge() const
{
    return merge;
}

LargeDrop::~LargeDrop()
{

}

bool LargeDrop::isLargeDrop()
{
    return true;
}

/*******************DropMark Class***************************/

DropMark::DropMark(DropID id, QColor color): DropItem (id, -1)
{
    this->color = color;
}

DropMark * DropMark::create(DropID id, QColor color)
{
    auto mark = new DropMark(-id, color);
    registeItem(mark);
    return mark;
}

DropMark::~DropMark()
{

}

bool DropMark::isLargeDrop()
{
    return false;
}

/*******************CleanerDrop Class***************************/

CleanerDrop::CleanerDrop(int pos): DropItem (getNewID(), pos)
{

}

CleanerDrop * CleanerDrop::create(int pos)
{
    auto cleaner = new CleanerDrop(pos);
    registeItem(cleaner);
    return cleaner;
}

CleanerDrop::~CleanerDrop()
{

}

bool CleanerDrop::isLargeDrop()
{
    return false;
}
