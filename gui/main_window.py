import sys
import os
import html
from PyQt5.QtWidgets import (
    QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QLabel, QTextEdit, QPushButton, QProgressBar,
    QTableWidget, QTableWidgetItem, QGroupBox, QStyle,
    QApplication, QHeaderView, QMessageBox, QDialog,
    QListWidget, QListWidgetItem, QDialogButtonBox,
    QCheckBox, QTreeWidget, QTreeWidgetItem
)
from PyQt5.QtCore import Qt, pyqtSignal
from PyQt5.QtGui import QFont, QPixmap
import core


# Экранирует текст, но разрешает теги <b>, </b>, <i>, </i>, <br>, <code>, </code>
def safe_html(text):
    escaped = html.escape(str(text))
    # Восстановление разрешённых тегов
    escaped = (escaped.replace('&lt;b&gt;', '<b>')\
                .replace('&lt;/b&gt;', '</b>') \
                .replace('&lt;i&gt;', '<i>') \
                .replace('&lt;/i&gt;', '</i>') \
                .replace('&lt;br&gt;', '<br>') \
                .replace('&lt;code&gt;', '<code>') \
                .replace('&lt;/code&gt;', '</code>'))
    return escaped


# Фильтрация по темам
class TopicSelectionDialog(QDialog):
    def __init__(self, all_topics, selected_topics, parent=None):
        super().__init__(parent)
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowContextHelpButtonHint)
        self.all_topics = all_topics
        self.selected_topics = selected_topics
        self.setWindowTitle("Выбор тем")
        self.setMinimumWidth(300)
        self.init_ui()

    def init_ui(self):
        layout = QVBoxLayout(self)

        # Выбрать все
        self.all_cb = QCheckBox("Все темы")
        self.all_cb.stateChanged.connect(self.on_all_changed)
        layout.addWidget(self.all_cb)

        # Лист чекбоксов по каждой теме
        self.list_widget = QListWidget()
        self.list_widget.setSelectionMode(QListWidget.NoSelection)
        for topic in self.all_topics:
            item = QListWidgetItem(topic)
            item.setFlags(item.flags() | Qt.ItemIsUserCheckable)
            item.setCheckState(Qt.Unchecked)
            self.list_widget.addItem(item)
        layout.addWidget(self.list_widget)

        # Кнопки Да и Отмена
        button_box = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        button_box.button(QDialogButtonBox.Ok).setText("Да")
        button_box.button(QDialogButtonBox.Cancel).setText("Отмена")
        button_box.accepted.connect(self.accept)
        button_box.rejected.connect(self.reject)
        layout.addWidget(button_box)

        self.update_ui()

    # Восстановление выбранных тем
    def update_ui(self):
        if not self.selected_topics:
            self.all_cb.setChecked(True)
            self.list_widget.setEnabled(False)
        else:
            self.all_cb.setChecked(False)
            self.list_widget.setEnabled(True)
            for i in range(self.list_widget.count()):
                item = self.list_widget.item(i)
                topic = item.text()
                if topic in self.selected_topics:
                    item.setCheckState(Qt.Checked)
                else:
                    item.setCheckState(Qt.Unchecked)

    def on_all_changed(self, state):
        self.list_widget.setEnabled(state != Qt.Checked)

    # Возвращает список выбранных тем
    def get_selected(self):
        if self.all_cb.isChecked():
            return []
        selected = []
        for i in range(self.list_widget.count()):
            item = self.list_widget.item(i)
            if item.checkState() == Qt.Checked:
                selected.append(item.text())
        return selected


# Список задач
class TaskListDialog(QDialog):
    def __init__(self, trainer, parent=None):
        super().__init__(parent)
        self.trainer = trainer
        self.selected_task_id = None
        self.setWindowTitle("Выбор задачи")
        self.setMinimumSize(600, 400)
        self.init_ui()

    def init_ui(self):
        layout = QVBoxLayout(self)

        # Древовидный виджет тема -> задачи
        self.tree = QTreeWidget()
        self.tree.setHeaderLabels(["Задача", "Сложность", "Источник"])
        self.tree.setAlternatingRowColors(True)
        self.tree.itemDoubleClicked.connect(self.accept)
        self.tree.header().setSectionResizeMode(0, QHeaderView.Stretch)
        self.tree.header().setSectionResizeMode(1, QHeaderView.ResizeToContents)
        self.tree.header().setSectionResizeMode(2, QHeaderView.ResizeToContents)
        layout.addWidget(self.tree)

        # Кнопки Выбрать и Отмена
        button_box = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        button_box.accepted.connect(self.accept)
        button_box.rejected.connect(self.reject)
        button_box.button(QDialogButtonBox.Ok).setText("Выбрать")
        button_box.button(QDialogButtonBox.Cancel).setText("Отмена")
        layout.addWidget(button_box)

        self.load_tasks()

    def load_tasks(self):
        tasks_by_topic = self.trainer.get_all_tasks()
        for topic, tasks in tasks_by_topic.items():
            topic_item = QTreeWidgetItem([topic])
            topic_item.setExpanded(True)
            for task in tasks:
                item = QTreeWidgetItem([task.title, str(task.difficulty), task.source])
                item.setData(0, Qt.UserRole, task.id)
                topic_item.addChild(item)
            self.tree.addTopLevelItem(topic_item)

    def get_selected_task_id(self):
        item = self.tree.currentItem()
        if item and item.parent():
            return item.data(0, Qt.UserRole)
        return None


# Основное окно
class MainWindow(QMainWindow):
    def __init__(self, data_path):
        super().__init__()
        self.data_path = data_path
        self.trainer = None
        self.current_task = None
        self.selected_topics = []
        self.stats_data = {}
        self.init_ui()
        self.init_core()
        self.apply_styles()

    def apply_styles(self):
        self.setStyleSheet("""
            QMainWindow {
                background-color: #f5f6fa;
            }
            QGroupBox {
                font-size: 14px;
                border: 2px solid #dcdde1;
                border-radius: 8px;
                margin-top: 10px;
                padding-top: 10px;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 5px 0 5px;
            }
            QPushButton {
                background-color: #3498db;
                color: white;
                border: none;
                padding: 8px 16px;
                border-radius: 6px;
                font-size: 14px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #2980b9;
            }
            QPushButton:pressed {
                background-color: #1c6ea4;
            }
            QPushButton#infoButton, QPushButton#settingsButton {
                background-color: transparent;
                border: none;
                border-radius: 14px;
                padding: 4px;
                icon-size: 20px 20px;
            }
            QPushButton#infoButton:hover, QPushButton#settingsButton:hover {
                background-color: rgba(52, 152, 219, 0.2);
            }
            QTextEdit {
                background-color: white;
                border: 1px solid #dcdde1;
                border-radius: 6px;
                padding: 10px;
                font-size: 14px;
            }
            QProgressBar {
                border: 1px solid #dcdde1;
                border-radius: 4px;
                text-align: center;
                background-color: white;
            }
            QProgressBar::chunk {
                background-color: #27ae60;
                border-radius: 4px;
            }
            QTableWidget {
                background-color: white;
                border: 1px solid #dcdde1;
                border-radius: 6px;
                gridline-color: #ecf0f1;
            }
            QTableWidget::item {
                padding: 4px;
            }
            QHeaderView::section {
                background-color: #ecf0f1;
                padding: 4px;
                border: none;
                font-weight: bold;
            }
            QCheckBox {
                font-size: 13px;
                padding: 2px;
            }
            QListWidget, QTreeWidget {
                background-color: white;
                border: 1px solid #dcdde1;
                border-radius: 4px;
            }
        """)

    def init_ui(self):
        self.setWindowTitle("Algorithm Trainer")
        self.setGeometry(100, 100, 700, 750)


        central = QWidget()
        self.setCentralWidget(central)
        main_layout = QVBoxLayout(central)

        # Группа задачи
        task_group = QGroupBox()
        task_layout = QVBoxLayout(task_group)

        # Название, информация и выбор тем
        header_layout = QHBoxLayout()
        self.task_title_label = QLabel()
        self.task_title_label.setStyleSheet("font-size: 16px; font-weight: bold;")
        header_layout.addWidget(self.task_title_label)
        header_layout.addStretch()

        self.info_btn = QPushButton()
        self.info_btn.setObjectName("infoButton")
        self.info_btn.setIcon(self.style().standardIcon(QStyle.SP_MessageBoxInformation))
        self.info_btn.setToolTip("Информация о задаче")
        self.info_btn.clicked.connect(self.show_task_info)
        self.info_btn.setFixedSize(28, 28)
        header_layout.addWidget(self.info_btn)

        self.settings_btn = QPushButton()
        self.settings_btn.setObjectName("settingsButton")
        self.settings_btn.setIcon(self.style().standardIcon(QStyle.SP_FileDialogDetailedView))
        self.settings_btn.setToolTip("Настроить темы для тренировки")
        self.settings_btn.setFixedSize(28, 28)
        self.settings_btn.clicked.connect(self.show_topic_dialog)
        header_layout.addWidget(self.settings_btn)

        task_layout.addLayout(header_layout)

        # Текст
        self.task_text = QTextEdit()
        self.task_text.setReadOnly(True)
        self.task_text.setMinimumHeight(200)
        task_layout.addWidget(self.task_text)
        # Изображение
        self.image_label = QLabel()
        self.image_label.setAlignment(Qt.AlignCenter)
        self.image_label.hide()
        task_layout.addWidget(self.image_label)
        main_layout.addWidget(task_group)

        # Кнопки действия
        btn_layout = QHBoxLayout()
        btn_layout.addStretch()

        self.show_answer_btn = QPushButton("Показать ответ")
        self.show_answer_btn.setIcon(self.style().standardIcon(QStyle.SP_MessageBoxQuestion))
        self.show_answer_btn.clicked.connect(self.show_answer)
        self.solved_btn = QPushButton("Решил верно")
        self.solved_btn.setIcon(self.style().standardIcon(QStyle.SP_DialogApplyButton))
        self.solved_btn.clicked.connect(lambda: self.submit_result(True))
        self.unsolved_btn = QPushButton("Не решил")
        self.unsolved_btn.setIcon(self.style().standardIcon(QStyle.SP_DialogCancelButton))
        self.unsolved_btn.clicked.connect(lambda: self.submit_result(False))
        self.task_list_btn = QPushButton("📋 Список задач")
        self.task_list_btn.setToolTip("Показать все задачи")
        self.task_list_btn.clicked.connect(self.show_task_list)

        btn_layout.addWidget(self.show_answer_btn)
        btn_layout.addWidget(self.solved_btn)
        btn_layout.addWidget(self.unsolved_btn)
        btn_layout.addWidget(self.task_list_btn)

        btn_layout.addStretch()
        main_layout.addLayout(btn_layout)

        # Статистика
        stats_group = QGroupBox("Статистика")
        stats_layout = QVBoxLayout(stats_group)

        # Общий рейтинг + кнопка сброса
        overall_layout = QHBoxLayout()
        overall_layout.addWidget(QLabel("Общий рейтинг:"))
        self.overall_bar = QProgressBar()
        self.overall_bar.setRange(0, 100)
        overall_layout.addWidget(self.overall_bar, 1)
        self.reset_btn = QPushButton("Сбросить прогресс")
        self.reset_btn.setIcon(self.style().standardIcon(QStyle.SP_DialogResetButton))
        self.reset_btn.clicked.connect(self.reset_progress)
        overall_layout.addWidget(self.reset_btn)
        stats_layout.addLayout(overall_layout)

        # Рейтинги по темам
        self.stats_table = QTableWidget()
        self.stats_table.setColumnCount(2)
        self.stats_table.setHorizontalHeaderLabels(["Тема", "Рейтинг"])
        self.stats_table.horizontalHeader().setSectionResizeMode(0, QHeaderView.Stretch)
        self.stats_table.horizontalHeader().setSectionResizeMode(1, QHeaderView.ResizeToContents)
        self.stats_table.setEditTriggers(QTableWidget.NoEditTriggers)
        stats_layout.addWidget(self.stats_table)

        main_layout.addWidget(stats_group)

    def init_core(self):
        try:
            self.trainer = core.CoreAPI(self.data_path)
            self.update_stats()
            self.next_task()
        except Exception as e:
            QMessageBox.critical(self, "Ошибка", f"Не удалось инициализировать ядро: {e}")
            sys.exit(1)

    def show_topic_dialog(self):
        if not self.stats_data:
            QMessageBox.warning(self, "Нет данных", "Сначала загрузите статистику.")
            return
        all_topics = list(self.stats_data.keys())
        dialog = TopicSelectionDialog(all_topics, self.selected_topics, self)
        if dialog.exec_() == QDialog.Accepted:
            new_selection = dialog.get_selected()
            self.selected_topics = new_selection
            self.trainer.set_enabled_topics(new_selection)
            self.next_task()

    def show_task_list(self):
        dialog = TaskListDialog(self.trainer, self)
        if dialog.exec_() == QDialog.Accepted:
            task_id = dialog.get_selected_task_id()
            if task_id is not None:
                self.current_task = self.trainer.get_task_by_id(task_id)
                self.display_task()

    def next_task(self):
        self.current_task = self.trainer.get_next_task()
        self.display_task()

    def display_task(self):
        if not self.current_task:
            return
        self.task_title_label.setText(self.current_task.title)
        self.task_text.setHtml(safe_html(self.current_task.question))
        if self.current_task.image_path:
            img_path = os.path.join(self.data_path, "images", self.current_task.image_path)
            if os.path.exists(img_path):
                pixmap = QPixmap(img_path)
                if not pixmap.isNull():
                    self.image_label.setPixmap(pixmap.scaled(400, 300, Qt.KeepAspectRatio, Qt.SmoothTransformation))
                    self.image_label.show()
                else:
                    self.image_label.hide()
            else:
                self.image_label.hide()
        else:
            self.image_label.hide()

    def show_answer(self):
        if not self.current_task:
            return
        dlg = QDialog(self)
        dlg.setWindowTitle("Разбор задачи")
        dlg.setMinimumSize(500, 400)
        layout = QVBoxLayout(dlg)

        text_edit = QTextEdit()
        text_edit.setReadOnly(True)
        text_edit.setHtml(safe_html(self.current_task.explanation))
        layout.addWidget(text_edit)

        button_box = QDialogButtonBox(QDialogButtonBox.Ok)
        button_box.accepted.connect(dlg.accept)
        layout.addWidget(button_box)

        dlg.exec_()

    def show_task_info(self):
        if not self.current_task:
            return
        task = self.current_task
        info = f"<b>Тема:</b> {safe_html(task.topic)}<br>"
        info += f"<b>Сложность:</b> {safe_html(str(task.difficulty))}/10<br>"
        info += f"<b>Источник:</b> {safe_html(task.source)}"
        QMessageBox.information(self, "Информация о задаче", info)

    def submit_result(self, solved):
        if not self.current_task:
            return
        self.trainer.submit_result(self.current_task.id, solved)
        self.update_stats()
        self.next_task()

    def update_stats(self):
        stats = self.trainer.get_stats()
        self.stats_data = stats["topics"]
        self.overall_bar.setValue(int(stats["overall"]))

        topics = stats["topics"]
        self.stats_table.setRowCount(len(topics))
        for row, (topic, rating) in enumerate(topics.items()):
            self.stats_table.setItem(row, 0, QTableWidgetItem(topic))
            progress = QProgressBar()
            progress.setRange(0, 100)
            progress.setValue(int(rating))
            self.stats_table.setCellWidget(row, 1, progress)

    def ask_yes_no(self, title, text):
        msg = QMessageBox(self)
        msg.setWindowTitle(title)
        msg.setText(text)
        yes_btn = msg.addButton("Да", QMessageBox.YesRole)
        no_btn = msg.addButton("Нет", QMessageBox.NoRole)
        msg.setDefaultButton(yes_btn)
        msg.exec_()
        return msg.clickedButton() == yes_btn

    def reset_progress(self):
        if self.ask_yes_no("Подтверждение", "Вы уверены, что хотите сбросить весь прогресс?"):
            self.trainer.reset_progress()
            self.update_stats()
            self.next_task()
            QMessageBox.information(self, "Сброс", "Прогресс сброшен.")

    def closeEvent(self, event):
        if self.trainer:
            self.trainer.shutdown()
        event.accept()


def main():
    app = QApplication(sys.argv)
    app.setFont(QFont("Segoe UI", 10))
    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    data_path = os.path.join(base_dir, "data")
    window = MainWindow(data_path)
    window.show()
    sys.exit(app.exec_())


if __name__ == "__main__":
    main()