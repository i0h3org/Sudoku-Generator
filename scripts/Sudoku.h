#pragma once

#define Sudoku_H

#include <cstdint>
#include <vector>
#include <array>
#include <set>

class Sudoku {

private:

    struct Box {

        std::array<uint8_t, 9> cells = { 0 };

        size_t band = 0;
        size_t stack = 0;

        // Returns a cell by reference through row and column coordinates
        uint8_t& cell(size_t r, size_t c) { size_t pos = 3 * r + c;  return cells[pos]; }

        // Returns an array of references for the numeric position indicating the relevant row
        std::array<std::reference_wrapper<uint8_t>, 3> row(size_t pos) {
            return { std::ref(cell(pos, 0)), std::ref(cell(pos, 1)), std::ref(cell(pos, 2)) };
        }
        // Returns an array of references for the numeric position indicating the relevant column
        std::array<std::reference_wrapper<uint8_t>, 3> col(size_t pos) {
            return { std::ref(cell(0, pos)), std::ref(cell(1, pos)), std::ref(cell(2, pos)) };
        }

        bool find(uint8_t val);

        const bool findRowVal(size_t pos, uint8_t val);
        const bool findColVal(size_t pos, uint8_t val);

        uint8_t* findRowNull(size_t pos);
        uint8_t* findColNull(size_t pos);

    };

    std::array<Box, 9> boxes;
    std::array<uint8_t*, 81> grid{nullptr};

    const std::array<size_t, 3> idxList = { 0, 1, 2 };
    const std::set<uint8_t> digits = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    void findNonAdjs(Box& rootBox, std::vector<Box*>& nAdjs);
    void findAdjs(Box& rootBox, std::vector<Box*>& bAdjs, std::vector<Box*>& sAdjs);

    void NonAdjFill(std::vector<Box*> Non_Adjs, std::vector<Box*> B_Adjs, std::vector<Box*> S_Adjs, std::array<std::set<uint8_t>, 4> avails, std::array<std::vector<uint8_t>, 4> pools);

protected:

    using Equiv_Map = std::array<uint8_t*, 16>;

    Equiv_Map ring = { nullptr };
    Equiv_Map assoc_map = { nullptr };

    void Phistemofel(Box& rootBox);
    void flattenGrid();

    virtual void Root_Propag(Box& rootBox, bool empty = true);
    virtual void Corner_Propag(Box& rootBox, bool empty = true);
    virtual void Line_Propag(std::array<uint8_t, 9> row, std::array<uint8_t, 9> col);

public:

    Sudoku();
    Sudoku& operator=(const Sudoku& source) {
      if (this == &source) return *this;  // self-assignment check
      
      this->boxes = source.boxes;

      return *this;  
    }

    Box& getBox(size_t pos);
    Box& getBox(size_t _band, size_t _stack);

    std::array<uint8_t*, 9> getRow(size_t pos);
    std::array<uint8_t*, 9> getRow(size_t bandIdx, size_t box_row);

    std::array<uint8_t*, 9> getCol(size_t pos);
    std::array<uint8_t*, 9> getCol(size_t stackIdx, size_t box_col);

    std::array<Box*, 3> getBand(size_t bandIndex);
    std::array<Box*, 3> getStack(size_t stackIndex);

    const bool findRowVal(size_t pos, uint8_t val);
    const bool findColVal(size_t pos, uint8_t val);

    void root_generate(bool type);
    void root_generate(bool type, size_t pos);
    void root_generate(bool type, Box box, size_t pos);
    void line_generate();
    void line_generate(std::array<uint8_t*, 9> row, std::array<uint8_t*, 9> col);

    virtual void torShift(size_t b_shift, size_t s_shift);

    void bandSwap(size_t idx1, size_t idx2);
    void bandRowSwap(size_t band, size_t idx1, size_t idx2);
    void stackSwap(size_t idx1, size_t idx2); 
    void stackColSwap(size_t stack, size_t idx1, size_t idx2);
    void digPermut(size_t num);
    void reflection(bool type);
    void rotation();
    void transpose();

    bool validateGrid();
    void clearGrid();

    void printGrid() const;

};
