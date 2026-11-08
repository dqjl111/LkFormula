#include "fmaopscommand.h"
#include "fma/fmammltable.h"

#include <QDebug>

QMap<QString, FmaOpsCommand::FmaOpFun> FmaOpsCommand::funs;


FmaOpsCommand::FmaOpsCommand(const QString &op, FmaDoc *_doc)
{
    doc = _doc;
    if(doc){
        beginIndexes = doc->math.activeIndexes();
        if(op.length()>1){
            handleOpEx(op);
        }else if(op.length() == 1){
            FmaRow* curRow = doc->math.activeRow(true);
            FmaText* fmaText = new FmaText(FmaType::munknown, op);

            editOps = curRow->putToActive(fmaText);
            fmaText->buildTextIndexes(endIndexes);
            doc->math.setActiveIndexes(endIndexes);
        }
        doc->math.calculateRect();
        doc->math.cursorOn = true;
        isDone = true;
    }
}

FmaOpsCommand::FmaOpsCommand(const QList<std::shared_ptr<FmaEditOp>>& _editOps,
                             const QVector<int>& _beginIndexes,
                             const QVector<int>& _endIndexes,
                             FmaDoc* _doc)
{
    doc = _doc;
    editOps = _editOps;
    beginIndexes = _beginIndexes;
    endIndexes = _endIndexes;
    isDone = true;
}

void FmaOpsCommand::undo()
{
    if(doc){
        if(isDone){
            doc->math.setActiveIndexes(endIndexes);
            QList<std::shared_ptr<FmaEditOp>>::reverse_iterator iter = editOps.rbegin();
            for(; iter != editOps.rend(); ++iter){
                iter->get()->undo(&doc->math);
            }
            doc->math.setActiveIndexes(beginIndexes);
            doc->math.calculateRect();
            doc->math.cursorOn = true;
            isDone = false;
        }
    }
}

void FmaOpsCommand::redo()
{
    if(doc){
        if(!isDone){
            doc->math.setActiveIndexes(beginIndexes);
            for(std::shared_ptr<FmaEditOp> op: editOps){
                op->redo(&doc->math);
            }
            doc->math.setActiveIndexes(endIndexes);
            doc->math.calculateRect();
            doc->math.cursorOn = true;
            isDone = true;
        }
    }
}


void FmaOpsCommand::init()
{
    funs["fence"] = opFmaFence;
    funs["frac"] = opFmaFrac;
    funs["root"] = opFmaRoot;
    funs["matrix"] = opFmaMatrix;
    funs["over"] = opFmaOver;
    funs["under"] = opFmaUnder;
    funs["underover"] = opFmaUnderOver;
    funs["sub"] = opFmaSub;
    funs["sup"] = opFmaSup;
    funs["subsup"] = opFmaSubSup;
    funs["box"] = opFmaBox;
    funs["strike"] = opFmaStrike;
    funs["longdiv"] = opFmaLongDiv;
    funs["sum"] = opFmaSum;
    funs["int"] = opFmaIntegral;
    funs["iint"] = opFmaMoreIntegral;
    funs["iiint"] = opFmaMoreIntegral;
    funs["oint"] = opFmaMoreIntegral;
    funs["Conint"] = opFmaMoreIntegral;
    funs["Cconint"] = opFmaMoreIntegral;
    funs["cwconint"] = opFmaMoreIntegral;
    funs["awconint"] = opFmaMoreIntegral;
    funs["deco"] = opFmaDeco;
}

IFmaBase* FmaOpsCommand::opFmaFence(FmaOpsCommand *cmd, const QString &, const QString &subType)
{
    FmaFenced* fenced = nullptr;
    switch(subType.toInt()){
    case 1:{
        fenced = new FmaFenced("(", ")");
        break;
    }
    case 2:{
        fenced = new FmaFenced("[", "]");
        break;
    }
    case 3:{
        fenced = new FmaFenced("{", "}");
        break;
    }
    case 4:{
        fenced = new FmaFenced(FmaMmlTable::searchCodeByName("lang"),
                               FmaMmlTable::searchCodeByName("rang"));
        break;
    }
    case 5:{
        fenced = new FmaFenced("|", "|");
        break;
    }
    case 6:{
        fenced = new FmaFenced(FmaMmlTable::searchCodeByName("Verbar"),
                               FmaMmlTable::searchCodeByName("Verbar"));
        break;
    }
    case 7:{
        fenced = new FmaFenced(FmaMmlTable::searchCodeByName("lfloor"),
                               FmaMmlTable::searchCodeByName("rfloor"));
        break;
    }
    case 8:{
        fenced = new FmaFenced(FmaMmlTable::searchCodeByName("lceil"),
                               FmaMmlTable::searchCodeByName("rceil"));
        break;
    }
    case 9:{
        fenced = new FmaFenced("[",")");
        break;
    }
    case 10:{
        fenced = new FmaFenced("(","]");
        break;
    }
    case 11:{
        fenced = new FmaFenced("|",FmaMmlTable::searchCodeByName("rang"));
        break;
    }
    case 12:{
        fenced = new FmaFenced(FmaMmlTable::searchCodeByName("lang"), "|");
        break;
    }
    case 13:{
        fenced = new FmaFenced("[", "[");
        break;
    }
    case 14:{
        fenced = new FmaFenced("]", "]");
        break;
    }
    case 15:{
        fenced = new FmaFenced("]", "[");
        break;
    }
    case 16:{
        fenced = new FmaFenced(FmaMmlTable::searchCodeByName("LeftDoubleBracket"),
                               FmaMmlTable::searchCodeByName("RightDoubleBracket"));
        break;
    }
    case 17:{
        fenced = new FmaFenced("(","");
        break;
    }
    case 18:{
        fenced = new FmaFenced("",")");
        break;
    }
    case 19:{
        fenced = new FmaFenced("[","");
        break;
    }
    case 20:{
        fenced = new FmaFenced("","]");
        break;
    }
    case 21:{
        fenced = new FmaFenced("{","");
        break;
    }
    case 22:{
        fenced = new FmaFenced("","}");
        break;
    }
    case 23:{
        fenced = new FmaFenced(FmaMmlTable::searchCodeByName("lang"),"");
        break;
    }
    case 24:{
        fenced = new FmaFenced("",FmaMmlTable::searchCodeByName("rang"));
        break;
    }
    case 25:{
        fenced = new FmaFenced("|","");
        break;
    }
    case 26:{
        fenced = new FmaFenced("","|");
        break;
    }
    case 27:{
        fenced = new FmaFenced(FmaMmlTable::searchCodeByName("Verbar"),"");
        break;
    }
    case 28:{
        fenced = new FmaFenced("",FmaMmlTable::searchCodeByName("Verbar"));
        break;
    }
    case 29:{
        fenced = new FmaFenced(FmaMmlTable::searchCodeByName("LeftDoubleBracket"),"");
        break;
    }
    case 30:{
        fenced = new FmaFenced("",FmaMmlTable::searchCodeByName("RightDoubleBracket"));
        break;
    }
    default:{
        break;
    }
    }
    if(fenced){
        FmaRow* curRow = cmd->doc->math.activeRow(true);
        cmd->editOps = curRow->putToActive(fenced);

        // 插入括号后把光标放进括号内部，否则输入会落在括号外层并与内容重叠。
        FmaRow* buddy = fenced->getBuddy();
        fenced->setActiveChild(buddy);
        buddy->buildIndexes(cmd->endIndexes);
    }
    return fenced;
}

IFmaBase* FmaOpsCommand::opFmaFrac(FmaOpsCommand *cmd, const QString &, const QString &subType)
{
    FmaFrac* frac = new FmaFrac;
    switch(subType.toInt()){
    case 2:{
        frac->setStyleScriptLevel(1);
        break;
    }
    case 3:{
        frac->bevelled = true;
        break;
    }
    case 4:{
        frac->setStyleScriptLevel(1);
        frac->bevelled = true;
        break;
    }
    default:{
        break;
    }
    }

    //cmd->fma = frac;
    FmaRow* curRow = cmd->doc->math.activeRow(true);
    cmd->editOps = curRow->putToActive(frac);
    frac->getUp()->buildIndexes(cmd->endIndexes);
    return frac;
}

IFmaBase* FmaOpsCommand::opFmaRoot(FmaOpsCommand *cmd, const QString &, const QString &subType)
{
    FmaBuddyBase* buddyBase = nullptr;
    if(subType.toInt() == 1){
        buddyBase = new FmaSquareRoot;
    }else{
        buddyBase = new FmaRoot;
    }
    //cmd->fma = buddyBase;
    FmaRow* curRow = cmd->doc->math.activeRow(true);
    cmd->editOps = curRow->putToActive(buddyBase);
    buddyBase->getBuddy()->buildIndexes(cmd->endIndexes);
    return buddyBase;
}

IFmaBase* FmaOpsCommand::opFmaMatrix(FmaOpsCommand *cmd, const QString &, const QString &subType)
{

    QStringList t = subType.split(",");

    int row = 1;
    if(t.size() >=1){
        row = t[0].toInt();
    }

    int col = 1;
    if(t.size() >=2){
        col = t[1].toInt();
    }

    int rowSpaceLevel = FmaTextFormats::Nonespace;
    if(t.size() >=3){
        rowSpaceLevel = t[2].toInt();
    }

    int colSpaceLevel = FmaTextFormats::Veryverythickmathspace;
    if(t.size() >=4){
        colSpaceLevel = t[3].toInt();
    }

    if(row == 1 && col == 1){
        FmaRow* row = new FmaRow;
        //cmd->fma = row;
        FmaRow* curRow = cmd->doc->math.activeRow(true);
        cmd->editOps = curRow->putToActive(row);
        row->buildIndexes(cmd->endIndexes);

        return row;
    }else{
        FmaTable* table = new FmaTable(row, col);
        table->rowspacing = rowSpaceLevel;
        table->columnspacing = colSpaceLevel;
        //cmd->fma = table;
        FmaRow* curRow = cmd->doc->math.activeRow(true);
        cmd->editOps = curRow->putToActive(table);

        // 插入矩阵后把活动光标放进第一个单元格，否则后续输入会落在表格外层。
        FmaRow* firstCell = table->getCell(0, 0);
        table->setActiveChild(firstCell);
        firstCell->buildIndexes(cmd->endIndexes);

        return table;
    }
}

IFmaBase* FmaOpsCommand::opFmaOver(FmaOpsCommand *cmd, const QString &, const QString &)
{
    FmaOver* t = new FmaOver;
    //cmd->fma = t;
    FmaRow* curRow = cmd->doc->math.activeRow(true);
    cmd->editOps = curRow->putToActive(t);
    t->getBuddy()->buildIndexes(cmd->endIndexes);

    return t;
}

IFmaBase* FmaOpsCommand::opFmaUnder(FmaOpsCommand *cmd, const QString &, const QString &)
{
    FmaUnder* t = new FmaUnder;
    //cmd->fma = t;
    FmaRow* curRow = cmd->doc->math.activeRow(true);
    cmd->editOps = curRow->putToActive(t);
    t->getBuddy()->buildIndexes(cmd->endIndexes);

    return t;
}

IFmaBase* FmaOpsCommand::opFmaUnderOver(FmaOpsCommand *cmd, const QString &, const QString &)
{
    FmaUnderOver* t = new FmaUnderOver;
    //cmd->fma = t;
    FmaRow* curRow = cmd->doc->math.activeRow(true);
    cmd->editOps = curRow->putToActive(t);
    t->getBuddy()->buildIndexes(cmd->endIndexes);
    return t;
}

IFmaBase* FmaOpsCommand::opFmaSub(FmaOpsCommand *cmd, const QString &, const QString &subType)
{
    FmaSub* t = new FmaSub;
    if(subType.toInt() == 2){
        t->dir = Qt::RightToLeft;
    }
    //cmd->fma = t;
    FmaRow* curRow = cmd->doc->math.activeRow(true);
    cmd->editOps = curRow->putToActive(t);
    t->getBuddy()->buildIndexes(cmd->endIndexes);
    return t;
}

IFmaBase* FmaOpsCommand::opFmaSup(FmaOpsCommand *cmd, const QString &, const QString &subType)
{
    FmaSuper* t = new FmaSuper;
    if(subType.toInt() == 2){
        t->dir = Qt::RightToLeft;
    }
    //cmd->fma = t;
    FmaRow* curRow = cmd->doc->math.activeRow(true);
    cmd->editOps = curRow->putToActive(t);
    t->getBuddy()->buildIndexes(cmd->endIndexes);
    return t;
}

IFmaBase* FmaOpsCommand::opFmaSubSup(FmaOpsCommand *cmd, const QString &, const QString &subType)
{
    FmaSubSuper* t = new FmaSubSuper;
    if(subType.toInt() == 2){
        t->dir = Qt::RightToLeft;
    }
    //cmd->fma = t;
    FmaRow* curRow = cmd->doc->math.activeRow(true);
    cmd->editOps = curRow->putToActive(t);
    t->getBuddy()->buildIndexes(cmd->endIndexes);
    return t;
}

IFmaBase* FmaOpsCommand::opFmaBox(FmaOpsCommand *cmd, const QString &, const QString &subType)
{
    FmaEnclose* t = nullptr;
    switch(subType.toInt()){
    case 1:{
        t = new FmaEnclose(int(FmaEnclose::TopLeft));
        break;
    }
    case 2:{
        t = new FmaEnclose(int(FmaEnclose::TopRight));
        break;
    }
    case 3:{
        t = new FmaEnclose(int(FmaEnclose::BottomLeft));
        break;
    }
    case 4:{
        t = new FmaEnclose(int(FmaEnclose::BottomRight));
        break;
    }
    case 5:{
        t = new FmaEnclose(int(FmaEnclose::Box));
        break;
    }
    default:{
        break;
    }
    }

    if(t){
        //cmd->fma = t;
        FmaRow* curRow = cmd->doc->math.activeRow(true);
        cmd->editOps = curRow->putToActive(t);
        t->getBuddy()->buildIndexes(cmd->endIndexes);
    }
    return t;
}

IFmaBase* FmaOpsCommand::opFmaStrike(FmaOpsCommand *cmd, const QString &, const QString &subType)
{
    FmaEnclose* t = nullptr;
    switch(subType.toInt()){
    case 1:{
        t = new FmaEnclose(int(FmaEnclose::Horizontalstrike));
        break;
    }
    case 2:{
        t = new FmaEnclose(int(FmaEnclose::Verticalstrike));
        break;
    }
    case 3:{
        t = new FmaEnclose(int(FmaEnclose::Updiagonalstrike));
        break;
    }
    case 4:{
        t = new FmaEnclose(int(FmaEnclose::Downdiagonalstrike));
        break;
    }
    case 5:{
        t = new FmaEnclose(int(FmaEnclose::Updiagonalstrike|FmaEnclose::Downdiagonalstrike));
        break;
    }
    default:{
        break;
    }
    }

    if(t){
        //cmd->fma = t;
        FmaRow* curRow = cmd->doc->math.activeRow(true);
        cmd->editOps = curRow->putToActive(t);
        t->getBuddy()->buildIndexes(cmd->endIndexes);
    }
    return t;
}

IFmaBase* FmaOpsCommand::opFmaLongDiv(FmaOpsCommand *cmd, const QString &, const QString &)
{
    FmaEnclose* t = new FmaEnclose(int(FmaEnclose::LongDiv));
    //cmd->fma = t;
    FmaRow* curRow = cmd->doc->math.activeRow(true);
    cmd->editOps = curRow->putToActive(t);
    t->getBuddy()->buildIndexes(cmd->endIndexes);
    return t;
}

IFmaBase* FmaOpsCommand::opFmaSum(FmaOpsCommand *cmd, const QString &mainType, const QString &subType)
{
    FmaRow* row = new FmaRow;
    row->setDisplayStyle(true);
    switch(subType.toInt()){
    case 1:{
        FmaText* fmaText = new FmaText(FmaType::munknown,
                                       FmaMmlTable::searchCodeByName(mainType));
        row->append(fmaText);
        break;
    }
    case 2:{
        FmaUnder* t = new FmaUnder;
        t->setBuddy(FmaMmlTable::searchCodeByName(mainType));
        row->append(t);
        break;
    }
    case 3:{
        FmaUnderOver* t = new FmaUnderOver;
        t->setBuddy(FmaMmlTable::searchCodeByName(mainType));
        row->append(t);
        break;
    }
    case 4:{
        FmaSub* t = new FmaSub;
        t->setBuddy(FmaMmlTable::searchCodeByName(mainType));
        row->append(t);
        break;
    }
    case 5:{
        FmaSubSuper* t = new FmaSubSuper;
        t->setBuddy(FmaMmlTable::searchCodeByName(mainType));
        row->append(t);
        break;
    }
    default:{
        break;
    }
    }

    row->append(new FmaRow);
    //cmd->fma = row;
    FmaRow* curRow = cmd->doc->math.activeRow(true);
    cmd->editOps = curRow->putToActive(row);
    row->lastBlock()->buildIndexes(cmd->endIndexes);
    return row;
}

IFmaBase* FmaOpsCommand::opFmaIntegral(FmaOpsCommand *cmd, const QString &mainType, const QString &subType)
{
    FmaRow* row = new FmaRow;
    row->setDisplayStyle(true);
    switch(subType.toInt()){
    case 1:
    case 4:{
        FmaText* fmaText = new FmaText(FmaType::munknown,
                                       FmaMmlTable::searchCodeByName(mainType));
        row->append(fmaText);
        break;
    }
    case 2:{
        FmaUnderOver* t = new FmaUnderOver;
        t->setBuddy(FmaMmlTable::searchCodeByName(mainType));
        row->append(t);
        break;
    }
    case 3:{
        FmaSubSuper* t = new FmaSubSuper;
        t->setBuddy(FmaMmlTable::searchCodeByName(mainType));
        row->append(t);
        break;
    }
    case 5:{
        FmaUnder* t = new FmaUnder;
        t->setBuddy(FmaMmlTable::searchCodeByName(mainType));
        row->append(t);
        break;
    }
    case 6:{
        FmaSub* t = new FmaSub;
        t->setBuddy(FmaMmlTable::searchCodeByName(mainType));
        row->append(t);
        break;
    }
    default:{
        break;
    }
    }

    row->append(new FmaRow);
    //cmd->fma = row;
    FmaRow* curRow = cmd->doc->math.activeRow(true);
    cmd->editOps = curRow->putToActive(row);
    row->lastBlock()->buildIndexes(cmd->endIndexes);
    return row;
}

IFmaBase* FmaOpsCommand::opFmaMoreIntegral(FmaOpsCommand *cmd, const QString &mainType, const QString &subType)
{
    FmaRow* row = new FmaRow;
    row->setDisplayStyle(true);
    switch(subType.toInt()){
    case 1:{
        FmaText* fmaText = new FmaText(FmaType::munknown,
                                       FmaMmlTable::searchCodeByName(mainType));
        row->append(fmaText);
        break;
    }
    case 2:{
        FmaUnder* t = new FmaUnder;
        t->setBuddy(FmaMmlTable::searchCodeByName(mainType));
        row->append(t);
        break;
    }
    case 3:{
        FmaSub* t = new FmaSub;
        t->setBuddy(FmaMmlTable::searchCodeByName(mainType));
        row->append(t);
        break;
    }
    default:{
        break;
    }
    }

    row->append(new FmaRow);
    //cmd->fma = row;
    FmaRow* curRow = cmd->doc->math.activeRow(true);
    cmd->editOps = curRow->putToActive(row);
    row->lastBlock()->buildIndexes(cmd->endIndexes);
    return row;
}

IFmaBase* FmaOpsCommand::opFmaDeco(FmaOpsCommand *cmd, const QString &, const QString &subType)
{
    FmaBuddyBase* t = nullptr;
    switch(subType.toInt()){
    case 1:{
        t = new FmaOver;
        ((FmaOver* )t)->setOver(FmaMmlTable::searchCodeByName("tilde"));
        break;
    }
    case 2:{
        t = new FmaOver;
        ((FmaOver* )t)->setOver(FmaMmlTable::searchCodeByName("Hat"));
        break;
    }
    case 3:{
        t = new FmaOver;
        ((FmaOver* )t)->setOver(FmaMmlTable::searchCodeByName("OverParenthesis"));
        break;
    }
    case 4:{
        t = new FmaOver;
        ((FmaOver* )t)->setOver(QChar(0xFE39));
        break;
    }
    case 5:{
        t = new FmaOver;
        ((FmaOver* )t)->setOver("_");
        break;
    }
    case 6:{
        t = new FmaOver;
        ((FmaOver* )t)->setOver("=");
        break;
    }
    case 7:{
        t = new FmaUnder;
        ((FmaUnder* )t)->setUnder("_");
        break;
    }
    case 8:{
        t = new FmaUnder;
        ((FmaUnder* )t)->setUnder("=");
        break;
    }
    case 9:{
        t = new FmaOver;
        ((FmaOver* )t)->setOver(FmaMmlTable::searchCodeByName("rightarrow"));
        break;
    }
    case 10:{
        t = new FmaOver;
        ((FmaOver* )t)->setOver(FmaMmlTable::searchCodeByName("leftarrow"));
        break;
    }
    case 11:{
        t = new FmaOver;
        ((FmaOver* )t)->setOver(FmaMmlTable::searchCodeByName("RightVector"));
        break;
    }
    case 12:{
        t = new FmaOver;
        ((FmaOver* )t)->setOver(FmaMmlTable::searchCodeByName("LeftRightArrow"));
        break;
    }
    case 13:{
        t = new FmaUnder;
        ((FmaUnder* )t)->setUnder(FmaMmlTable::searchCodeByName("rightarrow"));
        break;
    }
    case 14:{
        t = new FmaUnder;
        ((FmaUnder* )t)->setUnder(FmaMmlTable::searchCodeByName("leftarrow"));
        break;
    }
    case 15:{
        t = new FmaUnder;
        ((FmaUnder* )t)->setUnder(QChar(0x21C1)); // ⇁ 下方单肩箭头，肩朝下
        break;
    }
    case 16:{
        t = new FmaUnder;
        ((FmaUnder* )t)->setUnder(FmaMmlTable::searchCodeByName("LeftRightArrow"));
        break;
    }
    default:{
        break;
    }
    }

    //cmd->fma = t;
    FmaRow* curRow = cmd->doc->math.activeRow(true);
    cmd->editOps = curRow->putToActive(t);
    t->getBuddy()->buildIndexes(cmd->endIndexes);
    return t;
}

void FmaOpsCommand::handleOpEx(const QString& op)
{
    QStringList listStr =op.split("_");
    Q_ASSERT(listStr.size() == 2);

    QMap<QString, FmaOpFun>::iterator iter = funs.find(listStr[0]);
    if(iter != funs.end()){
        FmaOpFun fun = iter.value();
        IFmaBase* fma = fun(this, listStr[0], listStr[1]);
        if(fma){
            doc->math.setActiveIndexes(endIndexes);
        }
    }
}
