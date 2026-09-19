#include "CreateVaultDialog.h"

#include "Vault.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

CreateVaultDialog::CreateVaultDialog(
    Vault& vault,
    QWidget* parent
)
    : QDialog(parent),
      vault_{vault},
      passwordEdit_{new QLineEdit{this}},
      confirmationEdit_{new QLineEdit{this}},
      errorLabel_{new QLabel{this}},
      createButton_{new QPushButton{"Create Vault", this}},
      cancelButton_{new QPushButton{"Cancel", this}}
{
    setWindowTitle("Create Vault");
    setModal(true);
    setMinimumWidth(350);

    passwordEdit_->setEchoMode(QLineEdit::Password);
    confirmationEdit_->setEchoMode(QLineEdit::Password);

    passwordEdit_->setPlaceholderText("Enter master password");
    confirmationEdit_->setPlaceholderText("Confirm master password");

    errorLabel_->setVisible(false);

    auto* passwordLabel =
        new QLabel{"Master password:", this};

    auto* confirmationLabel =
        new QLabel{"Confirm password:", this};

    auto* buttonLayout =
        new QHBoxLayout{};

    buttonLayout->addStretch();
    buttonLayout->addWidget(createButton_);
    buttonLayout->addWidget(cancelButton_);

    auto* layout =
        new QVBoxLayout{this};

    layout->addWidget(passwordLabel);
    layout->addWidget(passwordEdit_);
    layout->addWidget(confirmationLabel);
    layout->addWidget(confirmationEdit_);
    layout->addWidget(errorLabel_);
    layout->addLayout(buttonLayout);

    connect(
        createButton_,
        &QPushButton::clicked,
        this,
        &CreateVaultDialog::handleCreate
    );

    connect(
        cancelButton_,
        &QPushButton::clicked,
        this,
        &CreateVaultDialog::handleCancel
    );

    connect(
        confirmationEdit_,
        &QLineEdit::returnPressed,
        this,
        &CreateVaultDialog::handleCreate
    );

    passwordEdit_->setFocus();
}

void CreateVaultDialog::handleCreate()
{
    const QString password = passwordEdit_->text();
    const QString confirmation = confirmationEdit_->text();

    if (password.isEmpty())
    {
        errorLabel_->setText("Password cannot be empty.");
        errorLabel_->setVisible(true);
        passwordEdit_->setFocus();
        return;
    }

    if (password != confirmation)
    {
        errorLabel_->setText("Passwords do not match.");
        errorLabel_->setVisible(true);
        confirmationEdit_->clear();
        confirmationEdit_->setFocus();
        return;
    }

    vault_.create(password.toStdString());

    accept();
}

void CreateVaultDialog::handleCancel()
{
    reject();
}