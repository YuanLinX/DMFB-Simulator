#ifndef DMFB_H
#define DMFB_H
#include <QList>
#include <QVector>
#include <QSet>
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
    void reset();
    QList <QString> errorInfo;
    QVector <QSet<DropItem *>> pollute;

    int row, col;
    QList<int> inputs;
    int output;
    bool cleaner;

    int getLastT() const;
    bool over() const;
    bool normalOver() const;
    const actionFlag & getFlag();
    int getT() const;
    DropItem * getDrop(int pos);
    void setObstacle(int pos);
    bool getObstacle(int pos);
    bool isWashing();


private:
    enum Mask
    {
        NONE,
        // zone of control
        ZOC,
        DROP,
        DROPMARK

    };

    // first for t, second for pos
    QVector <QList<Instruction *>> instructions;
    QVector <QList <LargeDrop *>> MergeSave;
    QVector <QList <LargeDrop *>> SplitSave;
    QVector <QVector <DropItem *>> allDrops;

    QVector <DropItem *> drops;
    QVector <bool> obstacle;

    QMap <QString, InstructionType> nameToType;
    int t;
    int lastInstructionT;
    bool washing;

    bool loadState(int target);
    bool executeInstruction(Instruction *);
    void clearInstrcutions();
    void clearDrops();
    void clear();

    actionFlag flag;
    static const int max_t = 200;
    static const int max_size = 200;

    bool check();
    Mask mask[max_size];
    Mask last_mask[max_size];

};

#endif // DMFB_H
