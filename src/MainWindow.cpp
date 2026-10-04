#include "MainWindow.h"

#include "EntryDialog.h"
#include "CreateVaultDialog.h"
#include "UnlockDialog.h"
#include "VaultMetadata.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QTimer>   
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include "AppConfig.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    sqlite_{AppConfig::getInstance().getDatabasePath().toStdString()},
    vault_{sqlite_},
    database_{sqlite_, vault_}
{
    setupUi();

    auto* vaultMenu = menuBar()->addMenu("Vault");

    //auto* lockAction = vaultMenu->addAction("Lock Vault");
    lockVaultAction_ = vaultMenu->addAction("Lock Vault");
    
    connect(
        //lockAction,
        lockVaultAction_,
        &QAction::triggered,
        this,
        &MainWindow::lockVault
    );
    
    if (!initializeVault())
    {
        QTimer::singleShot(0, this, &QWidget::close);
        return;
    }

    updateVaultActions();

    refreshTable();
}

void MainWindow::setupUi()
{
    setWindowTitle("Password Manager");
    resize(800, 500);

    table_ = new QTableWidget(this);

    table_->setColumnCount(3);

    table_->setHorizontalHeaderLabels({
        "Title",
        "Username",
        "URL"
    });

    table_->setColumnWidth(0, 250);
    table_->setColumnWidth(1, 200);
    table_->horizontalHeader()->setStretchLastSection(true);

    table_->verticalHeader()->setVisible(false);

    table_->setSelectionBehavior(
        QAbstractItemView::SelectRows
    );

    table_->setSelectionMode(
        QAbstractItemView::SingleSelection
    );

    table_->setEditTriggers(
        QAbstractItemView::NoEditTriggers
    );

    //auto* addButton = new QPushButton("Add", this);
    addEntryAction_ = menuBar()->addAction("Add Entry");
    updateEntryAction_ = menuBar()->addAction("Update Entry");
    deleteEntryAction_ = menuBar()->addAction("Delete Entry");

    connect(
        addEntryAction_,
        &QAction::triggered,
        this,
        &MainWindow::addEntry
    );

    connect(
        updateEntryAction_,
        &QAction::triggered,
        this,
        &MainWindow::editEntry
    );

    connect(
        deleteEntryAction_,
        &QAction::triggered,
        this,
        &MainWindow::deleteEntry
    );


    auto* buttonLayout = new QHBoxLayout;

    //buttonLayout->addWidget(addButton);
    //buttonLayout->addWidget(editButton);
    //buttonLayout->addWidget(deleteButton);
    auto* layout = new QVBoxLayout;

    layout->addWidget(table_);
    layout->addLayout(buttonLayout);

    auto* centralWidget = new QWidget(this);

    centralWidget->setLayout(layout);

    setCentralWidget(centralWidget);
}


void MainWindow::addEntry()
{
    EntryDialog dialog{this};

    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    VaultEntry entry = dialog.entry();

    try
    {
        database_.add(entry);
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(
            this,
            "Database Error",
            e.what()
        );

        return;
    }

    refreshTable();
}

void MainWindow::editEntry()
{
    const int row = table_->currentRow();

    if (row < 0)
    {
        return;
    }

    const int id =
        table_->item(row, 0)->data(Qt::UserRole).toInt();

    try
    {
        const auto existingEntry = database_.get(id);

        if (!existingEntry)
        {
            return;
        }

        EntryDialog dialog{*existingEntry, this};

        if (dialog.exec() != QDialog::Accepted)
        {
            return;
        }

        VaultEntry entry = dialog.entry();

        entry.id = existingEntry->id;

        database_.update(entry);
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(
            this,
            "Database Error",
            e.what()
        );

        return;
    }

    refreshTable();
}

void MainWindow::deleteEntry()
{
    const int row = table_->currentRow();

    if (row < 0)
    {
        return;
    }

    const int id =
        table_->item(row, 0)->data(Qt::UserRole).toInt();

    const QString title =
        table_->item(row, 0)->text();

    const auto result = QMessageBox::question(
        this,
        "Delete Entry",
        "Are you sure you want to delete \"" +
            title +
            "\"?"
    );

    if (result != QMessageBox::Yes)
    {
        return;
    }

    try
    {
        database_.remove(id);
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(
            this,
            "Database Error",
            e.what()
        );

        return;
    }

    refreshTable();
}

void MainWindow::refreshTable()
{
    const auto entries = database_.getAll();

    table_->setRowCount(
        static_cast<int>(entries.size())
    );

    for (int row = 0;
         row < static_cast<int>(entries.size());
         ++row)
    {
        const auto& entry = entries[row];

        auto* titleItem = new QTableWidgetItem(
            QString::fromStdString(entry.title)
        );

        titleItem->setData(
            Qt::UserRole,
            entry.id
        );

        table_->setItem(
            row,
            0,
            titleItem
        );

        table_->setItem(
            row,
            1,
            new QTableWidgetItem(
                QString::fromStdString(entry.username)
            )
        );

        table_->setItem(
            row,
            2,
            new QTableWidgetItem(
                QString::fromStdString(entry.url)
            )
        );
    }
}

bool MainWindow::initializeVault()
{
    VaultMetadata metadata{sqlite_};
    metadata.initialize();

    if (!metadata.exists())
    {
        CreateVaultDialog dialog{vault_, this};

        if (dialog.exec() != QDialog::Accepted)
            return false;

        return vault_.isUnlocked();
    }

    UnlockDialog dialog{vault_, this};

    if (dialog.exec() != QDialog::Accepted)
        return false;

    return vault_.isUnlocked();
}

void MainWindow::lockVault()
{
    vault_.lock();

    table_->setRowCount(0);

    updateVaultActions();

    UnlockDialog dialog{vault_, this};

    if (dialog.exec() == QDialog::Accepted)
    {
        updateVaultActions();
        refreshTable();
    }
    else
    {
        close();
    }
}

void MainWindow::updateVaultActions()
{
    const bool unlocked = vault_.isUnlocked();

    lockVaultAction_->setEnabled(unlocked);
    addEntryAction_->setEnabled(unlocked);
    updateEntryAction_->setEnabled(unlocked);
    deleteEntryAction_->setEnabled(unlocked);
}