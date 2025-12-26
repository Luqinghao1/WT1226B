/*
 * plottingdialog4.cpp
 * 文件作用：曲线属性管理实现
 */

#include "plottingdialog4.h"
#include "ui_plottingdialog4.h"

PlottingDialog4::PlottingDialog4(QStandardItemModel* model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlottingDialog4),
    m_dataModel(model)
{
    ui->setupUi(this);
    populateComboBoxes();
}

PlottingDialog4::~PlottingDialog4() { delete ui; }

void PlottingDialog4::populateComboBoxes() {
    if(!m_dataModel) return;
    QStringList headers;
    for(int i=0; i<m_dataModel->columnCount(); ++i) headers << m_dataModel->horizontalHeaderItem(i)->text();
    ui->combo_XCol->addItems(headers);
    ui->combo_YCol->addItems(headers);
}

void PlottingDialog4::setInitialData(const QString& name, int xCol, int yCol) {
    ui->lineEdit_Name->setText(name);
    if(xCol < ui->combo_XCol->count()) ui->combo_XCol->setCurrentIndex(xCol);
    if(yCol < ui->combo_YCol->count()) ui->combo_YCol->setCurrentIndex(yCol);
}

QString PlottingDialog4::getCurveName() const { return ui->lineEdit_Name->text(); }
int PlottingDialog4::getXColumn() const { return ui->combo_XCol->currentIndex(); }
int PlottingDialog4::getYColumn() const { return ui->combo_YCol->currentIndex(); }
