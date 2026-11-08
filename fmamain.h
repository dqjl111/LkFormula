#ifndef FMAMAIN_H
#define FMAMAIN_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "fma/widget/fmaeditview.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FmaMain; }
QT_END_NAMESPACE

class QDockWidget;
class FmaDictPanel;

class FmaMain : public QMainWindow
{
    Q_OBJECT
public:
    FmaMain(std::shared_ptr<FmaDoc> docPtr, QWidget *parent = nullptr);
    ~FmaMain();

    std::shared_ptr<FmaDoc> currentDoc();
    void bindShow();
    bool loadFile(const QString &fileName);

signals:
    void bindQuit();
public slots:
    void newFile();
    void save();
    void saveAs();
    void open();

    void setClipboard();
    void onAbout();
protected:
    virtual void dropEvent(QDropEvent *event);
    virtual void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event) override;
protected:
    Ui::FmaMain *ui;
    FmaEditView* mathView;
    std::shared_ptr<FmaDoc> docPtr;
    QString currentFile;

    QDockWidget *formulaDock = nullptr;
    FmaDictPanel *formulaPanel = nullptr;

    void save(QString fileName);

private:
    void applyLanguage(const QString &language);
    void updateLanguageActions();
    void updateWindowTitle();
};
#endif // FMAMAIN_H
