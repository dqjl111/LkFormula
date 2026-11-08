#ifndef FMADICTPANEL_H
#define FMADICTPANEL_H

#include <QWidget>
#include <QString>
#include <QByteArray>

class FmaDoc;
class QListWidget;
class QListWidgetItem;
class QPushButton;

// Read-only preview of a formula file, independent from the editor view.
class FmaFormulaPreview : public QWidget
{
public:
    explicit FmaFormulaPreview(QWidget *parent = nullptr);
    ~FmaFormulaPreview() override;

    bool load(const QString &filePath);
    void clear();
    QByteArray formulaData() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    FmaDoc *m_doc;
};

class FmaDictPanel : public QWidget
{
    Q_OBJECT
public:
    explicit FmaDictPanel(FmaDoc *doc, QWidget *parent = nullptr);

    // All formulas are stored as files in this fixed folder next to the executable.
    static QString formulaDir();

public slots:
    void refresh();

signals:
    void insertFormulaRequested(const QByteArray &cbor);

private slots:
    void addFormula();
    void removeFormula();
    void showPreview(QListWidgetItem *item);
    void insertCurrent();

private:
    static void seedExamples();

    FmaDoc *m_doc;
    QListWidget *m_list;
    FmaFormulaPreview *m_preview;
    QPushButton *m_insertButton;
};

#endif // FMADICTPANEL_H
