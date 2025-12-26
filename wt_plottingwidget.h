/*
 * wt_plottingwidget.h
 * 文件作用：图表分析主界面头文件
 * 功能描述：
 * 1. 管理左侧工具箱（增删改查曲线）和右侧绘图区域
 * 2. 负责数据导出、图片导出等功能
 * 3. 维护当前所有曲线的信息结构（增加详细的样式记录）
 */

#ifndef WT_PLOTTINGWIDGET_H
#define WT_PLOTTINGWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QMap>
#include <QListWidgetItem>
#include "mousezoom.h"
#include "pressurederivativecalculator.h"

namespace Ui {
class WT_PlottingWidget;
}

// [修改] 曲线信息结构体，支持详细的点线样式
struct CurveInfo {
    QString name;       // 内部ID
    QString legendName; // 图例显示的名称
    int xCol;
    int yCol;
    int y2Col;
    QCPGraph* graphMain;
    QCPGraph* graphSecond;
    bool isDerivative;

    // [新增] 样式属性
    QCPScatterStyle::ScatterShape pointShape;
    QColor pointColor;
    Qt::PenStyle lineStyle;
    QColor lineColor;

    CurveInfo() : xCol(-1), yCol(-1), y2Col(-1), graphMain(nullptr), graphSecond(nullptr), isDerivative(false),
        pointShape(QCPScatterStyle::ssDisc), pointColor(Qt::red), lineStyle(Qt::SolidLine), lineColor(Qt::red) {}
};

class WT_PlottingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WT_PlottingWidget(QWidget *parent = nullptr);
    ~WT_PlottingWidget();

    void setDataModel(QStandardItemModel* model);
    void setProjectPath(const QString& path);

private slots:
    // 左侧功能按钮
    void on_btn_NewCurve_clicked();
    void on_btn_PressureRate_clicked();
    void on_btn_Derivative_clicked();
    void on_btn_Manage_clicked();
    void on_btn_Delete_clicked();
    void on_listWidget_Curves_itemDoubleClicked(QListWidgetItem *item);

    // 右侧底部工具栏
    void on_check_ShowLines_toggled(bool checked);
    void on_btn_ExportData_clicked();
    void on_btn_ChartSettings_clicked();
    void on_btn_ExportImg_clicked();
    void on_btn_FitToData_clicked();

    // 图表点击事件（用于导出选点）
    void onGraphClicked(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event);

private:
    Ui::WT_PlottingWidget *ui;
    QStandardItemModel* m_dataModel;
    QMap<QString, CurveInfo> m_curves; // 以内部名称(ID)为Key
    QString m_projectPath;

    // 导出交互相关变量
    bool m_isSelectingForExport;
    int m_selectionStep;
    double m_exportStartIndex;
    double m_exportEndIndex;
    QCPGraph* m_exportTargetGraph;

    void setupPlotStyle();

    // [修改] 添加曲线到图表，参数增加详细的点线样式
    void addCurveToPlot(const QString& name, int xCol, int yCol, int y2Col = -1, bool isDeriv = false,
                        QCPScatterStyle::ScatterShape pointShape = QCPScatterStyle::ssDisc, QColor pointColor = Qt::red,
                        Qt::PenStyle lineStyle = Qt::SolidLine, QColor lineColor = Qt::red,
                        QString xLabel = "", QString yLabel = "", QString legendName = "");

    QListWidgetItem* getCurrentSelectedItem();
    void executeExport(QCPGraph* graph, bool fullRange, double startKey = 0, double endKey = 0);
};

#endif // WT_PLOTTINGWIDGET_H
