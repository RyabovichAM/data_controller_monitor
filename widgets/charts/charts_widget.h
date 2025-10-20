#ifndef CHARTS_WIDGET_H
#define CHARTS_WIDGET_H

#include <QWidget>
#include <QtCharts>

template<typename StorageType>
class ChartsWidget : public QWidget
{
public:
    explicit ChartsWidget(StorageType* storage, QWidget *parent = nullptr);

private:
    StorageType* storage_;
    QChartView chart_view_;
    QChart chart_;
};

template<typename StorageType>
ChartsWidget<StorageType>::ChartsWidget(StorageType* storage, QWidget *parent)
    :   storage_{storage}
    ,   QWidget{parent} {
        // QChartView *chart_view = new QChartView(this);
        // QChart *chart = new QChart();
        // chart->setTitle("График во времени");

        // QLineSeries *series = new QLineSeries();
        // series->setName("Данные");

        // chart->addSeries(series);
        chart_.createDefaultAxes();
        chart_.axes(Qt::Horizontal).first()->setTitleText("Time, с");
        chart_.axes(Qt::Vertical).first()->setTitleText("Values");

        chart_view_.setChart(&chart_);

        QGridLayout* main_layout = new QGridLayout;
        main_layout->addWidget(&chart_view_);
        setLayout(main_layout);
}

#endif // CHARTS_WIDGET_H
