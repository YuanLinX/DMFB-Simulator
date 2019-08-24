#include "dmfb.h"
#include <QDebug>
#include "functions.h"
#include <QQueue>

DMFB::DMFB(): row(3), col(3), inputs({3}), output(5), cleaner(false),
    washing(false), washer(nullptr), wastePos(-1)
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

void DMFB::initalize(bool reload)
{
    t = 0;
    clear();
    if(reload)
    {
        clearInstrcutions();
        lastInstructionT = -1;
    }
    lastT = lastInstructionT;
    pollute.resize(col * row);
    drops.resize(col * row);
    instructions.resize(max_t);
    memset(mask, 0, sizeof(mask));
    memset(last_mask, 0, sizeof(last_mask));
    if(cleaner)
    {
        obstacle.resize(col * row);
        obstacle.fill(false);
        wash_map.resize(col * row);
        wastePos = row * col - 1;
        distance.resize(col * row);
        comeFrom.resize(col * row);
    }
}

void DMFB::clear()
{
    errorInfo.clear();
    MergeSave.clear();
    SplitSave.clear();
    pollute.clear();
    drops.clear();
    obstacle.clear();
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

void DMFB::setObstacle(int pos)
{
    obstacle[pos] = !obstacle[pos];
}

bool DMFB::getObstacle(int pos)
{
    return obstacle[pos];
}

bool DMFB::isWashing()
{
    return washing;
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
    lastT = lastInstructionT;
}

bool DMFB::next()
{
    flag = {};
    if(t >= instructions.length())
        assert(0);
    if(t < allDrops.length())
    {
        return loadState(t + 1);
    }
    if(SplitSave[t].length())
    {
        flag.split2 = true;
        for(auto large: SplitSave[t])
        {
            auto pos = large->getPos();
            large->drop1->setVisible(true);
            large->drop2->setVisible(true);
            drops[pos] = large->drop->getMark();;
        }
    }
    if(MergeSave[t].length())
    {
        flag.merge2 = true;
        for(auto large: MergeSave[t])
        {
            auto pos = large->getPos();
            auto pos1 = large->pos1;
            auto pos2 = large->pos2;
            drops[pos] = large->drop;
            drops[pos1] = static_cast<Drop *>(drops[pos1])->getMark();
            drops[pos2] = static_cast<Drop *>(drops[pos2])->getMark();
            large->drop->setVisible(true);
        }
    }

    for(auto ins: instructions[t])
    {
        // error occurs
        if(!executeInstruction(ins))
        {
            // set this t as final
            lastT = t;
            break;
        }
    }

    allDrops.append(drops);
    t++;
    check();
    if(errorInfo.length())
    {
        lastT = t - 1;
        return false;
    }
    return true;
}

bool DMFB::last()
{
    if(!t)
    {
        qDebug()<<"error!";
        return false;
    }
    loadState(t - 1);
    return true;
}

bool DMFB::loadState(int target)
{
    assert("target beyond range" && target <= allDrops.length());
    t = target;
    if(target==0)
    {
        reset();
        return true;
    }
    drops = allDrops[target - 1];
    // initialize all drops visible
    for(auto drop:drops)
    {
        if(drop)
            drop->setVisible(true);
    }
    // then set invisible
    for(auto drop:drops)
    {
        if(drop && drop->isLargeDrop())
        {
            auto large = static_cast<LargeDrop *>(drop);
            large->drop1->setVisible(false);
            large->drop2->setVisible(false);
        }
    }
    // get sound flag
    flag = {};
    for(auto & ins: instructions[target - 1])
    {
        switch (ins->type)
        {
        case INPUT:
            flag.input = true;
            break;
        case OUTPUT:
            flag.output = true;
            break;
        case MERGE:
            flag.merge1 = true;
            break;
        case SPLIT:
            flag.split1 = true;
            break;
        case MOVE:
            flag.move = true;
            break;
        case MIX:
            assert(0);
            break;
        }
    }
    flag.split2 = bool(SplitSave[target - 1].length());
    flag.merge2 = bool(MergeSave[target - 1].length());

    if(t > lastT && errorInfo.length())
        return false;

    return true;
}

int DMFB::getT() const
{
    return t;
}

DropItem * DMFB::getDrop(int pos)
{
    assert(pos >= 0 && pos < col * row);
    return drops[pos];
}

int DMFB::getLastT() const
{
    return lastT;
}

const DMFB::actionFlag & DMFB::getFlag()
{
    return flag;
}

bool DMFB::over() const
{
    return t > lastT;
}

void DMFB::reset()
{
    t = 0;
    drops.clear();
    drops.resize(col * row);
    MergeSave.resize(lastInstructionT + 1);
    SplitSave.resize(lastInstructionT + 1);
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
        if(cleaner && drops[pos] && drops[pos]->isMark())
        {
            errorInfo.append(QString("input at (%1, %2) get polluted!").arg(x + 1).arg(y + 1));
            return false;
        }
        drops[pos] = drop;
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
        drops[pos] = static_cast<Drop *>(drops[pos])->getMark();
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
            if(!drops[pos1])
            {
                errorInfo.append(QString("no drop at (%1,%2)").arg(x1).arg(y1));
                return false;
            }
            if(!drops[pos2])
            {
                errorInfo.append(QString("no drop at (%1,%2)").arg(x2).arg(y2));
                return false;
            }
            auto y = (y1 + y2) >> 1;
            auto x = (x1 + x2) >> 1;
            auto pos = y * col + x;
            auto merged = Drop::create(pos);
            auto large = LargeDrop::create(pos
                                           , merged, drops[pos1], drops[pos2], true);
            merged->setVisible(false);
            drops[pos1]->setVisible(false);
            drops[pos2]->setVisible(false);

            if(cleaner && drops[pos] && drops[pos]->isMark())
            {
                errorInfo.append(QString("Merge at (%1, %2) get polluted!").arg(x + 1).arg(y + 1));
                return false;
            }

            drops[pos] = large;
            MergeSave[t + 1].append(large);
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
            auto drop = drops[pos];

            if(!drop)
            {
                errorInfo.append(QString("no drop at (%1,%2)").arg(x1).arg(y1));
                return false;
            }

            auto drop1 = Drop::create(pos1);
            auto drop2 = Drop::create(pos2);
            auto large = LargeDrop::create(pos, drop, drop1, drop2, false);
            drop->setVisible(false);
            drop1->setVisible(false);
            drop2->setVisible(false);

            if(cleaner && drops[pos1] && drops[pos1]->isMark())
            {
                errorInfo.append(QString("split at (%1, %2) get polluted!").arg(x2 + 1).arg(y2 + 1));
                return false;
            }

            if(cleaner && drops[pos2] && drops[pos2]->isMark())
            {
                errorInfo.append(QString("split at (%1, %2) get polluted!").arg(x3 + 1).arg(y3 + 1));
                return false;
            }

            drops[pos] = large;
            drops[pos1] = drop1;
            drops[pos2] = drop2;
            SplitSave[t + 1].append(large);
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
        auto pos1 = y1 * col + x1;
        auto pos2 = y2 * col + x2;

        if(cleaner && drops[pos2] && drops[pos2]->isMark() && drops[pos2]!= static_cast<Drop *>(drops[pos1])->getMark())
        {
            errorInfo.append(QString("drop move to (%1, %2) get polluted!").arg(x2 + 1).arg(y2 + 1));
            return false;
        }
        drops[pos2] = drops[pos1];
        drops[pos2]->move(y2 * col + x2);
        drops[pos1] = static_cast<Drop *>(drops[pos1])->getMark();;
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

bool DMFB::check()
{
    // generate static bound
    for (int pos = 0; pos < row * col; pos++)
    {
        auto drop = drops[pos];
        if(!drop || drop->isLargeDrop() || drop->isMark())
            continue;
        if(mask[pos])
        {
            errorInfo.append(QString("(%1,%2) disobey static bound!").arg(
                                                                         pos % col + 1).arg(pos / col + 1));
            return false;
        }
        if(last_mask[pos] && allDrops[t - 1][pos] != drop)
        {
            // check dynamic bound
            errorInfo.append(QString("(%1,%2) disobey dynamic bound!").arg(
                                                                          pos % col + 1).arg(pos / col + 1));
            return false;
        }
        mask[pos] = DROP;
        pollute[pos].insert(drop);
        auto x = pos % col;
        auto y = pos / col;
        for(int dy = y - 1; dy <= y + 1; dy++)
            for(int dx = x - 1; dx <= x + 1; dx++)
            {
                // itself
                if(dx == x && dy == y)
                    continue;
                // beyond range
                if(dx < 0 || dy < 0 || dx == col || dy == row)
                    continue;
                if(mask[dy * col + dx] == DROP)
                {
                    qDebug()<<dy<<dx<<dy*col+dx<<mask[dy * col + dx];
                    errorInfo.append(QString("(%1,%2) disobey static bound!").arg(dx).arg(dy));
                    return false;
                }
                mask[dy * col + dx] = ZOC;
            }
    }

    memcpy(last_mask, mask, sizeof(mask));
    memset(mask, 0, sizeof(mask));


    return true;
}

bool DMFB::normalOver() const
{
    return t > lastInstructionT && errorInfo.empty() && lastInstructionT > -1;
}

bool DMFB::wash()
{
    // return true if it needs keep washing

    if(!washer)
    {
        if(putWasher())
        {
            washing = true;
            return true;
        }
        return false;
    }

    if(path.empty())
    {
        if(targetList.empty())
        {
            if(washer->getPos() == wastePos)
            {
                outWasher();
                washing = false;
                return false;
            }
            else
            {
                updateDistance();
                getPath(wastePos);
            }
        }
        else
        {
            auto target = getNextTarget();
            getPath(target);
        }
    }

    // now we should have a path
    washerMove(path.back());
    path.pop_back();
    return true;
}

int DMFB::getNextTarget()
{
    updateDistance();
    int target = -1;
    int nearest = row * col;
    for(auto pos: targetList)
    {
        if(distance[pos] < nearest)
        {
            nearest = distance[pos];
            target = pos;
        }
    }
    return target;
}

void DMFB::getPath(int pos)
{
    path.clear();
    auto current = pos;
    for(int i=0;i<distance[pos];i++)
    {
        path.append(current);
        current = comeFrom[current];
    }
    assert(current == washer->getPos());
}

void DMFB::washerMove(int pos)
{
    if(wash_map[pos] == TARGET)
    {
        targetList.removeOne(pos);
        pollute[pos].remove(static_cast<DropMark *>(drops[pos])->getDrop());
        wash_map[pos] = PATH;
    }
    drops[washer->getPos()] = nullptr;
    drops[pos] = washer;
    washer->move(pos);
}

void DMFB::updateDistance()
{
    distance.fill(-1);
    comeFrom.fill(-1);
    QQueue<int> save;
    QQueue<int> from;
    save.enqueue(washer->getPos());
    from.enqueue(row * col);
    while(save.length())
    {
        auto pos = save.dequeue();
        auto past = from.dequeue();
        if(!reachable(pos) || comeFrom[pos] != -1)
            // can't visit or has been visited
            continue;
        comeFrom[pos] = past;
        distance[pos] = past != row * col? distance[past] + 1 : 0;
        if(pos % col)
        {
            save.enqueue(pos - 1);
            from.enqueue(pos);
        }
        if(pos / col)
        {
            save.enqueue(pos - col);
            from.enqueue(pos);
        }
        if((pos + 1) % col)
        {
            save.enqueue(pos + 1);
            from.enqueue(pos);
        }
        if(pos + col < row * col)
        {
            save.enqueue(pos + col);
            from.enqueue(pos);
        }
    }
}

bool DMFB::reachable(int pos)
{
    return wash_map[pos] == PATH || wash_map[pos] == TARGET;
}

bool DMFB::putWasher()
{
    wash_map.fill(UNVISITED);
    targetList.clear();
    searchPath(washPos);
    if(!reachable(wastePos))
        // can't find a way from input to output
        return false;
    if(targetList.empty())
        // nothing to wash
        return false;
    washer = CleanerDrop::create(washPos);
    washerMove(washPos);
    return true;
}

void DMFB::outWasher()
{
    drops[wastePos] = nullptr;
    washer = nullptr;
    allDrops[t - 1] = drops;
}

void DMFB::searchPath(int pos)
{
    // if this one has been visitied
    if(wash_map[pos])
        return;

    if(obstacle[pos] || last_mask[pos])
    {
        wash_map[pos] = OBSTACLE;
        return;
    }

    if(drops[pos] && drops[pos]->isMark())
    {
        wash_map[pos] = TARGET;
        targetList.append(pos);
    }
    else
        wash_map[pos] = PATH;

    if(pos % col)
        searchPath(pos - 1);
    if(pos / col)
        searchPath(pos - col);
    if((pos + 1) % col)
        searchPath(pos + 1);
    if(pos + col < col * row)
        searchPath(pos + col);
}













