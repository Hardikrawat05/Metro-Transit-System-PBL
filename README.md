# Metro Transit System (C++)

A terminal-based metro management system built as a Project-Based Learning (PBL) project at
Graphic Era. Passengers can plan journeys, book tickets and use smart cards. Admins manage the
network, trains, students and women's cards, see reports, and run a rush-hour simulation.

- Written entirely in **C++17**, with **no STL containers** (no `std::vector`).
- **17 data structures and algorithms written by the team**, each used by a real feature.
- **Terminal only:** no GUI, no extra libraries, no colours, no emoji; only standard `cin` and `cout`.
- All data is **saved automatically** to a text file; the starting network is read from editable text files.

## Highlights

- **Two routes for every journey:** shortest distance (Dijkstra) and fewest stops (BFS), with
  line changes, the distance to each station, and the direction of travel ("towards ...").
- **Fares from real track distance**, with discounts by passenger type (inheritance and polymorphism).
- **Priority waiting list** (heap): senior citizens and persons with disability first, then women
  with a verified women's card, then students, then regular passengers.
- **Smart cards:** recharge, tap in and tap out, day and monthly passes, journey history, overstay
  penalty, and blocking a lost card with the balance moved to a new card.
- **Safety check before closing a station** (DFS): shows which stations would be cut off.
- **BST vs AVL tree:** with 1,000 tickets the plain BST has height 1,000, the AVL tree only 10.
- **Undo** for admin actions (stack) and a **recent activity log** (deque).
- **Rush-hour simulation** with a fixed-size platform (circular queue) and a gate queue.
- **About 13,700 automatic test checks.**

## Project structure

```text
Metro-Transit-System-PBL/
├── metro.h                         shared header: every struct, class and function declaration
├── journey/                        Member 1
│   ├── ds/       dijkstra.cpp, heap_priority_queue.cpp, binary_search.cpp, deque.cpp,
│   │             doubly_linked_list.cpp
│   └── classes/  fare_policy.cpp, seed_loader.cpp, metro_network.cpp
├── tickets/                        Member 2
│   ├── ds/       bfs.cpp, avl_tree.cpp, bst.cpp, queue.cpp
│   └── classes/  ticket_booking.cpp, students_women_cards.cpp, trains.cpp
├── cards/                          Member 3
│   ├── ds/       dfs.cpp, hash_table.cpp, stack.cpp, linked_list.cpp
│   └── classes/  smart_cards.cpp, admin_undo.cpp, storage.cpp, clock.cpp
├── reports/                        Member 4
│   ├── ds/       graph.cpp, quick_sort.cpp, merge_sort.cpp, circular_queue.cpp
│   └── classes/  reports.cpp, simulation.cpp, ui.cpp, app_menus.cpp, main.cpp
├── data/                           starting data you can edit (see "Starting data")
└── tests/
    ├── test_ds.cpp                 tests for every data structure
    └── test_metro.cpp              tests for the application rules
```

In every member folder, `ds/` has one file per data structure and `classes/` has the features
that use them.

**`metro.h`** holds all declarations: the data structures, the record structs (such as `Ticket`
and `Card`) and the classes. Every `.cpp` file includes it with `#include "metro.h"`. It is
divided into one section per member. Member 4's section (the graph) comes first, because the
`Graph` type is used by the other members' declarations (Dijkstra, BFS, DFS, binary search).

## How it works

1. **Start-up** (`reports/classes/main.cpp`): if `metro_data.txt` exists, all saved data is loaded
   (`cards/classes/storage.cpp`). On the first run, the network is built from the `data/` files
   (`journey/classes/seed_loader.cpp`) and saved.
2. **Main menu** (`reports/classes/app_menus.cpp`): Passenger services, Smart card services,
   Admin panel, or Exit.
3. **Every action** calls the `Metro` class (the rules), which uses the data structures. `Metro`
   never prints anything; `App` does all the printing. This split makes the rules easy to test.
4. **After every change**, everything is saved to `metro_data.txt`: first to a temporary file,
   then that file replaces the old one, so a crash cannot leave a half-written file.
5. **Exit (0)** saves the data and frees all memory.

## Data structures and where they are used

| Data structure | Used for | File |
|----------------|----------|------|
| Graph (adjacency list) | Stations as vertices, tracks as weighted edges | `reports/ds/graph.cpp` |
| Dijkstra | Shortest route by distance; fares for smart cards | `journey/ds/dijkstra.cpp` |
| Heap (priority queue, two keys) | Dijkstra; priority waiting list | `journey/ds/heap_priority_queue.cpp` |
| BFS | Route with the fewest stops | `tickets/ds/bfs.cpp` |
| Queue | BFS; the gate in the simulation | `tickets/ds/queue.cpp` |
| DFS (recursive and iterative) | Which stations would be cut off if a station closes | `cards/ds/dfs.cpp` |
| Stack | Undo; iterative DFS | `cards/ds/stack.cpp` |
| Hash table (chaining, resizing) | Find cards, tickets, students and women's cards by number | `cards/ds/hash_table.cpp` |
| Singly linked list | Journey history of each card | `cards/ds/linked_list.cpp` |
| Doubly linked list | Line view in both directions; "towards ..." | `journey/ds/doubly_linked_list.cpp` |
| Deque | Recent activity log (newest 200 entries) | `journey/ds/deque.cpp` |
| Binary search | Find a station by name; "Did you mean ...?" | `journey/ds/binary_search.cpp` |
| BST | Ticket index, kept to show the degenerate case | `tickets/ds/bst.cpp` |
| AVL tree | Ticket index: tickets in ID order and ID ranges | `tickets/ds/avl_tree.cpp` |
| Merge sort (stable) | Reports: busiest stations, popular routes | `reports/ds/merge_sort.cpp` |
| Quick sort (median of three) | Tickets sorted by fare | `reports/ds/quick_sort.cpp` |
| Circular queue | Fixed-size platform in the simulation | `reports/ds/circular_queue.cpp` |

## OOP concepts used

| Concept | Where |
|---------|-------|
| Classes and objects | `Metro`, `App`, `Clock`, `Table` |
| Abstract class, inheritance, virtual functions (runtime polymorphism) | `FarePolicy` with `RegularFare`, `StudentFare`, `SeniorFare`, `DisabledFare` |
| Class template | `Array<T>` (our own replacement for `std::vector`) |
| Constructor, copy constructor, assignment operator, destructor | `Array<T>`, `Metro` |
| Operator overloading | `Array<T>`: `operator[]` and `operator=` |
| Friend class | `Storage` reads and writes `Metro`'s private data |
| Static members | `Clock` (one shared clock for the whole program) |
| Exceptions | `InputClosed` is thrown when the input ends |
| RAII (memory freed in the destructor) | `RouteHolder`, `LineList` |
| File handling | `storage.cpp` (`metro_data.txt`), `seed_loader.cpp` (`data/` files) |

## Build and run

You need `g++` with C++17 (for example MSYS2 UCRT64 on Windows). Open a terminal in the folder
that contains `metro.h`.

Windows PowerShell:

```powershell
g++ -std=c++17 -I. (Get-ChildItem journey,tickets,cards,reports -Recurse -Filter *.cpp).FullName -o metro
.\metro.exe
```

Linux / macOS:

```bash
g++ -std=c++17 -I. $(find journey tickets cards reports -name "*.cpp") -o metro
./metro
```

If the short command does not work, list every file (works everywhere):

```bash
g++ -std=c++17 -I. cards/classes/admin_undo.cpp cards/classes/clock.cpp cards/classes/smart_cards.cpp cards/classes/storage.cpp cards/ds/dfs.cpp cards/ds/hash_table.cpp cards/ds/linked_list.cpp cards/ds/stack.cpp journey/classes/fare_policy.cpp journey/classes/metro_network.cpp journey/classes/seed_loader.cpp journey/ds/binary_search.cpp journey/ds/deque.cpp journey/ds/dijkstra.cpp journey/ds/doubly_linked_list.cpp journey/ds/heap_priority_queue.cpp reports/classes/app_menus.cpp reports/classes/main.cpp reports/classes/reports.cpp reports/classes/simulation.cpp reports/classes/ui.cpp reports/ds/circular_queue.cpp reports/ds/graph.cpp reports/ds/merge_sort.cpp reports/ds/quick_sort.cpp tickets/classes/students_women_cards.cpp tickets/classes/ticket_booking.cpp tickets/classes/trains.cpp tickets/ds/avl_tree.cpp tickets/ds/bfs.cpp tickets/ds/bst.cpp tickets/ds/queue.cpp -o metro
```

Options:

- `--data FILE`: use another saved-data file instead of `metro_data.txt`.
- `--seed FOLDER`: read the starting data from another folder instead of `data/`.

Demo data:

- **Admin password:** `1234` (can be changed in the admin panel).
- **Smart cards:** `5001` (Regular, Rs. 500) and `5002` (Student, Rs. 300).
- **Registered student IDs:** 101, 102, 105, 110, 115.
- **Verified women's cards:** 9001, 9002, 9003.

## Menus

| Passenger services | Smart card services | Admin panel |
|--------------------|---------------------|-------------|
| View stations and lines | Get a new smart card | Add station, add line, add connection |
| View a line in both directions | Card details and balance | Close / reopen station |
| Plan a journey (route finder) | Recharge card | Register / deactivate student ID; student registry |
| Next trains and crowd levels | Tap in (enter a station) | Register / deactivate women's card; registry |
| Book a ticket | Tap out (exit a station) | Train and waiting-list status; dispatch train; set capacity |
| Cancel a ticket | Buy a day / monthly pass | Tickets by ID, by fare, in an ID range |
| Verify ticket and board | Journey history | Reports; recent activity; undo last action |
| Check ticket or waiting-list status | Report lost card | Rush hour simulation; advance clock (demo) |
| Leave the waiting list | | Change admin password; save data now |

## Rules

| Rule | Value |
|------|-------|
| Base fare by distance | up to 2 km: Rs. 10; 5 km: Rs. 20; 12 km: Rs. 30; 21 km: Rs. 40; 32 km: Rs. 50; more: Rs. 60 |
| Discounts | Student 50%, Senior citizen 30%, Person with disability 50%, Regular 0% |
| Paper ticket validity | 2 hours; unused tickets expire and their seat is freed |
| Smart card trip limit | 3 hours; after that a Rs. 50 overstay penalty |
| Minimum balance to tap in | Rs. 10 |
| Recharge / maximum balance | up to Rs. 5,000 at a time; Rs. 10,000 on the card |
| Passes | Day Rs. 200, Monthly Rs. 2,000 (discount by passenger type applies) |
| Service hours | 06:00 to 23:00 |
| Admin login | 3 wrong passwords lock the login for 60 seconds |

### Waiting-list priority

When a seat becomes free, the waiting passenger with the best level gets it:

1. Senior citizen or person with disability
2. Woman with a verified women's card
3. Student
4. Regular

Within a level it is first come, first served. A passenger in two groups gets the better level.
The women's card gives priority only; the fare discount still comes from the passenger type.

## Starting data

The `data/` folder holds the data the system starts with. These are plain text files that can be
edited in Notepad or VS Code.

| File | One line per | Format |
|------|--------------|--------|
| `lines.txt` | metro line | `name \| train capacity \| a train every N minutes` |
| `stations.txt` | station | `name` |
| `connections.txt` | track between two stations | `station \| station \| distance in metres \| line` |
| `students.txt` | registered student | `student ID` |
| `women_cards.txt` | verified women's card | `card number` |
| `cards.txt` | demo smart card | `name \| regular/student/senior/disabled \| student ID (0 if none) \| balance` |

Blank lines and lines starting with `#` are ignored. A mistake is reported with the file name and
line number, for example: `data/connections.txt line 7: station 'Rajiv Chowkk' is not in data/stations.txt`.

- **First run:** `metro_data.txt` does not exist, so the `data/` files are read and everything is
  saved to `metro_data.txt`.
- **Later runs:** only `metro_data.txt` is read. The `data/` files are not changed.
- **To start again from `data/`:** delete `metro_data.txt` and run the program.
- If the `data/` folder is missing, a small built-in network (3 lines, 14 stations) is used.

### The network in `data/`

An example network made for the project (the distances are not official figures):

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

6 lines, 29 stations, 39 tracks. Train capacity is 6 per line so that the waiting list is easy to
demonstrate; change it under Admin, "Set train capacity".

## Demo script (about 10 minutes)

1. **Route finder:** Passenger, Plan a journey, `ISBT` to `City Park`. Shortest route (Dijkstra):
   2.4 km on the Red Line. Fewest stops (BFS): 2 stops, 4.8 km, one line change.
2. **Station closure and undo:** Admin, close `Hospital`; plan ISBT to City Park again (the route
   now goes through Rajiv Chowk). Undo: Hospital opens again.
3. **DFS safety check:** try to close `Tech Park`: the program warns that Graphic Era would be cut
   off. Answer `n`.
4. **Priority waiting list:** set the Blue Line capacity to 1 and book `Airport` to `City Mall`.
   Book three more: a Regular passenger, a woman with card `9001`, and a Senior. Admin, Train and
   waiting-list status shows the order Senior, woman, Regular. Cancel the first ticket: the
   Senior gets a ticket automatically.
5. **Ticket:** book `Airport` to `Graphic Era` as a Student (ID `101`) with women's card `9002`:
   the ticket shows the route, the line change at Tech Park, and the fare (Rs. 40 base, 50%
   student discount, Rs. 20 paid).
6. **Smart card:** tap in card `5001` at Airport, tap out at Graphic Era: Rs. 40 for 12.5 km.
   Check the journey history.
7. **Pass and overstay:** buy a day pass, tap in, advance the clock by 200 minutes, tap out:
   fare Rs. 0 because of the pass, plus the Rs. 50 overstay penalty.
8. **Ticket expiry:** book a ticket, advance the clock by 130 minutes, try to board: the ticket has
   expired and its seat is freed.
9. **Line view and binary search:** View a line (Blue, then Ring, which loops). Type `raj` as a
   station name: "Did you mean: Rajiv Chowk?"
10. **AVL, BST and quick sort:** view tickets by ID (AVL), by fare (quick sort) and in an ID range.
    Reports show the BST and AVL heights side by side.
11. **Reports and simulation:** revenue, busiest stations, popular routes; then run the simulation
    and compare the average wait with a train every 3 minutes.
12. **Persistence:** exit, start again, and show that all the data is still there.

## Testing

Two test programs. `-DMETRO_NO_MAIN` leaves out the program's own `main()`, because each test
program has its own.

Windows PowerShell:

```powershell
$src = (Get-ChildItem journey,tickets,cards,reports -Recurse -Filter *.cpp).FullName
g++ -std=c++17 -I. -DMETRO_NO_MAIN tests/test_ds.cpp $src -o test_ds
.\test_ds.exe
g++ -std=c++17 -I. -DMETRO_NO_MAIN tests/test_metro.cpp $src -o test_metro
.\test_metro.exe
```

Linux / macOS:

```bash
src=$(find journey tickets cards reports -name "*.cpp")
g++ -std=c++17 -I. -DMETRO_NO_MAIN tests/test_ds.cpp $src -o test_ds && ./test_ds
g++ -std=c++17 -I. -DMETRO_NO_MAIN tests/test_metro.cpp $src -o test_metro && ./test_metro
```

- **`test_ds.cpp`** (12,370 checks): every data structure, including the heap's tie-break, 10,000
  hash-table entries with resizing, sort stability, Dijkstra versus BFS, recursive and iterative
  DFS, a 5,000-deep BST chain, all four AVL rotations, circular-queue wrap-around and binary search.
- **`test_metro.cpp`** (1,299 checks): fares, routing, the waiting list and promotion, expiry,
  cards, passes, penalties, lost cards, undo, login lockout, save and load, a damaged data file,
  reports, the simulation, BST and AVL heights with 1,000 tickets, and the `data/` files
  (including a deliberate mistake that must be reported with its file name and line number).

All tests pass, also under AddressSanitizer and UndefinedBehaviorSanitizer (no memory leaks).

## Team responsibilities

The data structures are divided equally: every member has two advanced topics (one graph
algorithm and one more) plus medium and basic structures, about 560 to 650 lines each. Data
structures are shared through `metro.h`, so one member's structure is used by the others'
features. Every member also owns the features in their `classes/` folder.

| Member | Advanced topics | Other data structures | Features (`classes/`) |
|--------|-----------------|-----------------------|-----------------------|
| **Member 1** (`journey/`) | Dijkstra, heap | Binary search, deque, doubly linked list | Fare classes (polymorphism), reading the `data/` files, stations, lines, routes, station closure check |
| **Member 2** (`tickets/`) | BFS, AVL tree | BST, queue | Booking, cancelling, boarding, ticket expiry, priority waiting list, students, women's cards, trains |
| **Member 3** (`cards/`) | DFS, hash table | Stack, singly linked list | Smart cards, tap in and out, passes, lost cards, admin login, undo, activity log, saving and loading files, clock |
| **Member 4** (`reports/`) | Graph (adjacency list), quick sort | Merge sort, circular queue | Reports, rush-hour simulation, screen and input, menus and ticket design, `main()`; tests |

## Time complexity

| Operation | Complexity |
|-----------|------------|
| Find card / ticket by number | O(1) average (hash table) |
| Shortest route | O((V + E) log V) (Dijkstra with heap) |
| Fewest-stops route | O(V + E) (BFS) |
| Closing-station check | O(V + E) (DFS) |
| Join / leave the waiting list | O(log n) / O(1) (lazy deletion) |
| Find station by name | O(log n) (binary search) |
| Ticket index insert / range (AVL) | O(log n) / O(log n + k) |
| Ticket index insert (plain BST) | O(n) here, because ticket IDs only increase |
| Tickets by fare | O(n log n) average (quick sort) |
| Reports | O(T + k log k) (T = trips, k = stations or routes; merge sort) |
| Undo | O(1) (stack) |
| Activity log add / drop oldest | O(1) (deque) |
| Platform enqueue / dequeue | O(1) (circular queue) |

## Assumptions and limitations

- **Fares and network:** example values made for the project, not official metro figures. Card
  fares always use the shortest distance.
- **Seats:** a paper ticket reserves a seat on the first train of its route only.
- **Timetable:** computed from the service hours and each line's frequency; not live data.
- **Simulation:** arrivals follow a triangular distribution peaking mid-rush; the same seed gives
  the same result.
- **Password:** stored as a salted djb2 hash: not plain text, but not cryptographically secure.
- **Undo:** lasts for the current session only.
- **Women's cards:** verification by staff is simulated with a list of verified card numbers.
- **Password:** typed in plain view (the program uses only standard input and output).
- **One program at a time** should use a data file.

## Contributors

Developed by a team of four at Graphic Era as a PBL project.

| Member | Name | Role |
|--------|------|------|
| Member 1 | _(name)_ | Journey and network (Team Lead) |
| Member 2 | _(name)_ | Tickets and waiting list |
| Member 3 | _(name)_ | Smart cards and storage |
| Member 4 | _(name)_ | Reports and interface |
