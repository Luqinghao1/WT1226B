/*
 * plottingdialog3.h
 * 文件作用：压力导数计算参数配置
 * 功能描述：设置导数计算所需的L-Spacing、平滑因子等参数
 */

#ifndef PLOTTINGDIALOG3_H
#define PLOTTINGDIALOG3_H

#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class PlottingDialog3;
}

class PlottingDialog3 : public QDialog
{
    Q_OBJECT

public:
    explicit PlottingDialog3(QStandardItemModel* model, QWidget *parent = nullptr);
    ~PlottingDialog3();

    QString getCurveName() const;
    int getTimeColumn() const;
    int getPressureColumn() const;
    bool isSmoothEnabled() const;
    int getSmoothFactor() const;
    double getLSpacing() const;
    bool isNewWindow() const;

private:
    Ui::PlottingDialog3 *ui;
    QStandardItemModel* m_dataModel;
    void populateComboBoxes();

private slots:
    void onSmoothToggled(bool checked);
};

#endif // PLOTTINGDIALOG3_H
