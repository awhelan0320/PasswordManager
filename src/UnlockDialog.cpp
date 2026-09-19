#include "UnlockDialog.h"

#include "Vault.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

UnlockDialog::UnlockDialog(
    Vault& vault,
    QWidget* parent
)
    : QDialog(parent),
      vault_{vault},
      passwordEdit_{new QLineEdit{this}},
      errorLabel_{new QLabel{this}},
      unlockButton_{new QPushButton{"Unlock", this}},
      cancelButton_{new QPushButton{"Cancel", this}}
{
    setWindowTitle("Unlock Vault");
    setModal(true);
    setMinimumWidth(350);

    passwordEdit_->setEchoMode(QLineEdit::Password);
    passwordEdit_->setPlaceholderText("Enter master password");

    errorLabel_->setVisible(false);

    auto* passwordLabel =
        new QLabel{"Master password:", this};

    auto* buttonLayout =
        new QHBoxLayout{};

    buttonLayout->addStretch();
    buttonLayout->addWidget(unlockButton_);
    buttonLayout->addWidget(cancelButton_);

    auto* layout =
        new QVBoxLayout{this};

    layout->addWidget(passwordLabel);
    layout->addWidget(passwordEdit_);
    layout->addWidget(errorLabel_);
    layout->addLayout(buttonLayout);

    connect(
        unlockButton_,
        &QPushButton::clicked,
        this,
        &UnlockDialog::handleUnlock
    );

    connect(
        cancelButton_,
        &QPushButton::clicked,
        this,
        &UnlockDialog::handleCancel
    );

    connect(
        passwordEdit_,
        &QLineEdit::returnPressed,
        this,
        &UnlockDialog::handleUnlock
    );

    passwordEdit_->setFocus();
}

QString UnlockDialog::password() const
{
    return passwordEdit_->text();
}

void UnlockDialog::handleUnlock()
{
    const QString enteredPassword = passwordEdit_->text();

    if (enteredPassword.isEmpty())
    {
        errorLabel_->setText("Password cannot be empty.");
        errorLabel_->setVisible(true);
        return;
    }

    const bool success =
        vault_.unlock(enteredPassword.toStdString());

    if (!success)
    {
        errorLabel_->setText("Incorrect master password.");
        errorLabel_->setVisible(true);

        passwordEdit_->clear();
        passwordEdit_->setFocus();

        return;
    }

    // Authentication succeeded.
    accept();
}

void UnlockDialog::handleCancel()
{
    reject();
}