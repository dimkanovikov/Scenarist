#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <3rd_party/Widgets/QLightBoxWidget/qlightboxdialog.h>

class QAbstractItemModel;

namespace Ui {
    class ExportDialog;
}

namespace BusinessLogic {
    class ExportParameters;
}


namespace UserInterface
{
    /**
     * @brief Диалог экспорта
     */
    class ExportDialog : public QLightBoxDialog
    {
        Q_OBJECT

    public:
        explicit ExportDialog(QWidget* _parent = 0);
        ~ExportDialog();

        /**
         * @brief Установить путь экспортируемого файла
         */
        void setExportFilePath(const QString& _filePath);

        /**
         * @brief Установить имя экспортируемого файла
         */
        void setExportFileName(const QString& _fileName);

        /**
         * @brief Установить необходимость проверять переносы страниц
         */
        void setCheckPageBreaks(bool _check);

        /**
         * @brief Установить модель стилей
         */
        void setStylesModel(QAbstractItemModel* _model);

        /**
         * @brief Установить текущий стиль
         */
        void setCurrentStyle(const QString& _styleName);

        /**
         * @brief Установить нумеровать ли страницы
         */
        void setPageNumbering(bool _isChecked);

        /**
         * @brief Установить нумеровать ли сцены
         */
        void setScenesNumbering(bool _isChecked);

        /**
         * @brief Установить приставку сцен
         */
        void setScenesPrefix(const QString& _prefix);

        /**
         * @brief Установить необходимость сохранения редакторских пометок
         */
        void setSaveReviewMarks(bool _save);

        /**
         * @brief Установить печатать ли титульный лист
         */
        void setPrintTitle(bool _isChecked);

        /**
         * @brief Получить настройки экспорта
         */
        BusinessLogic::ExportParameters exportParameters() const;

        /**
         * @brief Формат экспорта
         */
        QString exportFormat() const;

        /**
         * @brief Установить модель документов для экспорта
         */
        void setResearchModel(QAbstractItemModel* _model);

    signals:
        /**
         * @brief Сменился стиль
         */
        void currentStyleChanged(const QString& _styleName);

        /**
         * @brief Показать предварительный просмотр
         */
        void printPreview();

    private slots:
        /**
         * @brief Сменился формат
         */
        void aboutFormatChanged();

        /**
         * @brief Выбрать файл
         */
        void aboutChooseFile();

        /**
         * @brief При смене имени файла, обновить доступность кнопки экспорта,
         *		  а также проверить не будет ли произведено пересохранение
         */
        void aboutFileNameChanged();

    private:
        /**
         * @brief Настроить представление
         */
        void initView() override;

        /**
         * @brief Настроить соединения
         */
        void initConnections() override;

        /**
         * @brief Настроить внешний вид
         */
        void initStyleSheet();

        /**
         * @brief Переопределяем, чтобы возвратить таббар для заголовка
         */
        QWidget* titleWidget() const override;

    private:
        /**
         * @brief Интерфейс
         */
        Ui::ExportDialog* m_ui;

        /**
         * @brief Имя файла для экспорта
         */
        QString m_exportFileName;
    };
}

#endif // EXPORTDIALOG_H
