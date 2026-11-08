#ifndef FMAOPSBUTTON_H
#define FMAOPSBUTTON_H

#include <QPushButton>

class FmaOpWidget;

class FmaOpsButton : public QPushButton
{
    Q_OBJECT
public:
    explicit FmaOpsButton(QWidget *parent = nullptr);
    explicit FmaOpsButton(const QString &text, QWidget *parent = nullptr);
    FmaOpsButton(const QIcon& icon, const QString &text, QWidget *parent = nullptr);
    FmaOpsButton(const QString& text,
                      const QStringList& ops,
                      QWidget *parent = nullptr);

    FmaOpsButton(const QString& text,
                      const QStringList& ops,
                      int colNum,
                      QWidget *parent = nullptr);

    void setOps(const QStringList& ops, int colNum=-1);
    bool isOpsVisible();
    void setOpsPopCause(QWidget* w);
    void closePop();
    void openPop();

signals:
    void toOp(const QString& op);

protected slots:
    void onTrigger();
protected:
    FmaOpWidget* opTables = nullptr;

    void setup();
};

#endif // FMAOPSBUTTON_H
