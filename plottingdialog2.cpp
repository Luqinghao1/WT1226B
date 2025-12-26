/*
 * plottingdialog2.cpp
 * 文件作用：压力产量分析配置对话框实现
 * 功能描述：
 * 1. 初始化下列列表和样式选项
 * 2. 移除了是否新建窗口的逻辑
 */

#include "plottingdialog2.h"
#include "ui_plottingdialog2.h"

PlottingDialog2::PlottingDialog2(QStandardItemModel* model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlottingDialog2),
    m_dataModel(model)
{
    ui->setupUi(this);

    // 设置默认名称
    ui->linePressureName->setText("Pressure Data");
    ui->lineProductionName->setText("Production Data");

    populateComboBoxes();
    setupStyleOptions();
}

PlottingDialog2::~PlottingDialog2()
{
    delete ui;
}

void PlottingDialog2::populateComboBoxes()
{
    if (!m_dataModel) return;
    QStringList headers;
    for(int i=0; i<m_dataModel->columnCount(); ++i) {
        QStandardItem* item = m_dataModel->horizontalHeaderItem(i);
        headers << (item ? item->text() : QString("Column %1").arg(i+1));
    }

    ui->comboPressX->addItems(headers);
    ui->comboPressY->addItems(headers);
    ui->comboProdX->addItems(headers);
    ui->comboProdY->addItems(headers);
}

void PlottingDialog2::setupStyleOptions()
{
    auto addColors = [](QComboBox* box) {
        box->addItem("红色 (Red)", QColor(Qt::red));
        box->addItem("蓝色 (Blue)", QColor(Qt::blue));
        box->addItem("绿色 (Green)", QColor(Qt::darkGreen));
        box->addItem("黑色 (Black)", QColor(Qt::black));
        box->addItem("洋红 (Magenta)", QColor(Qt::magenta));
        box->addItem("青色 (Cyan)", QColor(Qt::cyan));
        box->addItem("灰色 (Gray)", QColor(Qt::gray));
    };

    // 压力样式
    ui->comboPressShape->addItem("实心圆 (Disc)", (int)QCPScatterStyle::ssDisc);
    ui->comboPressShape->addItem("空心圆 (Circle)", (int)QCPScatterStyle::ssCircle);
    ui->comboPressShape->addItem("三角形 (Triangle)", (int)QCPScatterStyle::ssTriangle);
    ui->comboPressShape->addItem("无 (None)", (int)QCPScatterStyle::ssNone);

    ui->comboPressLineStyle->addItem("无 (None)", (int)Qt::NoPen);
    ui->comboPressLineStyle->addItem("实线 (Solid)", (int)Qt::SolidLine);
    ui->comboPressLineStyle->addItem("虚线 (Dash)", (int)Qt::DashLine);

    addColors(ui->comboPressPointColor);
    addColors(ui->comboPressLineColor);
    ui->comboPressPointColor->setCurrentIndex(0); // Red
    ui->comboPressLineStyle->setCurrentIndex(0);  // NoPen

    // 产量样式
    ui->comboProdType->addItem("阶梯图 (Step Chart)", 0);
    ui->comboProdType->addItem("散点图 (Scatter)", 1);
    ui->comboProdType->addItem("折线图 (Line)", 2);

    addColors(ui->comboProdColor);
    ui->comboProdColor->setCurrentIndex(1); // Blue
}

QString PlottingDialog2::getPressureName() const { return ui->linePressureName->text(); }
int PlottingDialog2::getPressureXCol() const { return ui->comboPressX->currentIndex(); }
int PlottingDialog2::getPressureYCol() const { return ui->comboPressY->currentIndex(); }
QCPScatterStyle::ScatterShape PlottingDialog2::getPressurePointShape() const {
    return (QCPScatterStyle::ScatterShape)ui->comboPressShape->currentData().toInt();
}
QColor PlottingDialog2::getPressurePointColor() const {
    return ui->comboPressPointColor->currentData().value<QColor>();
}
Qt::PenStyle PlottingDialog2::getPressureLineStyle() const {
    return (Qt::PenStyle)ui->comboPressLineStyle->currentData().toInt();
}
QColor PlottingDialog2::getPressureLineColor() const {
    return ui->comboPressLineColor->currentData().value<QColor>();
}

QString PlottingDialog2::getProductionName() const { return ui->lineProductionName->text(); }
int PlottingDialog2::getProductionXCol() const { return ui->comboProdX->currentIndex(); }
int PlottingDialog2::getProductionYCol() const { return ui->comboProdY->currentIndex(); }
int PlottingDialog2::getProductionGraphType() const { return ui->comboProdType->currentData().toInt(); }
QColor PlottingDialog2::getProductionColor() const { return ui->comboProdColor->currentData().value<QColor>(); }
