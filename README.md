# Sudoku-Generator
Possibly the fastest Sudoku Generator available as it hits 1000 grids generated in 2.5 to 3 seconds with assurity and 2 to 2.5 without assurity on outdated hardware (2017 laptop) in a Windows environment with system interrupts, task scheduling, and no dedicated threading. It would need proper benchmark tests to see if it generates in sub-millisecond times on average with modern hardware. It is highly extensible, allowing both Samurai and Multi Sudoku grids. The Samurai grids are supported by box seeded grid generation, meaning that it can generate a puzzle from any box position. The grid generator handles standard 9x9 grids.

---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

The peak completion times in both spectrums are 54µs and 60ms. The variance is due to the architecture and task scheduling, so it needs to be tested on a modern PC in a Linux environment with O3 optimizations. As the benchmark tests are not optimal, one should be able to see the potential given its peak generation speed: averages are bogged down by the occasional 60ms spikes due to task scheduling and retries. If a retry is needed, the time is typically under 10ms, so 30-60ms times are due to the environment. I can't be certain because I do not possess the right architecture for benchmarking this.

It possesses the full suite of transforms, allowing 100,000 puzzles generated in the same time with all transforms applied. The objective is for this to become the fastest known grid generator, that can be used as the engine for solvers and puzzle generators. It inherently supports mask overlays to handle variants like Hyper Sudoku, Killer Sudoku, Diagonal/X Sudoku, Irregular Sudoku, all of which are not implemented currently. Samurai and Multi Sudoku is supported. The mask overlays can be accomplished in a manner similar to the Phistemofel mask produced from the Phistemofel function.

I believe it can generate a few hundred thousand (2-4) in a second with transforms on modern hardware if optimized appropriately. 

Current architecture:

Acer Nitro 5
- CPU: Intel Core i5‑7300HQ 2.5GHz
- GPU: NVIDIA GeForce GTX 1050 4GB
- RAM: 32 GB DDR4

Options list for testing:

--count n
--mode default/transform/xform/tf/t
--verbose
--assured

Example: --count 10000 --mode transform --verbose --assured

The --count option allows generation up to a certain count. 
The --verbose option enables stream output, someone could convert this to file stream.
The --assured option enables assurity, meaning the generation is looped until successful. The 90% success rate means most attempts will succeed in 2 tries or less, gravitating to 1 attempt.
