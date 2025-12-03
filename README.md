# Sudoku-Generator
Possibly the fastest Sudoku Generator available as it hits 1000 grids generated in 2.5 seconds on outdated hardware (2017 laptop) in a Windows environment with system interrupts, task scheduling, and no dedicated threading. The benchmark was performed with O2 optimizations, where fastest time generated is 54µs.

----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Not a seasoned C++ developer, so this needs those who know how to optimize this implementation of the algorithm, and perform the appropriate benchmark tests for the generator. There are currently 2 generation methods that post similar times for the current architecture: Intel Core i5 Windows laptop with 4 cores. Inexperience led to no appropriate data type translations for optimal speed, so the speed is coming purely from the algorithm itself. 

AI assisted in the construction of the code, so inefficiencies may be present from either direct logic or deferred. It suggested conversion of data types from std::set to std::bitset, and I would prefer convergence of that insight from experienced developers, as I was able to spot problems in the tool's suggestion during the project. The generator defaults to approximately 90% accuracy without assurity loops, but those loops generally complete in 1 try with occasional 2. The options list allows you to test these settings on your own, both with standard output or just pure generation.

I have a final propagation algorithm provided with only the necessary initializations in place. I'm willing to work with others on building that one, as it could be several times faster than the first 2 algorithms, given that it uses predetermined placements. I intend to add variety by applying a random set of transforms after completion of this last algorithm: the transforms are supplied in the code with example usage in the main file.

Contributors should try to extend this in C++, but if you find out how to rebuild it in another language, feel free to publish that version independently: C# Java, Jqeury, etc.
