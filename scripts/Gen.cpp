#include "Sudoku.h"

#ifndef Sudoku_H
  #error X0
#endif

#define exit_on_count if (total >= count) break;
#define passInc successCount++; i++; total++;
#define failInc failureCount++; i++; total++;
#define equiv(a, b) a == b

#include <thread>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <unordered_set>

static std::string as_percent(double value, int decimals = 1) {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(decimals) << (value * 100) << "%";
  return oss.str();
}

static bool isMicro(const std::string& b) {
  static const std::unordered_set<std::string> aliases = {
    "micro", "mic", "m"
  };

  return aliases.find(b) != aliases.end();
}

static bool isPipeline(const std::string& b) {
  static const std::unordered_set<std::string> aliases = {
    "pipeline", "pipe", "pl", "p"
  };

  return aliases.find(b) != aliases.end();
}

static bool isTransform(const std::string& m) {
  static const std::unordered_set<std::string> aliases = {
    "transform", "xform", "tf", "t"
  };

  return aliases.find(m) != aliases.end();
}

static std::string diagnostics(size_t sc, size_t fc, size_t t, double sr, long long td, double av, std::string m = "default", bool type = false) {
  bool transform = isTransform(m);
  double _td = double (td) / 1000;
  std::ostringstream oss;

  oss << "Mode: " << (transform ? "Transform" : "Default") << " | " << (type ? "Assured" : "Default");
  oss << '\n' << std::endl;
  oss << "Total Puzzles: " << t;
  oss << " | ";
  oss << "Valid: " << sc;
  oss << " | ";
  oss << "Invalid: " << fc;
  oss << " | ";
  oss << "Success Rate: " << as_percent(sr, 2);
  oss << " | ";
  oss << "Total Duration: " << std::fixed << std::setprecision(2) << ((td > 1000) ? _td : td) << ((td > 1000) ? " seconds" : " milliseconds");
  oss << " | ";
  oss << "Average: " << std::fixed << std::setprecision(5) << (transform ? av * 1000 : av) << (transform ? " microseconds" : " milliseconds") << std::endl;

  return oss.str();
}

int main(int argc, char* argv[]) {
  Sudoku s, _s;

  //do { s.root_generate(true); } while (!s.validateGrid());

  //s.digPermut(3, 5);
  //s.printGrid();
  //do { _s.line_generate(s.getRow(3), s.getCol(6)); } while (!_s.validateGrid());
  //_s.printGrid();
  //do { _s.line_generate(s.getRow(2), s.getRow(4)); } while (!_s.validateGrid());
  //_s.printGrid();
  //do { _s.line_generate(s.getCol(4), s.getCol(7)); } while (!_s.validateGrid());
  //_s.printGrid();
  //do { _s.line_generate(s.getRow(3), s.getRow(3)); } while (!_s.validateGrid());
  //_s.printGrid();
  //do { _s.line_generate(s.getCol(1), s.getCol(1)); } while (!_s.validateGrid());
  //_s.printGrid();
  //do { s.line_generate(); } while (!s.validateGrid());
  //s.printGrid();

  
  size_t count = 999;

  bool verbose = false, assured = false, transform = false, micro = false, pipe = false;

  std::string mode = "default";
  std::string bench = "pl";
  
  if (argc > 1) {
    for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg == "--count" && ((i + 1) < argc)) count = std::stoul(argv[++i]);
      else if (arg == "--mode" && ((i + 1) < argc)) mode = argv[++i];
      else if (arg == "--bench" && ((i + 1) < argc)) bench = argv[++i];
      else if (arg == "--verbose") verbose = true;
      else if (arg == "--assured") assured = true;
    }
  }

  transform = isTransform(mode);
  micro = isMicro(bench);
  pipe = isPipeline(bench);

  using clock = std::chrono::steady_clock;
  size_t successCount = 0;
  size_t failureCount = 0;
  size_t total = 0;

  std::chrono::steady_clock::time_point trueStart = clock::now(), end;
  for (size_t i = 0; i < count; i++) {
    long long gridDuration; bool validGrid = false;
    unsigned int tries = 0;
    size_t pos = size_t(i % 9), _pos = 8 - pos;

    auto start = clock::now();

    if (assured) {
      if (!transform || pipe || !i) {
        do { s.root_generate(true, s.getBox(pos), _pos); /*s.line_generate();*/ tries++; } while ( !s.validateGrid() );
      }
    } else { if (!transform || pipe || i == 0) s.root_generate(true, s.getBox(pos), _pos); /*s.line_generate();*/ }

    end = clock::now();

    gridDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    if (verbose) { 
      if (assured) {
        if (micro && i > 0) {} else { 
          std::cout << '\n' << "Grid completion took: " << gridDuration << " microseconds in " << tries << " tries" << std::endl;
        }
      } else {
        if (micro && i > 0) {} else { 
          std::cout << '\n' << "Grid completion took: " << gridDuration << " microseconds" << std::endl;
        }
      }
    }

    validGrid = s.validateGrid();

    auto printGrid = [&](){
      if (verbose) s.printGrid();
    };

    printGrid();

    if (validGrid) { successCount++; total = successCount + failureCount; } else { failureCount++; total = successCount + failureCount; continue; }
    
    if (!i) { _s = s; }

    if (transform) {

      for (int j = 0; j < 3; j++) {
        for (int k = 0; k < 3; k++) {
          for (int l = (k + 1); l < 3; l++) {
            if ( (!k && !l) ) {} else {
              s.torShift(k, l);
              if (equiv(_s, s)) { failInc exit_on_count } else { passInc exit_on_count }
              printGrid(); 
            }

            if (!(k == l)) {
              s.bandSwap(k, l); if (equiv(_s, s)) { failInc exit_on_count } else { passInc exit_on_count }
              printGrid();

              s.stackSwap(k, l); if (equiv(_s, s)) { failInc exit_on_count } else { passInc exit_on_count }
              printGrid();

              s.bandRowSwap(j, k, l); if (equiv(_s, s)) { failInc exit_on_count } else { passInc exit_on_count }
              printGrid();

              s.stackColSwap(j, k, l); if (equiv(_s, s)) { failInc exit_on_count } else { passInc exit_on_count }
              printGrid();
            }
          }
          exit_on_count
        }
        exit_on_count
      }
      exit_on_count

      s.transpose(); 
      if (equiv(_s, s)) { failInc exit_on_count} else { passInc exit_on_count }

      printGrid();

      s._transpose(); 
      if (equiv(_s, s)) { failInc exit_on_count } else { passInc exit_on_count }
      
      printGrid();
      
      s.reflection(true); 
      if (equiv(_s, s)) { failInc exit_on_count } else { passInc exit_on_count }
      
      printGrid();

      for (size_t j = 0; j < 3; j++) {
        s.rotation(); 
        if (equiv(_s, s)) { failInc exit_on_count } else { passInc exit_on_count }
        
        printGrid();
      }

      for (size_t j = 0; j < 3; j++) {
        s._rotation(); 
        if (equiv(_s, s)) { failInc exit_on_count } else { passInc exit_on_count }
        printGrid();
      }
      
      s.reflection(false); 
      if (equiv(_s, s)) { failInc exit_on_count } else { passInc exit_on_count }
      printGrid();

      for(uint8_t j = 0; j <= 18; j++) {
        for (uint8_t k = 2; k <= 9; k++) {
          s.digPermut(j, k); 
          if (equiv(_s, s)) { failInc exit_on_count } else { passInc exit_on_count }
          printGrid();
        }
        exit_on_count
      }
      exit_on_count
    }
  }
  end = clock::now();

  long long totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - trueStart).count();

  double average = double(totalDuration) / double(total);

  double successRate = (total > 0) ? double(successCount) / double(total) : 0.0;
  std::cout << std::endl;
  std::cerr << diagnostics(successCount, failureCount, count, successRate, totalDuration, average, mode, assured);
}
