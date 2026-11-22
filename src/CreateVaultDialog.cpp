#include "CreateVaultDialog.h"

#include "Vault.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

/**
 * @brief Constructs a CreateVaultDialog with the specified vault and parent widget.
 *
 * @param vault The vault object to be created.
 * @param parent The parent widget of the dialog (default is nullptr).
 *
 * This constructor initializes the dialog with the provided vault reference and
 * sets up the user interface elements for password input, confirmation, and
 * action buttons.
 */
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

/**
 * @brief Handles the creation of the vault when the create button is clicked.
 *
 * This method retrieves the password and confirmation from the input fields,
 * validates them, and attempts to create the vault. If validation fails, it
 * displays an error message. If successful, it accepts the dialog.
 */
void CreateVaultDialog::handleCreate()
{
    qDebug() << "Create Vault button handler entered";
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
    
    qDebug() << "Before vault_.create() call";
    vault_.create(password.toStdString());
    qDebug() << "Vault created successfully.";
    accept();
    qDebug() << "Create Vault dialog accepted.";
}

/**
 * @brief Handles the cancellation of the vault creation when the cancel button is clicked.
 *
 * This method rejects the dialog, effectively canceling the vault creation process.
 */
void CreateVaultDialog::handleCancel()
{
    reject();
}