#ifndef FMAOPWIDGET_H
#define FMAOPWIDGET_H

//#include <QWidget>
#include <QFrame>
#include <QPixmap>

class QGridLayout;
class FmaOpWidget : public QFrame
{
    Q_OBJECT
public:
    explicit FmaOpWidget(const QStringList& items, int colNum =-1, QWidget *parent = nullptr);
signals:
    void toOp(const QString& op);

protected slots:
    void onBtnClicked();
protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

protected:
    QGridLayout *mainLayout;
    void setOps(const QStringList& items, int colNum = -1);
    bool mouseEventTaken(QMouseEvent *e);
    friend class FmaOpsButton;
    QWidget *popCause = nullptr;
};

#endif // FMAOPWIDGET_H
