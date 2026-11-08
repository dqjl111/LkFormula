#include "fmamain.h"
#include "ui_fmamain.h"
#include <QDebug>
#include <QToolBar>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QDropEvent>
#include <QCoreApplication>
#include <QEvent>
#include <QActionGroup>
#include <QCborValue>
#include <QDockWidget>
#include "fma/widget/fmaaboutdialog.h"
#include "fma/widget/fmaclipboard.h"
#include "fma/widget/fmadictpanel.h"
#include "util/nodeleter.h"
#include "util/lktranslator.h"
#include "fma/wmf/fmawmfgenerator.h"

FmaMain* instPtr = nullptr;

FmaMain::FmaMain(std::shared_ptr<FmaDoc> _docPtr, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FmaMain)
{
    ui->setupUi(this);  

    if(_docPtr){
        docPtr = _docPtr;
    }else{
        docPtr = std::make_shared<FmaDoc>();
    }

    mathView = new FmaEditView(this);
    mathView->setObjectName(QStringLiteral("FmaEditView"));
    setCentralWidget(mathView);
    mathView->setFmaDoc(docPtr.get());

    ui->actionFmaDict->setCheckable(true);
    formulaDock = new QDockWidget(tr("Formula dict"), this);
    formulaDock->setObjectName(QStringLiteral("FormulaDictDock"));
    formulaDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    formulaPanel = new FmaDictPanel(docPtr.get(), formulaDock);
    formulaDock->setWidget(formulaPanel);
    addDockWidget(Qt::LeftDockWidgetArea, formulaDock);
    formulaDock->hide();
    connect(ui->actionFmaDict, &QAction::toggled, formulaDock, &QDockWidget::setVisible);
    connect(formulaDock, &QDockWidget::visibilityChanged, ui->actionFmaDict, &QAction::setChecked);
    connect(formulaPanel, &FmaDictPanel::insertFormulaRequested, this, [this](const QByteArray &data) {
        mathView->insertFormula(QCborValue::fromCbor(data).toMap());
    });

    connect(ui->actionUndo,SIGNAL(triggered()),mathView,SLOT(undo()));
    connect(ui->actionRedo,SIGNAL(triggered()),mathView,SLOT(redo()));
    connect(ui->actionCopy,SIGNAL(triggered()),mathView,SLOT(copy()));
    connect(ui->actionCut,SIGNAL(triggered()),mathView,SLOT(cut()));
    connect(ui->actionPaste,SIGNAL(triggered()),mathView,SLOT(paste()));

    mathView->addAction(ui->actionUndo);
    mathView->addAction(ui->actionRedo);
    mathView->addAction(ui->actionCopy);
    mathView->addAction(ui->actionCut);
    mathView->addAction(ui->actionPaste);
    mathView->setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(&(docPtr.get()->undoStack),SIGNAL(canUndoChanged(bool)),ui->actionUndo,SLOT(setEnabled(bool)));
    connect(&(docPtr.get()->undoStack),SIGNAL(canRedoChanged(bool)),ui->actionRedo,SLOT(setEnabled(bool)));

    ui->actionUndo->setEnabled(docPtr.get()->undoStack.canUndo());
    ui->actionRedo->setEnabled(docPtr.get()->undoStack.canRedo());

    addToolBar(mathView->createToolBar(this));

    connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(newFile()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(save()));
    connect(ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(open()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SIGNAL(bindQuit()));

    connect(ui->actionClipboardSettings, SIGNAL(triggered()), this, SLOT(setClipboard()));
    connect(ui->actionAbout, &QAction::triggered, this, &FmaMain::onAbout);

    QActionGroup *languageGroup = new QActionGroup(this);
    languageGroup->setExclusive(true);
    languageGroup->addAction(ui->actionChinese);
    languageGroup->addAction(ui->actionEnglish);
    connect(ui->actionChinese, &QAction::triggered, this, [this]() {
        applyLanguage(QStringLiteral("zh_CN"));
    });
    connect(ui->actionEnglish, &QAction::triggered, this, [this]() {
        applyLanguage(QStringLiteral("en"));
    });
    updateLanguageActions();
    setAcceptDrops(true);
    updateWindowTitle();

//
}

FmaMain::~FmaMain()
{
    delete ui;
}

std::shared_ptr<FmaDoc> FmaMain::currentDoc()
{
    return docPtr;
}

void FmaMain::bindShow()
{
    //setVisible(true);
    showNormal();
    mathView->setFocus();
}

void FmaMain::newFile()
{
    currentFile.clear();
    docPtr = std::make_shared<FmaDoc>();
    mathView->setFmaDoc(docPtr.get());
    updateWindowTitle();
}


void FmaMain::save()
{
    if(currentFile.isEmpty()){
        saveAs();
    }else{
        save(currentFile);
    }
}

void FmaMain::saveAs()
{
    QString fileName = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    fileName = QFileDialog::getSaveFileName(nullptr, tr("Save formula"),
                                            QFileInfo(QDir::currentPath(), fileName).absoluteFilePath(),
                                            "*.fma;;*.json;;*.mml;;*.tex;;*.wmf;;*.svg;;*.png");


    if(!fileName.isEmpty()){
        QFileInfo pathInfo(fileName);
        QDir::setCurrent(pathInfo.absolutePath());
        save(pathInfo.absoluteFilePath());
    }
}

void FmaMain::open()
{
    const QString fileName = QFileDialog::getOpenFileName(nullptr, tr("Open formula"),
                                                          QDir::currentPath(),
                                                          "(*.fma *.json *.mml *.wmf)");
    if (!fileName.isEmpty())
        loadFile(fileName);
}

bool FmaMain::loadFile(const QString &fileName)
{
    if (fileName.isEmpty())
        return false;

    const QFileInfo pathInfo(fileName);
    QDir::setCurrent(pathInfo.absolutePath());

    bool ok = false;
    if (fileName.endsWith(".fma"))
        ok = mathView->fmaDoc()->loadCbor(pathInfo.absoluteFilePath());
    else if (fileName.endsWith(".mml"))
        ok = mathView->fmaDoc()->loadMml(pathInfo.absoluteFilePath());
    else if (fileName.endsWith(".wmf"))
        ok = mathView->fmaDoc()->loadWmf(pathInfo.absoluteFilePath());
    else
        ok = mathView->fmaDoc()->loadJson(pathInfo.absoluteFilePath());

    if (ok) {
        currentFile = pathInfo.absoluteFilePath();
        updateWindowTitle();
    }
    mathView->update();
    return ok;
}

void FmaMain::onAbout()
{
    FmaAboutDialog dlg(this);
    dlg.exec();
}

void FmaMain::setClipboard()
{
    const QStringList opts = { tr("OLE"), tr("MathML"), tr("TEX") };
    bool ok = false;
    const QString item = QInputDialog::getItem(this, tr("Clipboard settings"),
                                               tr("Set clipboad format: "), opts,
                                               FmaClipboard::format(), false, &ok);
    if (ok) {
        const int index = opts.indexOf(item);
        if (index >= 0)
            FmaClipboard::setFormat(index);
    }
}

void FmaMain::applyLanguage(const QString &language)
{
    if (language == LkTranslator::currentLanguage()) {
        updateLanguageActions();
        return;
    }
    if (LkTranslator::setLanguage(language)) {
        ui->retranslateUi(this);
        updateLanguageActions();
    }
}

void FmaMain::updateLanguageActions()
{
    const bool chinese = LkTranslator::currentLanguage() == QLatin1String("zh_CN");
    ui->actionChinese->setChecked(chinese);
    ui->actionEnglish->setChecked(!chinese);
}

void FmaMain::updateWindowTitle()
{
    QString version = QCoreApplication::applicationVersion();
    if (version.isEmpty())
        version = QStringLiteral("0.1");

    QString title = tr("Fma Editor") + QStringLiteral(" v") + version;
    if (!currentFile.isEmpty())
        title += QStringLiteral(" - ") + QFileInfo(currentFile).fileName();
    setWindowTitle(title);
}

void FmaMain::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        if (formulaDock)
            formulaDock->setWindowTitle(tr("Formula dict"));
        updateLanguageActions();
        updateWindowTitle();
    }
    QMainWindow::changeEvent(event);
}


void FmaMain::dropEvent(QDropEvent *event)
{
    const QMimeData *data = event->mimeData();
    if(data!= nullptr){
        QStringList formats = data->formats();
        for(const QString& format : formats){
            if(data->hasFormat(format)){
                QByteArray fmtData = data->data(format);
                qWarning() << "Has: " << format
                           << ": " << fmtData;
            }
        }
    }
}

void FmaMain::closeEvent(QCloseEvent *event)
{
    event->ignore();
    emit bindQuit();
}

void FmaMain::save(QString fileName)
{
    bool isSaveOk =false;
    if(fileName.endsWith(".fma")){
        isSaveOk = mathView->fmaDoc()->saveCbor(fileName);
    }else if(fileName.endsWith(".mml")){
        isSaveOk = mathView->fmaDoc()->saveMml(fileName);
    }else if(fileName.endsWith(".tex")){
        isSaveOk = mathView->fmaDoc()->saveTex(fileName);
    }else if(fileName.endsWith(".wmf")){
        isSaveOk = mathView->fmaDoc()->saveWmf(fileName);
    }else if(fileName.endsWith(".svg")){
        isSaveOk = mathView->fmaDoc()->saveSvg(fileName);
    }else if(fileName.endsWith(".png")){
        isSaveOk = mathView->fmaDoc()->savePng(fileName);
    }else{
        isSaveOk = mathView->fmaDoc()->saveJson(fileName);
    }

    if(isSaveOk){
        currentFile = fileName;
        updateWindowTitle();
    }
}




