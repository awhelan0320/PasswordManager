#pragma once

#include <QDialog>

#include "VaultEntry.h"

class QLineEdit;
class QTextEdit;

class EntryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EntryDialog(QWidget* parent=nullptr);
    EntryDialog(const VaultEntry& entry, QWidget* parent = nullptr);

    VaultEntry entry() const;

private:
    QLineEdit* titleEdit_;
    QLineEdit* usernameEdit_;
    QLineEdit* passwordEdit_;
    QLineEdit* urlEdit_;
    QTextEdit* notesEdit_;
};