#include "fmadictpanel.h"
#include "fma/fmadoc.h"

#include <QCborValue>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

QString FmaDictPanel::formulaDir()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("formulas"));
}

void FmaDictPanel::seedExamples()
{
    QDir dir(formulaDir());
    if (!dir.exists() && !dir.mkpath(dir.absolutePath()))
        return;
    if (dir.exists(QStringLiteral(".seeded")))
        return;

    struct SeedFile {
        const char *resource;
        const char *target;
    };
    static const SeedFile seeds[] = {
        { "circle-area.mml", "圆面积.mml" }
    };
    for (const SeedFile &seed : seeds) {
        const QString target = dir.filePath(QString::fromUtf8(seed.target));
        if (!QFile::exists(target)) {
            QFile::copy(QStringLiteral(":/formulas/%1").arg(QString::fromLatin1(seed.resource)),
                        target);
        }
    }

    QFile flag(dir.filePath(QStringLiteral(".seeded")));
    if (flag.open(QIODevice::WriteOnly | QIODevice::Truncate))
        flag.close();
}

FmaFormulaPreview::FmaFormulaPreview(QWidget *parent)
    : QWidget(parent)
    , m_doc(new FmaDoc)
{
    setMinimumHeight(90);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
}

FmaFormulaPreview::~FmaFormulaPreview()
{
    delete m_doc;
}

bool FmaFormulaPreview::load(const QString &filePath)
{
    delete m_doc;
    m_doc = new FmaDoc;

    bool ok = false;
    if (!filePath.isEmpty()) {
        const QString suffix = QFileInfo(filePath).suffix().toLower();
        if (suffix == QLatin1String("mml"))
            ok = m_doc->loadMml(filePath);
        else if (suffix == QLatin1String("fma"))
            ok = m_doc->loadCbor(filePath);
        else if (suffix == QLatin1String("json"))
            ok = m_doc->loadJson(filePath);
    }
    if (ok)
        m_doc->math.calculateRect();
    update();
    return ok;
}

void FmaFormulaPreview::clear()
{
    delete m_doc;
    m_doc = new FmaDoc;
    update();
}

QByteArray FmaFormulaPreview::formulaData() const
{
    return m_doc->math.toCbor().toCborValue().toCbor();
}

void FmaFormulaPreview::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(QStringLiteral("#FFFFFF")));
    if (m_doc->math.fullWidth() > 0 && m_doc->math.fullHeight() > 0) {
        painter.translate(8, 8);
        m_doc->math.paintChilds(&painter);
    }
}

FmaDictPanel::FmaDictPanel(FmaDoc *doc, QWidget *parent)
    : QWidget(parent)
    , m_doc(doc)
{
    seedExamples();

    m_list = new QListWidget(this);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);

    m_preview = new FmaFormulaPreview(this);

    m_insertButton = new QPushButton(tr("Insert"), this);
    QPushButton *addButton = new QPushButton(tr("Add"), this);
    QPushButton *removeButton = new QPushButton(tr("Delete"), this);
    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(m_insertButton);
    buttons->addWidget(addButton);
    buttons->addWidget(removeButton);
    buttons->addStretch();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_list, 1);
    layout->addWidget(m_preview, 1);
    layout->addLayout(buttons);
    setLayout(layout);

    connect(m_insertButton, &QPushButton::clicked, this, &FmaDictPanel::insertCurrent);
    connect(addButton, &QPushButton::clicked, this, &FmaDictPanel::addFormula);
    connect(removeButton, &QPushButton::clicked, this, &FmaDictPanel::removeFormula);
    connect(m_list, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem *current, QListWidgetItem *) { showPreview(current); });

    refresh();
}

void FmaDictPanel::refresh()
{
    m_list->clear();
    m_preview->clear();
    m_insertButton->setEnabled(false);

    const QDir dir(formulaDir());
    const QFileInfoList files = dir.entryInfoList(
                QStringList() << QStringLiteral("*.mml")
                              << QStringLiteral("*.json")
                              << QStringLiteral("*.fma"),
                QDir::Files, QDir::Name);
    for (const QFileInfo &info : files) {
        QListWidgetItem *item = new QListWidgetItem(info.completeBaseName(), m_list);
        item->setData(Qt::UserRole, info.absoluteFilePath());
    }

    if (m_list->count() > 0)
        m_list->setCurrentRow(0);
}

void FmaDictPanel::showPreview(QListWidgetItem *item)
{
    if (!item) {
        m_preview->clear();
        m_insertButton->setEnabled(false);
        return;
    }

    const bool ok = m_preview->load(item->data(Qt::UserRole).toString());
    m_insertButton->setEnabled(ok);
    if (!ok)
        QMessageBox::warning(this, tr("Formula dict"), tr("Failed to load the formula."));
}

void FmaDictPanel::insertCurrent()
{
    QListWidgetItem *item = m_list->currentItem();
    if (!item) {
        QMessageBox::information(this, tr("Formula dict"), tr("Select a formula first."));
        return;
    }
    if (!m_preview->load(item->data(Qt::UserRole).toString())) {
        QMessageBox::warning(this, tr("Formula dict"), tr("Failed to load the formula."));
        return;
    }
    emit insertFormulaRequested(m_preview->formulaData());
}

void FmaDictPanel::addFormula()
{
    if (!m_doc)
        return;

    bool ok = false;
    const QString name = QInputDialog::getText(this, tr("Add formula"),
                                               tr("Formula name:"), QLineEdit::Normal,
                                               QString(), &ok);
    if (!ok)
        return;

    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        QMessageBox::warning(this, tr("Add formula"), tr("Please enter a name."));
        return;
    }
    static const QRegularExpression invalidChars(QStringLiteral("[\\\\/:*?\"<>|]"));
    if (invalidChars.match(trimmed).hasMatch()) {
        QMessageBox::warning(this, tr("Add formula"), tr("The name contains invalid characters."));
        return;
    }

    const QString filePath = QDir(formulaDir()).filePath(trimmed + QStringLiteral(".mml"));
    if (QFile::exists(filePath)) {
        QMessageBox::warning(this, tr("Add formula"), tr("A formula with this name already exists."));
        return;
    }
    if (!m_doc->saveMml(filePath)) {
        QMessageBox::warning(this, tr("Add formula"), tr("Failed to save the formula."));
        return;
    }
    refresh();
}

void FmaDictPanel::removeFormula()
{
    QListWidgetItem *item = m_list->currentItem();
    if (!item)
        return;

    const QString filePath = item->data(Qt::UserRole).toString();
    const QMessageBox::StandardButton answer = QMessageBox::question(
                this, tr("Delete formula"), tr("Delete \"%1\"?").arg(item->text()),
                QMessageBox::Yes | QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    if (!QFile::remove(filePath)) {
        QMessageBox::warning(this, tr("Delete formula"), tr("Failed to delete the formula."));
        return;
    }
    refresh();
}
