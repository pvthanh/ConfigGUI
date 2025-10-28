/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * format_selection_dialog.h
 * Qt GUI Dialog for format selection during configuration save
 * Provides user interface for choosing between JSON and INI formats
 */

#ifndef CONFIGGUI_UI_FORMAT_SELECTION_DIALOG_H
#define CONFIGGUI_UI_FORMAT_SELECTION_DIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>

#include "core/models/format_type.h"

using namespace configgui::core::models;

namespace configgui::ui {

/**
 * @class FormatSelectionDialog
 * Dialog for selecting configuration file format during save operation
 *
 * Provides a user-friendly interface to choose between supported formats:
 * - JSON (JavaScript Object Notation)
 * - INI (Initialization file format)
 *
 * Usage:
 *   FormatSelectionDialog dialog(this);
 *   if (dialog.exec() == QDialog::Accepted) {
 *       FormatType format = dialog.selected_format();
 *       // Use format for save operation
 *   }
 */
class FormatSelectionDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * Create a format selection dialog
     * @param parent Parent widget (typically main window)
     */
    explicit FormatSelectionDialog(QWidget* parent = nullptr);

    /**
     * Get the format selected by user
     * @return Selected FormatType (JSON or INI)
     */
    FormatType selected_format() const;

    /**
     * Get the format name as string
     * @return Format name ("JSON" or "INI")
     */
    QString selected_format_name() const;

    /**
     * Get the file extension for selected format
     * @return File extension (".json" or ".ini")
     */
    QString selected_format_extension() const;

private slots:
    /**
     * Handle OK button click
     */
    void on_ok_clicked();

    /**
     * Handle Cancel button click
     */
    void on_cancel_clicked();

    /**
     * Handle format selection change
     */
    void on_format_changed(int index);

private:
    /**
     * Initialize UI components
     */
    void setup_ui();

    /**
     * Populate format combo box with available formats
     */
    void populate_formats();

    // UI Components
    QComboBox* format_combo;
    QLabel* description_label;
    QLabel* format_info_label;
    QPushButton* ok_button;
    QPushButton* cancel_button;
    QVBoxLayout* main_layout;

    // Current selection
    FormatType current_format;

    /**
     * Map format index to FormatType
     */
    static const int INDEX_JSON = 0;
    static const int INDEX_INI = 1;
};

} // namespace configgui::ui

#endif // CONFIGGUI_UI_FORMAT_SELECTION_DIALOG_H
