#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <vector>

using namespace std;

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    bool IsCyclic(Position pos) const;
    bool DFS(Position current, std::set<Position>& visited, std::set<Position>& recStack) const;
    // Можете дополнить ваш класс нужными полями и методами

private:
    vector<vector<unique_ptr<Cell>>> sheet_;
    Size size_;

    void EnsureRowsAndCols(int rows, int cols);
    void UpdatePrintableSize();

};