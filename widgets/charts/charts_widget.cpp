#include "charts_widget.h"

ValuesListWidget::ValuesListWidget(QWidget *parent)
    : QListWidget(parent) {
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QListWidget::customContextMenuRequested,
            this, &ValuesListWidget::ShowContextMenu);
}

void ValuesListWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        QListWidgetItem *item = itemAt(event->pos());
        if (item) {
            setCurrentItem(item);
        }
    }
    QListWidget::mousePressEvent(event);
}

void ValuesListWidget::ShowContextMenu(const QPoint &pos) {
    QListWidgetItem *item = itemAt(pos);
    if (!item) return;

    QMenu contextMenu(this);

    QAction* deleteAction = contextMenu.addAction("Delete");
    QAction* selectedAction = contextMenu.exec(viewport()->mapToGlobal(pos));

    if (selectedAction == deleteAction) {
        delete item;
    }
}

ChartsWidget::ChartsWidget(data_storage::DataStorageInterface
                <QString,QList<QPair<QDateTime,QJsonDocument>>>* storage,
                    const QString& title, QWidget *parent)
    :   storage_{storage}
    ,   QWidget{parent} {
    setWindowTitle(title);

    QMenuBar* menuBar = new QMenuBar(this);
    QMenu* fileMenu = menuBar->addMenu("View");
    QAction* show_hide_params = fileMenu->addAction("Show/Hide Parameters");

    //----

    axis_y_.setTitleText("Values");
    axis_y_.setTitleVisible(true);
    axis_y_.setRange(0,1);
    chart_.addAxis(&axis_y_, Qt::AlignLeft);

    axis_x_.setTitleText("Date");
    axis_x_.setTitleVisible(true);
    QDateTime from_date = QDateTime::currentDateTime().addSecs(-3600);
    QDateTime to_date = QDateTime::currentDateTime();
    axis_x_.setRange(from_date, to_date);
    chart_.addAxis(&axis_x_, Qt::AlignBottom);

    chart_view_.setChart(&chart_);

    QGridLayout* main_layout = new QGridLayout;
    main_layout->setMenuBar(menuBar);
    QVBoxLayout* tools = new QVBoxLayout;

    value_name_edit_.setPlaceholderText("Value name");
    tools->addWidget(&value_name_edit_);
    tools->addWidget(&add_value_btn_);
    tools->addWidget(&values_list_);
    connect(&add_value_btn_, &QPushButton::clicked, this, [self = this] () {
        if(!self->value_name_edit_.text().isEmpty()) {
            self->values_list_.addItem(self->value_name_edit_.text());
        }
    });
    tools->addStretch();
    from_y_edit_.setPlaceholderText("Set min Y axes value");
    tools->addWidget(&from_y_edit_);
    to_y_edit_.setPlaceholderText("Set max Y axes value");
    tools->addWidget(&to_y_edit_);
    from_date_edit_.setDateTime(from_date);
    tools->addWidget(&from_date_edit_);
    to_date_edit_.setDateTime(to_date);
    tools->addWidget(&to_date_edit_);
    tools->addWidget(&build_btn_);
    connect(&build_btn_, &QPushButton::clicked, this, &ChartsWidget::BuildChart);
    connect(show_hide_params, &QAction::triggered, this, [self = this,tools]() {
        for (int i = 0; i < tools->count(); ++i) {
            QWidget *w = tools->itemAt(i)->widget();
            if (w) {
                self->is_tools_layout_visible_ ? w->hide() : w->show();
            }
        }
        self->is_tools_layout_visible_ ? self->is_tools_layout_visible_ = false :
            self->is_tools_layout_visible_ = true;
    });

    main_layout->addWidget(&chart_view_,0,0);
    main_layout->setColumnStretch(0,1);
    main_layout->addLayout(tools,0,1);
    setLayout(main_layout);
}

ChartsWidget::~ChartsWidget() {
    DeleteSeries();
}

void ChartsWidget::BuildChart() {
    if(!storage_)
        return;

    DeleteSeries();

    auto data = storage_->DataLoad(from_date_edit_.dateTime(),to_date_edit_.dateTime());

    QHash<QString, QLineSeries*> series;

    for (int i = 0; i < values_list_.count(); ++i) {
        QListWidgetItem* item = values_list_.item(i);
        QString value = item->text();
        series[value] = new QLineSeries;
        series[value]->setPointsVisible(true);
        series[value]->pen().setWidth(2);
        series[value]->setName(value);
    }

    for(auto& data_point : data) {
        QJsonDocument json_doc = std::move(data_point.second);
        QDateTime date_time = std::move(data_point.first);

        QStringList keys = json_doc.object().keys();
        for (const QString& key : keys) {
            if(series.contains(key)) {
                series[key]->append(date_time.toMSecsSinceEpoch(),json_doc[key].toString().toDouble());
            }
        }
    }

    for(auto& line_series : series) {
        chart_.addSeries(line_series);

        line_series->attachAxis(&axis_x_);
        line_series->attachAxis(&axis_y_);
    }

    axis_x_.setRange(from_date_edit_.dateTime(), to_date_edit_.dateTime());
    axis_y_.setRange(from_y_edit_.text().toDouble(), to_y_edit_.text().toDouble());
}

void ChartsWidget::DeleteSeries() {
    QList<QAbstractSeries*> seriesList = chart_.series();
    for (QAbstractSeries* series : seriesList) {
        chart_.removeSeries(series);
        delete series;
    }
}
