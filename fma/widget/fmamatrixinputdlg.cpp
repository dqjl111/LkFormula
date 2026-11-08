#include "fmamatrixinputdlg.h"
#include "ui_fmamatrixinputdlg.h"
#include "fma/fmatextformats.h"

FmaMatrixInputDlg::FmaMatrixInputDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FmaMatrixInputDlg)
{
    ui->setupUi(this);
    ui->comboRowSpacce->setCurrentIndex(FmaTextFormats::Nonespace);
    ui->comboColSpacce->setCurrentIndex(FmaTextFormats::Veryverythickmathspace);
}

FmaMatrixInputDlg::~FmaMatrixInputDlg()
{
    delete ui;
}

int FmaMatrixInputDlg::rowNum()
{
    return ui->spinRow->value();
}

int FmaMatrixInputDlg::colNum()
{
    return ui->spinCol->value();
}

int FmaMatrixInputDlg::colSpaceType()
{
    if(ui->comboColSpacce->currentIndex()==0){
        return 0;
    }
    if(ui->checkColSpaceSign->isChecked()){
        return -ui->comboColSpacce->currentIndex();
    }
    return ui->comboColSpacce->currentIndex();
}

int FmaMatrixInputDlg::rowSpaceType()
{
    if(ui->comboRowSpacce->currentIndex()==0){
        return 0;
    }
    if(ui->checkRowSpaceSign->isChecked()){
        return -ui->comboRowSpacce->currentIndex();
    }
    return ui->comboRowSpacce->currentIndex();
}

void FmaMatrixInputDlg::setRowNum(int row)
{
    ui->spinRow->setValue(row);
}

void FmaMatrixInputDlg::setColNum(int col)
{
    ui->spinCol->setValue(col);
}

void FmaMatrixInputDlg::on_btnOk_clicked()
{
    accept();
}


void FmaMatrixInputDlg::on_btnCancel_clicked()
{
    reject();
}

