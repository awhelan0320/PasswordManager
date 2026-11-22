#pragma once

#include <QDialog>

#include "VaultEntry.h"

/**
 * @brief The EntryDialog class provides a dialog for creating or editing a VaultEntry.
 */

class QLineEdit;
class QTextEdit;

class EntryDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs an EntryDialog with the specified parent widget.
     *
     * @param parent The parent widget of the dialog (default is nullptr).
     *
     * This constructor initializes the dialog for creating a new VaultEntry, setting up
     * the user interface elements for title, username, password, URL, and notes input,
     * along with action buttons for acceptance or cancellation.
     */
    explicit EntryDialog(QWidget* parent=nullptr);

    /**
     * @brief Constructs an EntryDialog for editing an existing VaultEntry.
     *
     * @param entry The VaultEntry to be edited.
     * @param parent The parent widget of the dialog (default is nullptr).
     *
     * This constructor initializes the dialog with the provided VaultEntry, populating
     * the input fields with the existing data for title, username, password, URL, and
     * notes. It allows users to modify the entry and save changes.
     */
    EntryDialog(const VaultEntry& entry, QWidget* parent = nullptr);

    /**
     * @brief Retrieves the VaultEntry data from the dialog's input fields.
     *
     * @return A VaultEntry structure containing the data entered by the user.
     *
     * This method collects the input from the title, username, password, URL, and notes
     * fields and constructs a VaultEntry object with this information. It is typically
     * called after the dialog is accepted to obtain the user's input for further processing.
     */
    VaultEntry entry() const;

private:
    // UI elements for user input
    QLineEdit* titleEdit_;
    QLineEdit* usernameEdit_;
    QLineEdit* passwordEdit_;
    QLineEdit* urlEdit_;
    QTextEdit* notesEdit_;
};