/*
 * Copyright (C) 2025 ConfigGUI Contributors
 * SPDX-License-Identifier: MIT
 *
 * format_selection_dialog.cpp
 * Qt GUI Dialog implementation for format selection
 */

#include "format_selection_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QTextEdit>
#include <QApplication>
#include <QScreen>

namespace configgui::ui {

FormatSelectionDialog::FormatSelectionDialog(QWidget* parent)
    : QDialog(parent)
    , current_format(FormatType::JSON)
{
    setWindowTitle("Select Configuration Format");
    setModal(true);
    setMinimumWidth(400);
    setMinimumHeight(300);

    setup_ui();
    populate_formats();

    // Center on parent screen
    if (parent) {
        QRect parentGeometry = parent->frameGeometry();
        QScreen* screen = QApplication::screenAt(parent->mapToGlobal(parent->rect().center()));
        if (screen) {
            QRect screenGeometry = screen->geometry();
            (void)screenGeometry; // Mark as intentionally unused for screen dimension reference
            int x = parentGeometry.center().x() - this->width() / 2;
            int y = parentGeometry.center().y() - this->height() / 2;
            this->move(x, y);
        }
    }
}

void FormatSelectionDialog::setup_ui()
{
    main_layout = new QVBoxLayout(this);

    // Title label
    QLabel* title = new QLabel("Choose Configuration File Format:");
    title->setStyleSheet("font-weight: bold; font-size: 12pt;");
    main_layout->addWidget(title);

    // Format selection group
    QGroupBox* format_group = new QGroupBox("File Format");
    QVBoxLayout* group_layout = new QVBoxLayout(format_group);

    format_combo = new QComboBox();
    group_layout->addWidget(new QLabel("Format:"));
    group_layout->addWidget(format_combo);

    main_layout->addWidget(format_group);

    // Format information
    QGroupBox* info_group = new QGroupBox("Format Information");
    QVBoxLayout* info_layout = new QVBoxLayout(info_group);

    description_label = new QLabel();
    description_label->setWordWrap(true);
    description_label->setMinimumHeight(80);
    info_layout->addWidget(description_label);

    format_info_label = new QLabel();
    format_info_label->setWordWrap(true);
    info_layout->addWidget(format_info_label);

    main_layout->addWidget(info_group);

    // Buttons
    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addStretch();

    ok_button = new QPushButton("Save");
    cancel_button = new QPushButton("Cancel");

    ok_button->setMinimumWidth(80);
    cancel_button->setMinimumWidth(80);

    button_layout->addWidget(ok_button);
    button_layout->addWidget(cancel_button);

    main_layout->addLayout(button_layout);

    // Connect signals
    connect(format_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FormatSelectionDialog::on_format_changed);
    connect(ok_button, &QPushButton::clicked,
            this, &FormatSelectionDialog::on_ok_clicked);
    connect(cancel_button, &QPushButton::clicked,
            this, &FormatSelectionDialog::on_cancel_clicked);
}

void FormatSelectionDialog::populate_formats()
{
    format_combo->addItem("JSON (.json)");
    format_combo->addItem("INI (.ini)");

    // Set initial selection and update info
    format_combo->setCurrentIndex(INDEX_JSON);
    on_format_changed(INDEX_JSON);
}

FormatType FormatSelectionDialog::selected_format() const
{
    return current_format;
}

QString FormatSelectionDialog::selected_format_name() const
{
    switch (current_format) {
    case FormatType::JSON:
        return "JSON";
    case FormatType::INI:
        return "INI";
    default:
        return "Unknown";
    }
}

QString FormatSelectionDialog::selected_format_extension() const
{
    switch (current_format) {
    case FormatType::JSON:
        return ".json";
    case FormatType::INI:
        return ".ini";
    default:
        return "";
    }
}

void FormatSelectionDialog::on_format_changed(int index)
{
    if (index == INDEX_JSON) {
        current_format = FormatType::JSON;
        description_label->setText(
            "JSON (JavaScript Object Notation)\n\n"
            "A lightweight text format for storing and exchanging data. "
            "JSON preserves data types and nested structures perfectly. "
            "Human-readable and widely supported."
        );
        format_info_label->setText(
            "✓ Preserves all data types\n"
            "✓ Supports nested structures\n"
            "✓ Human-readable format"
        );
    } else if (index == INDEX_INI) {
        current_format = FormatType::INI;
        description_label->setText(
            "INI (Initialization File)\n\n"
            "A simple key-value format commonly used for configuration files. "
            "Readable and compact, though with limited support for nested structures. "
            "Widely recognized by many applications."
        );
        format_info_label->setText(
            "✓ Simple key-value format\n"
            "✓ Compact and readable\n"
            "✓ Supports sections/groups"
        );
    }
}

void FormatSelectionDialog::on_ok_clicked()
{
    accept();
}

void FormatSelectionDialog::on_cancel_clicked()
{
    reject();
}

} // namespace configgui::ui
