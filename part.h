#ifndef PART_H
#define PART_H

#include <QString>
#include <QDate>

// A single item in the store room: a connector, boot, sleeve, backshell, or cable.
struct Part
{
    int id = -1;
    QString partNumber;
    QString category;          // Connector, Boot, Sleeve, Backshell, Cable
    QString description;
    QString manufacturer;
    QString cageCode;          // Commercial and Government Entity code
    QString lotNumber;         // ties stock back to its Certificate of Conformance
    double quantity = 0.0;
    double minQuantity = 0.0;  // reorder threshold
    QString unit = "ea";       // ea, ft, m, pkt ...
    QString location;          // rack / shelf / bin
    QDate dateReceived;
    QDate cureDate;            // only meaningful for Boot / Sleeve (elastomer aging)
    int shelfLifeMonths = 0;   // 0 = no shelf-life limit
    QString cocReference;      // Certificate of Conformance reference
    QString notes;

    QDate expiryDate() const
    {
        if (cureDate.isValid() && shelfLifeMonths > 0)
            return cureDate.addMonths(shelfLifeMonths);
        return QDate();
    }

    bool isLowStock() const { return quantity <= minQuantity; }
};

#endif // PART_H
