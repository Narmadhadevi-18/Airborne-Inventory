#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QTableView;
class QLineEdit;
class QComboBox;
class QLabel;
class DatabaseManager;
class PartsModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void refreshTable();
    void onAddPart();
    void onEditPart();
    void onDeletePart();
    void onSearchTextChanged(const QString& text);
    void onCategoryFilterChanged(const QString& category);
    void onTableDoubleClicked(const QModelIndex& index);
    void updateStatusSummary();


private:
    void setupUi();
    void setupTable();
    int selectedPartId() const;

    DatabaseManager* m_db;
    PartsModel* m_model;

    QTableView* m_tableView;
    QLineEdit* m_searchEdit;
    QComboBox* m_categoryFilterCombo;
    QLabel* m_statusLabel;
};

#endif // MAINWINDOW_H
