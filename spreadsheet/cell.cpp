#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

class Cell::Impl {
public:
    virtual ~Impl() = default;
    virtual Cell::Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

class Cell::EmptyImpl : public Impl {
public:
    Value GetValue() const override {
        return Value(0.0); // Возвращаем пустое значение для пустой ячейки
    }

    std::string GetText() const override {
        return std::string(""); // Возвращаем пустое значение для пустой ячейки
    }

    std::vector<Position> GetReferencedCells() const override {
        return {}; // Пустой список зависимостей для пустой ячейки
    }
};

class Cell::TextImpl : public Impl {
public:
    TextImpl(std::string text)
        : text_(std::move(text)) {}

    Value GetValue() const override {
        try
        {
            size_t pos;
            double integerValue = std::stod(text_, &pos);
            if (pos == text_.size()) {
                return Value(integerValue);
            }
            if (text_[0] == ESCAPE_SIGN) {
                return Value(text_.substr(1));
            }
            return Value(text_);
        }
        catch (const std::exception&)
        {
            if (text_[0] == ESCAPE_SIGN) {
                return Value(text_.substr(1));
            }
            return Value(text_);
        }
    }

    std::string GetText() const override {
        // Просто возвращаем текст как значение
        return text_;
    }

    std::vector<Position> GetReferencedCells() const override {
        // Здесь может быть логика для поиска зависимостей в тексте
        return {};
    }

private:
    std::string text_;
};

class Cell::FormulaImpl : public Impl {
public:
    explicit FormulaImpl(std::unique_ptr<FormulaInterface> formula, Sheet& sh)
        : formula_(std::move(formula)), sheet(sh){}

    Value GetValue() const override {
        auto result = formula_->Evaluate(sheet);
        if (std::holds_alternative<double>(result)) {
            return Value(std::get<double>(result));
        }
        else {
            return Value(std::get<FormulaError>(result));
        }
    }

    std::string GetText() const override {
        return FORMULA_SIGN + formula_->GetExpression();
    }

    std::vector<Position> GetReferencedCells() const override {
        return formula_->GetReferencedCells();
    }

private:
    std::unique_ptr<FormulaInterface> formula_;
    Sheet& sheet;
};

//==============================================================================================================================//
//==============================================================================================================================//
//==============================================================================================================================//

Cell::Cell(Sheet& sheet) 
: sheet_(sheet) {}

Cell::~Cell() = default;

void Cell::Set(std::string text, Position pos) {
    if (text.empty()) {
        Clear();
    }
    else if (text.size() > 1 && text[0] == FORMULA_SIGN) {
        GetTextCash();
        impl_ = std::make_unique<FormulaImpl>(ParseFormula(text.substr(1)), sheet_);
        ReferencedCells();
        FindCyclicDependencies(pos);
        UpdateCache();
    }
    else {
        impl_ = std::make_unique<TextImpl>(text);
    }
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    if (cache_.has_value()) {
        return cache_.value();
    }
    else {
        return impl_->GetValue();
    }
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

void Cell::FindCyclicDependencies(Position pos) {
    if (sheet_.IsCyclic(pos)) {
        if (is_text_cash) {
            if (text_cash.size() > 1 && text_cash[0] == FORMULA_SIGN) {
                impl_ = std::make_unique<FormulaImpl>(ParseFormula(text_cash.substr(1)), sheet_);
            }
            else {
                impl_ = std::make_unique<TextImpl>(text_cash);
            }
        }
        throw CircularDependencyException("CircularDependencyException");
    }
}

void Cell::UpdateCache() {
    if (std::holds_alternative<double>(impl_->GetValue())) {
        cache_ = std::get<double>(impl_->GetValue());
    }
}

void Cell::GetTextCash() {
    if (impl_ != nullptr) {
        text_cash = impl_->GetText();
        is_text_cash = true;
    }
}

void Cell::ReferencedCells() {
    for (auto cell : GetReferencedCells()) {
        if (sheet_.GetCell(cell) == nullptr) {
            sheet_.SetCell(cell, "");
        }
    }
}