// Generic Qt widget generator for simple "config" structs.
//
// USAGE:
//   1. Give your struct a `static auto fields()` returning a tuple of
//      FIELD(...) entries — one per member you want in the UI.
//   2. Call buildConfigWidget(myConfigInstance) to get a QWidget with a
//      QFormLayout of the right input types, already bound to the struct.
//
// Supported member types out of the box: int, float/double, bool,
// std::string / QString. Add more branches in addFieldRow() as needed
// (e.g. enums -> QComboBox).

#ifndef ALGOCONFIGWIDGET_H
#define ALGOCONFIGWIDGET_H

#include "Commons.h"
#include <tuple>
#include <type_traits>
#include <string>

#include <QWidget>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QString>

// ---------------------------------------------------------------------------
// addFieldRow: creates the correct widget for one field and binds it.
//
// NOTE ON LIFETIME: the lambdas below capture `config` by reference.
// That's fine as long as `config` outlives the widget (typical case: both
// owned by the same dialog/window). If that's not guaranteed in your app,
// capture a pointer/handle you control instead, or push changes through a
// signal rather than writing to `config` directly.
// ---------------------------------------------------------------------------
template <typename Struct, typename Member>
void addFieldRow(QFormLayout* layout, Struct& config, const FieldInfo<Struct, Member>& info) {
    if constexpr (std::is_same_v<Member, int>) {
        auto* spin = new QSpinBox;
        spin->setRange(info.hasRange ? int(info.minVal) : -1'000'000,
                       info.hasRange ? int(info.maxVal) :  1'000'000);
        spin->setSingleStep(int(info.step));
        spin->setValue(config.*(info.ptr));
        QObject::connect(spin, qOverload<int>(&QSpinBox::valueChanged),
                         [&config, ptr = info.ptr](int v) { config.*ptr = v; });
        layout->addRow(info.label, spin);
    }
    else if constexpr (std::is_same_v<Member, float> || std::is_same_v<Member, double>) {
        auto* spin = new QDoubleSpinBox;
        spin->setRange(info.hasRange ? info.minVal : -1e6, info.hasRange ? info.maxVal : 1e6);
        spin->setSingleStep(info.hasRange ? info.step : 0.1);
        spin->setValue(config.*(info.ptr));
        QObject::connect(spin, qOverload<double>(&QDoubleSpinBox::valueChanged),
                         [&config, ptr = info.ptr](double v) { config.*ptr = static_cast<Member>(v); });
        layout->addRow(info.label, spin);
    }
    else if constexpr (std::is_same_v<Member, bool>) {
        auto* check = new QCheckBox;
        check->setChecked(config.*(info.ptr));
        QObject::connect(check, &QCheckBox::toggled,
                         [&config, ptr = info.ptr](bool v) { config.*ptr = v; });
        layout->addRow(info.label, check);
    }
    else if constexpr (std::is_same_v<Member, std::string>) {
        auto* edit = new QLineEdit;
        edit->setText(QString::fromStdString(config.*(info.ptr)));
        QObject::connect(edit, &QLineEdit::textChanged,
                         [&config, ptr = info.ptr](const QString& v) { config.*ptr = v.toStdString(); });
        layout->addRow(info.label, edit);
    }
    else if constexpr (std::is_same_v<Member, QString>) {
        auto* edit = new QLineEdit;
        edit->setText(config.*(info.ptr));
        QObject::connect(edit, &QLineEdit::textChanged,
                         [&config, ptr = info.ptr](const QString& v) { config.*ptr = v; });
        layout->addRow(info.label, edit);
    }
    else {
        static_assert(sizeof(Member) == 0, "addFieldRow: unsupported member type - add a branch for it");
    }
}

// ---------------------------------------------------------------------------
// buildConfigWidget: builds a QWidget with one row per field, in order.
// ---------------------------------------------------------------------------
template <typename Struct>
QWidget* buildConfigWidget(Struct& config, QWidget* parent = nullptr) {
    auto* widget = new QWidget(parent);
    auto* layout = new QFormLayout(widget);

    std::apply(
        [&](auto&&... field) { (addFieldRow(layout, config, field), ...); },
        Struct::fields()
        );

    return widget;
}

#endif // ALGOCONFIGWIDGET_H
