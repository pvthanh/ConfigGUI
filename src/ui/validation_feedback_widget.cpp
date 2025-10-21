// SPDX-License-Identifier: MIT
// ValidationFeedbackWidget - Implementation

#include "validation_feedback_widget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QIcon>

namespace configgui {
namespace ui {

ValidationFeedbackWidget::ValidationFeedbackWidget(QWidget* parent)
    : QWidget(parent)
    , scroll_area_(new QScrollArea(this))
    , container_(new QWidget())
    , error_count_(0)
    , warning_count_(0)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    scroll_area_->setWidget(container_);
    scroll_area_->setWidgetResizable(true);
    scroll_area_->setMaximumHeight(200);
    layout->addWidget(scroll_area_);

    setStyleSheet(R"(
        QWidget {
            background-color: #f5f5f5;
            border: 1px solid #ddd;
            border-radius: 4px;
            padding: 8px;
        }
    )");

    hide();
}

void ValidationFeedbackWidget::addFeedback(const QString& message, const QString& field,
                                           FeedbackLevel level)
{
    FeedbackItem item{message, field, level};
    feedback_items_.append(item);

    if (level == FeedbackLevel::Error)
    {
        ++error_count_;
    }
    else if (level == FeedbackLevel::Warning)
    {
        ++warning_count_;
    }

    updateDisplay();
    emit feedbackAdded(message);
}

void ValidationFeedbackWidget::clearFeedback()
{
    feedback_items_.clear();
    error_count_ = 0;
    warning_count_ = 0;

    updateDisplay();
    emit feedbackCleared();
}

void ValidationFeedbackWidget::setHasFeedback(bool has_feedback)
{
    if (has_feedback)
    {
        show();
    }
    else
    {
        hide();
    }
}

void ValidationFeedbackWidget::updateDisplay()
{
    // Clear container layout
    QLayoutItem* item;
    QLayout* container_layout = container_->layout();
    if (container_layout == nullptr)
    {
        container_layout = new QVBoxLayout(container_);
    }
    else
    {
        while ((item = container_layout->takeAt(0)) != nullptr)
        {
            if (QWidget* widget = item->widget())
            {
                delete widget;
            }
            delete item;
        }
    }

    // Add feedback items
    for (const auto& feedback : feedback_items_)
    {
        auto* frame = new QFrame();
        frame->setStyleSheet(R"(
            QFrame {
                background-color: white;
                border: 1px solid #ccc;
                border-radius: 3px;
                padding: 8px;
                margin: 4px 0px;
            }
        )");

        auto* frame_layout = new QVBoxLayout(frame);
        frame_layout->setContentsMargins(5, 5, 5, 5);
        frame_layout->setSpacing(3);

        // Field name (if provided)
        if (!feedback.field.isEmpty())
        {
            auto* field_label = new QLabel("<b>" + feedback.field + "</b>");
            frame_layout->addWidget(field_label);
        }

        // Message
        auto* message_label = new QLabel(feedback.message);
        message_label->setWordWrap(true);

        // Color based on level
        QString color = "#d32f2f"; // Error red
        if (feedback.level == FeedbackLevel::Warning)
        {
            color = "#f57c00"; // Warning orange
        }
        else if (feedback.level == FeedbackLevel::Info)
        {
            color = "#1976d2"; // Info blue
        }

        message_label->setStyleSheet("color: " + color + ";");
        frame_layout->addWidget(message_label);

        container_layout->addWidget(frame);
    }

    // Add stretch if it's a box layout
    if (auto* box_layout = qobject_cast<QBoxLayout*>(container_layout))
    {
        box_layout->addStretch();
    }

    setHasFeedback(!feedback_items_.isEmpty());
}

QString ValidationFeedbackWidget::getLevelStylesheet(FeedbackLevel level) const
{
    switch (level)
    {
        case FeedbackLevel::Error:
            return "color: #d32f2f; font-weight: bold;";
        case FeedbackLevel::Warning:
            return "color: #f57c00; font-weight: bold;";
        case FeedbackLevel::Info:
            return "color: #1976d2;";
        default:
            return "";
    }
}

QString ValidationFeedbackWidget::getLevelIcon(FeedbackLevel level) const
{
    switch (level)
    {
        case FeedbackLevel::Error:
            return "❌";
        case FeedbackLevel::Warning:
            return "⚠️";
        case FeedbackLevel::Info:
            return "ℹ️";
        default:
            return "";
    }
}

} // namespace ui
} // namespace configgui
