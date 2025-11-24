#ifndef FILE_DATA_STORAGE_DOMAIN_H
#define FILE_DATA_STORAGE_DOMAIN_H

#include <algorithm>
#include <optional>

#include <QFile>
#include <QTime>
#include <QJsonDocument>

#include "data_storage_domain.h"

namespace data_storage {

enum class DataFormat {
    TEXT, BINARY
};

struct FileDataStorageSettings : public DataStorageSettings {
    DataFormat data_format;
};

FileDataStorageSettings GetFileDataStorageSettingsFromHashMap(const QHash<QString, QString>& settings_map);

template<typename Container>
std::optional<typename Container::const_iterator> FindLeftTimeBoundIt(const Container& data,
                                                                      const QTime& time_point) {
    if(data.empty())
        return std::nullopt;

    using dataType = typename Container::value_type;

    auto it = std::lower_bound(data.begin(), data.end(),dataType{},
                               [&time_point](const dataType& it_value,
                                             const dataType& value_to_check){
                                   if(it_value.first.time() < time_point) {
                                       return true;
                                   }
                                   return false;
                               });

    if(it == data.end())
        return std::nullopt;

    return it;
}

template<typename Container>
std::optional<typename Container::const_iterator> FindRightTimeBoundIt(const Container& data,
                                                                       const QTime& time_point) {
    if(data.empty())
        return std::nullopt;

    if(data.begin()->first.time() > time_point)
        return std::nullopt;

    using dataType = typename Container::value_type;

    auto it = std::upper_bound(data.begin(), data.end(),dataType{},
                               [&time_point](const dataType& it_value,
                                             const dataType& value_to_check){
                                   if(value_to_check.first.time() > time_point) {
                                       return true;
                                   }
                                   return false;
                               });

    return --it;
}

template<typename Container>
void ReadFromFileToData(QFile& file, Container& data, const QDate& date, bool is_binary = false) {
    if (is_binary) {
        QDataStream read_stream(&file);
        while (!read_stream.atEnd()) {
            QString time_str;
            QString json_data;

            read_stream >> time_str >> json_data;

            if (read_stream.status() != QDataStream::Ok || time_str.isEmpty() || json_data.isEmpty())
                break;

            data.emplaceBack(QDateTime{date, QTime::fromString(time_str, "hh:mm:ss")},
                             QJsonDocument::fromJson(json_data.toUtf8()));
        }
    } else {
        QTextStream read_stream(&file);
        while (!read_stream.atEnd()) {
            QString time_str, json_str;
            read_stream >> time_str >> json_str;
            if (read_stream.status() != QTextStream::Ok || time_str.isEmpty() || json_str.isEmpty())
                break;

            data.emplaceBack(QDateTime{date, QTime::fromString(time_str, "hh:mm:ss")},
                             QJsonDocument::fromJson(json_str.toUtf8()));
        }
    }
}

template<typename Container>
void AppendDataFromFile(QFile& file, Container& data,
                        DataFormat data_format, const QDate& date,
                        const std::optional<const QTime>& from = std::nullopt,
                        const std::optional<const QTime>& to = std::nullopt) {
    if(!from && !to) {
        ReadFromFileToData(file, data, date);
        return;
    }

    Container tmp;
    switch(data_format) {
        case DataFormat::TEXT:
            ReadFromFileToData(file, tmp, date,false);
            break;
        case DataFormat::BINARY:
            ReadFromFileToData(file, tmp, date,true);
            break;
        default:
            Q_ASSERT("data_storage::AppendDataFromFile: wrong data save format");
    }

    if(from && to) {
        auto left = FindLeftTimeBoundIt(tmp,*from);
        auto right = FindRightTimeBoundIt(tmp,*to);

        if(left && right) {
            ++(*right);
            data.append(*left, *right);
        }
        return;
    }

    if(from && !to) {
        auto it = FindLeftTimeBoundIt(tmp,*from);
        if(it)
            data.append(*it,tmp.end());
        return;
    }

    if(!from && to) {
        auto it = FindRightTimeBoundIt(tmp,*to);
        if(it) {
            ++(*it);
            data.append(tmp.begin(),(*it));
        }
        return;
    }
}

}

#endif // FILE_DATA_STORAGE_DOMAIN_H
