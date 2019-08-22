#ifndef DROPITEM_H
#define DROPITEM_H
#include <QColor>
#include <QMap>

typedef int DropID;

class DropItem
{
public:
    DropItem(DropID, int, bool);
    static DropID getNewID();
    QColor color;

    int getPos() const;
    void move(int pos);
    void setVisible(bool);
    bool visible();
    bool isMark();
    virtual bool isLargeDrop()=0;
    virtual ~DropItem();

    static void initialize();
    static DropItem * getItem(DropID);

protected:
    static void registeItem(DropItem *);
    DropID id;

private:
    int pos;
    bool _visible;
    static DropID next_id;
    const static DropID start_id;

    static QMap<DropID, DropItem *> idToItem;
};

class DropMark;

class Drop: public DropItem
{
public:
    virtual bool isLargeDrop();
    static Drop * create(int pos);
    DropMark * getMark() const;
private:

    // forbid to call construct and desconstruct function from outside
    Drop(int pos, QColor color);
    virtual ~Drop();
    DropMark * mark;

};

class LargeDrop: public DropItem
{
public:
    static LargeDrop * create(int pos, DropItem * d0, DropItem * d1, DropItem * d2, bool merge);
    virtual bool isLargeDrop();
    int pos1, pos2;
    Drop * drop, * drop1, * drop2;
    bool toMerge() const;


private:
    // true if this large drop comes from merging
    // false if this large drop comes from split
    bool merge;

    // forbid to call construct and desconstruct function from outside
    LargeDrop(int pos, DropItem * d0, DropItem * d1, DropItem * d2, bool merge);
    virtual ~LargeDrop();
};

class DropMark: public DropItem
{
public:

    static DropMark * create(DropID id, QColor color);
    virtual bool isLargeDrop();

private:

    // forbid to call construct and desconstruct function from outside
    virtual ~DropMark();
    DropMark(DropID id, QColor color);
};

class CleanerDrop: public DropItem
{
public:
    static CleanerDrop * create(int pos);
    virtual bool isLargeDrop();
private:
    // forbid to call construct and desconstruct function from outside
    CleanerDrop(int pos);
    virtual ~CleanerDrop();
};

#endif // DROPITEM_H
