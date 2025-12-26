/*
 * plottingdialog4.h
 * 文件作用：曲线属性管理
 * 功能描述：修改已有曲线的名称、数据源列
 */

#ifndef PLOTTINGDIALOG4_H
#define PLOTTINGDIALOG4_H

#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class PlottingDialog4;
}

class PlottingDialog4 : public QDialog
{
    Q_OBJECT

public:
    explicit PlottingDialog4(QStandardItemModel* model, QWidget *parent = nullptr);
    ~PlottingDialog4();

    void setInitialData(const QString& name, int xCol, int yCol);

    QString getCurveName() const;
    int getXColumn() const;
    int getYColumn() const;

private:
    Ui::PlottingDialog4 *ui;
    QStandardItemModel* m_dataModel;
    void populateComboBoxes();
};

#endif // PLOTTINGDIALOG4_H
