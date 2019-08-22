#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#include <QList>
#include <QVector>
#include <QMap>


enum InstructionType
{
    INPUT,
    OUTPUT,
    MOVE,
    MIX,
    MERGE,
    SPLIT,
};

class Instruction
{
public:
    Instruction(InstructionType);
    InstructionType type;
};

class InputData: public Instruction
{
public:
    InputData(QVector<int>);
    int t;
    int x1;
    int y1;
};

class OutputData: public Instruction
{
public:
    OutputData(QVector<int>);
    int t;
    int x1;
    int y1;
};

class MergeData: public Instruction
{
public:
    MergeData(QVector<int>);
    int t;
    int x1;
    int y1;
    int x2;
    int y2;
};

class SplitData: public Instruction
{
public:
    SplitData(QVector<int>);
    int t;
    int x1;
    int y1;
    int x2;
    int y2;
    int x3;
    int y3;
};

class MoveData: public Instruction
{
public:
    MoveData(QVector<int>);
    int t;
    int x1;
    int y1;
    int x2;
    int y2;
};

class MixData: public Instruction
{
public:
    MixData(QVector<int>);
    QList<MoveData> getMoveInstruction();
    int t;
    QVector<int> x;
    QVector<int> y;
};
#endif // INSTRUCTION_H
