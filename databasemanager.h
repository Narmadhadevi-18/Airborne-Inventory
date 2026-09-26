#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include "part.h"

class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager();

    bool open(const QString& path = "airborne_inventory.db");
    void close();

    bool addPart(const Part& part, QString* errorOut = nullptr);
    bool updatePart(const Part& part, QString* errorOut = nullptr);
    bool deletePart(int id, QString* errorOut = nullptr);

    // Builds a filtered SELECT (with a computed expiry_date column) for the table view.
    QString buildPartsQuery(const QString& searchText, const QString& categoryFilter) const;

    QSqlDatabase database() const { return m_db; }

private:
    QSqlDatabase m_db;
    QString m_connectionName;
    void createSchema();
};

#endif // DATABASEMANAGER_H
