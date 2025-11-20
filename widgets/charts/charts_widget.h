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

enum class ChartsMode {
    None, Dynamic, Static
};


class ChartsWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChartsWidget(data_storage::DataStorageInterface
        <QString,QList<QPair<QDateTime,QJsonDocument>>>* storage,
                          const QString&  title = "",
                          QWidget *parent = nullptr);

    ~ChartsWidget();
public slots:
    void BuildStaticChart();
    void StartDynamicChart();
    void StopDynamicShart();

private:
    data_storage::DataStorageInterface
        <QString,QList<QPair<QDateTime,QJsonDocument>>>* storage_;
    QGridLayout* main_layout_{nullptr};
    QVBoxLayout* tools_{nullptr};
    bool is_tools_layout_visible_{true};
    ChartsMode current_mode_{ChartsMode::Static};
    //---- CHARTS
    QChartView chart_view_;
    QChart chart_;
    QHash<QString, QLineSeries*> series_searcher_;
    QValueAxis axis_y_;
    QDateTimeAxis axis_x_;
    //----- TOOLS
    QLineEdit value_name_edit_;
    QPushButton add_value_btn_{"Add value"};
    ValuesListWidget values_list_;
    QLineEdit from_y_edit_;
    QLineEdit to_y_edit_;
    //----- STATIC TOOLS
    QDateTimeEdit from_date_edit_;
    QDateTimeEdit to_date_edit_;
    QPushButton build_btn_{"Build"};
    //----- DYNAMIC TOOLS
    QTimeEdit date_display_period_edit_;
    QLineEdit renewal_period_edit_;
    QPushButton start_btn_{"Start"};
    QPushButton stop_btn_{"Stop"};
    std::unique_ptr<QTimer> update_dynamic_chart_timer_;
    //-----
    void DeleteSeries();
    void SetToolsLayoutUsingMode(ChartsMode mode);
    void SetChart(const QDateTime& from, const QDateTime& to);
    void SetCommonTools();
    void SetStaticTools(const QDateTime& from, const QDateTime& to);
    void SetDynamicTools();
};

#endif // CHARTS_WIDGET_H
