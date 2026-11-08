#ifndef FMAMATRIXINPUTDLG_H
#define FMAMATRIXINPUTDLG_H

#include <QDialog>

namespace Ui {
class FmaMatrixInputDlg;
}

class FmaMatrixInputDlg : public QDialog
{
    Q_OBJECT

public:
    explicit FmaMatrixInputDlg(QWidget *parent = nullptr);
    ~FmaMatrixInputDlg();

    int rowNum();
    int colNum();
    int rowSpaceType();
    int colSpaceType();

    void setRowNum(int row);
    void setColNum(int col);

private slots:
    void on_btnOk_clicked();

    void on_btnCancel_clicked();

private:
    Ui::FmaMatrixInputDlg *ui;
};

#endif // FMAMATRIXINPUTDLG_H
