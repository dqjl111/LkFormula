#include "fmaeditview.h"
#include <QPainter>
#include <QPaintEvent>
#include "fma/fmarow.h"
#include "fma/fmatext.h"
#include "fma/fmasubsuperbase.h"
#include "fma/fmafrac.h"
#include "fma/fmaroot.h"
#include "fma/fmaunderover.h"
#include "fma/fmatable.h"
#include "fma/fmammltable.h"
#include <QStyleHints>
#include <QApplication>
#include <QDebug>
#include <QTextCursor>
#include <QGridLayout>
#include "fma/fmammltable.h"
#include "fma/fmaoperdict.h"
#include "fma/fmatextformats.h"
#include "fma/widget/fmaopwidget.h"
#include "fma/widget/fmatoolbar.h"
#include "fma/widget/fmaopsbutton.h"
#include "fma/cmd/fmaopscommand.h"
#include "fma/widget/fmamatrixinputdlg.h"
#include "util/nodeleter.h"
#include "fmaclipboard.h"
#include <iostream>

FmaEditView::FmaEditView(QWidget *parent) : QWidget(parent)
{
    // QWindowsMimeConverter registers itself with Qt and is owned by
    // QGuiApplication, so keep one heap instance instead of a stack member.
    static FmaMime *fmaMime = nullptr;
    if (!fmaMime)
        fmaMime = new FmaMime;

    setFocusPolicy(Qt::StrongFocus);
    setCursorVisible(true);
}

FmaEditView::~FmaEditView()
{
    FmaClipboard::instance.releaseClipboard();
}

QToolBar *FmaEditView::createToolBar(QWidget *parent)
{
    QToolBar* toolBar = new QToolBar(parent);
    FmaToolBar* toolWidget = new FmaToolBar(toolBar);
    toolBar->addWidget(toolWidget);
    QGridLayout* toolLayout = new QGridLayout(toolWidget);
    toolWidget->setLayout(toolLayout);
    toolLayout->setContentsMargins(0, 0, 0, 0);
    toolLayout->setSpacing(0);
    QFont tFont(FmaTextFormats::getDefaultFontFamily(), 11);
    toolWidget->setFont(tFont);
    {
        QStringList ops;
        FmaOpsButton* btn;

        ops = FmaMmlTable::compareLetters();
        btn = new FmaOpsButton("≤ ≠ ≈", ops, toolWidget);
        toolLayout->addWidget(btn, 0, 0);
        toolWidget->addWatch(btn);

        ops = FmaMmlTable::mathsetLetters();
        btn = new FmaOpsButton("∉ ∩ ⊂", ops, toolWidget);
        toolLayout->addWidget(btn, 0, 1);
        toolWidget->addWatch(btn);

        ops = FmaMmlTable::extraAddLetters();
        btn = new FmaOpsButton("± • ⊗", ops, toolWidget);
        toolLayout->addWidget(btn, 0, 2);
        toolWidget->addWatch(btn);

        ops = FmaMmlTable::arrowLetters();
        btn = new FmaOpsButton("→ ⇔ ↓", ops, toolWidget);
        toolLayout->addWidget(btn, 0, 3);
        toolWidget->addWatch(btn);

        ops = FmaMmlTable::conditionLetters();
        btn = new FmaOpsButton("∴ ∀ ∃", ops, toolWidget);
        toolLayout->addWidget(btn, 0, 4);
        toolWidget->addWatch(btn);

        ops = FmaMmlTable::extraNoteLetters();
        btn = new FmaOpsButton("∂ ∞ Δ", ops, toolWidget);
        toolLayout->addWidget(btn, 0, 5);
        toolWidget->addWatch(btn);

        ops = FmaMmlTable::littleGreekLetters();
        btn = new FmaOpsButton("λ θ ω", ops, toolWidget);
        toolLayout->addWidget(btn, 0, 6);
        toolWidget->addWatch(btn);

        ops = FmaMmlTable::bigGreekLetters();
        btn = new FmaOpsButton("Λ Θ Ω", ops, toolWidget);
        toolLayout->addWidget(btn, 0, 7);
        toolWidget->addWatch(btn);

        QPixmap icon;

        ops.clear();
        for(int i=1;i<=30; ++i){
            ops << QString(":/image/fence/fence_%0.png").arg(i, 2, 10, QLatin1Char('0'));
        }

        btn = new FmaOpsButton("", ops, 4, toolWidget);
        icon = QPixmap(":/image/fence/fence.png");
        btn->setIcon(icon);
        btn->setIconSize(icon.size());
        toolLayout->addWidget(btn, 1, 0);
        toolWidget->addWatch(btn);

        ops.clear();
        ops << ":/image/frac/frac_01.png";
        ops << ":/image/frac/frac_02.png";
        ops << ":/image/frac/frac_03.png";
        ops << ":/image/frac/frac_04.png";
        ops << ":/image/frac/root_01.png";
        ops << ":/image/frac/root_02.png";

        btn = new FmaOpsButton("", ops, toolWidget);
        icon = QPixmap(":/image/frac/frac.png");
        btn->setIcon(icon);
        btn->setIconSize(icon.size());
        toolLayout->addWidget(btn, 1, 1);
        toolWidget->addWatch(btn);

        ops.clear();
        ops << ":/image/subsup/sup_01.png";
        ops << ":/image/subsup/sub_01.png";
        ops << ":/image/subsup/subsup_01.png";
        ops << ":/image/subsup/sup_02.png";
        ops << ":/image/subsup/sub_02.png";
        ops << ":/image/subsup/subsup_02.png";
        ops << ":/image/subsup/over_01.png";
        ops << ":/image/subsup/under_01.png";
        ops << ":/image/subsup/underover_01.png";

        btn = new FmaOpsButton("", ops, toolWidget);
        icon = QPixmap(":/image/subsup/subsup.png");
        btn->setIcon(icon);
        btn->setIconSize(icon.size());
        toolLayout->addWidget(btn, 1, 2);
        toolWidget->addWatch(btn);

        ops.clear();
        ops << ":/image/sum/sum_01.png";
        ops << ":/image/sum/sum_02.png";
        ops << ":/image/sum/sum_03.png";
        ops << ":/image/sum/sum_04.png";
        ops << ":/image/sum/sum_05.png";

        btn = new FmaOpsButton("", ops, toolWidget);
        icon = QPixmap(":/image/sum/sum.png");
        btn->setIcon(icon);
        btn->setIconSize(icon.size());
        toolLayout->addWidget(btn, 1, 3);
        toolWidget->addWatch(btn);

        ops.clear();
        ops << ":/image/int/int_01.png";
        ops << ":/image/int/int_02.png";
        ops << ":/image/int/int_03.png";
        ops << ":/image/int/int_04.png";
        ops << ":/image/int/int_05.png";
        ops << ":/image/int/int_06.png";
        ops << ":/image/int/iint_01.png";
        ops << ":/image/int/iint_02.png";
        ops << ":/image/int/iint_03.png";
        ops << ":/image/int/iiint_01.png";
        ops << ":/image/int/iiint_02.png";
        ops << ":/image/int/iiint_03.png";
        ops << ":/image/int/oint_01.png";
        ops << ":/image/int/oint_02.png";
        ops << ":/image/int/oint_03.png";
        ops << ":/image/int/Conint_01.png";
        ops << ":/image/int/Conint_02.png";
        ops << ":/image/int/Conint_03.png";
        ops << ":/image/int/Cconint_01.png";
        ops << ":/image/int/Cconint_02.png";
        ops << ":/image/int/Cconint_03.png";
        ops << ":/image/int/cwconint_01.png";
        ops << ":/image/int/cwconint_02.png";
        ops << ":/image/int/cwconint_03.png";
        ops << ":/image/int/awconint_01.png";
        ops << ":/image/int/awconint_02.png";
        ops << ":/image/int/awconint_03.png";

        btn = new FmaOpsButton("", ops, 3, toolWidget);
        icon = QPixmap(":/image/int/int.png");
        btn->setIcon(icon);
        btn->setIconSize(icon.size());
        toolLayout->addWidget(btn, 1, 4);
        toolWidget->addWatch(btn);


        ops.clear();
        for(int i=1;i<=16; ++i){
            ops << QString(":/image/deco/deco_%0.png").arg(i, 2, 10, QLatin1Char('0'));
        }

        btn = new FmaOpsButton("", ops, 4, toolWidget);
        icon = QPixmap(":/image/deco/deco.png");
        btn->setIcon(icon);
        btn->setIconSize(icon.size());
        toolLayout->addWidget(btn, 1, 5);
        toolWidget->addWatch(btn);


        ops.clear();
        ops << ":/image/matrix/matrix_1,2.png";
        ops << ":/image/matrix/matrix_2,1.png";
        ops << ":/image/matrix/matrix_2,2.png";
        ops << ":/image/matrix/matrix_1,3.png";
        ops << ":/image/matrix/matrix_3,1.png";
        ops << ":/image/matrix/matrix_3,3.png";
        ops << ":/image/matrix/matrix_1,4.png";
        ops << ":/image/matrix/matrix_4,1.png";
        ops << ":/image/matrix/matrix_4,4.png";
        ops << ":/image/matrix/matrix_1,n.png";
        ops << ":/image/matrix/matrix_n,1.png";
        ops << ":/image/matrix/matrix_n,n.png";

        btn = new FmaOpsButton("", ops, 3, toolWidget);
        icon = QPixmap(":/image/matrix/matrix.png");
        btn->setIcon(icon);
        btn->setIconSize(icon.size());
        toolLayout->addWidget(btn, 1, 6);
        toolWidget->addWatch(btn);

        ops.clear();
        ops << ":/image/box/box_01.png";
        ops << ":/image/box/box_02.png";
        ops << ":/image/box/box_03.png";
        ops << ":/image/box/box_04.png";
        ops << ":/image/box/box_05.png";

        ops << ":/image/box/strike_01.png";
        ops << ":/image/box/strike_02.png";
        ops << ":/image/box/strike_03.png";
        ops << ":/image/box/strike_04.png";
        ops << ":/image/box/strike_05.png";

        ops << ":/image/box/longdiv_01.png";
        ops << ":/image/box/longdiv_02.png";

        btn = new FmaOpsButton("", ops, toolWidget);
        icon = QPixmap(":/image/box/box.png");
        btn->setIcon(icon);
        btn->setIconSize(icon.size());
        toolLayout->addWidget(btn, 1, 7);
        toolWidget->addWatch(btn);
    }
    connect(toolWidget,SIGNAL(toOp(QString)),this,SLOT(onOp(QString)));
    return toolBar;
}

void FmaEditView::setCursorVisible(bool visible)
{
    if (cursorVisible == visible){
        return;
    }

    cursorVisible = visible;
    updateCursorBlinking();

    if (cursorVisible){
        connect(qApp->styleHints(), &QStyleHints::cursorFlashTimeChanged, this, &FmaEditView::updateCursorBlinking);
    }
    else{
        disconnect(qApp->styleHints(), &QStyleHints::cursorFlashTimeChanged, this, &FmaEditView::updateCursorBlinking);
    }
}

void FmaEditView::selectAll()
{
    if(docPtr){
        docPtr->math.selectAll();
        update();
    }
}

void FmaEditView::copy()
{
    if(docPtr){
        if(docPtr->math.hasSelect()){
            QVector<int> index = docPtr->math.selectItemIndexes();
            FmaIndexItem iItem = docPtr->math.indexItem(index);
            if(iItem.baseItem){
                QCborMap clipUnit = iItem.baseItem->toCbor(true);
                //qWarning() << clipUnit.toJsonObject();
                //std::cout << QJsonDocument(clipUnit.toJsonObject()).toJson().data();

                QMimeData* clipMime = new QMimeData;
                clipMime->setData(FmaMime::fmaCbor, clipUnit.toCborValue().toCbor());
                FmaClipboard::instance.setMimeData(clipMime);
            }
        }
    }
}

void FmaEditView::cut()
{
    if(docPtr){
        if(docPtr->math.hasSelect()){
            QVector<int> index = docPtr->math.selectItemIndexes();
            FmaIndexItem iItem = docPtr->math.indexItem(index);
            if(iItem.baseItem){
                QCborMap clipUnit = iItem.baseItem->toCbor(true);

                QMimeData* clipMime = new QMimeData;
                clipMime->setData(FmaMime::fmaCbor, clipUnit.toCborValue().toCbor());
                FmaClipboard::instance.setMimeData(clipMime);
            }
            del(true);
        }
    }
}

void FmaEditView::paste(QClipboard::Mode mode)
{
    QCborMap clipUnit;

    if(docPtr){
        //QClipboard
        const QMimeData* mime = FmaClipboard::instance.mimeData();
        if(mime){
            if(mime->hasFormat(FmaMime::fmaCbor)){
                QByteArray data = mime->data(FmaMime::fmaCbor);
                if(!data.isEmpty()){
                    clipUnit = QCborValue::fromCbor(data).toMap();
//                    qWarning() << clipUnit.toJsonObject();
                }
            }else if(mime->hasFormat(FmaMime::fmaEmbedSource)
                     || mime->hasFormat(FmaMime::fmaEmbedObject)){
                QByteArray data = mime->data(FmaMime::fmaEmbedSource);
                if(data.isEmpty()){
                    data = mime->data(FmaMime::fmaEmbedObject);
                }
                if(!data.isEmpty()){
                    clipUnit = QCborValue::fromCbor(data).toMap();
                }
            }
        }


        if(!clipUnit.isEmpty()){
            QVector<int> b = docPtr->math.activeIndexes();
            QList<std::shared_ptr<FmaEditOp>> ops;
            if(docPtr->math.hasSelect()){
                ops.append(docPtr->math.removeSelection());
            }
            ops.append(docPtr->math.doPaste(clipUnit));
            if(!ops.isEmpty()){
                QVector<int> e = docPtr->math.activeIndexes();
                FmaOpsCommand* cmd = new FmaOpsCommand(ops, b, e, docPtr);
                docPtr->undoStack.push(cmd);
                docPtr->math.calculateRect();
                //qWarning() << docPtr->math.toCbor().toJsonObject();
                update();
            }
        }
    }
}

void FmaEditView::insertFormula(const QCborMap &cbor)
{
    if(!docPtr || cbor.isEmpty()){
        return;
    }

    QVector<int> b = docPtr->math.activeIndexes();
    QList<std::shared_ptr<FmaEditOp>> ops;
    if(docPtr->math.hasSelect()){
        ops.append(docPtr->math.removeSelection());
    }
    ops.append(docPtr->math.doPaste(cbor));
    if(!ops.isEmpty()){
        QVector<int> e = docPtr->math.activeIndexes();
        FmaOpsCommand* cmd = new FmaOpsCommand(ops, b, e, docPtr);
        docPtr->undoStack.push(cmd);
        docPtr->math.calculateRect();
        update();
    }
}

void FmaEditView::undo()
{
    if(docPtr){
        if(docPtr->undoStack.canUndo()){
            docPtr->undoStack.undo();
            update();
        }
    }
}

void FmaEditView::redo()
{
    if(docPtr){
        if(docPtr->undoStack.canRedo()){
            docPtr->undoStack.redo();
            update();
        }
    }
}

void FmaEditView::del(bool isForward)
{
    if(docPtr){
        QVector<int> b = docPtr->math.activeIndexes();
        QList<std::shared_ptr<FmaEditOp>> ops = docPtr->math.doDelete(isForward);

        if(!ops.isEmpty()){
            QVector<int> e = docPtr->math.activeIndexes();
            FmaOpsCommand* cmd = new FmaOpsCommand(ops, b, e, docPtr);
            docPtr->undoStack.push(cmd);
            docPtr->math.calculateRect();
        }
    }
}

void FmaEditView::input(const QString &text)
{
    if(docPtr){
        QVector<int> b = docPtr->math.activeIndexes();
        QList<std::shared_ptr<FmaEditOp>> ops;
        if(docPtr->math.hasSelect()){
            ops.append(docPtr->math.doDelete());
        }
        ops.append(docPtr->math.doInput(text));
        if(!ops.isEmpty()){
            QVector<int> e = docPtr->math.activeIndexes();
            FmaOpsCommand* cmd = new FmaOpsCommand(ops, b, e, docPtr);
            docPtr->undoStack.push(cmd);
            docPtr->math.calculateRect();
        }
    }
}

void FmaEditView::setScale(qreal scale)
{
    FmaTextFormats::setBaseFontPixSize(24*scale);
}

qreal FmaEditView::getScale()
{
    return FmaTextFormats::getBaseFontPixSize()/24.0;
}

FmaDoc *FmaEditView::fmaDoc()
{
    return docPtr;
}

void FmaEditView::setFmaDoc(FmaDoc *doc)
{
    docPtr = doc;
    update();
}

void FmaEditView::updateCursorBlinking()
{    
    cursorBlinkTimer.stop();
    if (cursorVisible) {
        int flashTime = QGuiApplication::styleHints()->cursorFlashTime();
        if (flashTime >= 2){
            cursorBlinkTimer.start(flashTime / 2, this);
        }
    }
    if(docPtr){
        docPtr->math.cursorOn = cursorVisible;
        update();
    }
}

void FmaEditView::onOp(const QString &op)
{
    if(docPtr){
        if(op == "matrix_1,n"
                || op == "matrix_n,1"
                || op == "matrix_n,n"){

            QStringList t = op.split("_");
            t = t[1].split(",");

            int row = 1;
            int col = 1;
            if(t[0] == "n"){
                row =4;
            }
            if(t[1] == "n"){
                col =4;
            }

            FmaMatrixInputDlg input;
            input.setRowNum(row);
            input.setColNum(col);
            if(input.exec() == QDialog::Accepted){
                row = input.rowNum();
                col = input.colNum();
                QString tOp = "matrix_"
                        + QString::number(row)
                        +"," + QString::number(col)
                        +"," + QString::number(input.rowSpaceType());
                        +"," + QString::number(input.colSpaceType());
                FmaOpsCommand* cmd = new FmaOpsCommand(tOp, docPtr);
                docPtr->undoStack.push(cmd);
                setFocus();
                update();
            }
        }else{
            FmaOpsCommand* cmd = new FmaOpsCommand(op, docPtr);
            docPtr->undoStack.push(cmd);
            setFocus();
            update();
        }
    }
}

void FmaEditView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(QStringLiteral("#FFFFFF")));

    if(docPtr){
        painter.save();
        painter.translate(leftGap, topGap);
        painter.setClipRegion( event->region() );
        painter.setBackground(QColor(QStringLiteral("#BBD6FF")));
        docPtr->math.paint(&painter);
        painter.restore();
    }
}

void FmaEditView::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == cursorBlinkTimer.timerId()) {
        if(docPtr){
            docPtr->math.cursorOn = !docPtr->math.cursorOn;
            update();
        }
    } else {
        QWidget::timerEvent(e);
    }
}

void FmaEditView::keyPressEvent(QKeyEvent *e)
{
    if(!docPtr){
        return;
    }

    if (e == QKeySequence::SelectAll) {
        e->accept();
        selectAll();
        return;
    }
    else if (e == QKeySequence::Copy) {
        e->accept();
        copy();
        return;
    }

    if (interactionFlags & Qt::TextSelectableByKeyboard && cursorMoveKeyEvent(e)){
        goto accept;
    }

    if (!(interactionFlags & Qt::TextEditable)) {
        e->ignore();
        return;
    }

    if (e->key() == Qt::Key_Backspace) {
        del(false);
        goto accept;
    }

    if (e == QKeySequence::Undo) {
        undo();
    }
    else if (e == QKeySequence::Redo) {
        redo();
    }
    else if (e == QKeySequence::Cut) {
        cut();
    }
    else if (e == QKeySequence::Paste) {
        QClipboard::Mode mode = QClipboard::Clipboard;
        if (QGuiApplication::clipboard()->supportsSelection()) {
            if (e->modifiers() == (Qt::CTRL | Qt::SHIFT) && e->key() == Qt::Key_Insert)
                mode = QClipboard::Selection;
        }
        paste(mode);
    }
    else if (e == QKeySequence::Delete
             || e == QKeySequence::DeleteEndOfWord
             || e == QKeySequence::DeleteEndOfLine){
        del(true);
    }else if(e == QKeySequence::Backspace
             || e == QKeySequence::DeleteStartOfWord) {
        del(false);
    }
    else {
        goto process;
    }
    goto accept;

process:
    {
        if (isAcceptableInput(e)) {
            input(e->text());
        } else {
            e->ignore();
            return;
        }
    }

accept:

    e->accept();
    docPtr->math.cursorOn = true;
    update();
}

void FmaEditView::mousePressEvent(QMouseEvent *e)
{
    if(e->buttons() & Qt::LeftButton){
        mousePressed = (interactionFlags & Qt::TextSelectableByMouse);
        if(docPtr){
            QPointF p = e->position();
            p.setX(p.x()-leftGap);
            p.setY(p.y()-topGap);
            //qWarning() << p;
            docPtr->math.mouseClick(p);
            update();
        }
    }
    QWidget::mousePressEvent(e);
}

void FmaEditView::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->buttons() & Qt::LeftButton) {
        if(mousePressed){
            if(docPtr){
                QPointF p = e->position();
                p.setX(p.x()-leftGap);
                p.setY(p.y()-topGap);
                //docPtr->math.mouseRelese(QPointF(p.x()/scale, p.y()/scale));
                docPtr->math.mouseRelese(p);
                e->accept();
                update();
            }
        }
    }
    mousePressed = false;
    QWidget::mouseReleaseEvent(e);
}

void FmaEditView::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() & Qt::LeftButton) {
        if(mousePressed){
            QPointF p = e->position();
            p.setX(p.x()-leftGap);
            p.setY(p.y()-topGap);
            //docPtr->math.mouseSelectMove(QPointF(p.x()/scale, p.y()/scale));
            docPtr->math.mouseRelese(p);
            e->accept();
            update();
        }
    }
    QWidget::mousePressEvent(e);
}

bool FmaEditView::cursorMoveKeyEvent(QKeyEvent *e)
{
    if(!docPtr){
        return false;
    }

    //qWarning() << "cursorMoveKeyEvent";
    QTextCursor::MoveMode mode = QTextCursor::MoveAnchor;
    QTextCursor::MoveOperation op = QTextCursor::NoMove;

    if (false) {
    }
#ifndef QT_NO_SHORTCUT
    if (e == QKeySequence::MoveToNextChar) {
        op = QTextCursor::Right;
    }
    else if (e == QKeySequence::MoveToPreviousChar) {
        op = QTextCursor::Left;
    }
    else if (e == QKeySequence::SelectNextChar) {
        op = QTextCursor::Right;
        mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectPreviousChar) {
        op = QTextCursor::Left;
        mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectNextWord) {
        op = QTextCursor::WordRight;
        mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectPreviousWord) {
        op = QTextCursor::WordLeft;
        mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectStartOfLine) {
        op = QTextCursor::StartOfLine;
        mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectEndOfLine) {
        op = QTextCursor::EndOfLine;
        mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectStartOfBlock) {
        op = QTextCursor::StartOfBlock;
        mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectEndOfBlock) {
        op = QTextCursor::EndOfBlock;
        mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectStartOfDocument) {
        op = QTextCursor::Start;
        mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectEndOfDocument) {
        op = QTextCursor::End;
        mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectPreviousLine) {
        op = QTextCursor::Up;
        mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectNextLine) {
        op = QTextCursor::Down;
        mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::MoveToNextWord) {
        op = QTextCursor::WordRight;
    }
    else if (e == QKeySequence::MoveToPreviousWord) {
        op = QTextCursor::WordLeft;
    }
    else if (e == QKeySequence::MoveToEndOfBlock) {
        op = QTextCursor::EndOfBlock;
    }
    else if (e == QKeySequence::MoveToStartOfBlock) {
        op = QTextCursor::StartOfBlock;
    }
    else if (e == QKeySequence::MoveToNextLine) {
        op = QTextCursor::Down;
    }
    else if (e == QKeySequence::MoveToPreviousLine) {
        op = QTextCursor::Up;
    }
    else if (e == QKeySequence::MoveToStartOfLine) {
        op = QTextCursor::StartOfLine;
    }
    else if (e == QKeySequence::MoveToEndOfLine) {
        op = QTextCursor::EndOfLine;
    }
    else if (e == QKeySequence::MoveToStartOfDocument) {
        op = QTextCursor::Start;
    }
    else if (e == QKeySequence::MoveToEndOfDocument) {
        op = QTextCursor::End;
    }
#endif // QT_NO_SHORTCUT
    else {
        return false;
    }

    return docPtr->math.movePosition(op, mode);
}

bool FmaEditView::isAcceptableInput(const QKeyEvent *event) const
{
    const QString text = event->text();
    if (text.isEmpty()){
        return false;
    }

    const QChar c = text.at(0);

    // Formatting characters such as ZWNJ, ZWJ, RLM, etc. This needs to go before the
    // next test, since CTRL+SHIFT is sometimes used to input it on Windows.
    if (c.category() == QChar::Other_Format){
        return true;
    }

    // QTBUG-35734: ignore Ctrl/Ctrl+Shift; accept only AltGr (Alt+Ctrl) on German keyboards
    if (event->modifiers() == Qt::ControlModifier
            || event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
        return false;
    }

    if (c.isPrint()){
        return true;
    }

    if (c.category() == QChar::Other_PrivateUse){
        return true;
    }

    if(c == "\r" || c== "\n"){
        return true;
    }
    return false;
}
