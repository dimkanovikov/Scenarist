#include "ScenarioTextView.h"
#include "ui_ScenarioTextView.h"



#include <BusinessLayer/ScenarioDocument/ScenarioTemplate.h>
#include <BusinessLayer/ScenarioDocument/ScenarioTextDocument.h>
#include <BusinessLayer/ScenarioDocument/ScenarioTemplate.h>
#include <BusinessLayer/ScenarioDocument/ScenarioTextBlockInfo.h>
#include <BusinessLayer/Chronometry/ChronometerFacade.h>

#include <UserInterfaceLayer/ScenarioTextEdit/ScenarioTextEdit.h>
#include <UserInterfaceLayer/ScenarioTextEdit/ScenarioTextEditHelpers.h>

#include <3rd_party/Helpers/ShortcutHelper.h>
#include <3rd_party/Widgets/ScalableWrapper/ScalableWrapper.h>

#include <QCryptographicHash>
#include <QKeyEvent>
#include <QScroller>

using UserInterface::ScenarioTextView;
using UserInterface::ScenarioTextEdit;
using BusinessLogic::ScenarioTemplateFacade;
using BusinessLogic::ScenarioTemplate;
using BusinessLogic::ScenarioBlockStyle;
using BusinessLogic::ScenarioTextBlockInfo;

namespace {
	const int SCROLL_DELTA = 140;

	/**
	 * @brief Получить хэш текста
	 */
	static QByteArray textMd5Hash(const QString& _text) {
		QCryptographicHash hash(QCryptographicHash::Md5);
		hash.addData(_text.toUtf8());
		return hash.result();
	}
}


ScenarioTextView::ScenarioTextView(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::ScenarioTextView),
	m_editor(new ScenarioTextEdit(this)),
	m_editorWrapper(new ScalableWrapper(m_editor, this))
{
	m_ui->setupUi(this);

	initView();
	initConnections();
	initStyleSheet();
}

ScenarioTextView::~ScenarioTextView()
{
	delete m_ui;
}

QWidget* ScenarioTextView::toolbar() const
{
	return m_ui->toolbar;
}

void ScenarioTextView::setScenarioName(const QString& _name)
{
	m_ui->scenarioName->setText(_name);
}

BusinessLogic::ScenarioTextDocument* ScenarioTextView::scenarioDocument() const
{
	return qobject_cast<BusinessLogic::ScenarioTextDocument*>(m_editor->document());
}

void ScenarioTextView::setScenarioDocument(BusinessLogic::ScenarioTextDocument* _document, bool _isDraft)
{
	removeEditorConnections();

	m_editor->setScenarioDocument(_document);
	m_editor->setWatermark(_isDraft ? tr("DRAFT") : QString::null);
	if (_document != 0) {
		m_lastTextMd5Hash = ::textMd5Hash(_document->toPlainText());
	}

	initEditorConnections();
}

//void ScenarioTextView::setDuration(const QString& _duration)
//{
//	m_duration->setText(_duration);
//}

//void ScenarioTextView::setCountersInfo(const QString& _counters)
//{
//	m_countersInfo->setText(_counters);
//}

void ScenarioTextView::setShowScenesNumbers(bool _show)
{
	m_editor->setShowSceneNumbers(_show);
}

void ScenarioTextView::setHighlightCurrentLine(bool _highlight)
{
	m_editor->setHighlightCurrentLine(_highlight);
}

void ScenarioTextView::setAutoReplacing(bool _replacing)
{
	m_editor->setAutoReplacing(_replacing);
}

void ScenarioTextView::setUsePageView(bool _use)
{
	//
	// Установка постраничного режима так же тянет за собой ряд настроек
	//
	QMarginsF pageMargins(15, 5, 5, 5);
	Qt::Alignment pageNumbersAlign;
	if (_use) {
		pageMargins = ScenarioTemplateFacade::getTemplate().pageMargins();
		pageNumbersAlign = ScenarioTemplateFacade::getTemplate().numberingAlignment();
	}

	m_editor->setUsePageMode(_use);
	m_editor->setPageMargins(pageMargins);
	m_editor->setPageNumbersAlignment(pageNumbersAlign);

	//
	// В дополнение установим шрифт по умолчанию для документа (шрифтом будет рисоваться нумерация)
	//
	m_editor->document()->setDefaultFont(
		ScenarioTemplateFacade::getTemplate().blockStyle(ScenarioBlockStyle::Action).font());
}

void ScenarioTextView::setUseSpellChecker(bool _use)
{
	m_editor->setUseSpellChecker(_use);
}

void ScenarioTextView::setSpellCheckLanguage(int _language)
{
	m_editor->setSpellCheckLanguage((SpellChecker::Language)_language);
}

void ScenarioTextView::setTextEditColors(const QColor& _textColor, const QColor& _backgroundColor)
{
	m_editor->viewport()->setStyleSheet(QString("color: %1; background-color: %2;").arg(_textColor.name(), _backgroundColor.name()));
	m_editor->setStyleSheet(QString("#scenarioEditor { color: %1; }").arg(_textColor.name()));
}

void ScenarioTextView::setTextEditZoomRange(qreal _zoomRange)
{
	m_editorWrapper->setZoomRange(_zoomRange);
}

int ScenarioTextView::cursorPosition() const
{
	return m_editor->textCursor().position();
}

void ScenarioTextView::setCursorPosition(int _position)
{
	//
	// Устанавливаем позицию курсора
	//
	QTextCursor cursor = m_editor->textCursor();

	//
	// Если это новая позиция
	//
	if (cursor.position() != _position) {
		//
		// Устанавливаем реальную позицию
		//
		cursor.setPosition(_position);
		m_editor->setTextCursor(cursor);
		m_editor->ensureCursorVisibleReimpl();
		//
		// Прокручиваем вниз, чтобы курсор стоял в верху экрана
		//
		m_editor->verticalScrollBar()->setValue(m_editor->verticalScrollBar()->maximum());

		//
		// Возвращаем курсор в поле зрения
		//
		m_editor->ensureCursorVisibleReimpl();
		m_editorWrapper->setFocus();
	}
	//
	// Если нужно обновить в текущей позиции курсора просто имитируем отправку сигнала
	//
	else {
		emit m_editor->cursorPositionChanged();
	}
}

void ScenarioTextView::addItem(int _position, int _type, const QString& _header,
	const QColor& _color, const QString& _description)
{
	QTextCursor cursor = m_editor->textCursor();
	cursor.beginEditBlock();

	cursor.setPosition(_position);
	m_editor->setTextCursor(cursor);
	ScenarioBlockStyle::Type type = (ScenarioBlockStyle::Type)_type;
	//
	// Если в позиции пустой блок, изменим его
	//
	if (cursor.block().text().isEmpty()) {
		m_editor->changeScenarioBlockType(type);
	}
	//
	// В противном случае добавим новый
	//
	else {
		m_editor->addScenarioBlock(type);
	}

	//
	// Устанавливаем текст в блок
	//
	m_editor->insertPlainText(_header);

	//
	// Устанавливаем цвет и описание
	//
	cursor = m_editor->textCursor();
	QTextBlockUserData* textBlockData = cursor.block().userData();
	ScenarioTextBlockInfo* info = dynamic_cast<ScenarioTextBlockInfo*>(textBlockData);
	if (info == 0) {
		info = new ScenarioTextBlockInfo;
	}
	info->setColors(_color.name());
	info->setDescription(_description);
	cursor.block().setUserData(info);

	//
	// Если это группирующий блок, то вставим и закрывающий текст
	//
	if (ScenarioTemplateFacade::getTemplate().blockStyle(type).isEmbeddableHeader()) {
		cursor = m_editor->textCursor();
		cursor.movePosition(QTextCursor::NextBlock);
		cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
		cursor.insertText(Helpers::footerText(_header));
	}

	cursor.endEditBlock();
}

void ScenarioTextView::removeText(int _from, int _to)
{
	QTextCursor cursor = m_editor->textCursor();
	cursor.beginEditBlock();

	//
	// Стираем текст
	//
	cursor.setPosition(_from);
	cursor.setPosition(_to, QTextCursor::KeepAnchor);
	cursor.removeSelectedText();

	//
	// Если остаётся пустой блок, стираем его тоже
	//
	if (cursor.block().text().isEmpty()) {
		//
		// Стирать необходимо через имитацию удаления редактором,
		// для корректного обновления модели сцен
		//
		QKeyEvent* event = 0;
		if (cursor.atStart()) {
			event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
		} else {
			event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
		}
		QApplication::sendEvent(m_editor, event);
	}

	cursor.endEditBlock();
}

void ScenarioTextView::updateStylesElements()
{
	//
	// Обновить выпадающий список стилей сценария
	//
	m_ui->textStyles->clear();
	initStylesCombo();
}

void ScenarioTextView::updateShortcuts()
{
	m_editor->updateShortcuts();
	updateStylesCombo();
}

void ScenarioTextView::setAdditionalCursors(const QMap<QString, int>& _cursors)
{
	m_editor->setAdditionalCursors(_cursors);
}

void ScenarioTextView::setCommentOnly(bool _isCommentOnly)
{
	m_editor->setReadOnly(_isCommentOnly);
}

void ScenarioTextView::aboutUndo()
{
	m_editor->undoReimpl();
}

void ScenarioTextView::aboutRedo()
{
	m_editor->redoReimpl();
}

void ScenarioTextView::aboutUpdateTextStyle()
{
	ScenarioBlockStyle::Type currentType = m_editor->scenarioBlockType();
	if (currentType == ScenarioBlockStyle::TitleHeader) {
		currentType = ScenarioBlockStyle::Title;
	} else if (currentType == ScenarioBlockStyle::SceneGroupFooter) {
		currentType = ScenarioBlockStyle::SceneGroupHeader;
	} else if (currentType == ScenarioBlockStyle::FolderFooter) {
		currentType = ScenarioBlockStyle::FolderHeader;
	}

	for (int itemIndex = 0; itemIndex < m_ui->textStyles->count(); ++itemIndex) {
		ScenarioBlockStyle::Type itemType =
				(ScenarioBlockStyle::Type)m_ui->textStyles->itemData(itemIndex).toInt();
		if (itemType == currentType) {
			m_ui->textStyles->setCurrentIndex(itemIndex);
			break;
		}
	}
}

void ScenarioTextView::aboutChangeTextStyle()
{
	ScenarioBlockStyle::Type type =
			(ScenarioBlockStyle::Type)m_ui->textStyles->itemData(m_ui->textStyles->currentIndex()).toInt();

	//
	// Меняем стиль блока, если это возможно
	//
	m_editor->changeScenarioBlockType(type);
	m_editorWrapper->setFocus();
}

void ScenarioTextView::aboutCursorPositionChanged()
{
	emit cursorPositionChanged(m_editor->textCursor().position());
}

void ScenarioTextView::aboutTextChanged()
{
	QByteArray currentTextMd5Hash = textMd5Hash(m_editor->document()->toPlainText());
	if (m_lastTextMd5Hash != currentTextMd5Hash) {
		emit textChanged();

		m_lastTextMd5Hash = currentTextMd5Hash;
	}
}

void ScenarioTextView::aboutStyleChanged()
{
	emit textChanged();
}

void ScenarioTextView::initView()
{
	m_ui->textEditContainer->addWidget(m_editorWrapper);

//	//
//	// Отключаем автоподсказки
//	//
//	m_ui->textEdit->setInputMethodHints(m_ui->textEdit->inputMethodHints() | Qt::ImhNoPredictiveText);

//	m_ui->textEdit->horizontalScrollBar()->hide();
//	m_ui->textEdit->verticalScrollBar()->hide();

//    QScroller::grabGesture(m_editorWrapper);
}

void ScenarioTextView::initStylesCombo()
{
	ScenarioTemplate style = ScenarioTemplateFacade::getTemplate();
	const bool BEAUTIFY_NAME = true;

	QList<ScenarioBlockStyle::Type> types;
	types << ScenarioBlockStyle::SceneHeading
		  << ScenarioBlockStyle::SceneCharacters
		  << ScenarioBlockStyle::Action
		  << ScenarioBlockStyle::Character
		  << ScenarioBlockStyle::Dialogue
		  << ScenarioBlockStyle::Parenthetical
		  << ScenarioBlockStyle::Title
		  << ScenarioBlockStyle::Note
		  << ScenarioBlockStyle::Transition
		  << ScenarioBlockStyle::NoprintableText
		  << ScenarioBlockStyle::SceneGroupHeader
		  << ScenarioBlockStyle::FolderHeader;

	foreach (ScenarioBlockStyle::Type type, types) {
		if (style.blockStyle(type).isActive()) {
			m_ui->textStyles->addItem(ScenarioBlockStyle::typeName(type, BEAUTIFY_NAME), type);
		}
	}

	updateStylesCombo();
}

void ScenarioTextView::updateStylesCombo()
{
	for (int index = 0; index < m_ui->textStyles->count(); ++index) {
		ScenarioBlockStyle::Type blockType =
				(ScenarioBlockStyle::Type)m_ui->textStyles->itemData(index).toInt();
		m_ui->textStyles->setItemData(index, m_editor->shortcut(blockType), Qt::ToolTipRole);
	}
}

void ScenarioTextView::initConnections()
{
	connect(m_ui->navigator, &QToolButton::clicked, this, &ScenarioTextView::showNavigatorClicked);
	connect(m_ui->tab, &QToolButton::clicked, [=](){
	   qApp->sendEvent(m_editor, new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier));
	});
	connect(m_ui->enter, &QToolButton::clicked, [=](){
	   qApp->sendEvent(m_editor, new QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier));
	});

	initEditorConnections();
}

void ScenarioTextView::initEditorConnections()
{
	connect(m_editor, SIGNAL(currentStyleChanged()), this, SLOT(aboutUpdateTextStyle()));
	connect(m_editor, SIGNAL(cursorPositionChanged()), this, SLOT(aboutUpdateTextStyle()));
	connect(m_editor, SIGNAL(cursorPositionChanged()), this, SLOT(aboutCursorPositionChanged()));
	connect(m_editor, SIGNAL(textChanged()), this, SLOT(aboutTextChanged()));
	connect(m_editor, SIGNAL(styleChanged()), this, SLOT(aboutStyleChanged()));
	connect(m_editor, SIGNAL(reviewChanged()), this, SIGNAL(textChanged()));
	connect(m_editorWrapper, SIGNAL(zoomRangeChanged(qreal)), this, SIGNAL(zoomRangeChanged(qreal)));
}

void ScenarioTextView::removeEditorConnections()
{
	disconnect(m_editor, SIGNAL(currentStyleChanged()), this, SLOT(aboutUpdateTextStyle()));
	disconnect(m_editor, SIGNAL(cursorPositionChanged()), this, SLOT(aboutUpdateTextStyle()));
	disconnect(m_editor, SIGNAL(cursorPositionChanged()), this, SLOT(aboutCursorPositionChanged()));
	disconnect(m_editor, SIGNAL(textChanged()), this, SLOT(aboutTextChanged()));
	disconnect(m_editor, SIGNAL(styleChanged()), this, SLOT(aboutStyleChanged()));
	disconnect(m_editor, SIGNAL(reviewChanged()), this, SIGNAL(textChanged()));
	disconnect(m_editorWrapper, SIGNAL(zoomRangeChanged(qreal)), this, SIGNAL(zoomRangeChanged(qreal)));
}

void ScenarioTextView::initStyleSheet()
{
	m_ui->toolbar->setProperty("toolbar", true);
	m_ui->scenarioName->setProperty("toolbar", true);
	m_ui->search->hide();
	m_ui->menu->hide();
}
