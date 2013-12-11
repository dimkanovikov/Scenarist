#include "TitleHandler.h"

#include <BusinessLogic/ScenarioTextEdit/ScenarioTextEdit.h>

#include <QKeyEvent>
#include <QTextBlock>

using namespace KeyProcessingLayer;


TitleHandler::TitleHandler(ScenarioTextEdit* _editor) :
	StandardKeyHandler(_editor)
{
}

void TitleHandler::handleEnter(QKeyEvent*)
{
	//
	// Получим необходимые значения
	//
	// ... курсор в текущем положении
	QTextCursor cursor = editor()->textCursor();
	// ... блок текста в котором находится курсор
	QTextBlock currentBlock = cursor.block();
	// ... текст до курсора
	QString cursorBackwardText = currentBlock.text().left(cursor.positionInBlock());
	// ... текст после курсора
	QString cursorForwardText = currentBlock.text().mid(cursor.positionInBlock());


	//
	// Обработка
	//
	if (editor()->isCompleterVisible()) {
		//! Если открыт подстановщик

		//
		// Ни чего не делаем
		//
	} else {
		//! Подстановщик закрыт

		if (cursor.hasSelection()) {
			//! Есть выделение

			//
			// Ни чего не делаем
			//
		} else {
			//! Нет выделения

			if (cursorBackwardText.isEmpty()
				&& cursorForwardText.isEmpty()) {
				//! Текст пуст

				//
				// Ни чего не делаем
				//
			} else {
				//! Текст не пуст

				if (cursorBackwardText.isEmpty()) {
					//! В начале блока

					//
					// Ни чего не делаем
					//
				} else if (cursorForwardText.isEmpty()) {
					//! В конце блока

					//
					// Вставляем блок и применяем ему стиль описания действия
					//
					cursor.insertBlock();
					editor()->setScenarioBlockType(ScenarioTextBlockStyle::Action);
				} else {
					//! Внутри блока

					//
					// Ни чего не делаем
					//
				}
			}
		}
	}
}

void TitleHandler::handleTab(QKeyEvent*)
{
	//
	// Получим необходимые значения
	//
	// ... курсор в текущем положении
	QTextCursor cursor = editor()->textCursor();
	// ... блок текста в котором находится курсор
	QTextBlock currentBlock = cursor.block();
	// ... текст до курсора
	QString cursorBackwardText = currentBlock.text().left(cursor.positionInBlock());
	// ... текст после курсора
	QString cursorForwardText = currentBlock.text().mid(cursor.positionInBlock());


	//
	// Обработка
	//
	if (editor()->isCompleterVisible()) {
		//! Если открыт подстановщик

		//
		// Ни чего не делаем
		//
	} else {
		//! Подстановщик закрыт

		if (cursor.hasSelection()) {
			//! Есть выделение

			//
			// Ни чего не делаем
			//
		} else {
			//! Нет выделения

			if (cursorBackwardText.isEmpty()
				&& cursorForwardText.isEmpty()) {
				//! Текст пуст

				//
				// Ни чего не делаем
				//
			} else {
				//! Текст не пуст

				if (cursorBackwardText.isEmpty()) {
					//! В начале блока

					//
					// Ни чего не делаем
					//
				} else if (cursorForwardText.isEmpty()) {
					//! В конце блока

					//
					// Действуем как нажатие клавиши ENTER
					//
					handleEnter();
				} else {
					//! Внутри блока

					//
					// Ни чего не делаем
					//
				}
			}
		}
	}
}

void TitleHandler::handleBackspace(QKeyEvent* _event)
{
	//
	// Получим необходимые значения
	//
	// ... курсор в текущем положении
	QTextCursor cursor = editor()->textCursor();

	//
	// Стираем символы и применяем соответствующий стиль
	//
	int destBlockPosition = cursor.position() - 1; // отнимаем 1 т.к. удаляём предыдущий текст
	if (cursor.hasSelection()) {
		destBlockPosition = qMin(cursor.selectionStart(), cursor.selectionEnd());
	}
	QTextCursor destBlockCursor(editor()->document());
	destBlockCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, destBlockPosition);
	ScenarioTextBlockStyle::Type destType = editor()->scenarioBlockType(destBlockCursor.block());

	//
	// Если предыдущим становится блок заголовка титра, то стираем так же и его
	//
	while (destType == ScenarioTextBlockStyle::TitleHeader) {
		destBlockCursor.movePosition(QTextCursor::Left);
		destType = editor()->scenarioBlockType(destBlockCursor.block());

		if (destBlockCursor.position() == 0) {
			destType = ScenarioTextBlockStyle::TimeAndPlace;
		}
	}

	//
	// Обновим выделение
	//
	cursor.setPosition(cursor.selectionEnd());
	cursor.setPosition(destBlockCursor.position(), QTextCursor::KeepAnchor);

	//
	// Удаляем символы
	//
	cursor.deletePreviousChar();

	//
	// Обновляем стиль текущего блока
	//
	editor()->setScenarioBlockType(destType);
}