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
        return Value(); // Возвращаем пустое значение для пустой ячейки
    }

    std::string GetText() const override {
        return std::string(); // Возвращаем пустое значение для пустой ячейки
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
            if (text_ == "") {
                return Value(0.0);
            }
            if (text_[0] == ESCAPE_SIGN) {
                return Value(text_.substr(1));
            }
            return Value(text_);
        }
        catch (const std::exception&)
        {
            if (text_ == "") {
                return Value(0.0);
            }
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
    if (text.size() > 1 && text[0] == FORMULA_SIGN) {
        string cash;
        bool is_cash = false;
        if (impl_ != nullptr) {
            cash = impl_->GetText();
            is_cash = true;
        }
        impl_ = std::make_unique<FormulaImpl>(ParseFormula(text.substr(1)), sheet_);
        for(auto cell : GetReferencedCells()) {
            if (sheet_.GetCell(cell) == nullptr) {
                sheet_.SetCell(cell, "");
            }
        }
        if (sheet_.IsCyclic(pos)) {
            if (is_cash) {
                if (cash.size() > 1 && cash[0] == FORMULA_SIGN) {
                    impl_ = std::make_unique<FormulaImpl>(ParseFormula(cash.substr(1)), sheet_);
                }
                else {
                    impl_ = std::make_unique<TextImpl>(cash);
                }
            }
            throw CircularDependencyException("CircularDependencyException");
        }
        if (std::holds_alternative<double>(impl_->GetValue())) {
            cache_ = std::get<double>(impl_->GetValue());
        }
    }
    else {
        impl_ = std::make_unique<TextImpl>(text);
    }
}

void Cell::Clear() {
    impl_.reset();
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

double Cell::GetCache() const {
    return cache_.value();
}

bool Cell::GetImpl() {
    if (impl_ == nullptr) {
        return false;
    }
    return true;
}