#pragma once

#include <QMainWindow>

#include "VaultDatabase.h"
#include "Vault.h"

class QTableWidget;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);


private:
    bool initializeVault();
    void setupUi();
    void addEntry();
    void editEntry();
    void deleteEntry();
    void refreshTable();
    void lockVault();

    SqliteDatabase sqlite_;
    Vault vault_;
    VaultDatabase database_;
    QTableWidget* table_;

    void updateVaultActions();
    QAction* lockVaultAction_;
    QAction* addEntryAction_;
    QAction* updateEntryAction_;
    QAction* deleteEntryAction_;
};