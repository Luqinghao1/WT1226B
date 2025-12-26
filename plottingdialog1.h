/*
 * plottingdialog1.h
 * 文件作用：新建曲线配置对话框头文件
 * 功能描述：
 * 1. 提供X轴、Y轴数据列的选择
 * 2. 自动根据选择的列名填充坐标轴标签和图例名称
 * 3. [修改] 提供独立的“点样式”和“线样式”设置
 * - 点样式：形状、颜色
 * - 线样式：线型、颜色
 * 4. 样式设置为白底黑字，符合统一样式要求
 */

#ifndef PLOTTINGDIALOG1_H
#define PLOTTINGDIALOG1_H

#include <QDialog>
#include <QStandardItemModel>
#include "qcustomplot.h" // 需要用到 QCPScatterStyle

namespace Ui {
class PlottingDialog1;
}

class PlottingDialog1 : public QDialog
{
    Q_OBJECT

public:
    explicit PlottingDialog1(QStandardItemModel* model, QWidget *parent = nullptr);
    ~PlottingDialog1();

    // Getter方法：获取用户配置的参数
    QString getCurveName() const;   // 获取内部管理用的唯一名称
    QString getLegendName() const;  // 获取图例显示的名称
    int getXColumn() const;         // 获取X轴数据列索引
    int getYColumn() const;         // 获取Y轴数据列索引
    QString getXLabel() const;      // 获取X轴标签
    QString getYLabel() const;      // 获取Y轴标签

    // [新增] 获取点样式参数
    QCPScatterStyle::ScatterShape getPointShape() const;
    QColor getPointColor() const;

    // [新增] 获取线样式参数
    Qt::PenStyle getLineStyle() const;
    QColor getLineColor() const;

    bool isNewWindow() const;       // 是否在新窗口显示

private slots:
    // 槽函数：当数据列改变时，自动更新标签和图例
    void onXColumnChanged(int index);
    void onYColumnChanged(int index);

    // [新增] 当点颜色改变时，同步修改线颜色（如果用户未手动修改过）
    void onPointColorChanged(int index);

private:
    Ui::PlottingDialog1 *ui;
    QStandardItemModel* m_dataModel;
    static int s_curveCounter; // 静态计数器

    void populateComboBoxes(); // 初始化数据列下拉框
    void setupStyleOptions();  // [修改] 初始化点和线的样式选项
};

#endif // PLOTTINGDIALOG1_H
