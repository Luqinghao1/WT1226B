/*
 * plottingdialog3.cpp
 * 文件作用：压力导数计算配置实现
 */

#include "plottingdialog3.h"
#include "ui_plottingdialog3.h"

PlottingDialog3::PlottingDialog3(QStandardItemModel* model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlottingDialog3),
    m_dataModel(model)
{
    ui->setupUi(this);
    ui->lineEdit_Name->setText("Pressure Derivative");
    populateComboBoxes();
    onSmoothToggled(ui->check_Smooth->isChecked());

    connect(ui->check_Smooth, &QCheckBox::toggled, this, &PlottingDialog3::onSmoothToggled);
}

PlottingDialog3::~PlottingDialog3() { delete ui; }

void PlottingDialog3::populateComboBoxes() {
    if(!m_dataModel) return;
    QStringList headers;
    for(int i=0; i<m_dataModel->columnCount(); ++i) headers << m_dataModel->horizontalHeaderItem(i)->text();
    ui->combo_Time->addItems(headers);
    ui->combo_Pressure->addItems(headers);
}

void PlottingDialog3::onSmoothToggled(bool checked) {
    ui->spin_SmoothFactor->setEnabled(checked);
}

QString PlottingDialog3::getCurveName() const { return ui->lineEdit_Name->text(); }
int PlottingDialog3::getTimeColumn() const { return ui->combo_Time->currentIndex(); }
int PlottingDialog3::getPressureColumn() const { return ui->combo_Pressure->currentIndex(); }
bool PlottingDialog3::isSmoothEnabled() const { return ui->check_Smooth->isChecked(); }
int PlottingDialog3::getSmoothFactor() const { return ui->spin_SmoothFactor->value(); }
double PlottingDialog3::getLSpacing() const { return ui->spin_LSpacing->value(); }
bool PlottingDialog3::isNewWindow() const { return ui->check_NewWindow->isChecked(); }
