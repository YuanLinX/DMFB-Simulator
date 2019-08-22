#include "dmfb.h"
#include <QDebug>
#include "functions.h"

DMFB::DMFB(): row(3), col(3), inputs({3}), output(5)
{
    nameToType["input"] = INPUT;
    nameToType["output"] = OUTPUT;
    nameToType["move"] = MOVE;
    nameToType["mix"] = MIX;
    nameToType["merge"] = MERGE;
    nameToType["split"] = SPLIT;
    initalize();
}

DMFB::~DMFB()
{
    clearInstrcutions();
}

void DMFB::initalize()
{
    t = 0;
    lastInstructionT = -1;
    clear();

    drops.resize(col * row);
    instructions.resize(200);

}

void DMFB::clear()
{
    errorInfo.clear();
    MergeSave.clear();
    SplitSave.clear();
    save.clear();
    pollute.clear();
    drops.clear();
    clearInstrcutions();
    clearDrops();
}

void DMFB::clearInstrcutions()
{
    for(auto & list: instructions)
    {
        for(auto p: list)
        {
            delete p;
        }
    }
    instructions.clear();
}

void DMFB::clearDrops()
{
    DropItem::initialize();
    drops.clear();
    allDrops.clear();
}

auto DMFB::getAllInstructions()
{
    return instructions;
}

auto DMFB::getInstructionList(int t)
{
    return instructions[t];
}

inline int max(int a, int b)
{
    return a < b? b : a;
}

void DMFB::addInstruction(QString s)
{
    auto pair = s.split(" ");
    if(pair.length()!=2)
        return;
    auto type = nameToType[pair.first().toLower()];
    auto string = pair.last().split(",");
    QVector<int> data;
    for (auto str: string)
    {
        // modify number to center(0,0)
        data.append(str.toInt() - 1);
    }
    // t shouldn't be modified
    auto _t = ++data[0];
    Instruction * p = nullptr;
    switch(type)
    {
    case INPUT:
        p = new InputData(data);
        break;
    case OUTPUT:
        p = new OutputData(data);
        break;
    case MERGE:
        p = new MergeData(data);
        lastInstructionT = max(_t + 1, lastInstructionT);
        break;
    case SPLIT:
        p = new SplitData(data);
        lastInstructionT = max(_t + 1, lastInstructionT);
        break;
    case MOVE:
        p = new MoveData(data);
        break;
    case MIX:
        p = new MixData(data);
        auto list = static_cast<MixData *>(p)->getMoveInstruction();
        for (auto ins:list)
        {
            instructions[ins.t].append(new MoveData(ins));
        }
        lastInstructionT = max(_t + list.length() - 1, lastInstructionT);
        return;
    }
    if(instructions.length() < _t + 1)
        instructions.resize(_t * 2);
    instructions[_t].append(p);
    lastInstructionT = max(_t, lastInstructionT);
}

bool DMFB::next()
{
    flag = {};
    if(t >= instructions.length())
        return false;
    if(t)
        drops = allDrops.last();
    if(SplitSave[t].length())
    {
        flag.split2 = true;
        for(auto large: SplitSave[t])
        {
            auto pos = large->getPos();
            large->drop1->setVisible(true);
            large->drop2->setVisible(true);
            drops[t][pos] = nullptr;
        }
        SplitSave.clear();
    }
    if(MergeSave[t].length())
    {
        flag.merge2 = true;
        for(auto large: MergeSave[t])
        {
            auto pos = large->getPos();
            auto pos1 = large->pos1;
            auto pos2 = large->pos2;
            drops[t][pos] = large->drop;
            drops[t][pos1] = nullptr;
            drops[t][pos2] = nullptr;
            large->drop->setVisible(true);
        }
        MergeSave.clear();
    }

    for(auto ins: instructions[t])
    {
        executeInstruction(ins);
    }

    if(errorInfo.length())
    {
        return false;
    }
    allDrops.append(drops);
    t++;
    return true;
}

bool DMFB::last()
{
    if(t)
    {
        qDebug()<<"error!";
        return false;
    }
    t--;
    loadState(t);
    return true;
}

void DMFB::loadState(int target)
{
    // load save
    save[target];
    save.pop_back();
    return;
}

int DMFB::getT() const
{
    return t;
}

DropItem * DMFB::getDrop(int pos)
{
    assert(pos >= 0 && pos < col * row);
    return drops[t][pos];
}

int DMFB::getLastT() const
{
    return lastInstructionT;
}

const DMFB::actionFlag & DMFB::getFlag()
{
    return flag;
}

bool DMFB::over() const
{
    return t > lastInstructionT;
}

bool DMFB::executeInstruction(Instruction * ins)
{
    switch (ins->type) {
    case INPUT:
    {
        auto p = static_cast<InputData *>(ins);
        auto x = p->x1;
        auto y = p->y1;
        auto pos = y * col + x;
        bool found = false;
        for(auto input: inputs)
        {
            if(input == pos)
                found = true;
        }
        if(!found)
        {
            errorInfo.append(QString("wrong input postion (%1, %2)").arg(x + 1).arg(y + 1));
            return false;
        }
        auto drop = Drop::create(pos);
        drops[t][pos] = drop;
        flag.input = true;
    }
    break;
    case OUTPUT:
    {
        auto p = static_cast<OutputData *>(ins);
        auto x = p->x1;
        auto y = p->y1;
        auto pos = y * col + x;
        if(pos != output)
        {
            errorInfo.append(QString("wrong output postion (%1, %2)").arg(x + 1).arg(y + 1));
            return false;
        }
        drops[t][pos] = nullptr;
        flag.output = true;
        break;
    }
    case MERGE:
    {
        auto p = static_cast<MergeData *>(ins);
        auto x1 = p->x1;
        auto y1 = p->y1;
        auto x2 = p->x2;
        auto y2 = p->y2;
        if((x1 == x2 && abs(y1 - y2) == 2) || (y1 == y2 && abs(x1 - x2) == 2))
        {
            auto pos1 = y1 * col + x1;
            auto pos2 = y2 * col + x2;
            if(!drops[t - 1][pos1])
            {
                errorInfo.append(QString("no drop at (%1,%2)").arg(x1).arg(y1));
                return false;
            }
            if(!drops[t - 1][pos2])
            {
                errorInfo.append(QString("no drop at (%1,%2)").arg(x2).arg(y2));
                return false;
            }
            auto y = (y1 + y2) >> 1;
            auto x = (x1 + x2) >> 1;
            auto pos = y * col + x;
            auto merged = Drop::create(pos);
            auto large = LargeDrop::create(pos
                                           , merged, drops[t][pos1], drops[t][pos2], true);
            merged->setVisible(false);
            drops[t][pos1]->setVisible(false);
            drops[t][pos2]->setVisible(false);
            drops[t][pos] = large;
            MergeSave[t].append(large);
            flag.merge1 = true;
        }
        else
        {
            errorInfo.append(QString("can't merge drops at (%1,%2) and (%3,%4)").arg(
                                                                                    x1 + 1).arg(y1 + 1).arg(x2 + 1).arg(y2 + 1));
            return false;
        }
        break;
    }
    case SPLIT:
    {
        auto p = static_cast<SplitData *>(ins);
        // notice that (x1, y1) is target
        auto x1 = p->x1;
        auto y1 = p->y1;
        auto x2 = p->x2;
        auto y2 = p->y2;
        auto x3 = p->x3;
        auto y3 = p->y3;
        if(checkAdjoin(x1, y1, x2, y2) && checkAdjoin(x1, y1, x3, y3) &&
            x1 << 1 == x2 + x3 && y1 << 1 == y2 + y3)
        {
            auto pos = y1 * col + x1;
            auto pos1 = y2 * col + x2;
            auto pos2 = y3 * col + x3;
            if(!drops[y1][x1])
            {
                errorInfo.append(QString("no drop at (%1,%2)").arg(x1).arg(y1));
                return false;
            }
            auto drop = drops[y1][x1];
            auto drop1 = Drop::create(pos1);
            auto drop2 = Drop::create(pos2);
            auto large = LargeDrop::create(pos, drop, drop1, drop2, false);
            drop->setVisible(false);
            drop1->setVisible(false);
            drop2->setVisible(false);
            drops[y1][x1] = large;
            drops[y2][x2] = drop1;
            drops[y3][x3] = drop2;
            SplitSave.append(large);
            flag.split1 = true;
        }
        else
        {
            errorInfo.append(QString("can't split (%1, %2) to (%3, %4) and (%5, %6)").arg(
                                                                                         x1 + 1).arg(y1 + 1).arg(x2 + 1).arg(y2 + 1).arg(x3 + 1).arg(y3 + 1));
            return false;
        }
        break;
    }
    case MOVE:
    {
        auto p = static_cast<MoveData *>(ins);
        auto x1 = p->x1;
        auto y1 = p->y1;
        auto x2 = p->x2;
        auto y2 = p->y2;
        if(!checkAdjoin(x1, y1, x2, y2))
        {
            errorInfo.append(QString("can't move from (%1, %2) to (%3, %4)").arg(
                                                                                x1 + 1).arg(y1 + 1).arg(x2 + 1).arg(y2 + 1));
            return false;
        }
        drops[y2][x2] = drops[y1][x1];
        drops[y2][x2]->move(y2 * col + x2);
        drops[y1][x1] = nullptr;
        flag.move = true;
        break;
    }
    case MIX:
    {
        qDebug()<<"error, MIX instruction should split into moves!";
        assert(0);
    }
    }
    return true;
}
