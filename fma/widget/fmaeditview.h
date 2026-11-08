#ifndef FMAEDITVIEW_H
#define FMAEDITVIEW_H

#include <QWidget>
#include <QBasicTimer>
#include <QClipboard>
#include <QToolBar>
#include "fma/fmadoc.h"
#include "fma/fmamime.h"

class FmaEditView : public QWidget
{
    Q_OBJECT
public:
    explicit FmaEditView(QWidget *parent = nullptr);
    ~FmaEditView();

    QToolBar* createToolBar(QWidget *parent);
    void setCursorVisible(bool visible);

    void setScale(qreal scale);
    qreal getScale();

    FmaDoc* fmaDoc();
    void setFmaDoc(FmaDoc* doc);
    void insertFormula(const QCborMap &cbor);

public slots:
    void updateCursorBlinking();
    void onOp(const QString& op);

    void selectAll();
    void copy();
    void cut();
    void paste(QClipboard::Mode mode = QClipboard::Clipboard);

    void undo();
    void redo();
    void del(bool isForward);
    void input(const QString& text);
signals:
protected:
    virtual void paintEvent(QPaintEvent *) override;
    virtual void timerEvent(QTimerEvent *e) override;
    virtual void keyPressEvent(QKeyEvent *e) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

    bool cursorMoveKeyEvent(QKeyEvent *e);

    bool isAcceptableInput(const QKeyEvent *event) const;
protected:
    FmaDoc* docPtr = nullptr;
    //QCborMap clipUnit;
    int leftGap = 9;
    int topGap = 9;

    QBasicTimer cursorBlinkTimer;
    bool cursorVisible = false;
    Qt::TextInteractionFlags interactionFlags = Qt::TextEditorInteraction;
    bool mousePressed = false;
};

#endif // FORMULAEDITFORM_H
