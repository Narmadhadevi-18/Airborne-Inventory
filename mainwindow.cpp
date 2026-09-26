#include "mainwindow.h"
#include "databasemanager.h"
#include "partsmodel.h"
#include "addeditpartdialog.h"

#include <QTableView>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDate>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    m_db = new DatabaseManager();
    if (!m_db->open()) {
        QMessageBox::critical(this, "Database Error", "Could not open the inventory database.");
    }

    m_model = new PartsModel(this);

    setupUi();
    setupTable();
    refreshTable();

    setWindowTitle("Airborne Hardware Inventory");
    resize(1200, 650);
}

MainWindow::~MainWindow()
{
    delete m_db;
}

void MainWindow::setupUi()
{
    QWidget* central = new QWidget(this);
    QVBoxLayout* outerLayout = new QVBoxLayout(central);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // --- Header bar ---
    QWidget* headerBar = new QWidget(this);
    headerBar->setObjectName("headerBar");
    QVBoxLayout* headerLayout = new QVBoxLayout(headerBar);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(0);

    QLabel* title = new QLabel("Airborne Hardware Inventory", this);
    title->setObjectName("appTitle");
    QLabel* subtitle = new QLabel("Connectors  ·  Boots  ·  Sleeves  ·  Backshells  ·  Cables", this);
    subtitle->setObjectName("appSubtitle");
    headerLayout->addWidget(title);
    headerLayout->addWidget(subtitle);

    // --- Content area (everything below the header gets margins) ---
    QWidget* content = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(content);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(10);

    QHBoxLayout* toolRow = new QHBoxLayout;
    toolRow->setSpacing(8);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search by part number, description, manufacturer, lot, or location...");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);

    m_categoryFilterCombo = new QComboBox(this);
    m_categoryFilterCombo->addItems({"All Categories", "Connector", "Boot", "Sleeve", "Backshell", "Cable"});
    connect(m_categoryFilterCombo, &QComboBox::currentTextChanged, this, &MainWindow::onCategoryFilterChanged);

    QPushButton* addButton = new QPushButton("+  Add Part", this);
    addButton->setObjectName("addButton");
    QPushButton* editButton = new QPushButton("Edit Part", this);
    editButton->setObjectName("editButton");
    QPushButton* deleteButton = new QPushButton("Delete Part", this);
    deleteButton->setObjectName("deleteButton");
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddPart);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::onEditPart);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeletePart);

    toolRow->addWidget(m_searchEdit, 3);
    toolRow->addWidget(m_categoryFilterCombo, 1);
    toolRow->addWidget(addButton);
    toolRow->addWidget(editButton);
    toolRow->addWidget(deleteButton);

    m_tableView = new QTableView(this);

    QHBoxLayout* legendRow = new QHBoxLayout;
    QLabel* legend = new QLabel(
        "<span style='background-color:#ffcdcd; border-radius:3px; padding:3px 8px;'>Expired</span> &nbsp; "
        "<span style='background-color:#ffe5b4; border-radius:3px; padding:3px 8px;'>Low Stock</span> &nbsp; "
        "<span style='background-color:#fffabe; border-radius:3px; padding:3px 8px;'>Expiring within 60 days</span>", this);
    legend->setObjectName("legendLabel");
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("statusLabel");
    legendRow->addWidget(legend);
    legendRow->addStretch();
    legendRow->addWidget(m_statusLabel);

    mainLayout->addLayout(toolRow);
    mainLayout->addWidget(m_tableView);
    mainLayout->addLayout(legendRow);

    outerLayout->addWidget(headerBar);
    outerLayout->addWidget(content);

    setCentralWidget(central);
}

void MainWindow::setupTable()
{
    m_tableView->setModel(m_model);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->verticalHeader()->setVisible(false);

    connect(m_tableView, &QTableView::doubleClicked, this, &MainWindow::onTableDoubleClicked);
}

void MainWindow::refreshTable()
{
    QString sql = m_db->buildPartsQuery(m_searchEdit->text(), m_categoryFilterCombo->currentText());
    m_model->setQuery(sql, m_db->database());

    // Hide internal/verbose columns after each query (setQuery rebuilds the model).
    m_tableView->setColumnHidden(PartsModel::ColId, true);
    m_tableView->setColumnHidden(PartsModel::ColNotes, true);
    m_tableView->resizeColumnsToContents();

    updateStatusSummary();
}

void MainWindow::updateStatusSummary()
{
    QSqlQuery q(m_db->database());

    q.exec("SELECT COUNT(*) FROM parts");
    int total = q.next() ? q.value(0).toInt() : 0;

    q.exec("SELECT COUNT(*) FROM parts WHERE quantity <= min_quantity");
    int lowStock = q.next() ? q.value(0).toInt() : 0;

    q.exec(
        "SELECT COUNT(*) FROM parts WHERE cure_date IS NOT NULL AND cure_date != '' "
        "AND shelf_life_months > 0 AND date(cure_date, '+' || shelf_life_months || ' months') < date('now')"
    );
    int expired = q.next() ? q.value(0).toInt() : 0;

    m_statusLabel->setText(QString("%1 parts total   |   %2 low stock   |   %3 expired")
                                .arg(total).arg(lowStock).arg(expired));
}

int MainWindow::selectedPartId() const
{
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty())
        return -1;
    return m_model->idAtRow(selected.first().row());
}

void MainWindow::onAddPart()
{
    AddEditPartDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Part p = dialog.part();
        QString error;
        if (!m_db->addPart(p, &error)) {
            QMessageBox::warning(this, "Error Adding Part", error);
            return;
        }

        // Add custom category to filter dropdown if it doesn't exist yet
        if (!p.category.isEmpty() && m_categoryFilterCombo->findText(p.category) == -1) {
            m_categoryFilterCombo->addItem(p.category);
        }

        refreshTable();
    }
}


void MainWindow::onEditPart()
{
    int id = selectedPartId();
    if (id < 0) {
        QMessageBox::information(this, "No Selection", "Please select a part to edit.");
        return;
    }

    QSqlQuery q(m_db->database());
    q.prepare("SELECT * FROM parts WHERE id = :id");
    q.bindValue(":id", id);
    if (!q.exec() || !q.next()) {
        QMessageBox::warning(this, "Error", "Could not load part details.");
        return;
    }

    Part p;
    p.id = q.value("id").toInt();
    p.partNumber = q.value("part_number").toString();
    p.category = q.value("category").toString();
    p.description = q.value("description").toString();
    p.manufacturer = q.value("manufacturer").toString();
    p.cageCode = q.value("cage_code").toString();
    p.lotNumber = q.value("lot_number").toString();
    p.quantity = q.value("quantity").toDouble();
    p.minQuantity = q.value("min_quantity").toDouble();
    p.unit = q.value("unit").toString();
    p.location = q.value("location").toString();
    p.dateReceived = QDate::fromString(q.value("date_received").toString(), Qt::ISODate);
    p.cureDate = QDate::fromString(q.value("cure_date").toString(), Qt::ISODate);
    p.shelfLifeMonths = q.value("shelf_life_months").toInt();
    p.cocReference = q.value("coc_reference").toString();
    p.notes = q.value("notes").toString();

    AddEditPartDialog dialog(this);
    dialog.setPart(p);
    if (dialog.exec() == QDialog::Accepted) {
        Part updated = dialog.part();
        updated.id = id;
        QString error;
        if (!m_db->updatePart(updated, &error)) {
            QMessageBox::warning(this, "Error Updating Part", error);
            return;
        }
        refreshTable();
    }
}

void MainWindow::onDeletePart()
{
    int id = selectedPartId();
    if (id < 0) {
        QMessageBox::information(this, "No Selection", "Please select a part to delete.");
        return;
    }

    if (QMessageBox::question(this, "Confirm Delete", "Delete the selected part? This cannot be undone.")
        != QMessageBox::Yes) {
        return;
    }

    QString error;
    if (!m_db->deletePart(id, &error)) {
        QMessageBox::warning(this, "Error Deleting Part", error);
        return;
    }
    refreshTable();
}

void MainWindow::onSearchTextChanged(const QString& text)
{
    Q_UNUSED(text);
    refreshTable();
}

void MainWindow::onCategoryFilterChanged(const QString& category)
{
    Q_UNUSED(category);
    refreshTable();
}

void MainWindow::onTableDoubleClicked(const QModelIndex& index)
{
    Q_UNUSED(index);
    onEditPart();
}
