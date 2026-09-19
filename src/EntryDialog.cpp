#include "EntryDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>

EntryDialog::EntryDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Add Password");

    titleEdit_ = new QLineEdit(this);
    usernameEdit_ = new QLineEdit(this);
    passwordEdit_ = new QLineEdit(this);
    urlEdit_ = new QLineEdit(this);
    notesEdit_ = new QTextEdit(this);

    passwordEdit_->setEchoMode(QLineEdit::Password);

    auto* formLayout = new QFormLayout;

    formLayout->addRow("Title:", titleEdit_);
    formLayout->addRow("Username:", usernameEdit_);
    formLayout->addRow("Password:", passwordEdit_);
    formLayout->addRow("URL:", urlEdit_);
    formLayout->addRow("Notes:", notesEdit_);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok |
        QDialogButtonBox::Cancel,
        this
    );

    connect(
        buttons,
        &QDialogButtonBox::accepted,
        this,
        &QDialog::accept
    );

    connect(
        buttons,
        &QDialogButtonBox::rejected,
        this,
        &QDialog::reject
    );

    auto* layout = new QVBoxLayout(this);

    layout->addLayout(formLayout);
    layout->addWidget(buttons);

    resize(450, 300);
}

EntryDialog::EntryDialog(
    const VaultEntry& entry,
    QWidget* parent)
    : EntryDialog(parent)
{
    titleEdit_->setText(
        QString::fromStdString(entry.title)
    );

    usernameEdit_->setText(
        QString::fromStdString(entry.username)
    );

    passwordEdit_->setText(
        QString::fromStdString(entry.password)
    );

    urlEdit_->setText(
        QString::fromStdString(entry.url)
    );

    notesEdit_->setPlainText(
        QString::fromStdString(entry.notes)
    );
}

VaultEntry EntryDialog::entry() const
{
    VaultEntry entry;

    entry.title = titleEdit_->text().toStdString();
    entry.username = usernameEdit_->text().toStdString();
    entry.password = passwordEdit_->text().toStdString();
    entry.url = urlEdit_->text().toStdString();
    entry.notes = notesEdit_->toPlainText().toStdString();

    return entry;
}