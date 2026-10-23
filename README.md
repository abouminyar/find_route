# find_route

A C++ program that finds a route between two cities on a road map. The road map is read from a text file (`city1 city2 km` on each line, ending with `END OF INPUT`). Roads are two-way. The program prints the total distance and the list of cities on the route, and prints "infinity / none" when there is no route.

## Algorithms

- **BFS** – Breadth First Search
- **DFS** – Depth First Search
- **IDT** – Iterative Deepening Tree Search
- **UCS** – Uniform Cost Search

## Authors

| Student | Algorithms |
|---|---|
| Abdalrahman Mohamed Beram (عبدالرحمن محمد بيرام) | BFS, DFS |
| Mohamed Ahmed abouminyar (محمد احمد بومنيار) | IDT, UCS |

## Build

```
g++ -o find_route find_route.cpp
```

## Run

```
./find_route <algorithm> <input_filename> <origin_city> <destination_city>
```

`algorithm` is one of `BFS`, `DFS`, `IDT`, `UCS`. On Windows use `.\find_route.exe` instead of `./find_route`.
