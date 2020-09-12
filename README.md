# Logic-based Sudoku Solver UI

[![MIT License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/HappyCerberus/sudoku/blob/master/LICENSE)

This is a complementary repository for the [sudoku solver](https://happycerberus.github.io/sudoku/) project.

The UI is written completely in C++ using webassembly, based on the [asm-dom-cmake](https://github.com/ArthurSonzogni/asm-dom-cmake) starter project.

All the magic relies on the [asm-dom](https://github.com/mbasso/asm-dom) library.

If you have any questions about the code, join me for one of the streams, every Sat&Sun 14:30 CE(S)T at [Youtube](https://www.youtube.com/user/HappyCerberus) or [Twitch](https://twitch.tv/happycerberus).

You can test the current build in the [sudoku_playground repository](https://happycerberus.github.io/sudoku_playground/).

## Stream archive

The video & audio quality improves over time and I'm continually making improvements.

* [2020/05/09](https://www.youtube.com/watch?v=5sWMJq4A7w8) Basic backtracking solver (bad sound quality)
* [2020/05/10](https://www.youtube.com/watch?v=V4AI9G5i_SE) Diagonal sudoku support
* [2020/05/16](https://www.youtube.com/watch?v=DRZIKmg4SlI) Working on pruning support
* [2020/05/17](https://www.youtube.com/watch?v=TaL9mO_nx_k) Backtracking with pruning working
* [2020/05/23](https://www.youtube.com/watch?v=bBplxuWt6Uw) Trying to crack 16x16 Sudokus
* [2020/05/24](https://www.youtube.com/watch?v=TJvSay58Kts) Non-backtracking solving: sets, block intersections, fish rules 
* [2020/05/30](https://www.youtube.com/watch?v=2NGetdNAMMY) Integration with CI & code cleanup
* [2020/05/31](https://www.youtube.com/watch?v=Z9soPicHYlQ) Code cleanup
* [2020/06/06](https://www.youtube.com/watch?v=1BDzCkx2H3M) Integrating solver stats, fixing warnings
* [2020/06/07](https://www.youtube.com/watch?v=tTQywLiZ9sk) Debugging & integrating a benchmark dataset
* [2020/06/13](https://www.youtube.com/watch?v=JLhMUNtEJoY) Finned fish rules
* [2020/06/14](https://www.youtube.com/watch?v=muoFClZMEsQ) Finned fish rules
* [2020/06/20](https://www.youtube.com/watch?v=D1WXQlXBE4c) Looking at performance
* [2020/06/21](https://www.youtube.com/watch?v=naaq102wGX4) Optimizing the solver
* [2020/06/27](https://www.youtube.com/watch?v=k1C8PTeFuDM) Optimizing the solver
* [2020/06/28](https://www.youtube.com/watch?v=KUlmMBJ86o0) Optimizing the solver
* [2020/07/04](https://www.youtube.com/watch?v=geyO9bl_UJ4) Solving the last 1% of puzzles
* [2020/07/05](https://www.youtube.com/watch?v=IP1qALIQAow) Solving the last 1% of puzzles (audio missing in first 30 minutes)
* [2020/07/11](https://www.youtube.com/watch?v=0jpjYkazQ9k) Looking at the performance again
* [2020/07/12](https://www.youtube.com/watch?v=8KMKMgw0GAE) UI: Fruitlessly trying to make Emscripten work
* [2020/07/18](https://www.youtube.com/watch?v=HRfkPYK9Ldg) UI: First UI results with asm-dom
* [2020/07/19](https://www.youtube.com/watch?v=WKybY8xVm6g) UI: Connecting the solver with the UI
* [2020/07/25](https://www.youtube.com/watch?v=Q0PMst3mHsM) UI: Storing the puzzle data in the URL
* [2020/07/26](https://www.youtube.com/watch?v=t9BHhHa8aHU) UI: Adding buttons & candidate notation
* [2020/08/01](https://www.youtube.com/watch?v=wzThLGjnNc4) Initial attempts at Killer Sudoku
* [2020/08/02](https://www.youtube.com/watch?v=VcP9sCdq2UA) Refactoring code for simplicity & C++20 Part 1
* [2020/08/08](https://www.youtube.com/watch?v=7HCIzoYdcgk) Refactoring code for simplicity & C++20 Part 2
* [2020/08/09](https://www.youtube.com/watch?v=sIlTAofJqI4) Refactoring code for simplicity & C++20 Part 3
* [2020/08/15](https://www.youtube.com/watch?v=42z1t8BwFcQ) Refactoring code for simplicity & C++20 Part 4
* [2020/08/16](https://www.youtube.com/watch?v=mmMSGK9da9o) Refactoring code for simplicity & C++20 Part 5
* [2020/08/22](https://www.youtube.com/watch?v=UhhJV4TsJcw) Refactoring code for simplicity & C++20 Part 6
* [2020/08/23](https://www.youtube.com/watch?v=qs7kNE5G6Ls) Refactoring code for simplicity & C++20 Part 7
* [2020/08/29](https://www.youtube.com/watch?v=P2tKrxSYwLA) Compile time datastructure for Killer Sudoku support
* [2020/08/30](https://www.youtube.com/watch?v=olakOvh8qgo) Initial Killer Sudoku support
* [2020/09/12](https://www.youtube.com/watch?v=exqbUQYD0Sk) Refactoring the UI & fixing square selection

## Features to be implemented

* drag selection of multiple squares
