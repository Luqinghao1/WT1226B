/*
 * plottingdialog1.cpp
 * 文件作用：新建曲线配置对话框实现文件
 * 功能描述：
 * 1. 初始化界面，加载数据模型中的列头到下拉框
 * 2. 初始化点形状、颜色、线型的选项
 * 3. 实现点颜色与线颜色的默认联动逻辑
 */

#include "plottingdialog1.h"
#include "ui_plottingdialog1.h"

int PlottingDialog1::s_curveCounter = 1;

PlottingDialog1::PlottingDialog1(QStandardItemModel* model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlottingDialog1),
    m_dataModel(model)
{
    ui->setupUi(this);

    // 1. 设置默认名称
    ui->lineEdit_Name->setText(QString("Curve %1").arg(s_curveCounter++));

    // 2. 初始化下拉框
    populateComboBoxes();
    setupStyleOptions();

    // 3. 信号连接
    connect(ui->combo_XCol, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PlottingDialog1::onXColumnChanged);
    connect(ui->combo_YCol, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PlottingDialog1::onYColumnChanged);

    // 连接点颜色改变信号，用于同步线颜色
    connect(ui->combo_PointColor, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PlottingDialog1::onPointColorChanged);

    // 4. 初始化触发
    if (ui->combo_XCol->count() > 0) onXColumnChanged(ui->combo_XCol->currentIndex());
    if (ui->combo_YCol->count() > 0) onYColumnChanged(ui->combo_YCol->currentIndex());
}

PlottingDialog1::~PlottingDialog1()
{
    delete ui;
}

void PlottingDialog1::populateComboBoxes()
{
    if (!m_dataModel) return;

    QStringList headers;
    for(int i=0; i<m_dataModel->columnCount(); ++i) {
        QStandardItem* item = m_dataModel->horizontalHeaderItem(i);
        headers << (item ? item->text() : QString("Column %1").arg(i+1));
    }

    ui->combo_XCol->addItems(headers);
    ui->combo_YCol->addItems(headers);
}

void PlottingDialog1::setupStyleOptions()
{
    // --- 点形状选项 ---
    ui->combo_PointShape->addItem("实心圆 (Disc)", (int)QCPScatterStyle::ssDisc);
    ui->combo_PointShape->addItem("空心圆 (Circle)", (int)QCPScatterStyle::ssCircle);
    ui->combo_PointShape->addItem("正方形 (Square)", (int)QCPScatterStyle::ssSquare);
    ui->combo_PointShape->addItem("菱形 (Diamond)", (int)QCPScatterStyle::ssDiamond);
    ui->combo_PointShape->addItem("三角形 (Triangle)", (int)QCPScatterStyle::ssTriangle);
    ui->combo_PointShape->addItem("十字 (Cross)", (int)QCPScatterStyle::ssCross);
    ui->combo_PointShape->addItem("加号 (Plus)", (int)QCPScatterStyle::ssPlus);

    // --- 颜色选项 (点和线共用) ---
    // 定义一个添加颜色的lambda辅助函数
    auto addColors = [](QComboBox* box) {
        box->addItem("红色 (Red)", QColor(Qt::red));
        box->addItem("蓝色 (Blue)", QColor(Qt::blue));
        box->addItem("绿色 (Green)", QColor(Qt::darkGreen));
        box->addItem("黑色 (Black)", QColor(Qt::black));
        box->addItem("洋红 (Magenta)", QColor(Qt::magenta));
        box->addItem("青色 (Cyan)", QColor(Qt::cyan));
        box->addItem("灰色 (Gray)", QColor(Qt::gray));
        box->addItem("黄色 (Yellow)", QColor(Qt::yellow));
    };

    addColors(ui->combo_PointColor);
    addColors(ui->combo_LineColor);

    // --- 线型选项 ---
    ui->combo_LineStyle->addItem("实线 (Solid)", (int)Qt::SolidLine);
    ui->combo_LineStyle->addItem("虚线 (Dash)", (int)Qt::DashLine);
    ui->combo_LineStyle->addItem("点线 (Dot)", (int)Qt::DotLine);
    ui->combo_LineStyle->addItem("点划线 (DashDot)", (int)Qt::DashDotLine);
}

void PlottingDialog1::onXColumnChanged(int index)
{
    if (index < 0) return;
    ui->lineEdit_XLabel->setText(ui->combo_XCol->itemText(index));
}

void PlottingDialog1::onYColumnChanged(int index)
{
    if (index < 0) return;
    QString colName = ui->combo_YCol->itemText(index);
    ui->lineEdit_YLabel->setText(colName);
    ui->lineEdit_Legend->setText(colName);
}

void PlottingDialog1::onPointColorChanged(int index)
{
    // 当点颜色改变时，自动将线颜色设置为相同
    // 用户如果之后手动改了线颜色，这里还是会覆盖，符合“默认一致”的逻辑
    if(index >= 0 && index < ui->combo_LineColor->count()) {
        ui->combo_LineColor->setCurrentIndex(index);
    }
}

// Getters
QString PlottingDialog1::getCurveName() const { return ui->lineEdit_Name->text(); }
QString PlottingDialog1::getLegendName() const { return ui->lineEdit_Legend->text(); }
int PlottingDialog1::getXColumn() const { return ui->combo_XCol->currentIndex(); }
int PlottingDialog1::getYColumn() const { return ui->combo_YCol->currentIndex(); }
QString PlottingDialog1::getXLabel() const { return ui->lineEdit_XLabel->text(); }
QString PlottingDialog1::getYLabel() const { return ui->lineEdit_YLabel->text(); }
bool PlottingDialog1::isNewWindow() const { return ui->check_NewWindow->isChecked(); }

// [新增] 获取点样式
QCPScatterStyle::ScatterShape PlottingDialog1::getPointShape() const {
    return (QCPScatterStyle::ScatterShape)ui->combo_PointShape->currentData().toInt();
}
QColor PlottingDialog1::getPointColor() const {
    return ui->combo_PointColor->currentData().value<QColor>();
}

// [新增] 获取线样式
Qt::PenStyle PlottingDialog1::getLineStyle() const {
    return (Qt::PenStyle)ui->combo_LineStyle->currentData().toInt();
}
QColor PlottingDialog1::getLineColor() const {
    return ui->combo_LineColor->currentData().value<QColor>();
}
