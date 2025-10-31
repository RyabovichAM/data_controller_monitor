#ifndef CHARTS_WIDGET_H
#define CHARTS_WIDGET_H

#include <QWidget>
#include <QtCharts>

#include "data_storage_interface.h"

class ValuesListWidget : public QListWidget {
    Q_OBJECT
public:
    ValuesListWidget(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void ShowContextMenu(const QPoint &pos);
};


class ChartsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChartsWidget(data_storage::DataStorageInterface
        <QString,QList<QPair<QDateTime,QJsonDocument>>>* storage,
                          const QString&  title = "",
                          QWidget *parent = nullptr);

    ~ChartsWidget();
public slots:
    void BuildChart();

private:
    data_storage::DataStorageInterface
        <QString,QList<QPair<QDateTime,QJsonDocument>>>* storage_;
    QChartView chart_view_;
    QChart chart_;
    QValueAxis axis_y_;
    QDateTimeAxis axis_x_;
    QLineEdit value_name_edit_;
    QPushButton add_value_btn_{"Add value"};
    ValuesListWidget values_list_;
    QLineEdit from_y_edit_;
    QLineEdit to_y_edit_;
    QDateTimeEdit from_date_edit_;
    QDateTimeEdit to_date_edit_;
    QPushButton build_btn_{"Build"};
    bool is_tools_layout_visible_{true};

    void DeleteSeries();
};

#endif // CHARTS_WIDGET_H
