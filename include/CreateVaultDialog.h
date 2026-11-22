#pragma once

#include <QDialog>

class QLineEdit;
class QLabel;
class QPushButton;
class Vault;

/**
 * @brief The CreateVaultDialog class provides a dialog for creating a new vault.
 *
 * This class allows users to input a password and confirm it to create a new vault.
 * It includes validation to ensure that the password and confirmation match, and
 * provides feedback to the user in case of errors.
 */
class CreateVaultDialog : public QDialog
{
    Q_OBJECT

public:

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
    explicit CreateVaultDialog(
        Vault& vault,
        QWidget* parent = nullptr
    );

private slots:
    // @brief Handles the creation of the vault when the create button is clicked.  
    void handleCreate();
    // @brief Handles the cancellation of the vault creation when the cancel button is clicked.
    void handleCancel();

private:

    Vault& vault_;
  
    QLineEdit* passwordEdit_;
    QLineEdit* confirmationEdit_;
    QLabel* errorLabel_;
    QPushButton* createButton_;
    QPushButton* cancelButton_;
};