#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <optional>
#include <set>
#include <map>

class Sheet;
class Impl;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text, Position pos);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    double GetCache() const;
    bool GetImpl();

private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;
    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;
    mutable std::optional<double> cache_;
    // Добавьте поля и методы для связи с таблицей, проверки циклических 
    // зависимостей, графа зависимостей и т. д.
};