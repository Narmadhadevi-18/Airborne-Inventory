#include "partsmodel.h"

#include <QDate>
#include <QColor>
#include <QBrush>
#include <QSqlRecord>

PartsModel::PartsModel(QObject* parent) : QSqlQueryModel(parent) {}

int PartsModel::idAtRow(int row) const
{
    return record(row).value("id").toInt();
}

bool PartsModel::isRowExpired(int row) const
{
    QVariant v = record(row).value("expiry_date");
    if (!v.isValid() || v.isNull())
        return false;
    QDate expiry = QDate::fromString(v.toString(), Qt::ISODate);
    return expiry.isValid() && expiry < QDate::currentDate();
}

bool PartsModel::isRowExpiringSoon(int row) const
{
    QVariant v = record(row).value("expiry_date");
    if (!v.isValid() || v.isNull())
        return false;
    QDate expiry = QDate::fromString(v.toString(), Qt::ISODate);
    if (!expiry.isValid())
        return false;
    qint64 daysLeft = QDate::currentDate().daysTo(expiry);
    return daysLeft >= 0 && daysLeft <= 60;
}

bool PartsModel::isRowLowStock(int row) const
{
    double qty = record(row).value("quantity").toDouble();
    double minQty = record(row).value("min_quantity").toDouble();
    return qty <= minQty;
}

QVariant PartsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::BackgroundRole) {
        if (isRowExpired(index.row()))
            return QBrush(QColor(255, 205, 205));   // red   = expired
        if (isRowLowStock(index.row()))
            return QBrush(QColor(255, 229, 180));   // orange = low stock
        if (isRowExpiringSoon(index.row()))
            return QBrush(QColor(255, 250, 190));   // yellow = expiring within 60 days
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        if (index.column() == ColQuantity || index.column() == ColMinQuantity) {
            double val = QSqlQueryModel::data(index, Qt::DisplayRole).toDouble();
            if (val == static_cast<int>(val))
                return QString::number(static_cast<int>(val));
            return QString::number(val, 'f', 2);
        }
        if (index.column() == ColShelfLifeMonths) {
            int months = QSqlQueryModel::data(index, Qt::DisplayRole).toInt();
            return months > 0 ? QString("%1 mo").arg(months) : QString("-");
        }
    }

    return QSqlQueryModel::data(index, role);
}

QVariant PartsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case ColId: return "ID";
        case ColPartNumber: return "Part Number";
        case ColCategory: return "Category";
        case ColDescription: return "Description";
        case ColManufacturer: return "Manufacturer";
        case ColCageCode: return "CAGE Code";
        case ColLotNumber: return "Lot No.";
        case ColQuantity: return "Qty";
        case ColMinQuantity: return "Min Qty";
        case ColUnit: return "Unit";
        case ColLocation: return "Location";
        case ColDateReceived: return "Received";
        case ColCureDate: return "Cure Date";
        case ColShelfLifeMonths: return "Shelf Life";
        case ColCocReference: return "C of C Ref";
        case ColNotes: return "Notes";
        case ColExpiryDate: return "Expiry";
        default: return QVariant();
        }
    }
    return QSqlQueryModel::headerData(section, orientation, role);
}
