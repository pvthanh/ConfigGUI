// SPDX-License-Identifier: MIT
// ValidationFeedbackWidget - Header

#ifndef CONFIGGUI_UI_VALIDATION_FEEDBACK_WIDGET_H
#define CONFIGGUI_UI_VALIDATION_FEEDBACK_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QString>
#include <QList>

namespace configgui {
namespace ui {

/**
 * @class ValidationFeedbackWidget
 * @brief Displays validation errors and warnings
 *
 * Shows detailed validation feedback to users with color coding
 * and clear error messages for form validation issues.
 */
class ValidationFeedbackWidget : public QWidget
{
    Q_OBJECT

public:
    enum class FeedbackLevel {
        Info,
        Warning,
        Error
    };

    explicit ValidationFeedbackWidget(QWidget* parent = nullptr);
    ~ValidationFeedbackWidget() override = default;

    /**
     * @brief Add validation feedback message
     * @param message Error/warning message
     * @param field Field name (optional)
     * @param level Severity level
     */
    void addFeedback(const QString& message, const QString& field = "",
                     FeedbackLevel level = FeedbackLevel::Error);

    /**
     * @brief Clear all feedback messages
     */
    void clearFeedback();

    /**
     * @brief Check if there are errors
     * @return True if any error messages
     */
    bool hasErrors() const { return error_count_ > 0; }

    /**
     * @brief Get error count
     * @return Number of error messages
     */
    int getErrorCount() const { return error_count_; }

    /**
     * @brief Set visibility based on feedback
     * @param has_feedback True if should show widget
     */
    void setHasFeedback(bool has_feedback);

signals:
    void feedbackAdded(const QString& message);
    void feedbackCleared();

private:
    struct FeedbackItem {
        QString message;
        QString field;
        FeedbackLevel level;
    };

    QScrollArea* scroll_area_;
    QWidget* container_;
    QList<FeedbackItem> feedback_items_;
    int error_count_;
    int warning_count_;

    void updateDisplay();
    QString getLevelStylesheet(FeedbackLevel level) const;
    QString getLevelIcon(FeedbackLevel level) const;
};

} // namespace ui
} // namespace configgui

#endif // CONFIGGUI_UI_VALIDATION_FEEDBACK_WIDGET_H
