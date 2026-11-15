# Data Structures in the Metro Smart Card Module

**Project:** Metro Transit System (PBL)
**Module:** Smart Cards (`cards/ds/`)
**Language:** C++ (data structures written by hand in C style, without the STL)

---

## 1. Overview

The `cards/ds/` folder holds four hand-built data structures. Each one was chosen for a specific smart card feature:

| File | Data structure | Used for |
|---|---|---|
| `hash_table.cpp` | Hash table (separate chaining) | Fast lookup of cards, tickets, waitlist entries, student and women-card IDs |
| `stack.cpp` | Dynamic array stack (LIFO) | Undo of admin actions |
| `linked_list.cpp` | Singly linked list (head and tail pointers) | Trip history, newest first |
| `dfs.cpp` | Depth-first search (recursive and with an explicit stack) | Finding stations cut off from the network when a station is closed |

All of them are **generic**: they store raw bytes and take an `itemSize` when created, so the same code stores `Card`, `Ticket`, `Trip`, `AdminAction` and so on.

---

## 2. Which data structure is used for which feature

| Feature | Data structure | Where in the project | Why this structure |
|---|---|---|---|
| Find a card by ID on every recharge, tap in, tap out and pass purchase | Hash table (`cards_`) | `cardPtr()`, `findCard()` in `smart_cards.cpp` | Average O(1) lookup by card ID. A list or array would need O(n) scanning on each gate tap |
| Look up tickets and waitlist entries by ID | Hash table (`tickets_`, `waitEntries_`) | ticket booking and waitlist code | Same reason: direct access by integer key |
| Check if a student ID or women-card ID is registered and active | Hash table (`students_`, `womenCards_`) | `isValidStudent()`, `isValidWomenCard()` | Fast membership test; the value is an active/inactive flag |
| Undo the last admin action | Stack (`undo_`) | `pushUndo()`, `undoLast()`, `undoHistory()` in `admin_undo.cpp` | Undo must reverse the most recent action first, which is exactly LIFO |
| Show a card's trip history, latest trip first | Linked list (`trips_`) | `tapOut()` adds to front; `cardHistory()` walks from head | Inserting at the front is O(1) and the list is already in newest-first order. No shifting of elements |
| Detect stations that become unreachable when a station is closed | DFS (`dfsGroups`) | `Metro::cutOffStations()` in `journey/classes/metro_network.cpp` | DFS visits every connected station and labels connected groups |
| Save and load data | Hash table iterator | `collectValues<T>()` used in `storage.cpp` | `tableIterStart` / `tableIterNext` walks all entries; results are then sorted by ID |

---

## 3. Structure by structure

### 3.1 Hash table (`hash_table.cpp`)

**Design**
- Array of buckets; each bucket is a linked list of entries (**separate chaining**).
- Hash function: `key % bucketCount` (the key is an integer ID).
- Starts with 16 buckets. When `(size + 1) * 4 > bucketCount * 3`, i.e. load factor above 0.75, the table **doubles** and every entry is rehashed.
- Values are copied into memory owned by the table (`memcpy` of `itemSize` bytes).

**Operations**

| Function | Purpose | Average time |
|---|---|---|
| `tablePut` | Insert, or overwrite if the key exists | O(1) |
| `tableGet` | Return a pointer to the stored value, or `nullptr` | O(1) |
| `tableHas` | Membership test | O(1) |
| `tableRemove` | Unlink and free one entry | O(1) |
| `tableIterStart` / `tableIterNext` | Walk all entries bucket by bucket | O(n + buckets) |
| `tableClear` / `freeTable` | Free all entries (and the table) | O(n) |

**Why it fits cards:** card IDs start at 5001 and increase by 1, so keys spread evenly across buckets. Every gate action begins with a lookup by card ID.

**Design point:** `tableGet` returns a pointer to the stored value, so the caller can update a card's balance in place without copying the whole record. The `Metro` class keeps this pointer private (`cardPtr()`) and exposes only a `const` version publicly.

### 3.2 Stack (`stack.cpp`)

**Design**
- Contiguous byte array that grows with `realloc`: capacity starts at 8 and doubles when full.
- `push` and `pop` work at the top using `memcpy`.

**Operations**

| Function | Purpose | Time |
|---|---|---|
| `stackPush` | Add to the top | O(1) amortised |
| `stackPop` | Remove and return the top | O(1) |
| `stackPeek` | Read the top without removing | O(1) |
| `stackFromTop(index)` | Read the n-th item from the top | O(1) |
| `stackSize`, `stackIsEmpty`, `stackClear` | State and reset | O(1) |

**Why it fits undo:** each admin action (add station, close station, register student, set capacity, and so on) is pushed as an `AdminAction`. `undoLast()` pops the latest one and reverses it. If the reversal is not possible, the action is pushed back, so history is not lost. `stackFromTop` lets the admin screen list the undo history without modifying the stack.

The same stack type is also used inside DFS (Section 3.4).

### 3.3 Linked list (`linked_list.cpp`)

**Design**
- Singly linked list with **head and tail pointers**, so adding at either end is O(1).
- Each node owns a copy of its data.

**Operations**

| Function | Purpose | Time |
|---|---|---|
| `listAddFront` | Insert at head | O(1) |
| `listAddBack` | Insert at tail | O(1) |
| `listRemoveFront` | Remove head | O(1) |
| `listTrim(maxSize)` | Keep the first `maxSize` items, free the rest | O(n) |
| `listClear` / `freeList` | Free all nodes | O(n) |

**Why it fits trip history:** `tapOut()` calls `listAddFront`, so the newest trip is always first. `cardHistory(cardId, maxTrips)` walks from the head and stops after `maxTrips` matching trips, which gives "last N trips" without sorting. When data is loaded from file, trips are saved newest-first and re-added with `listAddBack`, which keeps the same order.

**Trade-off:** `cardHistory` scans all trips and filters by card ID, so it is O(n) in total trips. A per-card list (hash table of lists) would make this faster at the cost of more memory and code.

### 3.4 Depth-first search (`dfs.cpp`)

Station connections form a graph. This file contains three DFS routines:

| Function | Method | Purpose |
|---|---|---|
| `dfsCountRecursive` | Recursion | Count stations reachable from a start station |
| `dfsCountLoop` | Explicit stack (`Stack`) | Same result without recursion, so no stack overflow on a very large network |
| `dfsGroups` | Loop over all stations + explicit stack | Label every station with a connected-group number and return the number of groups |

Closed stations are skipped (`canVisit` checks `graphIsOpen`), and one extra station can be treated as blocked.

**Why it fits the project:** `Metro::cutOffStations(id)` runs `dfsGroups` twice, once normally and once with station `id` blocked. If the number of groups increases, the stations that left the main group are the ones that would be cut off. This warns the admin before closing a station that would isolate part of the network.

**Complexity:** O(V + E) time, O(V) extra space.

**Recursive vs iterative:** the project keeps both versions. The iterative one is the one used in the real feature (`dfsGroups`); the recursive version is used in tests to check that both give the same count.

---

## 4. How these structures work together (tap-out example)

1. `tableGet(cards_, cardId)` finds the card. **Hash table**
2. Dijkstra computes the distance from entry to exit station on the graph (this is in the `journey` module).
3. The fare is calculated and the balance is reduced in place.
4. `listAddFront(trips_, &trip)` records the trip. **Linked list**
5. `log(...)` records the activity in a deque (activity log).

For an admin action such as closing a station:
1. `cutOffStations()` uses **DFS** to warn about isolated stations.
2. `pushUndo()` stores the action on the **stack**.
3. `undoLast()` can reverse it later.

---

## 5. Testing

The file `tests/test_ds.cpp` in the repository exercises these structures, including the DFS results (for example, on the test graph `dfsGroups` returns 1 group normally and 3 groups when station 1 is blocked) and `listTrim`.

Note: `dfsCountRecursive`, `dfsCountLoop` and `listTrim` are currently exercised by tests only; the main program uses `dfsGroups`.

---

## 6. Limitations and possible improvements

| Limitation | Possible improvement |
|---|---|
| Structures store `void*` / raw bytes with `memcpy`, so there is no type safety. This is only safe for plain-data structs without pointers | Convert to class templates (`HashMap<T>`, `Stack<T>`, `List<T>`) |
| Hash function is `key % buckets`; sequential IDs spread well, but patterned IDs could cluster | Use a mixing hash (multiply-shift) |
| `tablePut` ignores a failed `resize`, so the table keeps working but with a higher load factor | Propagate the error or retry |
| `cardHistory` scans the full trip list | Keep a trip list per card |
| `stackPop` and `stackPush` report failure only via `bool`; callers must check it | Use exceptions or a result type |
| `dfsGroups` allocates with `calloc` and manual `free` | Use `Array<bool>` (RAII) to avoid leaks on early return |

---

## 7. Summary

| Structure | Feature | Key property used |
|---|---|---|
| Hash table | Card, ticket, student lookup | O(1) average access by ID |
| Stack | Admin undo | LIFO order |
| Linked list | Trip history | O(1) insert at front, newest first |
| DFS | Cut-off station warning | Connected-component labelling in O(V + E) |
