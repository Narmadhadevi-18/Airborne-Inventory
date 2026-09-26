#ifndef PARTSMODEL_H
#define PARTSMODEL_H

#include <QSqlQueryModel>

// Wraps the parts query and adds visual alerts (background color) for
// low-stock and expired/expiring-soon rows, plus human-readable headers.
class PartsModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    enum Column {
        ColId = 0,
        ColPartNumber,
        ColCategory,
        ColDescription,
        ColManufacturer,
        ColCageCode,
        ColLotNumber,
        ColQuantity,
        ColMinQuantity,
        ColUnit,
        ColLocation,
        ColDateReceived,
        ColCureDate,
        ColShelfLifeMonths,
        ColCocReference,
        ColNotes,
        ColExpiryDate,
        ColumnCount
    };

    explicit PartsModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int idAtRow(int row) const;

private:
    bool isRowExpired(int row) const;
    bool isRowExpiringSoon(int row) const;
    bool isRowLowStock(int row) const;
};

#endif // PARTSMODEL_H
