#include "instruction.h"

Instruction::Instruction(InstructionType type): type (type)
{
}

InputData::InputData(QVector<int> v): Instruction (INPUT),
    t(v[0]), x1(v[1]), y1(v[2])
{

}

OutputData::OutputData(QVector<int> v): Instruction (OUTPUT),
    t(v[0]), x1(v[1]), y1(v[2])
{
}

MergeData::MergeData(QVector<int> v): Instruction (MERGE),
    t(v[0]), x1(v[1]), y1(v[2]), x2(v[3]), y2(v[4])
{

}

SplitData::SplitData(QVector<int> v): Instruction (SPLIT),
   t(v[0]), x1(v[1]), y1(v[2]), x2(v[3]), y2(v[4]), x3(v[5]), y3(v[6])
{

}

MoveData::MoveData(QVector<int> v): Instruction (MOVE),
    t(v[0]), x1(v[1]), y1(v[2]), x2(v[3]), y2(v[4])
{

}

MixData::MixData(QVector<int> v): Instruction (MIX), t(v[0])
{
    for(int i=1; i < v.length(); i++)
    {
        if(i% 2)
            x.append(v[i]);
        else
            y.append(v[i]);
    }
    assert(x.length()==y.length());
    assert(x.first() == x.last());
    assert(y.first() == y.last());
}

QList<MoveData> MixData::getMoveInstruction()
{
    QList<MoveData> list;
    auto last_x = x.first();
    auto last_y = y.first();
    auto it_x = x.constBegin();
    auto it_y = y.constBegin();
    for (int i = 0;i < x.length() - 1;i++)
    {
        it_x++;
        it_y++;
        list.append(MoveData({t + i, last_x,last_y, *it_x, *it_y}));
        last_x = *it_x;
        last_y = *it_y;
    }
    return list;
}
