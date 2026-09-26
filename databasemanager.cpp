#include "databasemanager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>
#include <QVariant>
#include <QDebug>

DatabaseManager::DatabaseManager()
{
    // Unique connection name so multiple DatabaseManager instances (e.g. in tests)
    // never collide on Qt's global connection registry.
    m_connectionName = QStringLiteral("inventory_conn_%1").arg(QUuid::createUuid().toString());
}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::open(const QString& path)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    createSchema();
    return true;
}

void DatabaseManager::close()
{
    if (m_db.isOpen())
        m_db.close();
    QSqlDatabase::removeDatabase(m_connectionName);
}

void DatabaseManager::createSchema()
{
    QSqlQuery query(m_db);
    query.exec(
        "CREATE TABLE IF NOT EXISTS parts ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "part_number TEXT NOT NULL, "
        "category TEXT NOT NULL, "
        "description TEXT, "
        "manufacturer TEXT, "
        "cage_code TEXT, "
        "lot_number TEXT, "
        "quantity REAL NOT NULL DEFAULT 0, "
        "min_quantity REAL NOT NULL DEFAULT 0, "
        "unit TEXT DEFAULT 'ea', "
        "location TEXT, "
        "date_received TEXT, "
        "cure_date TEXT, "
        "shelf_life_months INTEGER DEFAULT 0, "
        "coc_reference TEXT, "
        "notes TEXT"
        ")"
    );

    query.exec("CREATE INDEX IF NOT EXISTS idx_parts_partnumber ON parts(part_number)");
}

bool DatabaseManager::addPart(const Part& part, QString* errorOut)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO parts (part_number, category, description, manufacturer, cage_code, "
        "lot_number, quantity, min_quantity, unit, location, date_received, cure_date, "
        "shelf_life_months, coc_reference, notes) "
        "VALUES (:part_number, :category, :description, :manufacturer, :cage_code, "
        ":lot_number, :quantity, :min_quantity, :unit, :location, :date_received, :cure_date, "
        ":shelf_life_months, :coc_reference, :notes)"
    );

    query.bindValue(":part_number", part.partNumber);
    query.bindValue(":category", part.category);
    query.bindValue(":description", part.description);
    query.bindValue(":manufacturer", part.manufacturer);
    query.bindValue(":cage_code", part.cageCode);
    query.bindValue(":lot_number", part.lotNumber);
    query.bindValue(":quantity", part.quantity);
    query.bindValue(":min_quantity", part.minQuantity);
    query.bindValue(":unit", part.unit);
    query.bindValue(":location", part.location);
    query.bindValue(":date_received", part.dateReceived.isValid() ? QVariant(part.dateReceived.toString(Qt::ISODate)) : QVariant());
    query.bindValue(":cure_date", part.cureDate.isValid() ? QVariant(part.cureDate.toString(Qt::ISODate)) : QVariant());
    query.bindValue(":shelf_life_months", part.shelfLifeMonths);
    query.bindValue(":coc_reference", part.cocReference);
    query.bindValue(":notes", part.notes);

    if (!query.exec()) {
        if (errorOut) *errorOut = query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::updatePart(const Part& part, QString* errorOut)
{
    QSqlQuery query(m_db);
    query.prepare(
        "UPDATE parts SET part_number = :part_number, category = :category, "
        "description = :description, manufacturer = :manufacturer, cage_code = :cage_code, "
        "lot_number = :lot_number, quantity = :quantity, min_quantity = :min_quantity, "
        "unit = :unit, location = :location, date_received = :date_received, "
        "cure_date = :cure_date, shelf_life_months = :shelf_life_months, "
        "coc_reference = :coc_reference, notes = :notes "
        "WHERE id = :id"
    );

    query.bindValue(":part_number", part.partNumber);
    query.bindValue(":category", part.category);
    query.bindValue(":description", part.description);
    query.bindValue(":manufacturer", part.manufacturer);
    query.bindValue(":cage_code", part.cageCode);
    query.bindValue(":lot_number", part.lotNumber);
    query.bindValue(":quantity", part.quantity);
    query.bindValue(":min_quantity", part.minQuantity);
    query.bindValue(":unit", part.unit);
    query.bindValue(":location", part.location);
    query.bindValue(":date_received", part.dateReceived.isValid() ? QVariant(part.dateReceived.toString(Qt::ISODate)) : QVariant());
    query.bindValue(":cure_date", part.cureDate.isValid() ? QVariant(part.cureDate.toString(Qt::ISODate)) : QVariant());
    query.bindValue(":shelf_life_months", part.shelfLifeMonths);
    query.bindValue(":coc_reference", part.cocReference);
    query.bindValue(":notes", part.notes);
    query.bindValue(":id", part.id);

    if (!query.exec()) {
        if (errorOut) *errorOut = query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::deletePart(int id, QString* errorOut)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM parts WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        if (errorOut) *errorOut = query.lastError().text();
        return false;
    }
    return true;
}

QString DatabaseManager::buildPartsQuery(const QString& searchText, const QString& categoryFilter) const
{
    // expiry_date is computed on the fly from cure_date + shelf_life_months so
    // it never goes stale relative to the stored fields.
    QString sql =
        "SELECT id, part_number, category, description, manufacturer, cage_code, "
        "lot_number, quantity, min_quantity, unit, location, date_received, cure_date, "
        "shelf_life_months, coc_reference, notes, "
        "CASE WHEN cure_date IS NOT NULL AND cure_date != '' AND shelf_life_months > 0 "
        "THEN date(cure_date, '+' || shelf_life_months || ' months') ELSE NULL END AS expiry_date "
        "FROM parts WHERE 1=1";

    if (!categoryFilter.isEmpty() && categoryFilter != "All Categories") {
        QString escaped = categoryFilter;
        escaped.replace("'", "''");
        sql += QString(" AND category = '%1'").arg(escaped);
    }

    if (!searchText.trimmed().isEmpty()) {
        QString escaped = searchText.trimmed();
        escaped.replace("'", "''");
        sql += QString(
            " AND (part_number LIKE '%%1%' OR description LIKE '%%1%' OR "
            "manufacturer LIKE '%%1%' OR lot_number LIKE '%%1%' OR location LIKE '%%1%')"
        ).arg(escaped);
    }

    sql += " ORDER BY part_number ASC";
    return sql;
}
