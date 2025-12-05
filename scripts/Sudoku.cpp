#include "Sudoku.h"

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <algorithm>
#include <numeric>
#include <random>
#include <cassert>

static thread_local std::mt19937 rng {
		[] {
				std::random_device rd;
				std::array<std::uint32_t, 624> seed_data{};
				for (auto& s : seed_data) s = rd();
				std::seed_seq seq(seed_data.begin(), seed_data.end());
				return std::mt19937(seq);
		} ()
};

Sudoku::Sudoku() { flattenGrid(); }

uint8_t* Sudoku::Box::findRowNull(size_t pos) {// Find first empty position in a Box's row
	for (size_t c = 0; c < 3; c++) {
		uint8_t& _cell = cell(pos, c);
		if (!_cell) {
			return &_cell;
		}
	}

	return nullptr;
}

uint8_t* Sudoku::Box::findColNull(size_t pos) {// Find first empty position in a Box's column
	for (size_t r = 0; r < 3; r++) {
		uint8_t& _cell = cell(r, pos);
		if (!_cell) {
			return &_cell;
		}
	}

	return nullptr;
}

bool Sudoku::Box::find(uint8_t val) {// Check existence of val in a relevant Box
	for (uint8_t cell : cells) {
		if (val == cell) return true;
	}

	return false;
}

const bool Sudoku::Box::findRowVal(size_t pos, uint8_t val) {// Check existence of val in a Box's row
	for (std::reference_wrapper<uint8_t> ref : row(pos)) {
		if (ref.get() == val) return true;
	}

	return false;
}

const bool Sudoku::Box::findColVal(size_t pos, uint8_t val) {// Check existence of val in a Box's column
	for (std::reference_wrapper<uint8_t> ref : col(pos)) {
		if (ref.get() == val) return true;
	}

	return false;
}

Sudoku::Box& Sudoku::getBox(size_t pos) {// Grab a box from the boxes array using the overload
	if (pos > 8) {
		throw std::out_of_range("");
	}

	size_t _band = pos / 3;
	size_t _stack = pos % 3;
	return getBox(_band, _stack);
}

Sudoku::Box& Sudoku::getBox(size_t _band, size_t _stack) {// Grab a box from the boxes array
	if (_band > 2 || _stack > 2) {
		throw std::out_of_range("");
	}

	size_t pos = 3 * _band + _stack;
	boxes[pos].band = _band;
	boxes[pos].stack = _stack;
	return boxes[pos];
}

std::array<Sudoku::Box*, 3> Sudoku::getBand(size_t bandIdx) {// Extract an array of boxes to represent a band
	if (bandIdx > 2) {
		throw std::out_of_range("Band index out of range");
	}

	return { &boxes.at(bandIdx * 3 + 0),
					 &boxes.at(bandIdx * 3 + 1),
					 &boxes.at(bandIdx * 3 + 2)
	};
}

std::array<Sudoku::Box*, 3> Sudoku::getStack(size_t stackIdx) {// Extract an array of boxes to represent a stack
	if (stackIdx > 2) {
		throw std::out_of_range("Stack index out of range");
	}

	return { &boxes.at(0 + stackIdx),
					 &boxes.at(3 + stackIdx),
					 &boxes.at(6 + stackIdx)
	};
}

std::array<uint8_t*, 9> Sudoku::getRow(size_t pos) {// Extract a row from the grid by position
	if (pos > 8) {
		throw std::out_of_range("Row index out of range");
	}

	size_t bandIndex = pos / 3; // which horizontal band (0-2)
	size_t box_row = (pos % 3) * 3; // which row inside each box (0-2)

	std::array<uint8_t*, 9> row{ nullptr };

	std::array<Box*, 3> band = getBand(bandIndex);

	size_t i = 0;

	for (Box* box : band) {
		size_t col_pos1 = box_row, col_pos2 = box_row + 1, col_pos3 = box_row + 2;
		row[i] = &box->cells.at(col_pos1);
		row[i + 1] = &box->cells.at(col_pos2);
		row[i + 2] = &box->cells.at(col_pos3);
		i += 3;
	}

	return row;
}

std::array<uint8_t*, 9> Sudoku::getRow(size_t bandIdx, size_t box_row) {// Band and row version of getRow
	if (bandIdx > 2 || box_row > 2) {
		throw std::out_of_range("Band or row index out of range");
	}

	std::array<uint8_t*, 9> row{ nullptr };

	// Get the 3 boxes in this horizontal band
	std::array<Box*, 3> band = getBand(bandIdx);

	size_t i = 0;

	for (Box* box : band) {
		row[i] = &box->cells.at((box_row * 3) + 0);
		row[i + 1] = &box->cells.at((box_row * 3) + 1);
		row[i + 2] = &box->cells.at((box_row * 3) + 2);
		i += 3;
	}

	return row;
}

std::array<uint8_t*, 9> Sudoku::getCol(size_t pos) {// Extract the column from the grid by position
	if (pos > 8) {
		throw std::out_of_range("Row index out of range");
	}

	size_t stackIndex = pos / 3;   // which vertical stack (0–2)
	size_t box_col = pos % 3;   // which column inside each box (0–2)

	std::array<uint8_t*, 9> col{ nullptr };

	// Get the 3 boxes in this vertical stack
	std::array<Box*, 3> stack = getStack(stackIndex);

	size_t i = 0;
	for (Box* box : stack) {
		col[i] = &box->cells.at(0 + box_col);
		col[i + 1] = &box->cells.at(3 + box_col);
		col[i + 2] = &box->cells.at(6 + box_col);
		i += 3;
	}

	return col;
}

std::array<uint8_t*, 9> Sudoku::getCol(size_t stackIdx, size_t box_col) {// Stack and column version of getRow
	if (stackIdx > 2 || box_col > 2) {
		throw std::out_of_range("Stack or column index out of range");
	}

	std::array<uint8_t*, 9> col{ nullptr };

	// Get the 3 boxes in this vertical stack
	std::array<Box*, 3> stack = getStack(stackIdx);

	size_t i = 0;
	for (Box* box : stack) {

		// Assign cell references of box columns to full column
		col[i] = &box->cells.at(0 + box_col);
		col[i + 1] = &box->cells.at(3 + box_col);
		col[i + 2] = &box->cells.at(6 + box_col);
		i += 3;
	}

	return col;
}

const bool Sudoku::findRowVal(size_t pos, uint8_t val) {// Check existence of val in the grid's row
	if (pos > 8) {
		throw std::out_of_range("Row index out of range");
	}

	std::array<uint8_t*, 9> row = getRow(pos);

	for (uint8_t* cell : row) {
		if (*cell == val) return true;
	}

	return false;
}

const bool Sudoku::findColVal(size_t pos, uint8_t val) {// Check existence of val in the grid's column
	if (pos > 8) {
		throw std::out_of_range("Col index out of range");
	}

	std::array<uint8_t*, 9> col = getCol(pos);

	for (uint8_t* cell : col) {
		if (*cell == val) return true;
	}

	return false;
}

void Sudoku::findAdjs(Sudoku::Box& rootBox, std::vector<Box*>& bAdjs, std::vector<Box*>& sAdjs) {
	std::set<size_t> band_adj_pos(idxList.begin(), idxList.end());   // positions in stack (columns of boxes)
	std::set<size_t> stack_adj_pos(idxList.begin(), idxList.end());  // positions in band (rows of boxes)
	band_adj_pos.erase(rootBox.stack);
	stack_adj_pos.erase(rootBox.band);

	// Find boxes adjacent to the root box
	for (auto s : band_adj_pos) { bAdjs.push_back(&getBox(rootBox.band, s)); }
	for (auto b : stack_adj_pos) { sAdjs.push_back(&getBox(b, rootBox.stack)); }
}

void Sudoku::findNonAdjs(Sudoku::Box& rootBox, std::vector<Box*>& nAdjs) {
	for (size_t b : idxList) {
		for (size_t s : idxList) {
			if (b != rootBox.band && s != rootBox.stack) {
				nAdjs.push_back(&getBox(b, s));
			}
		}
	}
}

void Sudoku::NonAdjFill(std::vector<Box*> Non_Adjs, std::vector<Box*> B_Adjs, std::vector<Box*> S_Adjs, std::array<std::set<uint8_t>, 4> avails, std::array<std::vector<uint8_t>, 4> pools) {
	size_t iter = 0, rnd = 0;
	std::uniform_int_distribution<size_t> dist;

	avails.fill(digits);

	for (size_t i : idxList) {
		std::array<std::reference_wrapper<uint8_t>, 3> b0_mcol = B_Adjs[0]->col(1);
		std::array<std::reference_wrapper<uint8_t>, 3> b1_mcol = B_Adjs[1]->col(1);
		std::array<std::reference_wrapper<uint8_t>, 3> s0_mrow = S_Adjs[0]->row(1);
		std::array<std::reference_wrapper<uint8_t>, 3> s1_mrow = S_Adjs[1]->row(1);

		uint8_t val;

		val = b0_mcol.at(i).get(); avails[0].erase(val);
		val = b1_mcol.at(i).get(); avails[1].erase(val);
		val = s0_mrow.at(i).get(); avails[2].erase(val);
		val = s1_mrow.at(i).get(); avails[3].erase(val);
	}

	for (auto& pool : pools) {
		pool.clear();
	}

	for (auto val : avails[0]) {
		if (avails[2].find(val) != avails[2].end()) pools[0].push_back(val);
		if (avails[3].find(val) != avails[3].end()) pools[2].push_back(val);
	}

	for (auto val : avails[1]) {
		if (avails[2].find(val) != avails[2].end()) pools[1].push_back(val);
		if (avails[3].find(val) != avails[3].end()) pools[3].push_back(val);
	}

	for (auto& pool : pools) {
		std::shuffle(pool.begin(), pool.end(), rng);
	}

	iter = 0;

	do {

		auto& pool = pools[iter];

		dist = std::uniform_int_distribution<size_t>(0, size_t(pool.size() - 1));
		uint8_t choice;

		do { rnd = dist(rng); choice = pool[rnd]; } while (!choice);

		if (iter == 0) {
			for (auto& val : pools[1]) { if (val == choice) val = 0; }// pool nulling to prevent duplicates
			for (auto& val : pools[2]) { if (val == choice) val = 0; }// pool nulling to prevent duplicates
		}

		else if ((iter == 1 || iter == 2)) {
			for (auto& val : pools[3]) { if (val == choice) val = 0; }// pool nulling to prevent duplicates
		}

		Non_Adjs[iter++]->cell(1, 1) = choice; //central cell placements

	} while (iter <= 3);

	std::unordered_map<size_t, int> freq;

	for (uint8_t* cell : ring) {
		freq[*cell]++;  // increments count for this digit
	}

	std::vector<std::pair<size_t, int>> ordered_freq(freq.begin(), freq.end());

	std::unordered_map<size_t, int> ineligibleBoxes;
	for (auto& kv : ordered_freq) {
		uint8_t digit = static_cast<uint8_t>(kv.first);
		int _count = 0;

		for (size_t i = 0; i < Non_Adjs.size(); ++i) {
			if (Non_Adjs[i]->find(digit)) {
				++_count;
			}
		}

		ineligibleBoxes[digit] = _count;
	}

	std::sort(ordered_freq.begin(), ordered_freq.end(),
		[&](const auto& a, const auto& b) {
			size_t digitA = a.first, digitB = b.first;
			int freqA = a.second, freqB = b.second;
			int eligibleA = 4 - ineligibleBoxes[digitA];
			int eligibleB = 4 - ineligibleBoxes[digitB];

			// Forced case: required placements == eligible slots
			bool forcedA = (freqA == eligibleA);
			bool forcedB = (freqB == eligibleB);

			// Priority: forced with freq==2
			if (forcedA && freqA == 2 && !(forcedB && freqB == 2)) return true;
			if (forcedB && freqB == 2 && !(forcedA && freqA == 2)) return false;

			// Forced digits first
			if (forcedA != forcedB) return forcedA;

			// Compare by frequency
			if (freqA != freqB) return freqA > freqB;

			// Tie-break: ineligible boxes
			if (ineligibleBoxes[digitA] != ineligibleBoxes[digitB])
				return ineligibleBoxes[digitA] > ineligibleBoxes[digitB];

			// Final tie-break: digit ID
			return digitA < digitB;
		}
	);

	std::array<std::array<uint8_t*, 4>, 4> quads{ nullptr };
	for (auto& quad : quads) quad.fill(nullptr);

	for (size_t i = 0; i < assoc_map.size(); i++) {
		iter = i / 4;
		size_t index = i % 4;

		quads.at(iter).at(index) = assoc_map.at(i);
	}

	bool check = false;
	int tries = 0;
	int max = 100;
	int corner_max = 200;

	do {
		if (tries > max) break;
		int corner_tries = 0;

		do {
			if (corner_tries >= corner_max) break;

			std::vector<size_t> boxCandidates;
			std::vector<size_t> cellCandidates;
			freq.clear();

			for (auto& kv : ordered_freq) {
				int requiredPlacements = kv.second;
				int placed = 0;

				uint8_t digit = static_cast<uint8_t>(kv.first);
				for (size_t b = 0; b < Non_Adjs.size(); b++) {
					check = !Non_Adjs[b]->find(digit);
					if (check) boxCandidates.push_back(b);
				}

				std::shuffle(boxCandidates.begin(), boxCandidates.end(), rng);

				auto computeCheck = [&](size_t idx, Box* N_B_Adj, Box* N_S_Adj) -> bool {
					bool result;
					size_t oppPos;

					if (idx % 2 == 0) {
						oppPos = static_cast<size_t>((!idx) ? idx + 2 : idx - 2);
						result = (N_S_Adj->findColVal(1, digit) || Non_Adjs[oppPos]->findColVal(1, digit));
						oppPos = static_cast<size_t>(idx + 1);
						result = result && (N_B_Adj->findRowVal(1, digit) || Non_Adjs[oppPos]->findRowVal(1, digit));
					}
					else {
						oppPos = static_cast<size_t>((idx < 2) ? idx + 2 : idx - 2);
						result = (N_S_Adj->findColVal(1, digit) || Non_Adjs[oppPos]->findColVal(1, digit));
						oppPos = static_cast<size_t>(idx - 1);
						result = result && (N_B_Adj->findRowVal(1, digit) || Non_Adjs[oppPos]->findRowVal(1, digit));
					}

					return result;
				};

				std::stable_sort(boxCandidates.begin(), boxCandidates.end(),
					[&](const auto& a, const auto& b) {
						// find adjacents for a and b
						Box* N_B_Adj1 = nullptr, * N_S_Adj1 = nullptr;
						Box* N_B_Adj2 = nullptr, * N_S_Adj2 = nullptr;

						for (Box* S_Adj : S_Adjs) {
							if (S_Adj->band == Non_Adjs[a]->band) N_B_Adj1 = S_Adj;
							if (S_Adj->band == Non_Adjs[b]->band) N_B_Adj2 = S_Adj;
						}
						for (Box* B_Adj : B_Adjs) {
							if (B_Adj->stack == Non_Adjs[a]->stack) N_S_Adj1 = B_Adj;
							if (B_Adj->stack == Non_Adjs[b]->stack) N_S_Adj2 = B_Adj;
						}

						bool checkA = computeCheck(a, N_B_Adj1, N_S_Adj1);
						bool checkB = computeCheck(b, N_B_Adj2, N_S_Adj2);

						if (checkA != checkB) return checkA; // true if A should precede B
						return false; // treat as equivalent
					}
				);

				auto cellAllowsDigit = [&](size_t boxPos, size_t cellPos, uint8_t digit) {
					if (*quads[boxPos][cellPos] != 0) return false;

					bool check1, check2;
					Box* B_Adj, * S_Adj, * Non_Adj1, * Non_Adj2;

					if (boxPos % 2 == 0) { B_Adj = B_Adjs[0]; Non_Adj1 = (!boxPos) ? Non_Adjs[1] : Non_Adjs[3]; }
					else { B_Adj = B_Adjs[1]; Non_Adj1 = (boxPos < 2) ? Non_Adjs[0] : Non_Adjs[2]; }

					if (boxPos >= 2) { S_Adj = S_Adjs[1]; Non_Adj2 = (boxPos == 2) ? Non_Adjs[0] : Non_Adjs[1]; }
					else { S_Adj = S_Adjs[0]; Non_Adj2 = (!boxPos) ? Non_Adjs[2] : Non_Adjs[3]; }

					check1 = (cellPos < 2);
					check2 = (cellPos % 2 == 0);


					// --- Column elims --- 
					if (S_Adj->findRowVal(check1 ? 0 : 2, digit) ||
						Non_Adj1->findRowVal(check1 ? 0 : 2, digit)) {
						return false;
					}

					// --- Row elims --- 
					if (B_Adj->findColVal(check2 ? 0 : 2, digit) ||
						Non_Adj2->findColVal(check2 ? 0 : 2, digit)) {
						return false;
					}

					return true;
				};

				for (size_t i = 0; i < boxCandidates.size(); i++) {
					size_t chosenBox = boxCandidates[i];

					for (size_t c = 0; c < 4; c++) {
						if (cellAllowsDigit(chosenBox, c, digit)) cellCandidates.push_back(c);
					}

					if (cellCandidates.empty()) continue;

					if (cellCandidates.size() > 1) std::shuffle(cellCandidates.begin(), cellCandidates.end(), rng);
					size_t chosenCell = cellCandidates.front();
					cellCandidates.clear();

					*quads[chosenBox][chosenCell] = digit;
					placed++;
				}

				boxCandidates.clear();
				if (placed != requiredPlacements) break;
			}

			check = true;

			auto reset = [&]() {
				for (auto& cell : assoc_map) {
					*cell = 0;
				}
				};

			for (auto& cell : assoc_map) {
				if (!*cell) { check = false; corner_tries++; reset(); break; }
			}

			if (!check) continue;

			for (uint8_t* cell : assoc_map) {
				freq[*cell]++;  // increments count for this digit
			}

			for (auto& kv : ordered_freq) {
				size_t digit = kv.first;
				int expected = kv.second;
				int actual = freq[digit];  // defaults to 0 if missing

				if (actual != expected) {
					check = false;
					corner_tries++;
					reset();

					break;
				}
			}

		} while (!check);

		check = true;

		/* --- Final placements for sudoku grid --- */

		std::vector<std::array<uint8_t*, 9>> rows{};
		std::vector<std::array<uint8_t*, 9>> cols{};
		std::vector<uint8_t*> placed{};
		avails.fill(digits);

		for (size_t i = 0; i < 3; i++) {
			if (!i) {
				rows.push_back(getRow(Non_Adjs[i]->band, 0));
				rows.push_back(getRow(Non_Adjs[i]->band, 2));

				cols.push_back(getCol(Non_Adjs[i]->stack, 0));
				cols.push_back(getCol(Non_Adjs[i]->stack, 2));
			}

			if (i == 1) {
				cols.push_back(getCol(Non_Adjs[i]->stack, 0));
				cols.push_back(getCol(Non_Adjs[i]->stack, 2));
			}

			if (i == 2) {
				rows.push_back(getRow(Non_Adjs[i]->band, 0));
				rows.push_back(getRow(Non_Adjs[i]->band, 2));
			}
		}

		auto propagate = [&](auto& line, auto& pool, bool type, auto pickAdjacents) -> bool {
			// Find first two empty positions
			size_t emptyIdx[2] = { 9, 9 };
			for (size_t j = 0, k = 0; j < line.size() && k < 2; ++j) {
				if (!*line[j]) emptyIdx[k++] = j;
			}
			if (emptyIdx[0] == 9 || emptyIdx[1] == 9) return false;

			uint8_t cand1 = pool[0];
			uint8_t cand2 = pool[1];

			auto [NA1, NA2] = pickAdjacents();

			auto place = [&]() -> bool {

				bool NAC[2][2] = {{0,0},{0,0}};

				NAC[0][0] = NA1->find(cand1);
				NAC[0][1] = NA2->find(cand1);
				NAC[1][0] = NA1->find(cand2);
				NAC[1][1] = NA2->find(cand2);

				if ( (NAC[0][0] && NAC[0][1]) || (NAC[1][0] && NAC[1][1]) ) return false;

				auto crossCheck = [&](size_t idx, uint8_t val) -> bool {
					return type ? findColVal(idx, val) : findRowVal(idx, val);
					};

				bool EC[2][2] = {{0,0},{0,0}};

				EC[0][0] = crossCheck(emptyIdx[0], cand1);
				EC[0][1] = crossCheck(emptyIdx[1], cand1);
				EC[1][0] = crossCheck(emptyIdx[0], cand2);
				EC[1][1] = crossCheck(emptyIdx[1], cand2);

				auto pushPlaced = [&](size_t idx1, size_t idx2) {
					placed.push_back(line[emptyIdx[idx1]]);
					placed.push_back(line[emptyIdx[idx2]]);
					};

				if ( (EC[0][0] && EC[0][1]) || (EC[1][0] && EC[1][1]) ) return false;

				if (!NAC[0][0] && !NAC[0][1]) {
					if (!EC[0][0] && !EC[0][1]) {
						if (!NAC[1][0] && !NAC[1][1]) {
							if (!EC[1][0] && !EC[1][1]) {
								dist = std::uniform_int_distribution<size_t>(0, 1);

								rnd = dist(rng);
								const size_t oppPos = static_cast<size_t>(!rnd ? (rnd + 1) : (rnd - 1));

								*line[emptyIdx[rnd]] = cand1;
								*line[emptyIdx[oppPos]] = cand2;

								pushPlaced(rnd, oppPos);
							} else {
								if (!EC[1][0]) {*line[emptyIdx[0]] = cand2; *line[emptyIdx[1]] = cand1; pushPlaced(0,1);} 
								else {*line[emptyIdx[1]] = cand2; *line[emptyIdx[0]] = cand1; pushPlaced(0,1);}
							}
						} else {
							if (!NAC[1][0] && EC[1][0]) return false;
							if (!NAC[1][1] && EC[1][1]) return false;

							if (!NAC[1][0]) {*line[emptyIdx[0]] = cand2; *line[emptyIdx[1]] = cand1; pushPlaced(0,1);}
							if (!NAC[1][1]) {*line[emptyIdx[1]] = cand2; *line[emptyIdx[0]] = cand1; pushPlaced(0,1);}
						}
					} else {
						if (!EC[0][0]) {

							if (EC[1][1] || NAC[1][1]) { return false; } 
							else { *line[emptyIdx[0]] = cand1; *line[emptyIdx[1]] = cand2; pushPlaced(0, 1); }

						} else if (!EC[0][1]) {

							if (EC[1][0] || NAC[1][0]) { return false; } 
							else { *line[emptyIdx[1]] = cand1; *line[emptyIdx[0]] = cand2; pushPlaced(0, 1); }

						}
					}
				} else {
					if (!NAC[0][0]) {
						if (EC[0][0] || EC[1][1] || NAC[1][1]) { return false; } else { *line[emptyIdx[0]] = cand1; *line[emptyIdx[1]] = cand2; pushPlaced(0, 1); }
					} else if (!NAC[0][1]) {
						if (EC[0][1] || EC[1][0] || NAC[1][0]) { return false; } else { *line[emptyIdx[1]] = cand1; *line[emptyIdx[0]] = cand2; pushPlaced(0, 1); }
					}
				}
				return true;
				};

			return place(); // success
			};

		auto reset = [&]() {
			for (auto& cell : assoc_map) {
				*cell = 0;
			}

			for (auto& cell : placed) {
				*cell = 0;
			}
		};

		// --- Propagation for rows ---
		for (size_t i = 0; i < rows.size(); i++) {
			for (uint8_t* cell : rows[i]) {
				if (*cell) avails[i].erase(*cell);
			}

			pools[i].assign(avails[i].begin(), avails[i].end());
			check = propagate(rows[i], pools[i], 1, [&]() {
				return (i < 2)
					? std::pair<Box*, Box*>{ Non_Adjs[0], Non_Adjs[1] }
				: std::pair<Box*, Box*>{ Non_Adjs[2], Non_Adjs[3] };
				});

			if (!check) { reset(); tries++; break; }

			avails[i] = digits;

			for (uint8_t* cell : cols[i]) {
				if (*cell) avails[i].erase(*cell);
			}

			pools[i].assign(avails[i].begin(), avails[i].end());
			check = propagate(cols[i], pools[i], 0, [&]() {
				return (i < 2)
					? std::pair<Box*, Box*>{ Non_Adjs[0], Non_Adjs[2] }
				: std::pair<Box*, Box*>{ Non_Adjs[1], Non_Adjs[3] };
				});

			if (!check) { reset(); tries++; break; }
		}

	} while (!check);
}

void Sudoku::Phistemofel(Box& rootBox) {// Assigns references of cell positions indicative of the Phistemofel ring
	size_t inc1 = 0, inc2 = 0;

	for (Box& box : boxes) {
		if (box.band != rootBox.band && box.stack != rootBox.stack) {
			ring.at(inc1++) = &box.cell(1, 1);

			assoc_map.at(inc2++) = &box.cell(0, 0);
			assoc_map.at(inc2++) = &box.cell(0, 2);
			assoc_map.at(inc2++) = &box.cell(2, 0);
			assoc_map.at(inc2++) = &box.cell(2, 2);

		} else {
			if ((box.band == rootBox.band) != (box.stack == rootBox.stack)) {
				if (box.band == rootBox.band) {
					for (std::reference_wrapper<uint8_t> ref : box.col(1)) {
						ring.at(inc1++) = &ref.get();
					}
				} else {
					for (std::reference_wrapper<uint8_t> ref : box.row(1)) {
						ring.at(inc1++) = &ref.get();
					}
				}
			}
		}
	}
}

void Sudoku::Root_Propag(Box& rootBox, bool empty) {// Propagation algorithm for grid construction at any position
	std::array<size_t, 3> locIdxList = idxList; // Working copy of idxList to keep idxList static and shuffle the copy
	size_t rnd;

	std::array<std::set<uint8_t>, 4> avails; avails.fill(digits);
	std::array<std::vector<uint8_t>, 4> pools;

	std::vector<Box*> B_Adjs, S_Adjs, N_Adjs;
	findAdjs(rootBox, B_Adjs, S_Adjs);


	/* --- Stage 1: Fill root box --- */
	pools[0].assign(avails[0].begin(), avails[0].end());

	std::shuffle(pools[0].begin(), pools[0].end(), rng);

	size_t iter = 0;

	if (empty) {
		for (size_t r : idxList) {
			std::shuffle(locIdxList.begin(), locIdxList.end(), rng);

			for (size_t c : locIdxList) {
				rootBox.cell(r, c) = pools[0].at(iter++);
			}
		}
	}
	/*--------------------------------------------------*/

	/* --- Stage 2: Adjacent box fill --- */
	for (size_t p : idxList) {
		avails[0].erase(rootBox.cell(1, p));
		avails[1].erase(rootBox.cell(p, 1));
	}

	pools[0].assign(avails[0].begin(), avails[0].end());
	pools[1].assign(avails[1].begin(), avails[1].end());

	std::shuffle(pools[0].begin(), pools[0].end(), rng);
	std::shuffle(pools[1].begin(), pools[1].end(), rng);

	iter = 0;


	for (size_t i = 0; i < B_Adjs.size(); i++) {

		std::shuffle(locIdxList.begin(), locIdxList.end(), rng);

		for (size_t p : locIdxList) {
			B_Adjs[i]->cell(1, p) = pools[0].at(iter);
			S_Adjs[i]->cell(p, 1) = pools[1].at(iter++);
		}
	}

	avails.fill(digits);

	for (size_t p : idxList) {
		for (size_t i = 0; i < 4; i++) {
			if (i < 2) {
				avails[i].erase(rootBox.row(1).at(p).get());
				avails[i].erase(rootBox.row((i % 2 == 0) ? 0 : 2).at(p).get());
			} else {
				avails[i].erase(rootBox.col(1).at(p).get());
				avails[i].erase(rootBox.col((i % 2 == 0) ? 0 : 2).at(p).get());
			}
		}
	}

	for (size_t i = 0; i < pools.size(); i++) {
		pools[i].assign(avails[i].begin(), avails[i].end());
	}

	for (auto& pool : pools) {
		std::shuffle(pool.begin(), pool.end(), rng);
	}

	std::array<std::reference_wrapper<uint8_t>, 3> rootMRow = rootBox.row(1);
	std::array<std::reference_wrapper<uint8_t>, 3> rootMCol = rootBox.col(1);

	std::shuffle(rootMRow.begin(), rootMRow.end(), rng);
	std::shuffle(rootMCol.begin(), rootMCol.end(), rng);

	std::array<size_t, 2> offMidPos{ 0, 2 };
	std::array<size_t, 3> _locIdxList = idxList;

	std::shuffle(locIdxList.begin(), locIdxList.end(), rng);
	std::shuffle(_locIdxList.begin(), _locIdxList.end(), rng);

	std::uniform_int_distribution<size_t> dist(0, 1);

	// Lambda function for handling adjacent box completions
	auto adjFill = [&](std::vector<Box*> Adjs, size_t i, bool side) {
		rnd = dist(rng);

		for (size_t i : idxList) {
			size_t other = !rnd ? (rnd + 1) : (rnd - 1);
			uint8_t& cellRef1 = Adjs[rnd]->cell(!(side) ? offMidPos[0] : locIdxList[i], !(side) ? locIdxList[i] : offMidPos[0]);
			uint8_t& cellRef2 = Adjs[rnd]->cell(!(side) ? offMidPos[1] : _locIdxList[i], !(side) ? _locIdxList[i] : offMidPos[1]);
			uint8_t& cellRef3 = Adjs[other]->cell(!(side) ? offMidPos[0] : locIdxList[i], !(side) ? locIdxList[i] : offMidPos[0]);
			uint8_t& cellRef4 = Adjs[other]->cell(!(side) ? offMidPos[1] : _locIdxList[i], !(side) ? _locIdxList[i] : offMidPos[1]);

			for (std::reference_wrapper<uint8_t> ref : !(side) ? Adjs[rnd]->row(1) : Adjs[rnd]->col(1)) {
				if (pools[!(side) ? 0 : 2][i] == ref.get()) { cellRef3 = pools[!(side) ? 0 : 2][i]; }
				if (pools[!(side) ? 1 : 3][i] == ref.get()) { cellRef4 = pools[!(side) ? 1 : 3][i]; }
			}

			for (std::reference_wrapper<uint8_t> ref : !(side) ? Adjs[other]->row(1) : Adjs[other]->col(1)) {
				if (pools[!(side) ? 0 : 2][i] == ref.get()) { cellRef1 = pools[!(side) ? 0 : 2][i]; }
				if (pools[!(side) ? 1 : 3][i] == ref.get()) { cellRef2 = pools[!(side) ? 1 : 3][i]; }
			}
		}

		size_t _iter = 0;

		do {
			for (std::reference_wrapper<uint8_t> ref : !(side) ? Adjs[0]->row(0) : Adjs[0]->col(0)) {
				uint8_t& cel = ref.get();

				if (!cel) {
					cel = !(side) ? rootMRow.at(_iter).get() : rootMCol.at(_iter).get();
					uint8_t* cel2 = !(side) ? Adjs[1]->findRowNull(2) : Adjs[1]->findColNull(2);
					if (cel2 != nullptr) {
						*cel2 = !(side) ? rootMRow.at(_iter++).get() : rootMCol.at(_iter++).get();
					}
				}
			}

			if (iter < 3) {
				for (std::reference_wrapper<uint8_t> ref : !(side) ? Adjs[1]->row(0) : Adjs[1]->col(0)) {
					uint8_t& cel = ref.get();
					if (!cel) {
						cel = !(side) ? rootMRow.at(_iter).get() : rootMCol.at(_iter).get();
						uint8_t* cel2 = !(side) ? Adjs[0]->findRowNull(2) : Adjs[0]->findColNull(2);
						if (cel2 != nullptr) {
							*cel2 = !(side) ? rootMRow.at(_iter++).get() : rootMCol.at(_iter++).get();
						}
					}
				}
			}

		} while (_iter < 3);
	};

	iter = 0;

	adjFill(B_Adjs, iter, 0);
	adjFill(S_Adjs, iter, 1);

	/*--------------------------------------------------*/

	/* --- Stage 3: Non-Adjacent box fills --- */
	
	findNonAdjs(rootBox, N_Adjs);

	Phistemofel(rootBox);
	NonAdjFill(N_Adjs, B_Adjs, S_Adjs, avails, pools);

	/*--------------------------------------------------*/
}

void Sudoku::Line_Propag(std::array<uint8_t, 9> row, std::array<uint8_t, 9> col) { /* --- Alternative propagation algorithm --- */
	std::array<std::set<uint8_t>, 4> avails;
	std::array<std::vector<uint8_t>, 4> pools;

	std::vector<Box*> B_Adjs, S_Adjs, N_Adjs;

	std::uniform_int_distribution<size_t> dist;
	size_t rnd;

	avails.fill(digits);

	for (size_t i = 0; i < row.size(); i++) {
		if (!row[i] || !col[i]) return;
		avails[0].erase(row[i]);
		avails[1].erase(col[i]);
	}
	
	if (!(avails[0].empty() && avails[1].empty())) return;

	if (row == col) {
		dist = std::uniform_int_distribution<size_t>(0, 1);
		rnd = dist(rng);
		
		(rnd) ? std::shuffle(row.begin(), row.end(), rng) : std::shuffle(col.begin(), col.end(), rng); 
	}

	dist = std::uniform_int_distribution<size_t>(0, 8);
	bool validIntersection = false;
	size_t attempts = 0, MAXATTEMPTS = 20;

	do {
		if (attempts >= MAXATTEMPTS) return;

		/* --- Initial placements by intersection logic --- */
		rnd = dist(rng); // pick random index into row

		uint8_t candidate = row[rnd]; // digit from row

		for (size_t i = 0; i < col.size(); i++) {

			if (col[i] == candidate) {
				// Found intersection at (row[rnd], col[j])

				// Pull the 3-cell slices from the *parameters*
				size_t rowSliceStart = (rnd / 3) * 3;
				size_t colSliceStart = (i   / 3) * 3;

				std::array<uint8_t, 3> rowSlice = {
					row[rowSliceStart], row[rowSliceStart+1], row[rowSliceStart+2]
				};
				std::array<uint8_t, 3> colSlice = {
					col[colSliceStart], col[colSliceStart+1], col[colSliceStart+2]
				};

				// Check valid intersection
				bool duplicateFound = false;
				for (size_t r = 0; r < 3; r++) {
					for (size_t c = 0; c < 3; c++) {
						if (rowSlice[r] && colSlice[c] && rowSlice[r] == colSlice[c]) {
							// Skip the actual intersection cell
							size_t rowIdx = rowSliceStart + r;
							size_t colIdx = colSliceStart + c;
							if (!(rowIdx == rnd && colIdx == i)) {
								duplicateFound = true;
								break;
							}
						}
					}
					if (duplicateFound) { attempts++; break; }
				}

				if (!duplicateFound) {
					validIntersection = true;
				} else { break; }

				if (validIntersection) {
					size_t bandIdx = i / 3; // band position from row index
					size_t stackIdx = rnd / 3; // stack position from col index

					// Acquire actual row and column from current grid
					std::array<uint8_t*, 9> gridRow = getRow(i);
					std::array<uint8_t*, 9> gridCol = getCol(rnd);

					for (size_t j = 0; j < 9; j++) {
						*gridRow[j] = row[j];
						*gridCol[j] = col[j];
					}

					// Initialize and assign rootBox in local scope
					Box& rootBox = getBox(bandIdx, stackIdx);

					findAdjs(rootBox, B_Adjs, S_Adjs);
					findNonAdjs(rootBox, N_Adjs);
					Phistemofel(rootBox);

					size_t duplicateCount = 0;

					// Step 1: prune avails[2] by digits already in rootBox
					for (auto d : rootBox.cells) {
						if (d) avails[2].erase(d);
					}

					// Step 2: build pool[2] from remaining avails[2] and shuffle
					pools[2].assign(avails[2].begin(), avails[2].end());
					std::shuffle(pools[2].begin(), pools[2].end(), rng);

					// Step 3: fill empty cells in rootBox with remaining digits
					size_t idx = 0;
					for (auto& d : rootBox.cells) {
						if (!d && idx < pools[2].size()) {
							d = pools[2][idx++];
						}
					}

					avails.fill(digits);
					size_t placed = 0, _placed = 0;
					std::vector<uint8_t*> slice1, slice2, slice3, slice4;
					std::array<uint8_t*, 9> _gridRow{nullptr}, _gridCol{nullptr};

					for (auto idx : idxList) {
						if (B_Adjs[0]->findRowNull(idx)) { 
							if (slice1.empty()){
								gridRow = getRow(B_Adjs[0]->band, idx); 
								for (auto& cell : gridRow) {
									if (!*cell) { slice1.push_back(cell); }
								}
							} else {	
								_gridRow = getRow(B_Adjs[1]->band, idx);
								for (auto& cell : _gridRow) {
									if (!*cell) { slice3.push_back(cell); }
								}
							}
						}

						if (S_Adjs[0]->findColNull(idx)) { 
							if (slice2.empty()){
								gridCol = getCol(S_Adjs[0]->stack, idx);
								for (auto& cell : gridCol) {
									if (!*cell) { slice2.push_back(cell); }
								}
							} else {	
								_gridCol = getCol(S_Adjs[1]->stack, idx);
								for (auto& cell : _gridCol) {
									if (!*cell) { slice4.push_back(cell); }
								}
							}
						}
					}

					// Step 2: prune
					for (size_t c = 0; c < row.size(); c++) {
						avails[0].erase(*gridRow[c]);
						avails[1].erase(*gridCol[c]);
						avails[0].erase(B_Adjs[0]->cells[c]);
						avails[1].erase(S_Adjs[0]->cells[c]);
					}

					pools[0].assign(avails[0].begin(), avails[0].end());
					pools[1].assign(avails[1].begin(), avails[1].end());

					// Partition pools[0] so digits found in B_Adjs[1] come first
					std::stable_partition(
						pools[0].begin(), pools[0].end(),
						[&](uint8_t d) {
							return B_Adjs[1]->find(d);
						}
					);

					// Partition pools[1] so digits found in S_Adjs[1] come first
					std::stable_partition(
						pools[1].begin(), pools[1].end(),
						[&](uint8_t d) {
							return S_Adjs[1]->find(d);
						}
					);

					// Step 3: force placement
					for (auto d : pools[0]) {
						if (placed < 3) *slice1.at(placed++) = d;
					}

					for (auto d : pools[1]) {
							if (_placed < 3) *slice2.at(_placed++) = d;
					}

					for (auto& cell : gridRow) {
						avails[2].erase(*cell);
					}

					for (auto& cell : gridCol) {
						avails[3].erase(*cell);
					}

					pools[2].assign(avails[2].begin(), avails[2].end());
					pools[3].assign(avails[3].begin(), avails[3].end());

					for (auto d : pools[2]) {
						*slice1.at(placed++) = d;
					}

					for (auto d : pools[3]) {
						*slice2.at(_placed++) = d;
					}

					placed = 0; _placed = 0;
					avails.fill(digits);

					for (size_t it = 0; it < avails.size(); it++) {
						idx = (it % 2);
						if (it < 2) {
							for (auto digit : digits) {
								if (B_Adjs[idx]->find(digit)) avails[it].erase(digit);
							}
						} else { 
							for (auto digit : digits){
								if (S_Adjs[idx]->find(digit)) avails[it].erase(digit);
							}
						}
					}

					for (size_t c = 0; c < pools.size(); c++) {
						pools[c].assign(avails[c].begin(), avails[c].end());
					}

					for (size_t c = 0; c < pools.size(); c++) {
						for (auto d : pools[c]) {
							(c < 2) ? *slice3.at(placed++) = d : *slice4.at(_placed++) = d;
						}
					}
				}
				break;
			}
		}
	} while (!validIntersection);


	if (validIntersection) NonAdjFill(N_Adjs, B_Adjs, S_Adjs, avails, pools);
}

void Sudoku::Corner_Propag(Box& rootBox, bool empty) { /* --- Alternative propagation algorithm involving abstracted corner boxes --- */ 
	/* This algorithm is intended to fill non-adjacent boxes first and then fill the boxes adjacent to some root box. 
	 * The goal is significant speed, where transforms are applied after completion for variation.
	 * This one has the least variations, so transform count and randomization need to be biased toward substantial mutations and counts.
	 * The propagation routine is simple, so I'm leaving this for individuals who want to try their hand at building something similar.
	 * Whoever does this can also confirm that this is faster than the other two propagation algorithms.
	 * I have laid the foundation for how it should be constructed.
	 * pools[0] is holding the digits that should be placed in the corner cells and central cell of the non-adjacent boxes.
	 * pools[1] is holding the digits that should be placed in the remaining cells for the non-adjacent boxes.
	 * The goal is for the central box and non-adjacents to be completed first, because the adjacent boxes would need 3 digits placed per column and row.
	 * Use the other propagation algorithms as a benchmark comparison and for insight as to how to complete this one.
	 * It's not necessarily needed, so feel free to work on whatever.
	 */
	std::vector<Box*> B_Adjs, S_Adjs, N_Adjs;

	findNonAdjs(rootBox, N_Adjs);
	findAdjs(rootBox, B_Adjs, S_Adjs);
	std::array<std::vector<uint8_t>, 3> pools;

	pools[0].assign(digits.begin(), digits.end());

	std::shuffle(pools[0].begin(), pools[0].end(), rng);

	pools[1].assign(pools[0].begin(), (pools[0].begin() + 4));
	pools[0].erase(pools[1].begin(), pools[1].end());
}

void Sudoku::root_generate(bool type) {

	std::uniform_int_distribution<size_t> dist(0, 8);

	size_t rnd = dist(rng);

	clearGrid();

	(type) ? Root_Propag(getBox(rnd), true) : Corner_Propag(getBox(rnd), true);

	/* --- Puzzle building section --- */
}

void Sudoku::root_generate(bool type, size_t pos){
	
	clearGrid();

	(type) ? Root_Propag(getBox(pos), true) : Corner_Propag(getBox(pos), true);

	/* --- Puzzle building section --- */
}

void Sudoku::root_generate(bool type, Box box, size_t pos) {
	bool empty = false;

	for (auto& cell : box.cells){
		if (!cell) { empty = true; break; }
	}

	if (empty){
		for (auto& cell : box.cells){
			if (cell) cell = 0;
		}
	}

	clearGrid();

	getBox(pos) = box;

	(type) ? Root_Propag(getBox(pos), true) : Corner_Propag(getBox(pos), true);

	/* --- Puzzle building section --- */
}

void Sudoku::line_generate(){
	std::array<uint8_t, 9> row{1, 2, 3, 4, 5, 6, 7, 8, 9}, col{1, 2, 3, 4, 5, 6, 7, 8, 9};

	std::shuffle(row.begin(), row.end(), rng);
	std::shuffle(col.begin(), col.end(), rng);

	clearGrid();
	Line_Propag(row, col);
}

void Sudoku::line_generate(std::array<uint8_t*, 9> row, std::array<uint8_t*, 9> col) {
	std::array<uint8_t, 9> _row, _col;
	for (size_t i = 0; i < row.size(); i++) {
		_row[i] = *row[i];
		_col[i] = *col[i];
	}

	clearGrid();
	Line_Propag(_row, _col);

	/* --- Puzzle building section --- */
}

void Sudoku::flattenGrid() {
	size_t index = 0;
	for (auto idx : idxList){
		auto band = getBand(idx);

		for (size_t i = 0; i < idxList.size(); i++) {
			for (auto& box : band) {
				for (size_t j = 0; j < idxList.size(); j++) {
					grid[index++] = &box->cells[(i * 3) + j];
				}
			}
		}
	}
}

void Sudoku::printGrid() const {
	std::string line;
	line.reserve(64); // enough for one row

	for (size_t band = 0; band < 3; band++) {
		for (size_t row = 0; row < 3; row++) {
			line.clear();
			for (size_t box_pos = 0; box_pos < 3; box_pos++) {
				size_t pos = 3 * band + box_pos;
				size_t rel_row = 3 * row;
				const Box& box = boxes[pos];

				for (size_t rel_col = 0; rel_col < 3; rel_col++) {
					pos = rel_row + rel_col;
					int val = int(box.cells[pos]);
					line.push_back(val == 0 ? '#' : '0' + val);
					line.push_back(' ');
				}
				if (box_pos < 2) line += "| ";
			}
			std::cout << line << std::endl;
		}
		if (band < 2) std::cout << "------+-------+------\n";
	}

	std::cout << '\n' << std::endl;
}

bool Sudoku::validateGrid() {
	for (auto each : grid) {
		if (!each || !*each) return false;
	}

	for (size_t i = 0; i < 9; i++) {
		std::array<uint8_t*, 9> row = getRow(i), col = getCol(i);
		for (size_t j = 0; j < 9; j++) {
			for (size_t k = (j + 1); k < 9; k++) {
				if (*row[j] == *row[k]) return false;
				if (*col[j] == *col[k]) return false;
				if (boxes[i].cells[j] == boxes[i].cells[k]) return false;
			}
		}
	}

	return true;
}

void Sudoku::clearGrid(){
	for (auto& each : boxes){
		for (auto& cell : each.cells){
			cell = 0;
		}
	}

	ring.fill(nullptr);
	assoc_map.fill(nullptr);
}

void Sudoku::populate(std::string g) {
	for (size_t i = 0; i < grid.size(); ++i) {
		char c = g[i];
		if (c >= '1' && c <= '9') {
			*grid[i] = static_cast<uint8_t>(c - '0'); // convert char digit to number
		} else {
			*grid[i] = 0; // treat '.' or '0' or any non-digit as blank
		}
	}
}

std::string Sudoku::toString() {
	std::string s;
	s.reserve(81);
	for (size_t i = 0; i < grid.size(); ++i) {
		if (*grid[i] == 0) {
			s.push_back('.'); // tdoku uses '.' or '0' for blanks
		} else {
			s.push_back('0' + *grid[i]); // convert digit to char
		}
	}
	return s;
}

void Sudoku::torShift(size_t r_shift, size_t c_shift) {
	if (r_shift >= 3 || c_shift >= 3) return;

	std::array<Box, 9> temp;

	for (Box& box : boxes) {
		size_t band_shift = (box.band + r_shift) % 3;
		size_t stack_shift = (box.stack + c_shift) % 3;
		size_t new_index = 3 * band_shift + stack_shift;

		box.band = band_shift;
		box.stack = stack_shift;
		temp[new_index] = box;
	}

	boxes = temp;
};

void Sudoku::bandSwap(size_t idx1, size_t idx2) {
	if (idx1 == idx2 || idx1 > 2 || idx2 > 2) return;

	auto band1 = getBand(idx1); // returns std::array<Box*,3>
	auto band2 = getBand(idx2);

	for (size_t i = 0; i < band1.size(); i++){
		std::swap(*band1[i], *band2[i]);
	}
}

void Sudoku::bandRowSwap(size_t band, size_t idx1, size_t idx2) {
	if (band > 2 || idx1 == idx2 || idx1 > 2 || idx2 > 2) return;

	auto row1 = getRow(band, idx1);
	auto row2 = getRow(band, idx2);

	for (size_t i = 0; i < row1.size(); i++){
		std::swap(*row1[i], *row2[i]);
	}
}

void Sudoku::stackSwap(size_t idx1, size_t idx2) {
	if (idx1 == idx2 || idx1 > 2 || idx2 > 2) return;

	auto stack1 = getStack(idx1); // returns std::array<Box*,3>
	auto stack2 = getStack(idx2);

	for (size_t i = 0; i < stack1.size(); i++){
		std::swap(*stack1[i], *stack2[i]);
	}
}

void Sudoku::stackColSwap(size_t stack, size_t idx1, size_t idx2) {
	if (stack > 2 || idx1 == idx2 || idx1 > 2 || idx2 > 2) return;

	auto col1 = getCol(stack, idx1);
	auto col2 = getCol(stack, idx2);

	for (size_t i = 0; i < col1.size(); i++){
		std::swap(*col1[i], *col2[i]);
	}
}

void Sudoku::reflection(bool type) {
	auto swapLines = [&](std::array<uint8_t*, 9> line1, std::array<uint8_t*, 9> line2){
		for (size_t j = 0; j < 9; j++) {
			std::swap(*line1[j], *line2[j]);
		}
		};

	for (size_t i = 0; i < (9 / 2); i++)  {
		if (type){
			auto row1 = getRow(i);
			auto row2 = getRow((boxes.size() - (i + 1)));
			swapLines(row1, row2);
		} else {
			auto col1 = getCol(i);
			auto col2 = getCol((boxes.size() - (i + 1)));
			swapLines(col1, col2);
		}
	}
}

void Sudoku::transpose() {
	auto swapLines = [&](std::array<uint8_t*, 9> line1, std::array<uint8_t*, 9> line2, size_t idx) {
		for (size_t j = idx + 1; j < 9; j++) {
			std::swap(*line1[j], *line2[j]);
		}
	};

	for (size_t i = 0; i < 9; i++) {
		auto row = getRow(i);
		auto col = getCol(i);
		swapLines(row, col, i);
	}
}

void Sudoku::rotation() {
	transpose();
	reflection(true);
}

void Sudoku::_rotation() {
	transpose();
	reflection(false);
}

void Sudoku::_transpose(){
	_rotation();
	reflection(true);
}

void Sudoku::digPermut(uint8_t count, uint8_t initPart) {
	if (count > 18 || initPart < 2 || initPart > 9) return;

	// Start with one partition of size initPart
	std::vector<std::vector<uint8_t>> partitions(1, std::vector<uint8_t>(initPart, 0));
	std::uniform_int_distribution<size_t> coin_dist(0, 1);

	auto partition = [&](std::vector<std::vector<uint8_t>>& parts) -> bool {
		std::uniform_int_distribution<size_t> idx_dist(0, (parts.size() - 1));

		// Find index of largest partition
		size_t idx_from = 0, idx_to = idx_dist(rng), max_size = 0;

		for (size_t i = 0; i < parts.size(); ++i) {
			if (parts[i].size() > max_size) {
				max_size = parts[i].size();
				idx_from = i;
			}
		}

		if (initPart > 2){
			if (max_size > 1) {
				bool coin = (parts.size() > 1) ? bool(coin_dist(rng)) : true;
				if (coin) {
					// Partition: Create new fixed slot
					parts.push_back(std::vector<uint8_t>(1, 0));
					parts[idx_from].pop_back();
				} else {
					// Assumation: Inject into a different partition
					while (idx_to == idx_from) { idx_to = idx_dist(rng); }

					parts[idx_to].push_back(0);
					parts[idx_from].pop_back();
				}
			} else { parts[0].push_back(0); parts.pop_back(); } // Reached smallest partition state, so the last vector is removed and first increased
		}
		return true;
	};

	// Run partitioning up to count
	for (uint8_t i = 0; i < count; ++i) {
		if (!partition(partitions)) break;
	}

	// Assign digits randomly
	std::vector<uint8_t> digits = {1,2,3,4,5,6,7,8,9};
	std::shuffle(digits.begin(), digits.end(), rng);

	size_t idx = 0;
	for (auto& part : partitions) {
		for (auto& slot : part) {
			slot = digits[idx++];
		}
	}

	std::array<uint8_t, 10> mapping{}; // 1-based for digits 1..9

	for (auto part : partitions) {
		if (part.size() == 1) {
			continue; // Fixed point: mark with 0 so grid keeps digit unchanged
		} else {
			// Shuffle partition to randomize cycle order
			std::shuffle(part.begin(), part.end(), rng);
			for (size_t i = 0; i < part.size(); ++i) {
				mapping[part[i]] = part[(i + 1) % part.size()];
			}
		}
	}

	for (size_t i = 0; i < grid.size(); ++i) {
		uint8_t val = *grid[i];
		if (val >= 1 && val <= 9) {
			// Only remap if mapping[val] is non-zero
			if (mapping[val] != 0) {
				*grid[i] = mapping[val];
			}
			// else leave unchanged (fixed point)
		}
		// If val == 0, treat as blank and skip
	}
}

void Toroidal_Sudoku::torShift(size_t r_shift, size_t c_shift) { return; }
