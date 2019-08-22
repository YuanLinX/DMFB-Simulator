#ifndef DMFB_H
#define DMFB_H
#include <QList>
#include <QVector>
#include "instruction.h"
#include "dropitem.h"


class DMFB
{
public:
    struct actionFlag
    {
        bool input;
        bool output;
        bool move;
        bool split1;
        bool split2;
        bool merge1;
        bool merge2;
    };
    DMFB();
    virtual ~DMFB();
    auto getAllInstructions();
    auto getInstructionList(int t);
    void addInstruction(QString s);
    bool next();
    bool last();
    void initalize();
    void clear();
    QList <QString> errorInfo;

    int getT() const;
    DropItem * getDrop(int pos);
    int row, col;
    QList<int> inputs;
    int output;
    int getLastT() const;
    bool over() const;
    const actionFlag & getFlag();


private:
    class Save
    {

    };

    // first for t, second for pos
    QVector <QList<Instruction *>> instructions;
    QVector <QList <LargeDrop *>> MergeSave;
    QVector <QList <LargeDrop *>> SplitSave;
    QVector <QVector <DropItem *>> allDrops;


    QVector <DropItem *> drops;

    QVector <int> pollute;
    QVector <Save> save;
    QMap <QString, InstructionType> nameToType;
    int t;
    int lastInstructionT;

    void loadState(int target);
    bool executeInstruction(Instruction *);
    void clearInstrcutions();
    void clearDrops();
    actionFlag flag;

};

#endif // DMFB_H
