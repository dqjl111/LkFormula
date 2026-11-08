#include "fmaaboutdialog.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

FmaAboutDialog::FmaAboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("About LkFormula"));
    setWindowIcon(QApplication::windowIcon());
    setMinimumWidth(520);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel(tr("<b>LkFormula - FMA Equation Editor</b>"));
    title->setWordWrap(true);
    layout->addWidget(title);

    const QString version = QCoreApplication::applicationVersion().isEmpty()
            ? QStringLiteral("0.1")
            : QCoreApplication::applicationVersion();
    QLabel *versionLabel = new QLabel(tr("Version %1").arg(QStringLiteral("v") + version));
    layout->addWidget(versionLabel);

    QLabel *description = new QLabel(tr("Written in C++ / Qt 6. Edits and exports MathML, TeX and WMF formulas, "
                                       "keeps frequently used formulas in a dictionary panel, and supports "
                                       "Chinese / English UI."));
    description->setWordWrap(true);
    layout->addWidget(description);

    QGroupBox *info = new QGroupBox(tr("Author / Contact"));
    QFormLayout *form = new QFormLayout(info);
    auto addRow = [form](const QString &label, const QString &value) {
        QLineEdit *edit = new QLineEdit(value);
        edit->setReadOnly(true);
        form->addRow(label, edit);
    };
    addRow(tr("Organization:"), QStringLiteral("中国人民共同富裕实行研究会"));
    addRow(tr("Bilibili:"), QStringLiteral("实行宫周慧"));
    addRow(tr("Bilibili ID:"), QStringLiteral("bili_94689949015"));
    addRow(tr("QQ Group:"), QStringLiteral("1104274724"));
    layout->addWidget(info);

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addStretch(1);
    QPushButton *ok = new QPushButton(tr("OK"), this);
    buttons->addWidget(ok);
    layout->addLayout(buttons);
    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
}
