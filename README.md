# Airborne Hardware Inventory

A Qt Widgets desktop app for tracking store-room stock of connectors, boots,
sleeves, backshells, and cables. Backed by a local SQLite database (`QSQLITE`,
built into Qt — no extra driver install needed).

## Build (Qt 5.10.x)

**Option A — Qt Creator**
1. Open `AirborneInventory.pro` in Qt Creator.
2. Select a Qt 5.10 kit, click Build, then Run.

**Option B — command line**
```bash
qmake AirborneInventory.pro
make            # or nmake / mingw32-make on Windows
./AirborneInventory
```

On first run it creates `airborne_inventory.db` (SQLite) next to the
executable and sets up the schema automatically — nothing to configure.

## What's tracked per part

- Part Number, Category (Connector / Boot / Sleeve / Backshell / Cable)
- Manufacturer + CAGE code, Lot/Batch number (traceability)
- Quantity on hand + minimum quantity (reorder threshold)
- Location (rack/shelf/bin), unit (ea/ft/m/pkt)
- Cure date + shelf life in months — **only for Boots and Sleeves**, since
  rubber/elastomer parts age. Expiry date is computed automatically.
- Certificate of Conformance reference, free-text notes

## Alerts (row highlighting in the table)

- **Red** — expired (past computed expiry date)
- **Orange** — quantity at or below the minimum threshold
- **Yellow** — expiring within 60 days

The status bar under the table shows running totals: parts, low-stock count,
expired count.

## Structure

| File | Purpose |
|---|---|
| `part.h` | Plain data struct for one inventory item |
| `databasemanager.h/.cpp` | SQLite schema, CRUD, filtered query builder |
| `partsmodel.h/.cpp` | `QSqlQueryModel` subclass — headers + alert coloring |
| `addeditpartdialog.h/.cpp` | Add/Edit form dialog |
| `mainwindow.h/.cpp` | Main window: search, filter, table, toolbar buttons |
| `main.cpp` | Entry point |

## Extending later

- **Check-in/check-out log** — add a `transactions` table (part_id, qty_delta,
  user, timestamp, reason) and a history dialog; quantity becomes derived
  or kept in sync via a trigger.
- **Multi-user login** — add a `users` table + a login dialog before
  `MainWindow` is shown; stamp `transactions.user` for accountability.
- **CSV export** — iterate the current query's records and write with
  `QTextStream`; useful for audit handoffs.
