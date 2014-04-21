#ifndef PHOTOLABEL_H
#define PHOTOLABEL_H

#include <QLabel>

class QToolButton;


/**
 * @brief Виджет самовыбирающейся картинки
 */
class PhotoLabel : public QLabel
{
	Q_OBJECT

public:
	explicit PhotoLabel(QWidget* _parent = 0);

	/**
	 * @brief Установить фотографию
	 */
	void setPhoto(const QPixmap& _photo);

signals:
	/**
	 * @brief Сигнал щелчка по фотографии
	 */
	void photoClicked(const QPixmap& _photo);

protected:
	/**
	 * @brief Переопределяется для прорисовки кнопок действий во время наведения мышки
	 */
	void enterEvent(QEvent* _event);

	/**
	 * @brief Переопределяется для сокрытия кнопок действий во время выхода мышки
	 */
	void leaveEvent(QEvent* _event);

	/**
	 * @brief Переопределяется для запуска события выбора изображения
	 */
	void mousePressEvent(QMouseEvent* _event);

	void resizeEvent(QResizeEvent* _event);

private slots:
	/**
	 * @brief Очистить фотографию
	 */
	void clearPhoto();

private:
	/**
	 * @brief Выбранная фотография
	 */
	QPixmap m_photo;

	/**
	 * @brief Кнопка очистки фотографии
	 */
	QToolButton* m_clearButton;
};

#endif // PHOTOLABEL_H