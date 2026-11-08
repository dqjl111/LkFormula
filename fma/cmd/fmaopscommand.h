#ifndef FMAOPSCOMMAND_H
#define FMAOPSCOMMAND_H

#include <QUndoCommand>
#include <QVector>
#include "fma/fmadoc.h"



class FmaOpsCommand : public QUndoCommand
{
public:
    FmaOpsCommand(const QString& _op, FmaDoc* _doc);
    FmaOpsCommand(const QList<std::shared_ptr<FmaEditOp>>& _editOps,
                  const QVector<int>& _beginIndexes,
                  const QVector<int>& _endIndexes,
                  FmaDoc* _doc);

    virtual void undo() override;
    virtual void redo() override;

    typedef IFmaBase* (*FmaOpFun)(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);
    static QMap<QString, FmaOpFun> funs;
    static void init();

    static IFmaBase* opFmaFence(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);
    static IFmaBase* opFmaFrac(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);
    static IFmaBase* opFmaRoot(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);

    static IFmaBase* opFmaMatrix(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);

    static IFmaBase* opFmaOver(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);
    static IFmaBase* opFmaUnder(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);
    static IFmaBase* opFmaUnderOver(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);

    static IFmaBase* opFmaSub(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);
    static IFmaBase* opFmaSup(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);
    static IFmaBase* opFmaSubSup(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);

    static IFmaBase* opFmaBox(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);
    static IFmaBase* opFmaStrike(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);
    static IFmaBase* opFmaLongDiv(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);

    static IFmaBase* opFmaSum(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);
    static IFmaBase* opFmaIntegral(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);
    static IFmaBase* opFmaMoreIntegral(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);

    static IFmaBase* opFmaDeco(FmaOpsCommand* cmd, const QString& mainType, const QString& subType);

protected:
    FmaDoc* doc = nullptr;
    QVector<int> beginIndexes;
    QVector<int> endIndexes;
    bool isDone = false;

    QList<std::shared_ptr<FmaEditOp>> editOps;

    void handleOpEx(const QString& op);


};

#endif // FMAOPSCOMMAND_H
