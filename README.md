# Metro Transit System (C + C++)

A terminal-based metro management system built as a Project-Based Learning (PBL) project.
Passengers can plan journeys, book tickets, and use smart cards. Admins manage the network,
trains, and reports, and can run a rush-hour simulation.

The project uses **two languages on purpose**:

- **C** implements every data structure and algorithm (the `*_ds.c` file in each member folder).
- **C++** implements the application: classes, menus, rules, and file storage (the `.cpp` file in each member folder).

## Architecture

```text
Metro-Transit-System-PBL/
├── metro.h                     shared header: every struct, class and function declaration
├── journey/                    Member 1: journey and network
│   ├── journey_ds.c            C: graph (Dijkstra, BFS), DFS, binary search, doubly linked list
│   └── journey.cpp             C++: route planning, fare classes, stations and lines, starting data
├── tickets/                    Member 2: tickets and waiting list
│   ├── tickets_ds.c            C: heap, hash table, BST, AVL tree
│   └── tickets.cpp             C++: booking, cancelling, priority waiting list, students, women's cards, trains
├── cards/                      Member 3: smart cards and storage
│   ├── cards_ds.c              C: singly linked list, queue, stack, deque
│   └── cards.cpp               C++: smart cards, admin login, undo, activity log, saving and loading
├── reports/                    Member 4: reports and interface
│   ├── reports_ds.c            C: merge sort, quick sort, circular queue
│   └── reports.cpp             C++: reports, simulation, menus and screen, Array<T>, main()
├── data/                       starting data you can edit (see "Starting data")
└── tests/
    ├── test_ds.c               tests for every C data structure
    └── test_metro.cpp          tests for the C++ rules
```

### How C and C++ are joined

All declarations are in one shared header, `metro.h`. Its C part is wrapped like this, so the C++ compiler links to the C functions correctly:

```c
#ifdef __cplusplus
extern "C" {
#endif
/* declarations */
#ifdef __cplusplus
}
#endif
```

`.c` files are compiled with `gcc` and `.cpp` files with `g++`, then everything is linked with `g++`.
Do not compile `.c` files with `g++`, because it would treat them as C++.

The C containers store **copies** of fixed-size records, which they make with `memcpy`. So every record in
The record structs in `metro.h` (such as `Ticket` and `Card`) use only numbers and `char` arrays (no `std::string`), because a byte-by-byte copy
cannot follow the pointer inside a `std::string`.

The C++ part uses **no STL containers**. Where it needs a list (for example, the tickets to show
in a table), it uses `Array<T>` from `metro.h`: a class template that manages its own memory and
has a constructor, a copy constructor, an assignment operator (`=`) and a destructor, so that
each copy owns its own memory.

`Metro` contains the rules and never prints anything; `App` prints. This split is what makes the
logic testable.

## Features

| # | Feature | Data structure / concept | Where |
|---|---------|--------------------------|-------|
| 1 | Route finder (shortest distance) | Graph + Dijkstra with min-heap | `journey/journey_ds.c`, `tickets/tickets_ds.c` |
| 2 | Line-change guide, fewest-stops route | Graph + BFS with queue | `journey/journey_ds.c`, `cards/cards_ds.c` |
| 3 | Smart cards: issue, balance, recharge | Hash table | `tickets/tickets_ds.c`, `cards/cards.cpp` |
| 4 | Tap in / tap out, fare by real distance | Hash table + Dijkstra | `Metro::tapOut` |
| 5 | Distance-based fare slabs | Polymorphism | `journey/journey.cpp` |
| 6 | Day and monthly passes | Polymorphism (discounted price) | `Metro::buyPass` |
| 7 | Ticket validity (2 hours) and auto-expiry | Hash table iteration | `Metro::expireOldTickets` |
| 8 | Journey history (last 10 trips) | Linked list | `cards/cards_ds.c` |
| 9 | Block lost card, move balance to a new card | Hash table | `Metro::blockAndTransfer` |
| 10 | Priority waiting list: senior/disabled first, then women (verified card), then students, then regular; first come, first served within a level | Heap with two keys | `tickets/tickets_ds.c`, `Metro::waitPriority` |
| 11 | Several lines and trains, next-train timetable | Per-line state | `Metro::nextTrain` |
| 12 | Crowd level per train | Occupancy % | `Metro::crowdLevel` |
| 13 | Undo admin actions | Stack | `cards/cards_ds.c`, `Metro::undoLast` |
| 14 | Reports: revenue, busiest stations, popular routes | Hash-table counting + merge sort | `Metro::buildReport` |
| 15 | Close a station; routes avoid it | Graph vertex flag | `journey/journey_ds.c` |
| 16 | Hidden password input, lockout after 3 wrong tries | Hashing, terminal control | `reports/reports.cpp`, `Metro::adminLogin` |
| 17 | Save and load all data | File handling | `cards/cards.cpp` |
| 18 | Rush-hour simulation with a fixed-size platform and a gate queue | Circular queue + FIFO queue | `reports/reports.cpp`, `reports/reports_ds.c` |
| 19 | Colours, tables, clear screen | ANSI codes | `reports/reports.cpp` |
| 20 | Automatic tests | Unit tests | `tests/` |
| 21 | Safety check before closing a station: which stations would be cut off | DFS (recursive and iterative) | `journey/journey_ds.c`, `Metro::stationsCutOffIfClosed` |
| 22 | All tickets in ID order; tickets in an ID range; BST and AVL heights compared in Reports | AVL tree (rotations), in-order traversal and range search; plain BST for comparison | `tickets/tickets_ds.c` |
| 23 | Find a station by name (any case); "Did you mean ...?" hints | Binary search on a sorted index | `journey/journey_ds.c` |
| 24 | Line view in both directions; "towards ..." in every route | Doubly linked list | `journey/journey_ds.c`, `Metro::buildLineList` |
| 25 | Tickets sorted by fare | Quick sort (median of three) | `reports/reports_ds.c` |
| 26 | Recent activity log that keeps the newest 200 entries | Deque | `cards/cards_ds.c` |
| 27 | Verified women's cards: register, deactivate, undo; priority on the waiting list | Hash table + heap | `Metro::registerWomenCard`, `Metro::waitPriority` |
| 28 | Starting data (stations, tracks, students, women's cards, cards) read from editable text files | File handling (`ifstream`) | `journey/journey.cpp`, `data/` |
| 29 | Bigger network: 6 lines, 29 stations, a ring (loop) line | Graph; loop detection | `data/connections.txt`, `Metro::isLoopLine` |
| 30 | Ticket design: box, route with line changes, fare breakdown; the right border stays straight with emoji | UTF-8 display width | `App::printTicket`, `ui::displayWidth` |

## Build and run

You need `gcc` and `g++` (C++17). On Windows, use MinGW-w64.

Run these two commands from the project folder. They work the same on Windows, Linux and macOS.

```bash
gcc -std=c99 -I. -c journey/journey_ds.c tickets/tickets_ds.c cards/cards_ds.c reports/reports_ds.c
g++ -std=c++17 -I. journey/journey.cpp tickets/tickets.cpp cards/cards.cpp reports/reports.cpp *.o -o metro
```

Step 1 compiles the four C files with gcc and creates one `.o` file for each. Step 2 compiles the C++ files and joins them with those
`.o` files into one program. Run the build again whenever you change the code.

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

There are two test programs. Build the app first (the C++ tests reuse the `.o` files), then run the commands below. `-DMETRO_NO_MAIN` leaves out the program's own `main()`, because the test program has its own.

```bash
gcc -std=c99 -I. tests/test_ds.c journey/journey_ds.c tickets/tickets_ds.c cards/cards_ds.c reports/reports_ds.c -o test_ds
./test_ds

g++ -std=c++17 -I. -DMETRO_NO_MAIN tests/test_metro.cpp journey/journey.cpp tickets/tickets.cpp cards/cards.cpp reports/reports.cpp *.o -o test_metro
./test_metro
```

On Windows, run them as `.\test_ds.exe` and `.\test_metro.exe`.

- **`test_ds.c`:** about 12,400 checks covering every C structure. These include FIFO/LIFO
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

Every member worked on both parts of the project: data structures in C and the application in C++.
Each member has a folder with one C file and one C++ file, and each member's data structures are
used by that member's own features. The shared header `metro.h` is divided into one section per member.

| Member | C file (data structures) | C++ file (features) |
|--------|--------------------------|---------------------|
| **Member 1: Journey and network (Team Lead)** | `journey/journey_ds.c`: graph with Dijkstra and BFS, DFS, binary search, doubly linked list | `journey/journey.cpp`: route planning, fare classes (inheritance and polymorphism), stations, lines and tracks, station closure check, line view, reading the `data/` files |
| **Member 2: Tickets and waiting list** | `tickets/tickets_ds.c`: heap (priority queue), hash table, BST, AVL tree | `tickets/tickets.cpp`: booking, cancelling, boarding, ticket expiry, priority waiting list, students, women's cards, trains |
| **Member 3: Smart cards and storage** | `cards/cards_ds.c`: singly linked list, queue, stack, deque | `cards/cards.cpp`: smart cards, tap in and out, passes, lost cards, admin login, undo, activity log, saving and loading files |
| **Member 4: Reports and interface** | `reports/reports_ds.c`: merge sort, quick sort, circular queue | `reports/reports.cpp`: reports, rush-hour simulation, menus and screen, ticket design, `Array<T>` template, `main()`; plus the tests |

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

Developed by a team of four as part of the PBL programme at Graphic Era.
