#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <stack>
#include <set>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    EnsureRowsAndCols(pos.row + 1, pos.col + 1);

    if (!sheet_[pos.row][pos.col]) {
        sheet_[pos.row][pos.col] = std::make_unique<Cell>(*this);
    }

    sheet_[pos.row][pos.col]->Set(text, pos);

    // Обновляем размер области, участвующей в печати
    size_.rows = std::max(size_.rows, pos.row + 1);
    size_.cols = std::max(size_.cols, pos.col + 1);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (pos.IsValid() && pos.row < static_cast<int>(sheet_.size()) &&
        pos.col < static_cast<int>(sheet_[pos.row].size())) {
        return sheet_[pos.row][pos.col].get();
    }
    else if(!pos.IsValid()) {
        throw InvalidPositionException("InvalidPositionException");
    }
    return nullptr;
}
CellInterface* Sheet::GetCell(Position pos) {
    return const_cast<CellInterface*>(static_cast<const Sheet*>(this)->GetCell(pos));
}

void Sheet::ClearCell(Position pos) {
    //Не разобрался, как красиво и без дублирования можно сделать
    if (pos.IsValid() && pos.row < static_cast<int>(sheet_.size()) && pos.col < static_cast<int>(sheet_[pos.row].size())) {
        sheet_[pos.row][pos.col].reset();
        UpdatePrintableSize();
    }
    else if(!pos.IsValid()){
        throw InvalidPositionException("InvalidPositionException");
    }
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int row = 0; row < size_.rows; ++row) {
        bool is_t = false;
        for (int col = 0; col < size_.cols; ++col) {
            if (auto cell = GetCell({ row, col })) {
                if (is_t) {
                    output << '\t';
                }

                if (holds_alternative<string>(cell->GetValue())) {
                    output << get<string>(cell->GetValue());
                }
                else if (holds_alternative<double>(cell->GetValue())) {
                    output << get<double>(cell->GetValue());
                }
                else if (holds_alternative<FormulaError>(cell->GetValue())) {
                    output << get<FormulaError>(cell->GetValue());
                }

                is_t = true;
            }
            else {
                if (is_t) {
                    output << '\t';
                }
                is_t = true;
            }
        }
        output << '\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for (int row = 0; row < size_.rows; ++row) {
        bool is_t = false;
        for (int col = 0; col < size_.cols; ++col) {
            if (auto cell = GetCell({ row, col })) {
                if (is_t) {
                    output << '\t';
                }
                output << cell->GetText();
                is_t = true;
            }
            else {
                if (is_t) {
                    output << '\t';
                }
                is_t = true;
            }
        }
        output << '\n';
    }
}

void Sheet::EnsureRowsAndCols(int rows, int cols) {
    if (rows > static_cast<int>(sheet_.size())) {
        sheet_.resize(rows);
    }

    for (int row = 0; row < rows; ++row) {
        if (cols > static_cast<int>(sheet_[row].size())) {
            sheet_[row].resize(cols);
        }
    }
}

void Sheet::UpdatePrintableSize() {
    size_ = { 0, 0 };

    for (int row = 0; row < static_cast<int>(sheet_.size()); ++row) {
        for (int col = 0; col < static_cast<int>(sheet_[row].size()); ++col) {
            if (sheet_[row][col]) {
                size_.rows = std::max(size_.rows, row + 1);
                size_.cols = std::max(size_.cols, col + 1);
            }
        }
    }
}

bool Sheet::IsCyclic(Position pos) const {
    std::set<Position> visited;
    std::set<Position> recStack;

    return DFS(pos, visited, recStack);
}

bool Sheet::DFS(Position current, std::set<Position>& visited, std::set<Position>& recStack) const {
    visited.insert(current);
    recStack.insert(current);

    const CellInterface* currentCell = GetCell(current);
    if (currentCell) {
        for (const Position& neighbor : currentCell->GetReferencedCells()) {
            if (!visited.count(neighbor)) {
                if (DFS(neighbor, visited, recStack)) {
                    return true;
                }
            }
            else if (recStack.count(neighbor)) {
                return true;
            }
        }
    }

    recStack.erase(current);
    return false;
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}