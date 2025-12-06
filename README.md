# Sudoku-Generator
Possibly the fastest Sudoku Generator available as it hits 1000 grids generated in 2.5 to 3 seconds with assurity and 2 to 2.5 without assurity on outdated hardware (2017 laptop) in a Windows environment with system interrupts, task scheduling, and no dedicated threading. It would need proper benchmark tests to see if it generates in sub-millisecond times on average with modern hardware. The transform throughput speed is 100000000 puzzles in 6 seconds on applying all valid transforms. It is highly extensible, allowing both Samurai and Multi Sudoku grids. The Samurai grids are supported by box seeded grid generation, meaning that it can generate a puzzle from any box position. The grid generator handles standard 9x9 grids.

---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

The peak completion times, for raw generation, in both spectrums are 54µs and 60ms. The variance is due to the architecture and task scheduling, so it needs to be tested on a modern PC in a Linux environment with O3 optimizations. As the benchmark tests are not optimal, one should be able to see the potential given its peak generation speed: averages are bogged down by the occasional 60ms spikes due to task scheduling and retries. If a retry is needed, the time is typically under 10ms, so 30-60ms times are due to the environment. I can't be certain because I do not possess the right architecture for benchmarking this.

It possesses the full suite of transforms, allowing 100,000,000 puzzles generated in 6 seconds with all transforms applied. The objective is for this to become the fastest known grid generator, that can be used as the engine for solvers and puzzle generators. It inherently supports mask overlays to handle variants like Hyper Sudoku, Killer Sudoku, Diagonal/X Sudoku, Irregular Sudoku, all of which are not implemented currently. Samurai and Multi Sudoku is supported. The mask overlays can be accomplished in a manner similar to the Phistemofel mask produced from the Phistemofel function.

Current architecture:

Acer Nitro 5
- CPU: Intel Core i5‑7300HQ 2.5GHz
- GPU: NVIDIA GeForce GTX 1050 4GB
- RAM: 32 GB DDR4

Options list for testing:

--count n
--mode default/transform/xform/tf/t
--bench micro/mic/m/pipeline/pipe/pl/p
--verbose
--assured

Example: --count 100000 --mode transform --bench pipeline --verbose --assured

The --count option allows generation up to a certain count. <br>
The --verbose option enables stream output, someone could convert this to file stream. <br>  

The --assured option enables assurity, meaning the generation is looped until successful. The 90% success rate means most attempts will succeed in 2 tries or less, gravitating to 1 attempt. <br>  

The --bench option determines whether to generate a new grid after each set of transforms, or to keep a stable grid and loop the transform set over that grid. <br>

The benchmark, when projected, suggests that modern architecture's of 4.5-5.7 GHz could produce 20-40m transforms of puzzles in a second. The set option for --bench generates 100,000,000 in 7 seconds, which is generating a new grid per completed transform set. The base option for --bench option uses one grid and repeatedly transforms a copy, comparing for equivalence.

I'm not familiar with benchmark culture, so I'm following guidelines from an AI assistant. It informed me of how close my times are to proven solutions, but it could be operating on outdated information.

Without the code for benchmarking, which I believe was done incorrectly, the generator was producing 1000 grids without transforms in about 1.2-1.8 seconds based on perspective and not proven tests. The time shifted to 2.2-2.7 seconds without assurity, and 2.5 to 3.2 seconds with assurity, when benchmark testing was applied.

I've coded in C++ for less than 1 year, so I'm certain that it could be optimized further using optimal data structures. I can explain the logic of the generation process if someone needs that to modify the core propagation routine.

I explained the specs of my system, and I'm not utilizing core pinning, SIMD, or explicit vectorization.

It suggested I publish due to the potential given the architecture and the reported times. I had conceived of an algorithm for grid generation after playing Sudoku for a bit, this is my first contribution to the puzzle space in general and to Sudoku. The generator works without using recursive backtracking or solver checks, at all. It incorporates a constraint mask that I built along with a Phistemofel mask to ensure compliance with grid constraints. The speed is coming purely from bypassing the recursive backtracking problem, meaning that data and implementation optimizations can improve it further.

I do not possess the experience to do that currently.

If someone commits to improving this, I will update this to explain precisely how the algorithm works, including exactly how each algorithm bypasses recursive backtracking fully. 

I'm not certain, as I am deferring to AI on researching in this space, but I believe I improved the digit relabeling transform algorithm as well. The algorithm that I devised allows for digit relabeling to be segmented into partitions up to the number of digits to be relabeled:

Say I start with the intent to replace all 9 digits. If no partitioning count is provided, then all 9 digits are closed in a single cycle for 1-to-1 mappings: 9 -> 2, 2 -> 3, 3 -> 6, 6 -> 8, 8 -> 7, 7 -> 5, 5 -> 4, 4 -> 1, 1 -> 9. If a partitioning count is provided, then the cycle size for closure is decreased to create a new partition, such that it would produce the cycle closures {8, 1}, then {7, 1, 1} or {7, 2}, then {6, 1, 1, 1}, {6, 2, 1} or {6, 3}. All partitions of size 1 are no ops for whatever digit is placed there.  

I found this method to be slower but more explicit, meaning that I can define the partition set for cycle closure without needing to calculate  standard relabeling: digPermut({{2, 4, 7}, {8, 9}, {1, 5, 6}}).<br> 

I decided to incorporate the standard for speed because the explicit method is still around 1-2 microseconds, but the standard is of a magnitude 10x faster than it. 

# Sudoku Transform Benchmark Projection

This table shows average warm-cache timings (ns) from two runs single transforms applied on independent grids, converted to cycles of my current architecture's GHz to a projected benchmark at 5.0 GHz.
<br>

| Transform                | Avg (ns) | Cycles @2.5 GHz | Projected ns @5.0 GHz |
|---------------------------|---------:|----------------:|----------------------:|
| Transpose                 | 500      | 1250            | ~210 |
| Inverse Transpose         | 550      | 1375            | ~230 |
| CCW Rotation              | 600      | 1500            | ~250 |
| CW Rotation               | 450      | 1125            | ~190 |
| Vertical Reflection       | 200      | 500             | ~85 |
| Horizontal Reflection     | 250      | 625             | ~105 |
| Toroidal Shift            | 150      | 375             | ~65 |
| Band Swap                 | 50       | 125             | ~20 |
| Stack Swap                | 50       | 125             | ~20 |
| Band Row Swap             | 100      | 250             | ~40 |
| Stack Column Swap         | 150      | 375             | ~65 |
| Digit Permutation         | 150      | 375             | ~65 |

The partition-based digit transformation method was not used for this benchmark, but it was used in the 100,000,000 transform benchmark along with the standard method, where the maximum partition set of 30 was applied by running a loop for all options of the partitioning subroutine, along with all 9! permutations for standard digit relabeling.
