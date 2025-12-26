/*
 * plottingdialog2.h
 * 文件作用：压力产量分析配置对话框头文件
 * 功能描述：
 * 1. 移除了新建窗口勾选框（默认强制新建窗口）
 * 2. 左右分栏设置压力和产量的样式
 */

#ifndef PLOTTINGDIALOG2_H
#define PLOTTINGDIALOG2_H

#include <QDialog>
#include <QStandardItemModel>
#include "qcustomplot.h"

namespace Ui {
class PlottingDialog2;
}

class PlottingDialog2 : public QDialog
{
    Q_OBJECT

public:
    explicit PlottingDialog2(QStandardItemModel* model, QWidget *parent = nullptr);
    ~PlottingDialog2();

    // 压力数据接口
    QString getPressureName() const;
    int getPressureXCol() const;
    int getPressureYCol() const;
    QCPScatterStyle::ScatterShape getPressurePointShape() const;
    QColor getPressurePointColor() const;
    Qt::PenStyle getPressureLineStyle() const;
    QColor getPressureLineColor() const;

    // 产量数据接口
    QString getProductionName() const;
    int getProductionXCol() const;
    int getProductionYCol() const;
    int getProductionGraphType() const; // 0=Step, 1=Scatter, 2=Line
    QColor getProductionColor() const;

private:
    Ui::PlottingDialog2 *ui;
    QStandardItemModel* m_dataModel;

    void populateComboBoxes();
    void setupStyleOptions();
};

#endif // PLOTTINGDIALOG2_H
