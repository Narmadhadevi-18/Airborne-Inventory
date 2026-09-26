#ifndef ADDEDITPARTDIALOG_H
#define ADDEDITPARTDIALOG_H

#include <QDialog>
#include "part.h"

class QLineEdit;
class QComboBox;
class QDoubleSpinBox;
class QDateEdit;
class QSpinBox;
class QTextEdit;
class QLabel;

// Modal dialog used for both adding a new part and editing an existing one.
class AddEditPartDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddEditPartDialog(QWidget* parent = nullptr);

    void setPart(const Part& part); // populate fields for editing
    Part part() const;              // read back the entered fields

private slots:
    void onCategoryChanged(const QString& category);
    void validateAndAccept();

private:
    void setupUi();
    void updateShelfLifeFieldsEnabled();

    int m_id = -1;

    QLineEdit* m_partNumberEdit;
    QComboBox* m_categoryCombo;
    QLineEdit* m_descriptionEdit;
    QLineEdit* m_manufacturerEdit;
    QLineEdit* m_cageCodeEdit;
    QLineEdit* m_lotNumberEdit;
    QDoubleSpinBox* m_quantitySpin;
    QDoubleSpinBox* m_minQuantitySpin;
    QComboBox* m_unitCombo;
    QLineEdit* m_locationEdit;
    QDateEdit* m_dateReceivedEdit;
    QDateEdit* m_cureDateEdit;
    QSpinBox* m_shelfLifeSpin;
    QLabel* m_shelfLifeHintLabel;
    QLineEdit* m_cocReferenceEdit;
    QTextEdit* m_notesEdit;
};

#endif // ADDEDITPARTDIALOG_H
