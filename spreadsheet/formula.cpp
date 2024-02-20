#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
    class Formula : public FormulaInterface {
    public:
        // Реализуйте следующие методы:
        explicit Formula(std::string expression)
            : ast_(ParseFormulaAST(expression)) {}

        Value Evaluate(const SheetInterface& args) const override {
            try
            {
                double result = ast_.Execute(args);
                return Value(result);
            }
            catch (const FormulaError& ex)
            {
                return ex;
            }
        }

        std::string GetExpression() const override {
            std::ostringstream oss;
            ast_.PrintFormula(oss);
            return oss.str();
        }

        std::vector<Position> GetReferencedCells() const override {
            std::vector<Position> vec(ast_.GetCells().begin(), ast_.GetCells().end());
            auto last = std::unique(vec.begin(), vec.end());
            vec.erase(last, vec.end());
            return vec;
        }

    private:
        FormulaAST ast_;
    };

}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try
    {
        return std::make_unique<Formula>(std::move(expression));
    }
    catch (const std::exception&)
    {
        throw FormulaException("");
    }
}