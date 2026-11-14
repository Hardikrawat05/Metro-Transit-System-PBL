# Metro Transit System (C++)

A terminal-based metro management system built as a Project-Based Learning (PBL) project.
Passengers can plan journeys, book tickets, and use smart cards. Admins manage the network,
trains, and reports, and can run a rush-hour simulation.

The whole project is written in **C++**, with **no STL containers** (no `std::vector`):

- Every data structure and algorithm is written by us; no STL containers are used.
- Each team member has **one `.cpp` file** with two parts: first the data structures, then the
  features (classes, rules and menus) that use them.

## Architecture

```text
Metro-Transit-System-PBL/
├── metro.h                         shared header: every struct, class and function declaration
├── journey/                        Member 1: journey and network
│   ├── ds/       graph_dijkstra_bfs.cpp, dfs.cpp, binary_search.cpp, doubly_linked_list.cpp
│   └── classes/  fare_policy.cpp, seed_loader.cpp, metro_network.cpp
├── tickets/                        Member 2: tickets and waiting list
│   ├── ds/       heap_priority_queue.cpp, hash_table.cpp, bst.cpp, avl_tree.cpp
│   └── classes/  ticket_booking.cpp, students_women_cards.cpp, trains.cpp
├── cards/                          Member 3: smart cards and storage
│   ├── ds/       linked_list.cpp, queue.cpp, stack.cpp, deque.cpp
│   └── classes/  smart_cards.cpp, admin_undo.cpp, storage.cpp, clock.cpp
├── reports/                        Member 4: reports and interface
│   ├── ds/       merge_sort.cpp, quick_sort.cpp, circular_queue.cpp
│   └── classes/  reports.cpp, simulation.cpp, ui.cpp, app_menus.cpp, main.cpp
├── data/                           starting data you can edit (see "Starting data")
└── tests/
    ├── test_ds.cpp                 tests for every data structure
    └── test_metro.cpp              tests for the application rules
```

In every member folder, `ds/` has one file per data structure and `classes/` has one file per
feature that uses them.

### One shared header

All declarations are in one shared header, `metro.h`: the data structures, the record structs (such as
`Ticket` and `Card`) and the classes. Every `.cpp` file includes it with `#include "metro.h"`, and the
whole program is compiled with one `g++` command.

Our data structures store **copies** of fixed-size records, which they make with `memcpy`. So every
record struct in `metro.h` (such as `Ticket` and `Card`) uses only numbers and `char` arrays (no
`std::string`), because a byte-by-byte copy cannot follow the pointer inside a `std::string`.

The project uses **no STL containers**. Where it needs a list (for example, the tickets to show
in a table), it uses `Array<T>` from `metro.h`: a class template that manages its own memory and
has a constructor, a copy constructor, an assignment operator (`=`) and a destructor, so that
each copy owns its own memory.

`Metro` contains the rules and never prints anything; `App` prints. This split is what makes the
logic testable.

## Features

| # | Feature | Data structure / concept | Where |
|---|---------|--------------------------|-------|
| 1 | Route finder (shortest distance) | Graph + Dijkstra with min-heap | `journey/`, `tickets/` |
| 2 | Line-change guide, fewest-stops route | Graph + BFS with queue | `journey/`, `cards/` |
| 3 | Smart cards: issue, balance, recharge | Hash table | `tickets/`, `cards/` |
| 4 | Tap in / tap out, fare by real distance | Hash table + Dijkstra | `Metro::tapOut` |
| 5 | Distance-based fare slabs | Polymorphism | `journey/` |
| 6 | Day and monthly passes | Polymorphism (discounted price) | `Metro::buyPass` |
| 7 | Ticket validity (2 hours) and auto-expiry | Hash table iteration | `Metro::expireOldTickets` |
| 8 | Journey history (last 10 trips) | Linked list | `cards/` |
| 9 | Block lost card, move balance to a new card | Hash table | `Metro::blockAndTransfer` |
| 10 | Priority waiting list: senior/disabled first, then women (verified card), then students, then regular; first come, first served within a level | Heap with two keys | `tickets/`, `Metro::waitPriority` |
| 11 | Several lines and trains, next-train timetable | Per-line state | `Metro::nextTrain` |
| 12 | Crowd level per train | Occupancy % | `Metro::crowdLevel` |
| 13 | Undo admin actions | Stack | `cards/`, `Metro::undoLast` |
| 14 | Reports: revenue, busiest stations, popular routes | Hash-table counting + merge sort | `Metro::buildReport` |
| 15 | Close a station; routes avoid it | Graph vertex flag | `journey/` |
| 16 | Hidden password input, lockout after 3 wrong tries | Hashing, terminal control | `reports/`, `Metro::adminLogin` |
| 17 | Save and load all data | File handling | `cards/` |
| 18 | Rush-hour simulation with a fixed-size platform and a gate queue | Circular queue + FIFO queue | `reports/` |
| 19 | Colours, tables, clear screen | ANSI codes | `reports/` |
| 20 | Automatic tests | Unit tests | `tests/` |
| 21 | Safety check before closing a station: which stations would be cut off | DFS (recursive and iterative) | `journey/`, `Metro::cutOffStations` |
| 22 | All tickets in ID order; tickets in an ID range; BST and AVL heights compared in Reports | AVL tree (rotations), in-order traversal and range search; plain BST for comparison | `tickets/` |
| 23 | Find a station by name (any case); "Did you mean ...?" hints | Binary search on a sorted index | `journey/` |
| 24 | Line view in both directions; "towards ..." in every route | Doubly linked list | `journey/`, `Metro::makeLineList` |
| 25 | Tickets sorted by fare | Quick sort (median of three) | `reports/` |
| 26 | Recent activity log that keeps the newest 200 entries | Deque | `cards/` |
| 27 | Verified women's cards: register, deactivate, undo; priority on the waiting list | Hash table + heap | `Metro::registerWomenCard`, `Metro::waitPriority` |
| 28 | Starting data (stations, tracks, students, women's cards, cards) read from editable text files | File handling (`ifstream`) | `journey/`, `data/` |
| 29 | Bigger network: 6 lines, 29 stations, a ring (loop) line | Graph; loop detection | `data/connections.txt`, `Metro::isLoopLine` |
| 30 | Ticket design: box, route with line changes, fare breakdown; the right border stays straight with emoji | UTF-8 display width | `App::printTicket`, `ui::displayWidth` |

## Build and run

You need `g++` (C++17). On Windows, use MinGW-w64.

Run these two commands from the project folder. They work the same on Windows, Linux and macOS.

Windows PowerShell:

```powershell
g++ -std=c++17 -I. (Get-ChildItem journey,tickets,cards,reports -Recurse -Filter *.cpp).FullName -o metro
```

Linux / macOS:

```bash
g++ -std=c++17 -I. $(find journey tickets cards reports -name "*.cpp") -o metro
```

If the short command does not work, list every file (works everywhere):

```bash
g++ -std=c++17 -I. cards/classes/admin_undo.cpp cards/classes/clock.cpp cards/classes/smart_cards.cpp cards/classes/storage.cpp cards/ds/deque.cpp cards/ds/linked_list.cpp cards/ds/queue.cpp cards/ds/stack.cpp journey/classes/fare_policy.cpp journey/classes/metro_network.cpp journey/classes/seed_loader.cpp journey/ds/binary_search.cpp journey/ds/dfs.cpp journey/ds/doubly_linked_list.cpp journey/ds/graph_dijkstra_bfs.cpp reports/classes/app_menus.cpp reports/classes/main.cpp reports/classes/reports.cpp reports/classes/simulation.cpp reports/classes/ui.cpp reports/ds/circular_queue.cpp reports/ds/merge_sort.cpp reports/ds/quick_sort.cpp tickets/classes/students_women_cards.cpp tickets/classes/ticket_booking.cpp tickets/classes/trains.cpp tickets/ds/avl_tree.cpp tickets/ds/bst.cpp tickets/ds/hash_table.cpp tickets/ds/heap_priority_queue.cpp -o metro
```

This finds every `.cpp` file in the four member folders, compiles them and links them into one program. Run it again whenever you change the code.
Start the program:

```bash
./metro          # Linux / macOS
.\metro.exe      # Windows PowerShell (in Command Prompt: metro.exe)
```

Options:

- `--no-color`: no colours.
- `--ascii`: plain `+--+` borders and no emoji, for old terminals. (This also happens
  automatically when the terminal cannot show them.)
- `--data FILE`: use another saved-data file instead of `metro_data.txt`.
- `--seed FOLDER`: read the starting data from another folder instead of `data/`.

Run the program from the project folder, so that it finds the `data/` folder.

- **Admin password:** `1234`. Change it from the admin panel.
- **Demo smart cards:** `5001` is a Regular card with Rs. 500. `5002` is a Student card with Rs. 300.
- **Registered student IDs:** 101, 102, 105, 110, 115.
- **Verified women's cards:** 9001, 9002, 9003.

## Starting data

The `data/` folder holds the data the system starts with. These are plain text files: open them
in Notepad or VS Code and add stations, tracks, student IDs or women's cards.

| File | One line per | Format |
|------|--------------|--------|
| `lines.txt` | metro line | `name \| train capacity \| a train every N minutes` |
| `stations.txt` | station | `name` |
| `connections.txt` | track between two stations | `station \| station \| distance in metres \| line` |
| `students.txt` | registered student | `student ID` |
| `women_cards.txt` | verified women's card | `card number` |
| `cards.txt` | demo smart card | `name \| regular/student/senior/disabled \| student ID (0 if none) \| balance` |

Blank lines and lines starting with `#` are ignored. If a file has a mistake, the program says
which file and line, e.g. `data/connections.txt line 7: station 'Rajiv Chowkk' is not in data/stations.txt`.

**How the two kinds of file work together**

1. **First run:** `metro_data.txt` does not exist, so the program reads the `data/` files,
   builds the system, and saves everything to `metro_data.txt`.
2. **Every run after that:** only `metro_data.txt` is read. Everything added while the program
   runs (new students, stations, tickets, balances, journeys) is saved there automatically.
   The `data/` files are not changed.
3. **To start again from the `data/` files** (for example after editing them): delete
   `metro_data.txt` and run the program.

If the `data/` folder is missing, a small built-in network (3 lines, 14 stations) is used instead.

### The network in `data/`

This is an example network made for the project; the distances are not official figures.

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

6 lines, 29 stations, 39 tracks. Train capacity is 6 per line, so the waiting list is easy to
demonstrate; change it under Admin, then "Set train capacity".

### Waiting-list priority

When a seat becomes free, the waiting passenger with the best priority level gets it:

1. Senior citizen or person with disability
2. Woman with a verified women's card
3. Student
4. Regular

Within a level it is first come, first served. A passenger in two groups gets the better level
(a woman who is a senior is level 1). The women's card only gives priority; the fare discount
still comes from the passenger type, so a woman student pays the student fare.

## Demo script (about 10 minutes)

1. **Route finder.** Go to Passenger, then Plan a journey, from `ISBT` to `City Park`. The shortest
   route (Dijkstra) is 2.4 km on the Red Line. The fewest-stops route (BFS) is 2 stops but 4.8 km,
   with a line change. Every station shows its distance from the source, and a table compares
   the two routes.
2. **Station closure.** Go to Admin, then Close station `Hospital`. Plan ISBT to City Park again:
   the route now goes through Rajiv Chowk.
3. **Undo.** In Admin, use Undo. Hospital opens again.
4. **Priority waiting list.** Set the Blue Line capacity to 1. Book `Airport` to `City Mall` (Regular).
   Then book three more: a Regular passenger, a woman with women's card `9001`, and a Senior.
   All three go to the waiting list in the order Senior, woman, Regular (see Admin, then Train and
   waiting-list status). Cancel the first ticket: the Senior gets a ticket automatically.
5. **Smart card.** Tap in card `5001` at Airport and tap out at Graphic Era. The fare is Rs. 40
   for 12.5 km (via the Pink Line). Check its journey history.
6. **Pass and overstay.** Buy a day pass, tap in, use Admin to advance the clock by 200 minutes,
   then tap out. The fare is Rs. 0 because of the pass, but the Rs. 50 overstay penalty applies.
7. **Ticket expiry.** Book a ticket, advance the clock by 130 minutes, and try to board: the
   ticket has expired and its seat is freed.
8. **Reports.** Show revenue, busiest stations, and popular routes.
9. **Simulation.** Run it with the defaults, then again with a train every 3 minutes. Compare
   the average wait. Also note "Held at gate": the platform (a circular queue) holds only 200
   people by default.
10. **Persistence.** Exit, start again, and show that all the data is still there.
11. **DFS safety check.** In Admin, try to close `Tech Park`: the program warns that Graphic Era
    would be cut off. Answer `n`. Try `Rajiv Chowk`: the check passes, because the bigger network
    has other ways round it.
12. **Doubly linked list.** Passenger, then View a line: the Blue Line in both directions, and the
    Ring Line, which loops back to its first station. Every route also says which way to travel,
    e.g. "Blue Line (towards River Front)".
13. **Binary search.** Type `raj` as a station name: "Did you mean: Rajiv Chowk?"
14. **AVL, BST and quick sort.** Book a few tickets, then in Admin view tickets sorted by ID (AVL
    in-order traversal), sorted by fare (quick sort), and in an ID range (AVL range search). Reports
    show both heights: the plain BST's height equals the number of tickets (a chain), while the AVL
    tree stays balanced (1,000 tickets: BST height 1000, AVL height 10).
15. **Women's cards.** Admin, then Register women's card `9050`; view the registry; undo it.
16. **Ticket.** Book `Airport` to `Graphic Era` as a Student (ID `101`) with women's card `9002`:
    the ticket shows the route with the line change at Tech Park, the fare breakdown (Rs. 40
    base, 50% student discount, Rs. 20 paid) and the women's priority.
17. **Starting data.** Open `data/students.txt`, add a student ID, delete `metro_data.txt`, run
    the program again, and show that the new ID is registered.

## Testing

There are two test programs. Run the commands below. `-DMETRO_NO_MAIN` leaves out the program's own `main()` (in `reports/classes/main.cpp`), because each test program has its own.

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

- **`test_ds.cpp`:** about 12,400 checks covering every data structure. These include FIFO/LIFO
  order, the heap's priority tie-break, 10,000 hash-table entries with resizing, sort
  stability, Dijkstra versus BFS, closed stations, recursive and iterative DFS giving the same
  result, a 5,000-deep BST chain, all four AVL rotations and AVL balance after 10,000 inserts, circular-queue wrap-around, deque growth, doubly linked list
  removal at both ends, recursive and iterative binary search, and quick sort on random,
  sorted, reversed and duplicate-heavy data.
- **`test_metro.cpp`:** about 1,300 checks covering fares, routing, the timetable, the waiting
  list and promotion, dispatch and expiry, cards, passes, penalties, lost cards, undo,
  login lockout, a save/load round trip, reading a corrupt file, reports, simulation
  determinism, the DFS cut-off check, line directions, the BST index after reloading,
  the station index after undo, the platform limit, and the `Array<T>`
  template (separate copies, assigning to itself, growth, nested arrays), BST and AVL heights with 1,000 tickets, the four priority levels with women's cards,
  loading an older data file, and the `data/` starting-data files (including a deliberate mistake
  that must be reported with its file name and line number).

The tests also pass under AddressSanitizer and UndefinedBehaviorSanitizer (Linux / macOS),
with no memory leaks or invalid accesses. To check, add `-g -fsanitize=address,undefined`
to every command above.

## Team responsibilities

Every member worked on both parts of the project: data structures and application features.
Each member has a folder with `ds/` (one file per data structure) and `classes/` (one file per
feature). The shared header `metro.h` is divided into one section per member.

| Member | Data structures (`ds/`) | Features (`classes/`) |
|--------|-------------------------|-----------------------|
| **Member 1: Journey and network (Team Lead)** | `journey/ds/`: graph with Dijkstra and BFS, DFS, binary search, doubly linked list | `journey/classes/`: fare classes (inheritance and polymorphism), reading the `data/` files, stations, lines, routes, station closure check, line view |
| **Member 2: Tickets and waiting list** | `tickets/ds/`: heap (priority queue), hash table, BST, AVL tree | `tickets/classes/`: booking, cancelling, boarding, ticket expiry, priority waiting list, students, women's cards, trains |
| **Member 3: Smart cards and storage** | `cards/ds/`: singly linked list, queue, stack, deque | `cards/classes/`: smart cards, tap in and out, passes, lost cards, admin login, undo, activity log, saving and loading files, clock |
| **Member 4: Reports and interface** | `reports/ds/`: merge sort, quick sort, circular queue | `reports/classes/`: reports, rush-hour simulation, screen and input, menus and ticket design, `main()`; plus the tests |

## Time complexity

| Operation | Complexity |
|-----------|------------|
| Find card / ticket by ID | O(1) average (hash table) |
| Shortest route | O((V + E) log V) (Dijkstra with heap) |
| Fewest-stops route | O(V + E) (BFS) |
| Join / leave the waiting list | O(log n) / O(1) (lazy deletion) |
| Undo | O(1) (stack) |
| Reports | O(T + k log k), where T = trips and k = stations or routes |
| Closing-station check | O(V + E) (DFS, run twice) |
| Find station by name | O(log n) (binary search); index rebuilt in O(n log n) only after stations change |
| Ticket index insert / range (AVL) | O(log n) / O(log n + k) |
| Ticket index insert (plain BST, for comparison) | O(n) here, because IDs only increase and the BST becomes a chain |
| Tickets by fare | O(n log n) average (quick sort) |
| Activity log add / drop oldest | O(1) / O(1) (deque) |
| Platform enqueue / dequeue | O(1) (circular queue) |
| Line view | O(V + E) to build the list, O(n) to print each direction |

## Assumptions and limitations

- **Fares:** the slab values are for illustration and are not any real metro's official fares.
  Card fares always use the shortest distance, even if the passenger took a longer route.
- **Seats:** a paper ticket reserves a seat on the first train of its route. Changing to
  another line later does not reserve a seat on that line.
- **Timetable:** it is computed from the service hours and each line's frequency. It is not
  live data.
- **Simulation:** arrivals follow a triangular distribution that peaks in the middle of the
  rush period, and each passenger waits for a single line. The same seed gives the same
  result on the same compiler.
- **Password:** it is stored as a salted djb2 hash. This avoids storing plain text but is not
  cryptographically secure.
- **Undo:** the undo stack lasts only for the current session and is not saved to the file.
- **BST and AVL:** every ticket ID goes into both trees. The plain BST is kept only to show
  the degenerate case in Reports; listings and range searches use the AVL tree.
- **Women's cards:** in real life, staff verify the card holder when the card is issued. The
  project simulates this with a list of verified card numbers; it stores only the number.
- **Network:** the network in `data/` is an example made for the project, not a real map.
- **Emoji and box lines:** shown on modern terminals (Windows Terminal, VS Code, Linux, macOS).
  Old consoles get plain ASCII automatically, or use `--ascii`.
- **Line view:** a line is expected to be a single path (or a loop). If a line branches,
  only its main path is shown, with a warning.
- **Concurrency:** only one program instance should use a data file at a time.

## Contributors
Hardik Rawat
Aaryan Rawat
Alok Singh
Saloni Bisht
