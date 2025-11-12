#ifndef DATA_STORAGE_SETTINGS_WIDGET_H
#define DATA_STORAGE_SETTINGS_WIDGET_H

#include <QWidget>

namespace Ui {
class DataStorageSettingsWidget;
}

class DataStorageSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DataStorageSettingsWidget(QWidget *parent = nullptr);
    ~DataStorageSettingsWidget();

    std::optional<QHash<QString, QString>> GetSettings();

private slots:
    void on_data_storage_enable_check_stateChanged(int arg1);
    void OnBrowseDirClicked();

private:
    Ui::DataStorageSettingsWidget *ui;
};

#endif // DATA_STORAGE_SETTINGS_WIDGET_H
