# Metro Transit System — Journey & Network Module (`journey/`)

**Author:** Aaryan Rawat (`Aaryan-Rawat28`)  
**Branch:** `aaryan`

<<<<<<< HEAD
A modular, terminal-based **Metro Transit Management System** built in **C++17**. Passengers can plan multi-line journeys, book tickets with priority waitlisting, and travel using smart cards or passes; administrators can manage the transit network, trains, student/women's card registries, analytics reports, and rush-hour simulations.

- **Zero STL containers:** Built with a custom template `Array<T>` and **17 hand-implemented data structures and algorithms**—no `std::vector`, `std::map`, or external libraries.
- **Pure C++17 CLI:** Standard `cin`/`cout` terminal interface with no GUI dependencies, no color escape assumptions, and deterministic behavior.
- **Crash-safe persistence:** Network topology is seeded from editable `data/*.txt` files on first launch and persisted atomically to `metro_data.txt` after every mutation.
=======
This branch contains the **Journey and Network** subsystem of the Metro Transit System. It handles everything needed to model stations and metro lines, load the starting network from text files, search stations by name or prefix, find the shortest route between stations, calculate distance-based fares with passenger discounts, and maintain a recent activity log.
>>>>>>> origin/aaryan

---

## Table of Contents

<<<<<<< HEAD
- [Key Capabilities](#key-capabilities)
- [System & Graph Architecture](#system--graph-architecture)
  - [Layered System Architecture](#1-layered-system-architecture)
  - [Graph & Routing Engine](#2-graph--routing-engine)
  - [Ticketing, Waitlist & Smart Card Flows](#3-ticketing-waitlist--smart-card-flows)
- [Project Structure](#project-structure)
- [Data Structures & Algorithms](#data-structures--algorithms)
- [Time Complexity Matrix](#time-complexity-matrix)
- [Object-Oriented Design](#object-oriented-design)
- [Business Rules & Fare Policy](#business-rules--fare-policy)
- [Getting Started](#getting-started)
  - [Prerequisites & Build](#prerequisites--build)
  - [Running & CLI Options](#running--cli-options)
  - [Demo Credentials & Seed Accounts](#demo-credentials--seed-accounts)
- [Menus & Operations](#menus--operations)
- [Starting Data & Network Topology](#starting-data--network-topology)
- [End-to-End Demo Walkthrough](#end-to-end-demo-walkthrough)
- [Testing & Quality Assurance](#testing--quality-assurance)
- [Team & Contributors](#team--contributors)
=======
- [What This Module Does](#what-this-module-does)
- [Architecture & Flow Diagrams](#architecture--flow-diagrams)
  - [1. Module Architecture](#1-module-architecture)
  - [2. Journey Planning & Fare Flow](#2-journey-planning--fare-flow)
  - [3. FarePolicy Class Hierarchy](#3-farepolicy-class-hierarchy)
- [Folder Structure](#folder-structure)
- [Data Structures & Algorithms (`journey/ds/`)](#data-structures--algorithms-journeyds)
- [Core Classes & Features (`journey/classes/`)](#core-classes--features-journeyclasses)
- [Time Complexity](#time-complexity)
- [OOP Concepts Used](#oop-concepts-used)
- [Fare Rules & Seed Data Format](#fare-rules--seed-data-format)
- [Compile Check](#compile-check)
>>>>>>> origin/aaryan

---

## What This Module Does

In simple terms, the `journey/` module acts as the **map, route planner, and fare calculator** of the metro system:

1. **Reads the starting map** (`seed_loader.cpp`) from plain text files (lines, stations, track distances, and initial accounts) and reports exact file and line numbers if anything is mistyped.
2. **Finds stations quickly** (`binary_search.cpp`) even if a user only types the first few letters (for example, typing `"raj"` suggests `"Rajiv Chowk"`).
3. **Finds the shortest path** (`dijkstra.cpp` + `heap_priority_queue.cpp`) between any two open stations, respecting closed stations when needed.
4. **Builds line views in both directions** (`doubly_linked_list.cpp`) so the system can walk forward or backward along a line and tell passengers which terminal station a train is heading towards (`"towards ..."`).
5. **Calculates ticket and card fares** (`fare_policy.cpp`) based on real track distance in meters and applies discounts based on passenger type.
6. **Checks if closing a station cuts off the network** (`metro_network.cpp`) and keeps a rolling log of the newest 200 actions (`deque.cpp`).

---

## Architecture & Flow Diagrams

### 1. Module Architecture

```mermaid
flowchart TD
    Header["metro.h (Shared Declarations & Structs)"]
    Header --> Classes
    Header --> DS

    subgraph Classes["Feature Layer (journey/classes/)"]
        Seed["seed_loader.cpp (Startup File Parser)"]
        Net["metro_network.cpp (Station, Line & Route Rules)"]
        Fare["fare_policy.cpp (Polymorphic Fare Calculation)"]
    end

    subgraph DS["Data Structures & Algorithms (journey/ds/)"]
        Dijkstra["dijkstra.cpp (Shortest Path)"]
        Heap["heap_priority_queue.cpp (Two-Key Min-Heap)"]
        BinSearch["binary_search.cpp (Sorted Station Index)"]
        DList["doubly_linked_list.cpp (Bidirectional Line Walk)"]
        Deque["deque.cpp (200-Item Activity Buffer)"]
    end

    Seed -->|"Populates network"| Net
    Net -->|"Exact & prefix lookup"| BinSearch
    Net -->|"Shortest distance route"| Dijkstra
    Dijkstra -->|"Next closest station"| Heap
    Net -->|"Line order & direction"| DList
    Net -->|"Distance to fare"| Fare
    Net -->|"Logs recent events"| Deque
```

### 2. Journey Planning & Fare Flow

```mermaid
flowchart LR
    Input["Passenger Enters Source & Destination"] --> Lookup["Binary Search Index (findInIndex / findByPrefix)"]
    Lookup -->|"Valid station IDs"| Route["dijkstraRoute (Shortest Track Distance)"]
    Route -->|"Uses Min-Heap"| Path["RouteHolder (RAII Route Result)"]
    Path -->|"Total meters"| Base["fareForDistance (Distance Slab)"]
    Base -->|"Base fare"| Policy["FarePolicy::apply (Student / Senior / Disabled / Regular)"]
    Path -->|"Line & direction"| Dir["DList Walk (towardsStation)"]
    Policy --> Output["Final Route, Direction & Fare"]
    Dir --> Output
```

### 3. FarePolicy Class Hierarchy

```mermaid
classDiagram
    class FarePolicy {
        <<abstract>>
        +name() string*
        +discountPercent() int*
        +waitlistPriority() int
        +apply(baseFare int) int
    }
    class RegularFare {
        +name() string
        +discountPercent() int
    }
    class StudentFare {
        +name() string
        +discountPercent() int
        +waitlistPriority() int
    }
    class SeniorFare {
        +name() string
        +discountPercent() int
        +waitlistPriority() int
    }
    class DisabledFare {
        +name() string
        +discountPercent() int
        +waitlistPriority() int
    }

    FarePolicy <|-- RegularFare
    FarePolicy <|-- StudentFare
    FarePolicy <|-- SeniorFare
    FarePolicy <|-- DisabledFare
```

---

## Folder Structure

```text
aaryan/
├── .gitignore
├── README.md
├── metro.h                             # Shared structs, class declarations, and function prototypes
└── journey/
    ├── ds/
    │   ├── binary_search.cpp           # Sorted station index, exact lookup, and prefix suggestions
    │   ├── deque.cpp                   # Circular-buffer double-ended queue
    │   ├── dijkstra.cpp                # Weighted shortest-path algorithm
    │   ├── doubly_linked_list.cpp      # Bidirectional linked list for metro lines
    │   └── heap_priority_queue.cpp     # Dynamic two-key binary min-heap
    └── classes/
        ├── fare_policy.cpp             # Distance fare slabs and polymorphic passenger discounts
        ├── metro_network.cpp           # Station/line management, routing, closure safety check
        └── seed_loader.cpp             # Reads and validates startup text files
```

| File | Category | Responsibility |
| :--- | :--- | :--- |
| `metro.h` | Shared Header | Declares `Graph`, `Route`, `Heap`, `Deque`, `DList`, `StationIndex`, `FarePolicy`, and `Metro`. |
| `journey/ds/dijkstra.cpp` | Algorithm | Finds the shortest route by meters between two stations using a min-heap. |
| `journey/ds/heap_priority_queue.cpp` | Data Structure | Two-key min-heap used by Dijkstra and priority ordering. |
| `journey/ds/binary_search.cpp` | Algorithm | Builds a case-insensitive sorted station table for fast name and prefix lookup. |
| `journey/ds/deque.cpp` | Data Structure | Circular dynamic array supporting $O(1)$ front/back push and pop. |
| `journey/ds/doubly_linked_list.cpp` | Data Structure | Doubly linked list used to inspect metro lines forward and backward. |
| `journey/classes/fare_policy.cpp` | OOP / Domain | Maps track meters to base fare and applies category discounts via virtual functions. |
| `journey/classes/metro_network.cpp` | Domain Logic | Implements `Metro` network setup, station/line queries, routing, and cut-off checks. |
| `journey/classes/seed_loader.cpp` | File I/O | Parses pipe-delimited seed files with UTF-8 BOM handling and line-numbered error messages. |

---

## Data Structures & Algorithms (`journey/ds/`)

| Data Structure / Algorithm | How It Works | Where It Is Used |
| :--- | :--- | :--- |
| **Dijkstra (`dijkstra.cpp`)** | Starts at the source station with distance `0` and all others at infinity (`LLONG_MAX`). Repeatedly pops the closest unvisited station from the min-heap and relaxes outgoing track edges (`meters`). Skips closed stations unless `allowClosed` is `true`. | Shortest-distance route planning and smart-card fare calculation. |
| **Two-Key Min-Heap (`heap_priority_queue.cpp`)** | Dynamic binary heap stored in a contiguous array (starts at capacity `16`, doubles on growth). Compares items by `key1` first, then breaks ties with `key2`. | Drives Dijkstra's priority queue (distance) and priority ordering (tier + sequence). |
| **Binary Search (`binary_search.cpp`)** | Lowercases all station names, sorts `StationIndexEntry` records, and runs both iterative and recursive binary search. `findByPrefix` uses lower-bound binary search to collect matching station IDs. | `Metro::findStation()` and `"Did you mean ...?"` station suggestions. |
| **Circular Deque (`deque.cpp`)** | Generic byte-buffer deque (`itemSize`) using modular wrap-around (`head`) and automatic capacity doubling from `8`. | Powers the `activity_` log, keeping the newest 200 events in $O(1)$ time. |
| **Doubly Linked List (`doubly_linked_list.cpp`)** | Heap-allocated `DNode` chain with `prev` and `next` pointers plus `head`/`tail` tracking. Supports front/back insertion, removal, index lookup, and full cleanup. | `Metro::makeLineList()` and `Metro::towardsStation()` for bidirectional line traversal. |

---

## Core Classes & Features (`journey/classes/`)

### `FarePolicy` (`fare_policy.cpp`)
- Abstract base class defining `name()`, `discountPercent()`, `waitlistPriority()`, and `apply(int baseFare)`.
- Derived classes (`RegularFare`, `StudentFare`, `SeniorFare`, `DisabledFare`) override the virtual methods so fare calculation works cleanly through a single `const FarePolicy&` reference.
- `fareForDistance(long long meters)` converts total track distance into base ticket slabs.

### `Metro` Network Operations (`metro_network.cpp`)
- **Constructor / Destructor (`Metro::Metro`, `Metro::~Metro`):** Allocates internal structures safely and frees all memory in `freeAll()` if any allocation fails or when `Metro` is destroyed.
- **Built-in Fallback (`loadDefaultNetwork`):** Loads 3 lines (`Blue`, `Yellow`, `Red`), 14 stations, 14 tracks, sample students, women's cards, and 2 demo smart cards if seed files are unavailable.
- **Network Management:** `addStation`, `addLine`, `addConnection`, `setStationOpen`, `linesAtStation`, `trackDistance`.
- **Routing & Safety:** `planRoute` (selects shortest distance or fewest stops), `suggestStations` (prefix match), `cutOffStations` (finds stations disconnected if a target station is closed), `makeLineList`, `isLoopLine`, `towardsStation`.

### Seed Loader (`seed_loader.cpp`)
- `loadStartingData(Metro&, folder, error, summary)` reads `lines.txt`, `stations.txt`, `connections.txt`, `students.txt`, `women_cards.txt`, and `cards.txt`.
- Ignores blank lines and `#` comments, strips UTF-8 BOM if present, trims whitespace around `|` separators, and rejects duplicate stations or negative distances with exact `file line N:` error messages.

---

## Time Complexity

| Operation | Complexity | Reason |
| :--- | :--- | :--- |
| Shortest route (`dijkstraRoute`) | $O((V + E) \log V)$ | Each vertex/edge relaxation uses binary min-heap `heapPush` / `heapPop` |
| Exact station lookup (`findInIndex`) | $O(\log V)$ | Binary search over sorted `StationIndexEntry` array |
| Prefix station search (`findByPrefix`) | $O(\log V + k)$ | Lower-bound binary search + scanning $k$ matching prefixes |
| Build station index (`buildStationIndex`) | $O(V \log V)$ | Copies $V$ station names and sorts them |
| Add/remove activity log (`dequeAddFront` / `dequeRemoveBack`) | $O(1)$ | Circular buffer index arithmetic (amortized on resize) |
| Line list traversal (`makeLineList` / `towardsStation`) | $O(L)$ | Walks $L$ stations along a single metro line using `DList` |
| Fare calculation (`fareForDistance`, `FarePolicy::apply`) | $O(1)$ | Constant-time slab comparison and integer percentage math |

---

## OOP Concepts Used

| Concept | Where It Appears in `journey/` |
| :--- | :--- |
| **Abstract Base Class & Pure Virtual Functions** | `FarePolicy` in `metro.h` (`name()` and `discountPercent()` are pure virtual `= 0`) |
| **Inheritance & Function Overriding** | `RegularFare`, `StudentFare`, `SeniorFare`, and `DisabledFare` inherit from `FarePolicy` and use `override` |
| **Runtime Polymorphism** | `farePolicy(int category)` returns `const FarePolicy&`, dispatching virtual calls at runtime |
| **RAII (Resource Acquisition Is Initialization)** | `RouteHolder` and `LineList` allocate resources in constructors and free them automatically in destructors |
| **Constructor & Destructor Resource Safety** | `Metro::Metro()` rolls back with `freeAll()` and throws `std::bad_alloc` on partial failure |
| **File Handling (`std::ifstream`)** | `seed_loader.cpp` parses and validates startup configuration files |

---

## Fare Rules & Seed Data Format

### Distance-Based Base Fares & Discounts

| Track Distance | Base Fare |
| :--- | :--- |
| Up to 2 km (`<= 2000 m`) | Rs. 10 |
| Up to 5 km (`<= 5000 m`) | Rs. 20 |
| Up to 12 km (`<= 12000 m`) | Rs. 30 |
| Up to 21 km (`<= 21000 m`) | Rs. 40 |
| Up to 32 km (`<= 32000 m`) | Rs. 50 |
| Above 32 km (`> 32000 m`) | Rs. 60 |

| Passenger Category | Discount | Final Fare Formula |
| :--- | :--- | :--- |
| Regular (`REGULAR`) | 0% | `baseFare` |
| Student (`STUDENT`) | 50% | `baseFare - (baseFare * 50) / 100` |
| Senior Citizen (`SENIOR`) | 30% | `baseFare - (baseFare * 30) / 100` |
| Person with Disability (`DISABLED`) | 50% | `baseFare - (baseFare * 50) / 100` |

### Startup Seed File Formats (`seed_loader.cpp`)

| File | Expected Format | Example |
| :--- | :--- | :--- |
| `lines.txt` | `name \| capacity \| frequency_minutes` | `Blue \| 6 \| 5` |
| `stations.txt` | `station_name` | `Rajiv Chowk` |
| `connections.txt` | `station_a \| station_b \| meters \| line_name` | `Airport \| City Mall \| 4200 \| Blue` |
| `students.txt` | `student_id` | `101` |
| `women_cards.txt` | `card_number` | `9001` |
| `cards.txt` | `name \| category \| student_id \| balance` | `Priya Negi \| student \| 101 \| 300` |

---

## Compile Check

This branch contains only the `journey/` module and `metro.h`. You can verify that every `.cpp` file in `journey/` compiles cleanly to object files:

**Windows (PowerShell):**
```powershell
g++ -std=c++17 -I. -c journey/classes/*.cpp journey/ds/*.cpp
```

**Linux / macOS:**
```bash
g++ -std=c++17 -I. -c journey/classes/*.cpp journey/ds/*.cpp
```

<<<<<<< HEAD
**Explicit File List (Cross-Platform Fallback):**
```bash
g++ -std=c++17 -I. cards/classes/admin_undo.cpp cards/classes/clock.cpp cards/classes/smart_cards.cpp cards/classes/storage.cpp cards/ds/dfs.cpp cards/ds/hash_table.cpp cards/ds/linked_list.cpp cards/ds/stack.cpp journey/classes/fare_policy.cpp journey/classes/metro_network.cpp journey/classes/seed_loader.cpp journey/ds/binary_search.cpp journey/ds/deque.cpp journey/ds/dijkstra.cpp journey/ds/doubly_linked_list.cpp journey/ds/heap_priority_queue.cpp reports/classes/app_menus.cpp reports/classes/main.cpp reports/classes/reports.cpp reports/classes/simulation.cpp reports/classes/ui.cpp reports/ds/circular_queue.cpp reports/ds/graph.cpp reports/ds/merge_sort.cpp reports/ds/quick_sort.cpp tickets/classes/students_women_cards.cpp tickets/classes/ticket_booking.cpp tickets/classes/trains.cpp tickets/ds/avl_tree.cpp tickets/ds/bfs.cpp tickets/ds/bst.cpp tickets/ds/queue.cpp -o metro
```

### Running & CLI Options

```bash
./metro [--data FILE] [--seed FOLDER]
```

- `--data FILE`: Use a custom persistence file instead of `metro_data.txt`.
- `--seed FOLDER`: Load initial network configuration from a custom directory instead of `data/`.

### Demo Credentials & Seed Accounts

- **Default Admin Password:** `1234` (changeable inside the Admin panel)
- **Demo Smart Cards:** `5001` (Regular, Rs. 500) and `5002` (Student, Rs. 300)
- **Registered Student IDs:** `101`, `102`, `105`, `110`, `115`
- **Verified Women's Cards:** `9001`, `9002`, `9003`

---

## Menus & Operations

| Passenger Services | Smart Card Services | Admin Panel |
| :--- | :--- | :--- |
| View stations and lines | Get a new smart card | Add station, add line, add connection |
| View a line in both directions | Card details and balance | Close / reopen station (with DFS check) |
| Plan a journey (Dijkstra & BFS) | Recharge card | Register / deactivate student ID & view registry |
| Next trains and crowd levels | Tap in (enter a station) | Register / deactivate women's card & view registry |
| Book a ticket | Tap out (exit a station) | Train & waitlist status, dispatch train, set capacity |
| Cancel a ticket | Buy a day / monthly pass | View tickets by ID (AVL), by fare (Quick Sort), or ID range |
| Verify ticket and board | Journey history | Reports, recent activity log (Deque), undo action (Stack) |
| Check ticket or waitlist status | Report lost card | Rush-hour simulation & advance demo clock |
| Leave the waiting list | — | Change admin password & manual save |

---

## Starting Data & Network Topology

On first run (when `metro_data.txt` is absent), `seed_loader.cpp` parses plain-text pipe-delimited files in `data/` and writes the initial state to `metro_data.txt`. Subsequent runs load directly from `metro_data.txt`; delete `metro_data.txt` to re-seed from `data/`. If `data/` is missing, `Metro::loadDefaultNetwork()` loads a built-in 3-line, 14-station fallback network.

| Seed File | Record Type | Format (`#` comments and blank lines ignored) |
| :--- | :--- | :--- |
| `data/lines.txt` | Metro line | `name \| train capacity \| frequency in minutes` |
| `data/stations.txt` | Station | `name` |
| `data/connections.txt` | Weighted edge | `station A \| station B \| distance in metres \| line` |
| `data/students.txt` | Registered student | `student ID` |
| `data/women_cards.txt` | Verified women's card | `card number` |
| `data/cards.txt` | Demo smart card | `name \| regular/student/senior/disabled \| student ID (0 if none) \| balance` |

Validation errors pinpoint the exact file and line number (e.g., `data/connections.txt line 7: station 'Rajiv Chowkk' is not in data/stations.txt`).

### Built-In `data/` Network (6 Lines · 29 Stations · 39 Tracks)

```text
Blue:   Airport - City Mall* - Rajiv Chowk* - ISBT* - Railway Station - Clock Tower* - Old Market* - River Front
Yellow: Forest Institute - DU - Civil Lines* - Rajiv Chowk* - City Park* - Tech Park* - Graphic Era
Red:    ISBT* - Hospital - Exhibition Ground - City Park* - Stadium* - Sports Complex
Green:  Hill View - University - Science City* - Civil Lines* - Secretariat* - Clock Tower* - Bus Depot*
Pink:   Airport* - Aerocity - Transport Nagar - Industrial Area* - Stadium* - Tech Park* - Botanical Garden*
Ring:   City Mall* - Science City* - Secretariat* - Mall Road - Old Market* - Bus Depot*
        - Botanical Garden* - Industrial Area* - Lake View - (back to City Mall)
                                                              (* = interchange)
```

*(Default train capacity per line in `data/lines.txt` is `6` to make waitlist promotion easy to demonstrate; adjustable in the Admin panel.)*

---

## End-to-End Demo Walkthrough

1. **Route Finder:** Passenger $\to$ Plan a journey (`ISBT` to `City Park`). Dijkstra finds `2.4 km` on the Red Line; BFS finds `2 stops` (`4.8 km`, 1 interchange).
2. **Station Closure & Undo:** Admin $\to$ close `Hospital` $\to$ re-plan `ISBT` to `City Park` (reroutes via `Rajiv Chowk`) $\to$ Admin Undo restores `Hospital`.
3. **DFS Safety Check:** Attempt to close `Tech Park`; system warns `Graphic Era` would be cut off (`n` to abort).
4. **Priority Waiting List:** Set Blue Line capacity to `1`, book `Airport` $\to$ `City Mall`, then book 3 more passengers (Regular, Woman `9001`, Senior). Waitlist orders: Senior $\to$ Woman $\to$ Regular. Cancel ticket 1: Senior is promoted automatically.
5. **Concession Ticket:** Book `Airport` $\to$ `Graphic Era` as Student `101` with women's card `9002` (Rs. 40 base $\to$ Rs. 20 paid, showing interchange at `Tech Park`).
6. **Smart Card Trip:** Tap in `5001` at `Airport`, tap out at `Graphic Era` (Rs. 40 deducted for `12.5 km`); inspect journey history.
7. **Pass & Overstay:** Buy a day pass, tap in, advance clock by `200` minutes, tap out: Rs. 0 fare + Rs. 50 overstay penalty.
8. **Ticket Expiry:** Book a ticket, advance clock by `130` minutes, attempt to board: ticket expires and frees the seat.
9. **Line View & Prefix Search:** Inspect Blue Line and looping Ring Line; enter `raj` to trigger `"Did you mean: Rajiv Chowk?"`.
10. **AVL vs. BST & Quick Sort:** View tickets by ID (AVL), by fare (Quick Sort), and by ID range; inspect BST vs. AVL tree heights in Reports.
11. **Reports & Simulation:** View revenue, busiest stations, popular routes; run rush-hour simulation and compare average wait times across train frequencies.
12. **Persistence:** Exit, relaunch, and verify all state survives via `metro_data.txt`.

---

## Testing & Quality Assurance

Both test binaries compile with `-DMETRO_NO_MAIN` to swap in dedicated test harnesses.

**Windows (PowerShell):**
```powershell
$src = (Get-ChildItem journey,tickets,cards,reports -Recurse -Filter *.cpp).FullName
g++ -std=c++17 -I. -DMETRO_NO_MAIN tests/test_ds.cpp $src -o test_ds
.\test_ds.exe
g++ -std=c++17 -I. -DMETRO_NO_MAIN tests/test_metro.cpp $src -o test_metro
.\test_metro.exe
```

**Linux / macOS:**
```bash
src=$(find journey tickets cards reports -name "*.cpp")
g++ -std=c++17 -I. -DMETRO_NO_MAIN tests/test_ds.cpp $src -o test_ds && ./test_ds
g++ -std=c++17 -I. -DMETRO_NO_MAIN tests/test_metro.cpp $src -o test_metro && ./test_metro
```

- **`tests/test_ds.cpp` (12,370 checks):** Covers two-key heap tie-breaking, 10,000+ hash table insertions with dynamic rehashing, merge/quick sort stability & edge cases, Dijkstra vs. BFS correctness, recursive & iterative DFS, 5,000-node degenerate BST chain vs. all four AVL rotations, circular-queue wrap-around, deque, doubly linked list, and binary search.
- **`tests/test_metro.cpp` (1,299 checks):** Covers fare policies, routing, waitlist promotion & lazy leave, ticket expiry, smart cards, passes, overstay penalties, lost-card transfers, admin undo, login lockout, atomic save/load, corrupt-file recovery, reports, simulation, BST vs. AVL heights at 1,000 tickets, and seed-file error diagnostics.
- Verified with **AddressSanitizer** and **UndefinedBehaviorSanitizer** with zero memory leaks or undefined behavior.

---

## Team & Contributors

Built by a team of four, with each member owning a dedicated module (`journey/`, `tickets/`, `cards/`, `reports/`), its underlying data structures and algorithms, and its feature classes connected through `metro.h`.

| Member | Module | Role & Responsibilities | Data Structures & Algorithms |
| :--- | :--- | :--- | :--- |
| **Member 1** | `journey/` | Journey planning, fare policies, seed loading, stations, lines, closure checks | Dijkstra, Min-Heap, Binary Search, Deque, Doubly Linked List |
| **Member 2** | `tickets/` | Ticket booking, cancellation, expiry, priority waiting list, student/women registries, trains | BFS, AVL Tree, BST, Queue |
| **Member 3** | `cards/` | Smart cards, tap-in/out, passes, lost cards, admin login/undo, persistence, clock | DFS, Hash Table, Stack, Singly Linked List |
| **Member 4** | `reports/` | Analytics reports, rush-hour simulation, UI/table formatting, menus, `main()`, test suites | Graph (Adjacency List), Quick Sort, Merge Sort, Circular Queue |
=======
*(Full executable linking requires combining all subsystem folders on the `main` branch.)*
>>>>>>> origin/aaryan
