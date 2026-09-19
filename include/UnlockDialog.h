#pragma once

#include <QDialog>

class QLineEdit;
class QLabel;
class QPushButton;
class Vault;

class UnlockDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UnlockDialog(
        Vault& vault,
        QWidget* parent = nullptr
    );

    QString password() const;

private slots:
    void handleUnlock();
    void handleCancel();

private:
    Vault& vault_;

    QLineEdit* passwordEdit_;
    QLabel* errorLabel_;
    QPushButton* unlockButton_;
    QPushButton* cancelButton_;
};