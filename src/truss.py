import os
import subprocess
import sys
import tempfile

from PySide6.QtCore import Qt
from PySide6.QtGui import QColor, QFont, QIcon, QPalette, QTextCursor
from PySide6.QtWidgets import (QApplication, QFileDialog, QGroupBox,
                               QHBoxLayout, QLabel, QLineEdit, QMainWindow,
                               QMessageBox, QPushButton, QSizePolicy, QSpinBox,
                               QStatusBar, QStyle, QTabWidget, QTextEdit,
                               QVBoxLayout, QWidget)

def resource_path(relative_path):
    """Get path relative to the executable or script."""
    if getattr(sys, 'frozen', False):
        base_path = os.path.dirname(sys.executable)
    else:
        base_path = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(base_path, relative_path)


class TrussAnalysisGUI(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Truss Analysis Tool - Created By: Youness")
        self.setGeometry(100, 100, 900, 700)

        # Set application icon
        self.set_app_icon()

        # Initialize UI
        self.init_ui()
        self.create_actions()

    def set_app_icon(self):
        """Set application icon with fallback to standard icon"""
        try:
            # Try to load a custom icon if available
            app_icon = resource_path("assets/icon.png")
            if os.path.exists(app_icon):
                self.setWindowIcon(QIcon(app_icon))
            else:
                # Use a standard icon as fallback
                self.setWindowIcon(QIcon.fromTheme("applications-engineering"))
        except:
            # If all else fails, use a simple system icon
            self.setWindowIcon(self.style().standardIcon(QStyle.SP_ComputerIcon))

    def init_ui(self):
        # Central widget and main layout
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)
        main_layout.setSpacing(15)
        main_layout.setContentsMargins(15, 15, 15, 15)

        # Create tab widget for input/output
        self.tabs = QTabWidget()
        self.tabs.setDocumentMode(True)
        self.tabs.setTabPosition(QTabWidget.North)
        main_layout.addWidget(self.tabs)

        # Create input tab
        self.input_tab = QWidget()
        self.tabs.addTab(self.input_tab, "Input")
        self.setup_input_tab()

        # Create output tab
        self.output_tab = QWidget()
        self.tabs.addTab(self.output_tab, "Output")
        self.setup_output_tab()

        # Create status bar
        self.status_bar = QStatusBar()
        self.setStatusBar(self.status_bar)
        self.status_bar.showMessage("Ready")

    def setup_icons(self):
        """Set up icons for buttons using system theme icons"""
        # Get system icons
        browse_icon = self.style().standardIcon(QStyle.SP_DirOpenIcon)
        run_icon = self.style().standardIcon(QStyle.SP_MediaPlay)
        clear_icon = self.style().standardIcon(QStyle.SP_DialogResetButton)
        save_icon = self.style().standardIcon(QStyle.SP_DialogSaveButton)
        copy_icon = self.style().standardIcon(QStyle.SP_FileDialogContentsView)

        # Apply icons to buttons
        self.browse_btn.setIcon(browse_icon)
        self.run_btn.setIcon(run_icon)
        self.clear_btn.setIcon(clear_icon)
        self.save_btn.setIcon(save_icon)
        self.copy_btn.setIcon(copy_icon)

    def setup_input_tab(self):
        layout = QVBoxLayout(self.input_tab)
        layout.setSpacing(15)
        layout.setContentsMargins(10, 10, 10, 10)

        # File selection group
        file_group = QGroupBox("Input File")
        file_layout = QVBoxLayout(file_group)

        file_input_layout = QHBoxLayout()
        self.file_path = QLineEdit()
        self.file_path.setPlaceholderText("Select a CSV file...")
        file_input_layout.addWidget(self.file_path)

        self.browse_btn = QPushButton("Browse...")
        self.browse_btn.setFixedWidth(100)
        file_input_layout.addWidget(self.browse_btn)

        file_layout.addLayout(file_input_layout)
        layout.addWidget(file_group)

        # Decimal places selection
        decimal_group = QGroupBox("Precision Settings")
        decimal_layout = QHBoxLayout(decimal_group)

        decimal_layout.addWidget(QLabel("Decimal Places:"))
        self.decimal_spin = QSpinBox()
        self.decimal_spin.setRange(1, 12)
        self.decimal_spin.setValue(6)
        self.decimal_spin.setFixedWidth(80)
        decimal_layout.addWidget(self.decimal_spin)
        decimal_layout.addStretch()

        layout.addWidget(decimal_group)

        # CSV input area
        csv_group = QGroupBox("CSV Data Input")
        csv_layout = QVBoxLayout(csv_group)

        self.csv_editor = QTextEdit()
        self.csv_editor.setPlaceholderText(
            "Paste your CSV data here or load from file above...\n\n"
            "Example:\n"
            "# Node id, x, y, z, fix_x, fix_y, fix_z, load_x, load_y, load_z\n"
            "1, 0, 0, 0, 1, 1, 1, 0, 0, 0\n"
            "2, 60, 60, 0, 0, 0, 0, 0, -10, -1\n"
            "3, 120, 0, 0, 0, 1, 1, 0, 0, 0\n\n"
            "# Member id, start_node, end_node, E, A\n"
            "1, 2, 1, 29000, 0.75\n"
            "2, 2, 3, 29000, 0.75\n"
            "3, 1, 3, 29000, 0.5"
        )
        self.csv_editor.setFont(QFont("Consolas", 10))
        self.csv_editor.setAcceptRichText(False)
        csv_layout.addWidget(self.csv_editor)

        layout.addWidget(csv_group)

        # Action buttons
        button_layout = QHBoxLayout()
        self.run_btn = QPushButton("Run Analysis")
        self.run_btn.setFixedHeight(40)
        self.run_btn.setStyleSheet("font-weight: bold;")
        button_layout.addWidget(self.run_btn)

        self.clear_btn = QPushButton("Clear Input")
        self.clear_btn.setFixedHeight(40)
        button_layout.addWidget(self.clear_btn)

        layout.addLayout(button_layout)

    def setup_output_tab(self):
        layout = QVBoxLayout(self.output_tab)
        layout.setSpacing(10)
        layout.setContentsMargins(10, 10, 10, 10)

        # Output display
        output_group = QGroupBox("Analysis Results")
        output_layout = QVBoxLayout(output_group)

        self.output_display = QTextEdit()
        self.output_display.setReadOnly(True)
        self.output_display.setFont(QFont("Consolas", 10))
        self.output_display.setLineWrapMode(QTextEdit.NoWrap)
        output_layout.addWidget(self.output_display)

        layout.addWidget(output_group)

        # Output action buttons
        output_btn_layout = QHBoxLayout()
        self.save_btn = QPushButton("Save Output")
        self.save_btn.setFixedHeight(35)
        output_btn_layout.addWidget(self.save_btn)

        self.copy_btn = QPushButton("Copy to Clipboard")
        self.copy_btn.setFixedHeight(35)
        output_btn_layout.addWidget(self.copy_btn)

        layout.addLayout(output_btn_layout)

    def create_actions(self):
        # Setup icons
        self.setup_icons()

        # Connect buttons to functions
        self.browse_btn.clicked.connect(self.browse_file)
        self.run_btn.clicked.connect(self.run_analysis)
        self.clear_btn.clicked.connect(self.clear_input)
        self.save_btn.clicked.connect(self.save_output)
        self.copy_btn.clicked.connect(self.copy_to_clipboard)

    def browse_file(self):
        file_path, _ = QFileDialog.getOpenFileName(
            self, "Select CSV File", "", "CSV Files (*.csv);;All Files (*)"
        )
        if file_path:
            self.file_path.setText(file_path)
            try:
                with open(file_path, "r") as f:
                    self.csv_editor.setText(f.read())
                self.status_bar.showMessage(f"Loaded: {os.path.basename(file_path)}")
            except Exception as e:
                QMessageBox.critical(
                    self, "File Error", f"Could not read file: {str(e)}"
                )

    def clear_input(self):
        self.file_path.clear()
        self.csv_editor.clear()
        self.status_bar.showMessage("Input cleared")

    def run_analysis(self):
        # Validate input
        file_path = self.file_path.text().strip()
        csv_data = self.csv_editor.toPlainText().strip()
        decimal_places = self.decimal_spin.value()

        if not file_path and not csv_data:
            QMessageBox.warning(
                self, "Input Error", "Please select a file or enter CSV data"
            )
            return

        self.status_bar.showMessage("Running analysis...")
        QApplication.processEvents()  # Update UI

        try:
            # Create temp file if using pasted data
            if csv_data:
                with tempfile.NamedTemporaryFile(
                    mode="w+", suffix=".csv", delete=False
                ) as tmpfile:
                    tmpfile.write(csv_data)
                    input_file = tmpfile.name
            else:
                input_file = file_path

            # Run the C program
            exe_path = resource_path("truss_engine")
            if not os.path.exists(exe_path):
                # Windows fallback
                exe_path = resource_path("truss_engine.exe")

            if not os.path.exists(exe_path):
                QMessageBox.critical(
                    self,
                    "Executable Not Found",
                    f"Could not find truss engine executable at: {exe_path}",
                )
                return

            result = subprocess.run(
                [exe_path, input_file, str(decimal_places)],
                capture_output=True,
                text=True,
            )

            if result.returncode == 0:
                self.output_display.setPlainText(result.stdout)
                self.tabs.setCurrentIndex(1)  # Switch to output tab
                self.status_bar.showMessage("Analysis completed successfully")
            else:
                error_msg = result.stderr if result.stderr else "Unknown error occurred"
                QMessageBox.critical(
                    self,
                    "Analysis Error",
                    f"Analysis failed with error:\n\n{error_msg}",
                )
                self.status_bar.showMessage("Analysis failed")

        except Exception as e:
            QMessageBox.critical(
                self, "Error", f"An unexpected error occurred: {str(e)}"
            )
            self.status_bar.showMessage("Error occurred")
        finally:
            # Clean up temp file
            if csv_data and "tmpfile" in locals():
                os.unlink(input_file)

    def save_output(self):
        content = self.output_display.toPlainText()
        if not content.strip():
            QMessageBox.warning(self, "No Output", "There is no output to save")
            return

        file_path, _ = QFileDialog.getSaveFileName(
            self, "Save Output", "", "Text Files (*.txt);;All Files (*)"
        )

        if file_path:
            try:
                with open(file_path, "w") as f:
                    f.write(content)
                self.status_bar.showMessage(f"Output saved to: {file_path}")
            except Exception as e:
                QMessageBox.critical(
                    self, "Save Error", f"Could not save file: {str(e)}"
                )

    def copy_to_clipboard(self):
        content = self.output_display.toPlainText()
        if content:
            clipboard = QApplication.clipboard()
            clipboard.setText(content)
            self.status_bar.showMessage("Output copied to clipboard")
        else:
            QMessageBox.warning(self, "No Output", "There is no output to copy")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setStyle("Fusion")  # Use Fusion style for consistent look across platforms

    # Set palette to respect system theme
    palette = app.palette()

    # Adjust palette for better readability in both light and dark modes
    if app.style().objectName().lower() == "fusion":
        # Detect dark mode based on window color brightness
        window_color = palette.color(QPalette.Window)
        if window_color.lightness() < 128:
            # Dark mode adjustments
            palette.setColor(QPalette.Text, QColor(220, 220, 220))
            palette.setColor(QPalette.Base, QColor(45, 45, 45))
            palette.setColor(QPalette.Window, QColor(35, 35, 35))
            palette.setColor(QPalette.Button, QColor(60, 60, 60))
            palette.setColor(QPalette.Highlight, QColor(42, 130, 218))
        else:
            # Light mode adjustments
            palette.setColor(QPalette.Highlight, QColor(42, 130, 218))

        app.setPalette(palette)

    window = TrussAnalysisGUI()
    window.show()
    sys.exit(app.exec())
