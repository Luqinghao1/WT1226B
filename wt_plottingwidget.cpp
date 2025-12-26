/*
 * 文件名: wt_plottingwidget.cpp
 * 文件作用: 图表分析主界面的逻辑实现
 * 功能描述:
 * 1. 负责初始化主绘图区域的样式（Log-Log双对数坐标系）。
 * 2. 处理用户点击工具栏按钮的事件，如新建曲线、压力产量分析、求导等。
 * 3. 实现数据的导出功能，包括将图表数据保存为CSV文件，支持全部导出和图上选点导出。
 * 4. 管理当前显示的曲线列表和属性。
 */

#include "wt_plottingwidget.h"
#include "ui_wt_plottingwidget.h"
#include "plottingdialog1.h"       // 新建曲线设置弹窗
#include "plottingdialog2.h"       // 压力产量分析设置弹窗
#include "plottingdialog3.h"       // 压力导数设置弹窗
#include "plottingdialog4.h"       // 曲线管理弹窗
#include "plottingsinglewidget.h"  // 单坐标系独立显示窗口
#include "plottingstackwidget.h"   // 双坐标系（压力-产量）独立显示窗口
#include "pressurederivativecalculator1.h" // 导数计算器
#include "chartsetting1.h"         // 图表通用设置
#include "modelparameter.h"        // 模型参数（用于获取工程路径）

#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDir>

// 辅助函数：设置统一的消息弹窗样式（白底黑字）
static void applyMessageBoxStyle(QMessageBox& msgBox) {
    msgBox.setStyleSheet(
        "QMessageBox { background-color: white; color: black; }"
        "QPushButton { color: black; background-color: #f0f0f0; border: 1px solid #555; padding: 5px; min-width: 60px; }"
        "QLabel { color: black; }"
        );
}

// 构造函数：初始化UI和变量
WT_PlottingWidget::WT_PlottingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WT_PlottingWidget),
    m_dataModel(nullptr),
    m_isSelectingForExport(false), // 初始状态下不处于选点模式
    m_selectionStep(0),
    m_exportTargetGraph(nullptr)
{
    ui->setupUi(this);

    // 初始化图表的基本样式
    setupPlotStyle();

    // 连接图表点击信号，用于处理交互式选点（如导出部分数据时的起始/结束点选择）
    connect(ui->customPlot, &QCustomPlot::plottableClick, this, &WT_PlottingWidget::onGraphClicked);
}

// 析构函数：释放UI资源
WT_PlottingWidget::~WT_PlottingWidget()
{
    delete ui;
}

// 设置数据模型，用于获取表格数据
void WT_PlottingWidget::setDataModel(QStandardItemModel* model)
{
    m_dataModel = model;
}

// 设置工程路径，用于文件导出的默认目录
void WT_PlottingWidget::setProjectPath(const QString& path)
{
    m_projectPath = path;
}

// 初始化图表样式：Log-Log坐标系、网格、标签等
void WT_PlottingWidget::setupPlotStyle()
{
    // 添加图表标题
    ui->customPlot->plotLayout()->insertRow(0);
    QCPTextElement* title = new QCPTextElement(ui->customPlot, "试井分析图表", QFont("Microsoft YaHei", 12, QFont::Bold));
    title->setTextColor(Qt::black);
    ui->customPlot->plotLayout()->addElement(0, 0, title);

    // 设置坐标轴为对数刻度（Logarithmic）
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    ui->customPlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->customPlot->xAxis->setTicker(logTicker);
    ui->customPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->customPlot->yAxis->setTicker(logTicker);

    // 设置数字显示格式：科学计数法
    ui->customPlot->xAxis->setNumberFormat("eb");
    ui->customPlot->xAxis->setNumberPrecision(0);
    ui->customPlot->yAxis->setNumberFormat("eb");
    ui->customPlot->yAxis->setNumberPrecision(0);

    // 设置封闭式坐标系（顶部和右侧也显示坐标轴，但不显示刻度值）
    ui->customPlot->xAxis2->setVisible(true);
    ui->customPlot->yAxis2->setVisible(true);
    ui->customPlot->xAxis2->setTickLabels(false);
    ui->customPlot->yAxis2->setTickLabels(false);
    ui->customPlot->xAxis2->setScaleType(QCPAxis::stLogarithmic);
    ui->customPlot->xAxis2->setTicker(logTicker);
    ui->customPlot->yAxis2->setScaleType(QCPAxis::stLogarithmic);
    ui->customPlot->yAxis2->setTicker(logTicker);

    // 连接坐标轴范围变化信号，保持四边坐标轴范围一致
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // 设置网格样式
    ui->customPlot->xAxis->grid()->setVisible(true);
    ui->customPlot->xAxis->grid()->setSubGridVisible(true);
    ui->customPlot->yAxis->grid()->setVisible(true);
    ui->customPlot->yAxis->grid()->setSubGridVisible(true);
    ui->customPlot->xAxis->grid()->setPen(QPen(QColor(220, 220, 220), 1, Qt::SolidLine));
    ui->customPlot->yAxis->grid()->setPen(QPen(QColor(220, 220, 220), 1, Qt::SolidLine));
    ui->customPlot->xAxis->grid()->setSubGridPen(QPen(QColor(240, 240, 240), 1, Qt::DotLine));
    ui->customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(240, 240, 240), 1, Qt::DotLine));

    // 启用图例和交互操作（拖拽、缩放、选择）
    ui->customPlot->legend->setVisible(true);
    ui->customPlot->legend->setBrush(Qt::white);
    ui->customPlot->setBackground(Qt::white);
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    // 设置默认轴标签
    ui->customPlot->xAxis->setLabel("时间 Time");
    ui->customPlot->yAxis->setLabel("压力 Pressure");
}

// 槽函数：点击“新建曲线”按钮
void WT_PlottingWidget::on_btn_NewCurve_clicked()
{
    if(!m_dataModel) return;

    // 弹出设置对话框
    PlottingDialog1 dlg(m_dataModel, this);
    if(dlg.exec() == QDialog::Accepted) {
        // 获取用户设置的参数
        QString name = dlg.getCurveName();
        QString legend = dlg.getLegendName();
        int xCol = dlg.getXColumn();
        int yCol = dlg.getYColumn();
        QString xLabel = dlg.getXLabel();
        QString yLabel = dlg.getYLabel();

        // 获取样式参数（点形状、颜色、线型）
        QCPScatterStyle::ScatterShape pointShape = dlg.getPointShape();
        QColor pointColor = dlg.getPointColor();
        Qt::PenStyle lineStyle = dlg.getLineStyle();
        QColor lineColor = dlg.getLineColor();

        // 从模型中提取数据
        QVector<double> xData, yData;
        for(int i=0; i<m_dataModel->rowCount(); ++i) {
            xData.append(m_dataModel->item(i, xCol)->text().toDouble());
            yData.append(m_dataModel->item(i, yCol)->text().toDouble());
        }

        // 根据选择决定是在新窗口显示还是在当前图表添加
        if(dlg.isNewWindow()) {
            // 创建并显示独立窗口
            PlottingSingleWidget* w = new PlottingSingleWidget();
            w->setProjectPath(m_projectPath);
            w->setWindowTitle(name);
            w->addCurve(legend.isEmpty() ? name : legend, xData, yData,
                        pointShape, pointColor, lineStyle, lineColor,
                        xLabel, yLabel);
            w->show();
        } else {
            // 添加到主界面图表
            addCurveToPlot(name, xCol, yCol, -1, false,
                           pointShape, pointColor, lineStyle, lineColor,
                           xLabel, yLabel, legend);
        }
    }
}

// 槽函数：点击“压力产量分析”按钮
void WT_PlottingWidget::on_btn_PressureRate_clicked()
{
    if(!m_dataModel) return;

    // 弹出压力产量设置对话框
    PlottingDialog2 dlg(m_dataModel, this);
    if(dlg.exec() == QDialog::Accepted) {

        // 提取压力数据
        int pressXCol = dlg.getPressureXCol();
        int pressYCol = dlg.getPressureYCol();
        QVector<double> pressX, pressY;
        for(int i=0; i<m_dataModel->rowCount(); ++i) {
            pressX.append(m_dataModel->item(i, pressXCol)->text().toDouble());
            pressY.append(m_dataModel->item(i, pressYCol)->text().toDouble());
        }

        // 提取产量数据
        int prodXCol = dlg.getProductionXCol();
        int prodYCol = dlg.getProductionYCol();
        QVector<double> prodX, prodY;
        for(int i=0; i<m_dataModel->rowCount(); ++i) {
            prodX.append(m_dataModel->item(i, prodXCol)->text().toDouble());
            prodY.append(m_dataModel->item(i, prodYCol)->text().toDouble());
        }

        // 始终创建新的双坐标系窗口显示分析结果
        PlottingStackWidget* w = new PlottingStackWidget();
        w->setProjectPath(m_projectPath);
        w->setWindowTitle("压力产量分析 - " + dlg.getPressureName());

        // 传递数据和样式配置给新窗口
        w->setData(pressX, pressY, prodX, prodY,
                   dlg.getPressureName(),
                   dlg.getPressurePointShape(), dlg.getPressurePointColor(),
                   dlg.getPressureLineStyle(), dlg.getPressureLineColor(),
                   dlg.getProductionName(),
                   dlg.getProductionGraphType(), dlg.getProductionColor());
        w->show();
    }
}

// 槽函数：点击“压力导数曲线”按钮
void WT_PlottingWidget::on_btn_Derivative_clicked()
{
    if(!m_dataModel) return;
    PlottingDialog3 dlg(m_dataModel, this);
    if(dlg.exec() == QDialog::Accepted) {
        PressureDerivativeConfig config;
        config.timeColumnIndex = dlg.getTimeColumn();
        config.pressureColumnIndex = dlg.getPressureColumn();
        config.lSpacing = dlg.getLSpacing();
        config.autoTimeOffset = true;

        PressureDerivativeResult result;
        // 根据是否选择平滑处理调用不同的计算方法
        if(dlg.isSmoothEnabled()) {
            PressureDerivativeCalculator1 calc;
            result = calc.calculateSmoothedDerivative(m_dataModel, config, dlg.getSmoothFactor());
        } else {
            PressureDerivativeCalculator calc;
            result = calc.calculatePressureDerivative(m_dataModel, config);
        }

        if(result.success) {
            QString name = dlg.getCurveName().isEmpty() ? result.columnName : dlg.getCurveName();
            addCurveToPlot(name, config.timeColumnIndex, result.addedColumnIndex, -1, true);
        } else {
            QMessageBox::warning(this, "错误", "导数计算失败: " + result.errorMessage);
        }
    }
}

// 槽函数：点击“管理”按钮，修改曲线属性
void WT_PlottingWidget::on_btn_Manage_clicked()
{
    QListWidgetItem* item = getCurrentSelectedItem();
    if(!item) return;
    QString name = item->text();
    if(!m_curves.contains(name)) return;

    CurveInfo info = m_curves[name];
    PlottingDialog4 dlg(m_dataModel, this);
    dlg.setInitialData(name, info.xCol, info.yCol);
    if(dlg.exec() == QDialog::Accepted) {
        // 先删除旧曲线
        on_btn_Delete_clicked();
        // 重新添加曲线（此处使用默认样式，如需保留原样式需扩展逻辑）
        addCurveToPlot(dlg.getCurveName(), dlg.getXColumn(), dlg.getYColumn());
    }
}

// 槽函数：点击“删除”按钮
void WT_PlottingWidget::on_btn_Delete_clicked()
{
    QListWidgetItem* item = getCurrentSelectedItem();
    if(!item) return;
    QString name = item->text();
    if(m_curves.contains(name)) {
        // 移除图表中的图形对象
        ui->customPlot->removeGraph(m_curves[name].graphMain);
        if(m_curves[name].graphSecond) ui->customPlot->removeGraph(m_curves[name].graphSecond);
        m_curves.remove(name);
    }
    delete item; // 移除列表项
    ui->customPlot->replot();
}

// 核心函数：向主图表添加一条曲线
void WT_PlottingWidget::addCurveToPlot(const QString& name, int xCol, int yCol, int y2Col, bool isDeriv,
                                       QCPScatterStyle::ScatterShape pointShape, QColor pointColor,
                                       Qt::PenStyle lineStyle, QColor lineColor,
                                       QString xLabel, QString yLabel, QString legendName)
{
    if(m_curves.contains(name)) {
        QMessageBox::warning(this, "警告", "曲线ID已存在，请使用其他名称。");
        return;
    }

    QCPGraph* graph = ui->customPlot->addGraph();
    graph->setName(legendName.isEmpty() ? name : legendName);

    // 提取并设置数据
    QVector<double> xData, yData;
    for(int i=0; i<m_dataModel->rowCount(); ++i) {
        xData.append(m_dataModel->item(i, xCol)->text().toDouble());
        yData.append(m_dataModel->item(i, yCol)->text().toDouble());
    }
    graph->setData(xData, yData);

    // 设置点样式
    graph->setLineStyle(QCPGraph::lsNone); // 默认不显示连线
    QCPScatterStyle ss;
    ss.setShape(pointShape);
    ss.setSize(6);
    ss.setBrush(pointColor);
    ss.setPen(QPen(pointColor));
    graph->setScatterStyle(ss);

    // 设置线样式（记录下来，供“连接数据点”功能使用）
    QPen pen(lineColor);
    pen.setStyle(lineStyle);
    pen.setWidth(2);
    graph->setPen(pen);

    // 设置轴标签
    if(!xLabel.isEmpty()) ui->customPlot->xAxis->setLabel(xLabel);
    if(!yLabel.isEmpty()) ui->customPlot->yAxis->setLabel(yLabel);

    // 保存曲线信息
    CurveInfo info;
    info.name = name;
    info.legendName = legendName;
    info.xCol = xCol;
    info.yCol = yCol;
    info.graphMain = graph;
    info.pointShape = pointShape;
    info.pointColor = pointColor;
    info.lineStyle = lineStyle;
    info.lineColor = lineColor;

    // 双Y轴处理（如果指定了第二列数据）
    if(y2Col != -1) {
        QCPGraph* g2 = ui->customPlot->addGraph(ui->customPlot->xAxis, ui->customPlot->yAxis2);
        g2->setName(name + " (Rate)");
        QVector<double> y2Data;
        for(int i=0; i<m_dataModel->rowCount(); ++i)
            y2Data.append(m_dataModel->item(i, y2Col)->text().toDouble());
        g2->setData(xData, y2Data);
        g2->setLineStyle(QCPGraph::lsNone);
        g2->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangle, 6));
        g2->setPen(QPen(Qt::blue, 2, Qt::DashLine));
        info.graphSecond = g2;
    }

    m_curves.insert(name, info);
    ui->listWidget_Curves->addItem(name);

    // 如果当前勾选了“连接数据点”，则立即应用连线样式
    if(ui->check_ShowLines->isChecked()) {
        graph->setLineStyle(QCPGraph::lsLine);
        if(info.graphSecond) info.graphSecond->setLineStyle(QCPGraph::lsLine);
    }

    // 自动缩放坐标轴以适应数据
    ui->customPlot->rescaleAxes();
    ui->customPlot->replot();
}

// 槽函数：切换“连接数据点”复选框
void WT_PlottingWidget::on_check_ShowLines_toggled(bool checked)
{
    // 遍历所有曲线，切换线型
    for(int i=0; i<ui->customPlot->graphCount(); ++i) {
        ui->customPlot->graph(i)->setLineStyle(checked ? QCPGraph::lsLine : QCPGraph::lsNone);
    }
    ui->customPlot->replot();
}

// 槽函数：点击“适应数据范围”
void WT_PlottingWidget::on_btn_FitToData_clicked()
{
    ui->customPlot->rescaleAxes();
    ui->customPlot->replot();
}

// 槽函数：点击“导出数据”
void WT_PlottingWidget::on_btn_ExportData_clicked()
{
    if(ui->customPlot->graphCount() == 0) {
        QMessageBox msg(this);
        msg.setWindowTitle("提示");
        msg.setText("当前图表中没有曲线，无法导出。");
        msg.setIcon(QMessageBox::Warning);
        applyMessageBoxStyle(msg);
        msg.exec();
        return;
    }

    // 确定要导出的曲线
    QCPGraph* targetGraph = nullptr;
    if(ui->customPlot->selectedGraphs().size() > 0) {
        targetGraph = ui->customPlot->selectedGraphs().first();
    } else {
        targetGraph = ui->customPlot->graph(0);
        QMessageBox msg(this);
        msg.setWindowTitle("提示");
        msg.setText("未选中曲线，默认导出第一条曲线：" + targetGraph->name());
        msg.setIcon(QMessageBox::Information);
        applyMessageBoxStyle(msg);
        msg.exec();
    }
    m_exportTargetGraph = targetGraph;

    // 弹出导出选项对话框
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("导出选项");
    msgBox.setText("请选择导出范围：");
    msgBox.setIcon(QMessageBox::Question);
    applyMessageBoxStyle(msgBox);

    QPushButton* btnAll = msgBox.addButton("全部数据", QMessageBox::ActionRole);
    QPushButton* btnPartial = msgBox.addButton("部分数据", QMessageBox::ActionRole);
    msgBox.addButton("取消", QMessageBox::RejectRole);

    msgBox.exec();

    if(msgBox.clickedButton() == btnAll) {
        // 全部导出
        executeExport(targetGraph, true);
    } else if(msgBox.clickedButton() == btnPartial) {
        // 部分导出：进入选点模式
        m_isSelectingForExport = true;
        m_selectionStep = 1;
        ui->customPlot->setCursor(Qt::CrossCursor); // 改变鼠标为十字光标

        QMessageBox tip(this);
        tip.setWindowTitle("操作提示");
        tip.setText("请在曲线上点击【起始点】。");
        applyMessageBoxStyle(tip);
        tip.exec();
    }
}

// 槽函数：处理图表点击事件（用于交互式选点）
void WT_PlottingWidget::onGraphClicked(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event)
{
    if(!m_isSelectingForExport) return;

    // 只要点击的是曲线对象，就认为是有效的
    QCPGraph* graph = qobject_cast<QCPGraph*>(plottable);
    if(!graph) return;

    double key = graph->dataMainKey(dataIndex); // 获取点击点的时间（X值）

    if(m_selectionStep == 1) {
        // 记录起始点
        m_exportStartIndex = key;
        m_selectionStep = 2;
        QMessageBox tip(this);
        tip.setWindowTitle("操作提示");
        tip.setText("起点已记录。请在曲线上点击【终止点】。");
        applyMessageBoxStyle(tip);
        tip.exec();
    } else if(m_selectionStep == 2) {
        // 记录终止点并执行导出
        m_exportEndIndex = key;
        if(m_exportStartIndex > m_exportEndIndex) std::swap(m_exportStartIndex, m_exportEndIndex);

        m_isSelectingForExport = false;
        m_selectionStep = 0;
        ui->customPlot->setCursor(Qt::ArrowCursor); // 恢复鼠标样式

        executeExport(m_exportTargetGraph, false, m_exportStartIndex, m_exportEndIndex);
    }
}

// 执行导出文件的逻辑
void WT_PlottingWidget::executeExport(QCPGraph* graph, bool fullRange, double startKey, double endKey)
{
    if(!graph) return;

    // 设置默认格式为CSV
    QString filter = "CSV Files (*.csv);;Excel Files (*.xls);;Text Files (*.txt)";

    // 获取默认导出路径
    QString defaultDir = ModelParameter::instance()->getProjectPath();
    if(defaultDir.isEmpty()) defaultDir = QDir::currentPath();
    QString defaultName = defaultDir + "/" + graph->name() + "_export.csv";

    QString fileName = QFileDialog::getSaveFileName(this, "保存数据", defaultName, filter);
    if(fileName.isEmpty()) return;

    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法打开文件进行写入！");
        return;
    }

    QTextStream out(&file);
    QString sep = ",";
    // 根据文件后缀判断分隔符
    if(fileName.endsWith(".txt") || fileName.endsWith(".xls")) sep = "\t";

    // 写入表头
    if (fullRange) {
        // 全部导出：2列 [时间, 数值]
        out << "Time" << sep << "Value" << "\n";
    } else {
        // 部分导出：3列 [调整时间, 数值, 原始时间]
        out << "Adjusted Time" << sep << "Value" << sep << "Original Time" << "\n";
    }

    // 遍历数据并写入
    QSharedPointer<QCPGraphDataContainer> data = graph->data();
    for(auto it = data->begin(); it != data->end(); ++it) {
        double t = it->key;
        double v = it->value;

        if(!fullRange) {
            // 过滤范围外的数据
            if(t < startKey - 1e-9 || t > endKey + 1e-9) continue;
            // 计算调整时间 (t - t0)
            out << (t - startKey) << sep << v << sep << t << "\n";
        } else {
            out << t << sep << v << "\n";
        }
    }

    file.close();
    QMessageBox::information(this, "成功", "数据已导出至:\n" + fileName);
}

// 槽函数：导出图片
void WT_PlottingWidget::on_btn_ExportImg_clicked()
{
    QString defaultDir = ModelParameter::instance()->getProjectPath();
    if(defaultDir.isEmpty()) defaultDir = QDir::currentPath();
    QString defaultName = defaultDir + "/chart.png";

    QString fileName = QFileDialog::getSaveFileName(this, "导出图片", defaultName, "Images (*.png *.jpg *.pdf)");
    if (!fileName.isEmpty()) {
        if(fileName.endsWith(".pdf")) ui->customPlot->savePdf(fileName);
        else ui->customPlot->savePng(fileName);
    }
}

// 槽函数：图表设置
void WT_PlottingWidget::on_btn_ChartSettings_clicked()
{
    QCPLayoutElement* el = ui->customPlot->plotLayout()->element(0,0);
    QCPTextElement* titleElement = qobject_cast<QCPTextElement*>(el);
    ChartSetting1 dlg(ui->customPlot, titleElement, this);
    dlg.exec();
}

// 辅助函数：获取当前列表中选中的项
QListWidgetItem* WT_PlottingWidget::getCurrentSelectedItem()
{
    auto items = ui->listWidget_Curves->selectedItems();
    if(items.isEmpty()) {
        QMessageBox::information(this, "提示", "请先在列表中选择一条曲线");
        return nullptr;
    }
    return items.first();
}

// 槽函数：列表项双击，切换曲线可见性
void WT_PlottingWidget::on_listWidget_Curves_itemDoubleClicked(QListWidgetItem *item)
{
    if(!m_curves.contains(item->text())) return;
    CurveInfo& info = m_curves[item->text()];
    bool visible = !info.graphMain->visible();
    info.graphMain->setVisible(visible);
    if(info.graphSecond) info.graphSecond->setVisible(visible);

    // 灰色表示隐藏，黑色表示显示
    item->setForeground(visible ? Qt::black : Qt::gray);
    ui->customPlot->replot();
}
