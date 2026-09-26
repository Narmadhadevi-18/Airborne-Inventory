#include "addeditpartdialog.h"

#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QDateEdit>
#include <QTextEdit>
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>

AddEditPartDialog::AddEditPartDialog(QWidget* parent) : QDialog(parent)
{
    setupUi();
}

void AddEditPartDialog::setupUi()
{
    setWindowTitle("Part Details");
    setMinimumWidth(480);
    setMinimumHeight(360); // Compact vertical layout (reduced from 800)

    // --- Compact Form Controls ---
    m_partNumberEdit = new QLineEdit(this);

    m_lotNumberEdit = new QLineEdit(this);
    m_lotNumberEdit->setPlaceholderText("Serial / Lot #");

    m_manufacturerEdit = new QLineEdit(this);
    m_manufacturerEdit->setPlaceholderText("Company Name");

    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->setEditable(true); // Allows adding future custom items dynamically
    m_categoryCombo->addItems({"Connector", "Boot", "Sleeve", "Backshell", "Cable"});

    m_descriptionEdit = new QLineEdit(this);

    m_quantitySpin = new QDoubleSpinBox(this);
    m_quantitySpin->setRange(0, 1000000);
    m_quantitySpin->setDecimals(2);

    m_minQuantitySpin = new QDoubleSpinBox(this);
    m_minQuantitySpin->setRange(0, 1000000);
    m_minQuantitySpin->setDecimals(2);

    m_unitCombo = new QComboBox(this);
    m_unitCombo->setEditable(true);
    m_unitCombo->addItems({"ea", "ft", "m", "pkt"});

    m_locationEdit = new QLineEdit(this);
    m_locationEdit->setPlaceholderText("e.g. Rack B - Shelf 3 - Bin 12");

    m_cageCodeEdit = new QLineEdit(this);

    m_dateReceivedEdit = new QDateEdit(this);
    m_dateReceivedEdit->setCalendarPopup(true);
    m_dateReceivedEdit->setDate(QDate::currentDate());

    m_cureDateEdit = new QDateEdit(this);
    m_cureDateEdit->setCalendarPopup(true);
    m_cureDateEdit->setDate(QDate::currentDate());

    m_shelfLifeSpin = new QSpinBox(this);
    m_shelfLifeSpin->setRange(0, 240);
    m_shelfLifeSpin->setSuffix(" mos");

    m_cocReferenceEdit = new QLineEdit(this);

    m_notesEdit = new QTextEdit(this);
    m_notesEdit->setFixedHeight(40); // Reduced height text field

    // --- Compact Multi-Column Form Layout ---
    QFormLayout* form = new QFormLayout;
    form->setSpacing(6);

    // Row 1: Part Number & Serial/Lot Number
    QHBoxLayout* row1 = new QHBoxLayout;
    row1->addWidget(m_partNumberEdit, 2);
    row1->addWidget(new QLabel("Serial/Lot:"));
    row1->addWidget(m_lotNumberEdit, 2);
    form->addRow("Part No. *", row1);

    // Row 2: Company Name & Category
    QHBoxLayout* row2 = new QHBoxLayout;
    row2->addWidget(m_manufacturerEdit, 2);
    row2->addWidget(new QLabel("Category:"));
    row2->addWidget(m_categoryCombo, 2);
    form->addRow("Company", row2);

    // Row 3: Description
    form->addRow("Description", m_descriptionEdit);

    // Row 4: Quantity, Unit & Min Stock
    QHBoxLayout* row3 = new QHBoxLayout;
    row3->addWidget(m_quantitySpin, 2);
    row3->addWidget(new QLabel("Unit:"));
    row3->addWidget(m_unitCombo, 1);
    row3->addWidget(new QLabel("Min Stock:"));
    row3->addWidget(m_minQuantitySpin, 1);
    form->addRow("Qty on Hand", row3);

    // Row 5: Location
    form->addRow("Location", m_locationEdit);

    // Row 6: Cure Date & Shelf Life (Horizontal row)
    QHBoxLayout* row4 = new QHBoxLayout;
    row4->addWidget(new QLabel("Cure Date:"));
    row4->addWidget(m_cureDateEdit);
    row4->addWidget(new QLabel("Shelf Life:"));
    row4->addWidget(m_shelfLifeSpin);
    form->addRow("Aging", row4);

    // Row 7: Notes
    form->addRow("Notes", m_notesEdit);

    // --- Action Buttons ---
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setObjectName("dialogOkButton");
    buttons->button(QDialogButtonBox::Ok)->setText("Save");
    buttons->button(QDialogButtonBox::Cancel)->setObjectName("dialogCancelButton");

    connect(buttons, &QDialogButtonBox::accepted, this, &AddEditPartDialog::validateAndAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->addLayout(form);
    mainLayout->addWidget(buttons);

    connect(m_categoryCombo, &QComboBox::currentTextChanged, this, &AddEditPartDialog::onCategoryChanged);
    onCategoryChanged(m_categoryCombo->currentText());
}

void AddEditPartDialog::onCategoryChanged(const QString& category)
{
    bool relevant = (category == "Boot" || category == "Sleeve");
    m_cureDateEdit->setEnabled(relevant);
    m_shelfLifeSpin->setEnabled(relevant);
    if (!relevant) {
        m_shelfLifeSpin->setValue(0);
    }
}

void AddEditPartDialog::validateAndAccept()
{
    if (m_partNumberEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Part Number is required.");
        m_partNumberEdit->setFocus();
        return;
    }
    accept();
}

void AddEditPartDialog::setPart(const Part& p)
{
    m_id = p.id;
    m_partNumberEdit->setText(p.partNumber);

    int catIdx = m_categoryCombo->findText(p.category);
    if (catIdx >= 0) {
        m_categoryCombo->setCurrentIndex(catIdx);
    } else {
        m_categoryCombo->setEditText(p.category); // Dynamically set user's custom category
    }

    m_descriptionEdit->setText(p.description);
    m_manufacturerEdit->setText(p.manufacturer);
    m_cageCodeEdit->setText(p.cageCode);
    m_lotNumberEdit->setText(p.lotNumber);
    m_quantitySpin->setValue(p.quantity);
    m_minQuantitySpin->setValue(p.minQuantity);
    m_unitCombo->setCurrentText(p.unit);
    m_locationEdit->setText(p.location);

    if (p.dateReceived.isValid())
        m_dateReceivedEdit->setDate(p.dateReceived);
    if (p.cureDate.isValid())
        m_cureDateEdit->setDate(p.cureDate);

    m_shelfLifeSpin->setValue(p.shelfLifeMonths);
    m_cocReferenceEdit->setText(p.cocReference);
    m_notesEdit->setPlainText(p.notes);

    onCategoryChanged(m_categoryCombo->currentText());
}

Part AddEditPartDialog::part() const
{
    Part p;
    p.id = m_id;
    p.partNumber = m_partNumberEdit->text().trimmed();
    p.category = m_categoryCombo->currentText().trimmed(); // Reads new/custom typed category text
    p.description = m_descriptionEdit->text().trimmed();
    p.manufacturer = m_manufacturerEdit->text().trimmed();
    p.cageCode = m_cageCodeEdit->text().trimmed();
    p.lotNumber = m_lotNumberEdit->text().trimmed();
    p.quantity = m_quantitySpin->value();
    p.minQuantity = m_minQuantitySpin->value();
    p.unit = m_unitCombo->currentText().trimmed();
    p.location = m_locationEdit->text().trimmed();
    p.dateReceived = m_dateReceivedEdit->date();
    p.cureDate = m_shelfLifeSpin->isEnabled() ? m_cureDateEdit->date() : QDate();
    p.shelfLifeMonths = m_shelfLifeSpin->value();
    p.cocReference = m_cocReferenceEdit->text().trimmed();
    p.notes = m_notesEdit->toPlainText().trimmed();
    return p;
}
