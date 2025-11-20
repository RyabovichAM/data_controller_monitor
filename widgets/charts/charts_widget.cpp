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
    ,   QWidget{parent}
    ,   main_layout_{new QGridLayout}
    ,   tools_{new QVBoxLayout} {
    setWindowTitle(title);

    QMenuBar* menu_bar = new QMenuBar(this);
    main_layout_->setMenuBar(menu_bar);

    QMenu* view_menu = menu_bar->addMenu("View");
    QMenu* mode_menu = menu_bar->addMenu("Mode");

    QAction* show_hide_params = view_menu->addAction("Show/Hide Parameters");
    connect(show_hide_params, &QAction::triggered, this, [self = this, mode_menu]() {
        mode_menu->setEnabled(!self->is_tools_layout_visible_);
        if(!self->is_tools_layout_visible_) {
            switch(self->current_mode_) {
            case ChartsMode::Dynamic:
                self->SetToolsLayoutUsingMode(ChartsMode::Dynamic);
                break;
            case ChartsMode::Static:
                self->SetToolsLayoutUsingMode(ChartsMode::Static);
                break;
            case ChartsMode::None:
            default:
                Q_ASSERT("ChartsWidget::ChartsWidget: do not mode for handler");
            }
        } else {
            for (int i = 0; i < self->tools_->count(); ++i) {
                QWidget *w = self->tools_->itemAt(i)->widget();
                if (w) {
                    w->hide();
                }
            }
        }
        self->is_tools_layout_visible_ ? self->is_tools_layout_visible_ = false :
            self->is_tools_layout_visible_ = true;
    });

    QAction* select_mode = mode_menu->addAction("Change mode");
    connect(select_mode, &QAction::triggered, this, [self = this]() {
        switch(self->current_mode_) {
        case ChartsMode::Dynamic:
            self->SetToolsLayoutUsingMode(ChartsMode::Static);
            self->current_mode_ = ChartsMode::Static;
            break;
        case ChartsMode::Static:
            self->SetToolsLayoutUsingMode(ChartsMode::Dynamic);
            self->current_mode_ = ChartsMode::Dynamic;
            break;
        case ChartsMode::None:
        default:
            Q_ASSERT("ChartsWidget::ChartsWidget: do not mode for handler");
        }
    });

    QDateTime from_date = QDateTime::currentDateTime().addSecs(-3600);
    QDateTime to_date = QDateTime::currentDateTime();

    SetChart(from_date,to_date);

    SetCommonTools();

    SetStaticTools(from_date,to_date);

    SetDynamicTools();

    SetToolsLayoutUsingMode(ChartsMode::Static);

    main_layout_->addWidget(&chart_view_,0,0);
    main_layout_->setColumnStretch(0,1);
    main_layout_->addLayout(tools_,0,1);
    setLayout(main_layout_);
}

ChartsWidget::~ChartsWidget() {
    DeleteSeries();
}

void ChartsWidget::BuildStaticChart() {
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
    series_searcher_.clear();
}

void ChartsWidget::StartDynamicChart() {
    if(!storage_)
        return;

    for (int i = 0; i < tools_->count(); ++i) {
        QLayoutItem *item = tools_->itemAt(i);
        if (item && item->widget()) {
            QWidget *widget = item->widget();
            widget->setDisabled(true);
        }
    }
    stop_btn_.setDisabled(false);

    DeleteSeries();

    axis_y_.setRange(from_y_edit_.text().toDouble(), to_y_edit_.text().toDouble());

    for (int i = 0; i < values_list_.count(); ++i) {
        QListWidgetItem* item = values_list_.item(i);
        QString value = item->text();
        series_searcher_[value] = new QLineSeries;

        chart_.addSeries(series_searcher_[value]);

        series_searcher_[value]->setPointsVisible(true);
        series_searcher_[value]->pen().setWidth(2);
        series_searcher_[value]->setName(value);
        series_searcher_[value]->attachAxis(&axis_x_);
        series_searcher_[value]->attachAxis(&axis_y_);

    }

    update_dynamic_chart_timer_ = std::make_unique<QTimer>();
    update_dynamic_chart_timer_->setInterval(renewal_period_edit_.text().toInt());

    connect(&(*update_dynamic_chart_timer_), &QTimer::timeout, this, [self = this]() {
        QTime period_time = self->date_display_period_edit_.time();
        int period_seconds = period_time.hour() * 3600 + period_time.minute() * 60 + period_time.second();

        QDateTime currentTime = QDateTime::currentDateTime();
        QDateTime pastTime = currentTime.addSecs(-period_seconds);

        auto data = self->storage_->DataLoad(pastTime, currentTime);

        if(!data.empty()) {
            for(auto series : self->chart_.series()) {
                static_cast<QLineSeries*>(series)->clear();
            }
            for(auto& data_point : data) {
                QJsonDocument json_doc = std::move(data_point.second);
                QDateTime date_time = std::move(data_point.first);

                QStringList keys = json_doc.object().keys();
                for (const QString& key : keys) {
                    if(self->series_searcher_.contains(key)) {
                        self->series_searcher_[key]->append(date_time.toMSecsSinceEpoch(),json_doc[key].toString().toDouble());
                    }
                }
            }
        }

        self->axis_x_.setRange(currentTime.addSecs(-period_seconds),
                               currentTime);
        self->chart_.update();
    });
    update_dynamic_chart_timer_->start();
}

void ChartsWidget::StopDynamicShart() {
    for (int i = 0; i < tools_->count(); ++i) {
        QLayoutItem *item = tools_->itemAt(i);
        if (item && item->widget()) {
            QWidget *widget = item->widget();
            widget->setDisabled(false);
        }
    }
}

void ChartsWidget::SetToolsLayoutUsingMode(ChartsMode charts_mode) {
    value_name_edit_.show();
    add_value_btn_.show();
    values_list_.show();
    from_y_edit_.show();
    to_y_edit_.show();

    if(charts_mode == ChartsMode::Dynamic) {
        from_date_edit_.hide();
        to_date_edit_.hide();
        build_btn_.hide();

        date_display_period_edit_.show();
        renewal_period_edit_.show();
        start_btn_.show();
        stop_btn_.show();
    }
    if(charts_mode == ChartsMode::Static) {
        date_display_period_edit_.hide();
        renewal_period_edit_.hide();
        start_btn_.hide();
        stop_btn_.hide();

        from_date_edit_.show();
        to_date_edit_.show();
        build_btn_.show();
    }
}

void ChartsWidget::SetChart(const QDateTime& from, const QDateTime& to) {
    axis_y_.setTitleText("Values");
    axis_y_.setTitleVisible(true);
    axis_y_.setRange(0,1);
    chart_.addAxis(&axis_y_, Qt::AlignLeft);
    axis_x_.setTitleText("Date");
    axis_x_.setTitleVisible(true);
    axis_x_.setRange(from, to);
    chart_.addAxis(&axis_x_, Qt::AlignBottom);
    chart_view_.setChart(&chart_);
}

void ChartsWidget::SetCommonTools() {
    value_name_edit_.setPlaceholderText("Value name");
    connect(&value_name_edit_, &QLineEdit::editingFinished, this, [self = this]() {
        if(!self->value_name_edit_.text().isEmpty()) {
            self->values_list_.addItem(self->value_name_edit_.text());
        }
        self->value_name_edit_.clear();
    });
    connect(&add_value_btn_, &QPushButton::clicked, this, [self = this] () {
        if(!self->value_name_edit_.text().isEmpty()) {
            self->values_list_.addItem(self->value_name_edit_.text());
        }
        self->value_name_edit_.clear();
    });
    to_y_edit_.setPlaceholderText("Set max Y axes value");
    from_y_edit_.setPlaceholderText("Set min Y axes value");

    tools_->addWidget(&value_name_edit_);
    tools_->addWidget(&add_value_btn_);
    tools_->addWidget(&values_list_);
    tools_->addStretch();
    tools_->addWidget(&to_y_edit_);
    tools_->addWidget(&from_y_edit_);
}

void ChartsWidget::SetStaticTools(const QDateTime& from, const QDateTime& to) {
    from_date_edit_.setDateTime(from);
    to_date_edit_.setDateTime(to);
    connect(&build_btn_, &QPushButton::clicked, this, &ChartsWidget::BuildStaticChart);

    tools_->addWidget(&from_date_edit_);
    tools_->addWidget(&to_date_edit_);
    tools_->addWidget(&build_btn_);
}

void ChartsWidget::SetDynamicTools() {
    date_display_period_edit_.setDisplayFormat("hh:mm");
    date_display_period_edit_.setTime(QTime(0, 10));
    date_display_period_edit_.setMinimumTime(QTime(0, 0));
    date_display_period_edit_.setMaximumTime(QTime(23, 59));
    renewal_period_edit_.setPlaceholderText("Set renewal period in sec");
    connect(&start_btn_, &QPushButton::clicked, this, &ChartsWidget::StartDynamicChart);
    connect(&stop_btn_, &QPushButton::clicked, this, &ChartsWidget::StopDynamicShart);

    tools_->addWidget(&date_display_period_edit_);
    tools_->addWidget(&renewal_period_edit_);
    tools_->addWidget(&start_btn_);
    tools_->addWidget(&stop_btn_);
}
