#pragma once

#include <QDialog>

class QLineEdit;
class QLabel;
class QPushButton;
class Vault;

class CreateVaultDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateVaultDialog(
        Vault& vault,
        QWidget* parent = nullptr
    );

private slots:
    void handleCreate();
    void handleCancel();

private:
    Vault& vault_;

    QLineEdit* passwordEdit_;
    QLineEdit* confirmationEdit_;
    QLabel* errorLabel_;
    QPushButton* createButton_;
    QPushButton* cancelButton_;
};