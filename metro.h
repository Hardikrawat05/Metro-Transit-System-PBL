#ifndef METRO_H
#define METRO_H

#include <cstddef>

// ===== Member 4: Graph, quick sort, merge sort and circular queue =====

#define GRAPH_NAME_MAX 48

struct EdgeNode {
    int to;
    int meters;
    int line;
    struct EdgeNode *next;
};

struct Station {
    char name[GRAPH_NAME_MAX];
    bool open;
    EdgeNode *adj;
};

struct Line {
    char name[GRAPH_NAME_MAX];
};

struct Graph {
    Station *stations;
    int stationCount;
    int stationCapacity;
    Line *lines;
    int lineCount;
    int lineCapacity;
};

struct Route {
    int length;
    int *stations;
    int *lines;
    long long totalMeters;
};

Graph *newGraph(void);
void freeGraph(Graph *g);

int graphAddLine(Graph *g, const char *name);
int graphLineCount(const Graph *g);
const char *graphLineName(const Graph *g, int line);

int graphAddStation(Graph *g, const char *name);
bool graphRemoveLastStation(Graph *g);
int graphStationCount(const Graph *g);
bool graphHasStation(const Graph *g, int id);
const char *graphStationName(const Graph *g, int id);
bool graphIsOpen(const Graph *g, int id);
bool graphSetOpen(Graph *g, int id, bool open);
int graphFindStation(const Graph *g, const char *name);
int graphStationLines(const Graph *g, int id, int *out, int maxOut);

bool graphAddTrack(Graph *g, int a, int b, int meters, int line);
bool graphRemoveTrack(Graph *g, int a, int b, int line);
bool graphHasTrack(const Graph *g, int a, int b, int line);
const EdgeNode *graphTracks(const Graph *g, int id);

void initRoute(Route *r);
void freeRoute(Route *r);

struct SearchArrays {
    int *prev;
    int *prevLine;
    int *prevDist;
};

bool makeRoute(int src, int dst, const int *prev, const int *prevLine, const int *prevDist, Route *out);
bool allocSearch(SearchArrays *search, int n);
void freeSearch(SearchArrays *search);
bool canStart(const Graph *g, int src, int dst, bool allowClosed, Route *out);
void rememberStep(SearchArrays *search, int from, const EdgeNode *track);

bool quickSort(void *base, size_t count, size_t itemSize,
                int (*cmp)(const void *a, const void *b));

bool mergeSort(void *base, size_t count, size_t itemSize,
                int (*cmp)(const void *a, const void *b));

struct CircularQueue {
    int *items;
    size_t capacity;
    size_t front;
    size_t count;
};

CircularQueue *newCircularQueue(size_t capacity);
bool cqEnqueue(CircularQueue *q, int value);
bool cqDequeue(CircularQueue *q, int *out);
bool cqPeek(const CircularQueue *q, int *out);
bool cqIsFull(const CircularQueue *q);
bool cqIsEmpty(const CircularQueue *q);
size_t cqSize(const CircularQueue *q);
size_t cqCapacity(const CircularQueue *q);
void cqClear(CircularQueue *q);
void freeCircularQueue(CircularQueue *q);

// ===== Member 1: Dijkstra, heap, binary search, deque and doubly linked list =====

bool dijkstraRoute(const Graph *g, int src, int dst, bool allowClosed, Route *out);

struct HeapItem {
    long long key1;
    long long key2;
    int value;
};

struct Heap {
    HeapItem *items;
    size_t count;
    size_t capacity;
};

Heap *newHeap(void);
bool heapPush(Heap *h, HeapItem item);
bool heapPop(Heap *h, HeapItem *out);
bool heapPeek(const Heap *h, HeapItem *out);
size_t heapSize(const Heap *h);
Heap *heapCopy(const Heap *h);
void heapClear(Heap *h);
void freeHeap(Heap *h);

struct StationIndexEntry {
    char key[GRAPH_NAME_MAX];
    int id;
};

struct StationIndex {
    StationIndexEntry *entries;
    int count;
};

StationIndex *newStationIndex(void);
bool buildStationIndex(StationIndex *index, const Graph *g);
int findInIndex(const StationIndex *index, const char *name);
int findInIndexRecursive(const StationIndex *index, const char *name);
int findByPrefix(const StationIndex *index, const char *prefix, int *outIds,
                         int maxOut);
void freeStationIndex(StationIndex *index);

struct Deque {
    unsigned char *items;
    size_t itemSize;
    size_t capacity;
    size_t head;
    size_t count;
};

Deque *newDeque(size_t itemSize);
bool dequeAddFront(Deque *d, const void *item);
bool dequeAddBack(Deque *d, const void *item);
bool dequeRemoveFront(Deque *d, void *out);
bool dequeRemoveBack(Deque *d, void *out);
bool dequePeekFront(const Deque *d, void *out);
bool dequePeekBack(const Deque *d, void *out);
const void *dequeAt(const Deque *d, size_t index);
size_t dequeSize(const Deque *d);
bool dequeIsEmpty(const Deque *d);
void dequeClear(Deque *d);
void freeDeque(Deque *d);

struct DNode {
    int value;
    struct DNode *prev;
    struct DNode *next;
};

struct DList {
    DNode *head;
    DNode *tail;
    size_t size;
};

DList *newDList(void);
bool dlistAddBack(DList *list, int value);
bool dlistAddFront(DList *list, int value);
bool dlistRemove(DList *list, int value);
int dlistIndexOf(const DList *list, int value);
bool dlistContains(const DList *list, int value);
size_t dlistSize(const DList *list);
void dlistClear(DList *list);
void freeDList(DList *list);

// ===== Member 2: BFS, AVL tree, BST and queue =====

bool bfsRoute(const Graph *g, int src, int dst, bool allowClosed, Route *out);

struct AvlNode {
    int key;
    int height;
    struct AvlNode *left;
    struct AvlNode *right;
};

struct Avl {
    AvlNode *root;
    size_t size;
};

Avl *newAvl(void);
bool avlInsert(Avl *tree, int key);
bool avlContains(const Avl *tree, int key);
size_t avlSize(const Avl *tree);
int avlHeight(const Avl *tree);
size_t avlInorder(const Avl *tree, int *out, size_t maxOut);
size_t avlRange(const Avl *tree, int low, int high, int *out, size_t maxOut);
void freeAvl(Avl *tree);

struct BstNode {
    int key;
    struct BstNode *left;
    struct BstNode *right;
};

struct Bst {
    BstNode *root;
    size_t size;
};

Bst *newBst(void);
bool bstInsert(Bst *tree, int key);
bool bstContains(const Bst *tree, int key);
bool bstContainsRecursive(const Bst *tree, int key);
size_t bstSize(const Bst *tree);
int bstHeight(const Bst *tree);
size_t bstInorder(const Bst *tree, int *out, size_t maxOut);
size_t bstRange(const Bst *tree, int low, int high, int *out, size_t maxOut);
void freeBst(Bst *tree);

struct QueueNode {
    int value;
    struct QueueNode *next;
};

struct Queue {
    QueueNode *front;
    QueueNode *rear;
    size_t size;
};

Queue *newQueue(void);
bool enqueue(Queue *q, int value);
bool dequeue(Queue *q, int *out);
bool queuePeek(const Queue *q, int *out);
bool queueIsEmpty(const Queue *q);
size_t queueSize(const Queue *q);
void queueClear(Queue *q);
void freeQueue(Queue *q);

// ===== Member 3: DFS, hash table, stack and linked list =====

int dfsCountRecursive(const Graph *g, int start, int blocked, bool *visited);
int dfsCountLoop(const Graph *g, int start, int blocked, bool *visited);

int dfsGroups(const Graph *g, int blocked, int *groupOf);

struct HmEntry {
    int key;
    void *value;
    struct HmEntry *next;
};

struct HashMap {
    HmEntry **buckets;
    size_t bucketCount;
    size_t size;
    size_t itemSize;
};

struct HmIter {
    const HashMap *map;
    size_t bucket;
    HmEntry *entry;
};

HashMap *newTable(size_t itemSize);
void *tablePut(HashMap *map, int key, const void *value);
void *tableGet(const HashMap *map, int key);
bool tableHas(const HashMap *map, int key);
bool tableRemove(HashMap *map, int key);
size_t tableSize(const HashMap *map);
void tableIterStart(const HashMap *map, HmIter *it);
bool tableIterNext(HmIter *it, int *keyOut, void **valueOut);
void tableClear(HashMap *map);
void freeTable(HashMap *map);

struct Stack {
    unsigned char *items;
    size_t itemSize;
    size_t count;
    size_t capacity;
};

Stack *newStack(size_t itemSize);
bool stackPush(Stack *s, const void *item);
bool stackPop(Stack *s, void *out);
bool stackPeek(const Stack *s, void *out);
const void *stackFromTop(const Stack *s, size_t index);
size_t stackSize(const Stack *s);
bool stackIsEmpty(const Stack *s);
void stackClear(Stack *s);
void freeStack(Stack *s);

struct ListNode {
    void *data;
    struct ListNode *next;
};

struct List {
    ListNode *head;
    ListNode *tail;
    size_t size;
    size_t itemSize;
};

List *newList(size_t itemSize);
bool listAddFront(List *list, const void *item);
bool listAddBack(List *list, const void *item);
bool listRemoveFront(List *list, void *out);
void listTrim(List *list, size_t maxSize);
size_t listSize(const List *list);
void listClear(List *list);
void freeList(List *list);

#include <cstddef>
#include <initializer_list>
#include <cstring>
#include <string>
#include <stdexcept>
#include <cctype>
#include <cstdlib>

using std::initializer_list;
using std::size_t;
using std::memcpy;
using std::string;
using std::runtime_error;
using std::to_string;

template <typename T>
class Array {
public:
    Array()
    {
        items_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    Array(size_t count)
    {
        items_ = nullptr;
        size_ = 0;
        capacity_ = 0;
        for (size_t i = 0; i < count; i++)
            push_back(T());
    }

    Array(size_t count, T value)
    {
        items_ = nullptr;
        size_ = 0;
        capacity_ = 0;
        for (size_t i = 0; i < count; i++)
            push_back(value);
    }

    Array(initializer_list<T> list)
    {
        items_ = nullptr;
        size_ = 0;
        capacity_ = 0;
        for (const T& value : list)
            push_back(value);
    }

    Array(const Array& other)
    {
        items_ = nullptr;
        size_ = 0;
        capacity_ = 0;
        for (size_t i = 0; i < other.size_; i++)
            push_back(other.items_[i]);
    }

    Array& operator=(const Array& other)
    {
        if (this != &other) {
            clear();
            for (size_t i = 0; i < other.size_; i++)
                push_back(other.items_[i]);
        }
        return *this;
    }

    ~Array()
    {
        delete[] items_;
    }

    void push_back(T value)
    {
        if (size_ == capacity_) {
            size_t newCapacity = (capacity_ == 0) ? 8 : capacity_ * 2;
            T* bigger = new T[newCapacity];
            for (size_t i = 0; i < size_; i++)
                bigger[i] = items_[i];
            delete[] items_;
            items_ = bigger;
            capacity_ = newCapacity;
        }
        items_[size_] = value;
        size_++;
    }

    void pop_back()
    {
        if (size_ > 0)
            size_--;
    }

    void resize(size_t count)
    {
        while (size_ < count)
            push_back(T());
        if (size_ > count)
            size_ = count;
    }

    void clear() { size_ = 0; }
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    T& operator[](size_t i) { return items_[i]; }
    const T& operator[](size_t i) const { return items_[i]; }

    T& back() { return items_[size_ - 1]; }

    T* data() { return items_; }
    const T* data() const { return items_; }

    T* begin() { return items_; }
    T* end() { return items_ + size_; }
    const T* begin() const { return items_; }
    const T* end() const { return items_ + size_; }

private:
    T* items_;
    size_t size_;
    size_t capacity_;
};

const int NAME_LEN = GRAPH_NAME_MAX;

enum Category : int { REGULAR = 0, STUDENT = 1, SENIOR = 2, DISABLED = 3 };
enum TicketStatus : int { T_CONFIRMED = 0, T_BOARDED = 1, T_CANCELLED = 2, T_EXPIRED = 3 };
enum CardStatus : int { C_ACTIVE = 0, C_BLOCKED = 1 };
enum PassType : int { PASS_NONE = 0, PASS_DAY = 1, PASS_MONTHLY = 2 };

enum ActionType : int {
    ACT_ADD_STATION = 0,
    ACT_ADD_CONNECTION,
    ACT_CLOSE_STATION,
    ACT_OPEN_STATION,
    ACT_REGISTER_STUDENT,
    ACT_DEACTIVATE_STUDENT,
    ACT_SET_CAPACITY,
    ACT_REGISTER_WOMEN_CARD,
    ACT_DEACTIVATE_WOMEN_CARD
};

enum Priority : int {
    PRIORITY_SENIOR_DISABLED = 0,
    PRIORITY_WOMEN = 1,
    PRIORITY_STUDENT = 2,
    PRIORITY_REGULAR = 3
};

struct Card {
    int id;
    char name[NAME_LEN];
    int category;
    int studentId;
    int balance;
    int status;
    int tappedIn;
    int entryStation;
    long long entryTime;
    int passType;
    long long passExpiry;
    long long createdAt;
};

struct Ticket {
    int id;
    char name[NAME_LEN];
    int category;
    int studentId;
    int source;
    int destination;
    int line;
    int trainId;
    int fare;
    int status;
    long long issuedAt;
    long long validUntil;
    int womenCardId;
};

struct WaitEntry {
    int id;
    char name[NAME_LEN];
    int category;
    int studentId;
    int source;
    int destination;
    int line;
    long long seq;
    int active;
    long long joinedAt;
    int ticketId;
    int womenCardId;
};

struct Trip {
    int cardId;
    int ticketId;
    int source;
    int destination;
    int fare;
    long long time;
};

struct LineState {
    int trainId;
    int capacity;
    int booked;
    int boarded;
    int frequencyMin;
};

struct AdminAction {
    int type;
    int a, b, c, d;
    char text[96];
};

struct Activity {
    long long time;
    char text[120];
};

struct StudentRecord {
    int id;
    int active;
};

inline void copyText(char* dst, size_t size, const string& src)
{
    if (size == 0)
        return;
    size_t n = src.size() < size - 1 ? src.size() : size - 1;
    memcpy(dst, src.data(), n);
    dst[n] = '\0';
}

const int MIN_FARE = 10;

class FarePolicy {
public:
    virtual ~FarePolicy() {}
    virtual string name() const = 0;
    virtual int discountPercent() const = 0;
    virtual int waitlistPriority() const { return 3; }

    int apply(int baseFare) const
    {
        return baseFare - (baseFare * discountPercent()) / 100;
    }
};

class RegularFare : public FarePolicy {
public:
    string name() const override { return "Regular"; }
    int discountPercent() const override { return 0; }
};

class StudentFare : public FarePolicy {
public:
    string name() const override { return "Student"; }
    int discountPercent() const override { return 50; }
    int waitlistPriority() const override { return 2; }
};

class SeniorFare : public FarePolicy {
public:
    string name() const override { return "Senior citizen"; }
    int discountPercent() const override { return 30; }
    int waitlistPriority() const override { return 0; }
};

class DisabledFare : public FarePolicy {
public:
    string name() const override { return "Person with disability"; }
    int discountPercent() const override { return 50; }
    int waitlistPriority() const override { return 0; }
};

const FarePolicy& farePolicy(int category);

int fareForDistance(long long meters);

bool isValidCategory(int category);

template <typename T>
Array<T> collectValues(const HashMap* map)
{
    Array<T> out;
    HmIter it;
    tableIterStart(map, &it);
    int key = 0;
    void* value = nullptr;
    while (tableIterNext(&it, &key, &value)) {
        T* record = (T*)value;
        out.push_back(*record);
    }
    return out;
}

template <typename T>
void sortRecords(Array<T>& items, int (*compare)(const void*, const void*))
{
    if (items.size() > 1)
        mergeSort(items.data(), items.size(), sizeof(T), compare);
}

template <typename T>
int compareById(const void* a, const void* b)
{
    int x = ((const T*)a)->id;
    int y = ((const T*)b)->id;
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

class RouteHolder {
public:
    RouteHolder() { initRoute(&route); }
    ~RouteHolder() { freeRoute(&route); }

    Route route;
};

class LineList {
public:
    LineList() { list = newDList(); }
    ~LineList() { freeDList(list); }

    DList* list;
};

class Clock {
public:
    static long long now();
    static void advanceMinutes(long long minutes);
    static void reset();
    static long long offsetMinutes();
    static string format(long long t);
    static int minutesOfDay(long long t);

private:
    static long long offsetSeconds_;
};

const int MAX_LINES = 8;
const int TICKET_VALID_MIN = 120;
const int MAX_TRIP_MIN = 180;
const int OVERSTAY_PENALTY = 50;
const int MIN_TAPIN_BALANCE = 10;
const int MAX_RECHARGE = 5000;
const int MAX_BALANCE = 10000;
const int DAY_PASS_PRICE = 200;
const int MONTHLY_PASS_PRICE = 2000;
const int SERVICE_START_MIN = 6 * 60;
const int SERVICE_END_MIN = 23 * 60;
const int MAX_TRAIN_CAPACITY = 2000;
const int ACTIVITY_LOG_MAX = 200;
const int LOGIN_MAX_ATTEMPTS = 3;
const int LOGIN_LOCK_SECONDS = 60;

struct Result {
    bool ok = false;
    string message;
};

struct BookingResult {
    bool ok = false;
    bool waitlisted = false;
    int ticketId = 0;
    int waitId = 0;
    int position = 0;
    int fare = 0;
    string message;
};

struct TapOutResult {
    bool ok = false;
    int fare = 0;
    int penalty = 0;
    bool passUsed = false;
    long long distanceM = 0;
    int balanceLeft = 0;
    string message;
};

struct NextTrain {
    bool running = false;
    int minutes = 0;
};

struct StationCount {
    int station;
    int count;
};

struct RouteCount {
    int source;
    int destination;
    int count;
};

struct Report {
    long long ticketRevenue = 0;
    long long cardRevenue = 0;
    long long passRevenue = 0;
    int ticketsByStatus[4] = {0, 0, 0, 0};
    int totalCards = 0;
    int blockedCards = 0;
    int cardsInside = 0;
    int activePasses = 0;
    int totalTrips = 0;
    int ticketIndexSize = 0;
    int ticketIndexHeight = 0;
    int ticketAvlHeight = 0;
    Array<StationCount> busiestStations;
    Array<RouteCount> popularRoutes;
};

class Metro {
public:
    Metro();
    ~Metro();

    void loadDefaultNetwork();
    void finishSetup(const string& note);

    int stationCount() const;
    int lineCount() const;
    string stationName(int id) const;
    string lineName(int line) const;
    int findLine(const string& name) const;
    bool isStationOpen(int id) const;
    int findStation(const string& idOrName) const;
    Array<int> linesAtStation(int id) const;
    Result addStation(const string& name);
    Result addLine(const string& name, int capacity, int frequencyMin);
    Result addConnection(int a, int b, int distanceM, int line);
    Result setStationOpen(int id, bool open);
    bool planRoute(int src, int dst, bool fewestStops, RouteHolder& out,
                   string& error) const;
    Array<int> suggestStations(const string& prefix, int maxResults) const;
    Array<int> cutOffStations(int id) const;
    bool makeLineList(int line, LineList& out, bool& complete) const;
    bool isLoopLine(int line) const;
    string towardsStation(int line, int from, int to) const;
    int trackDistance(int a, int b, int line) const;

    Result registerStudent(int id);
    Result deactivateStudent(int id);
    bool isValidStudent(int id) const;
    Array<StudentRecord> students() const;

    Result registerWomenCard(int id);
    Result deactivateWomenCard(int id);
    bool isValidWomenCard(int id) const;
    Array<StudentRecord> womenCards() const;
    static int waitPriority(int category, int womenCardId);
    static string priorityName(int priority);

    Result createCard(const string& name, int category, int studentId,
                      int initialBalance, int& newId);
    Result recharge(int cardId, int amount);
    Result tapIn(int cardId, int station);
    TapOutResult tapOut(int cardId, int station);
    Result buyPass(int cardId, int passType);
    Result blockAndTransfer(int cardId, int& newCardId);
    const Card* findCard(int id) const;
    bool hasValidPass(const Card& card) const;
    Array<Trip> cardHistory(int cardId, int maxTrips) const;
    int passPrice(int category, int passType) const;

    BookingResult bookTicket(const string& name, int category, int studentId,
                             int src, int dst, int womenCardId = 0);
    Result cancelTicket(int ticketId, Array<string>& notes);
    Result boardTicket(int ticketId);
    Result leaveWaitlist(int waitId);
    const Ticket* findTicket(int id) const;
    const WaitEntry* findWaitEntry(int id) const;
    int waitlistPosition(int line, int waitId) const;
    Array<WaitEntry> waitlistInOrder(int line) const;
    Array<Ticket> allTickets() const;
    Array<Ticket> ticketsInRange(int low, int high) const;
    Array<Ticket> ticketsByFare() const;
    int ticketIndexHeight() const;
    int ticketAvlHeight() const;
    int expireOldTickets(Array<string>* notes = nullptr);

    const LineState& lineState(int line) const;
    Result dispatchTrain(int line, Array<string>& notes);
    Result setCapacity(int line, int capacity, Array<string>& notes);
    NextTrain nextTrain(int line, long long now) const;
    static string crowdLevel(const LineState& ls);

    Result adminLogin(const string& password);
    Result changePassword(const string& oldPassword, const string& newPassword);
    Result undoLast();
    Array<string> undoHistory() const;
    Array<Activity> recentActivity(int maxItems) const;
    Report buildReport() const;

    bool isDirty() const { return dirty_; }
    void clearDirty() { dirty_ = false; }

private:
    friend class Storage;

    Card* cardPtr(int id);
    Ticket* ticketPtr(int id);
    bool validLine(int line) const;
    bool journeyFare(int src, int dst, int category, int& fare, int& line,
                        string& error) const;
    int createTicket(const string& name, int category, int studentId, int src,
                     int dst, int line, int fare, int womenCardId);
    void processWaitlist(int line, Array<string>* notes);
    bool stationInUse(int station) const;
    void pushUndo(int type, int a, int b, int c, int d, const string& text);
    Result applyUndo(const AdminAction& action);
    void log(const string& text);
    void freeAll();
    void rebuildStationIndex();
    Array<Ticket> ticketsFromIds(const Array<int>& ids) const;

    Graph* graph_ = nullptr;
    HashMap* students_ = nullptr;
    HashMap* womenCards_ = nullptr;
    HashMap* cards_ = nullptr;
    HashMap* tickets_ = nullptr;
    HashMap* waitEntries_ = nullptr;
    Heap* waitlists_[MAX_LINES] = {};
    List* trips_ = nullptr;
    Deque* activity_ = nullptr;
    Bst* ticketIndex_ = nullptr;
    Avl* ticketAvl_ = nullptr;
    StationIndex* stationIndex_ = nullptr;
    Stack* undo_ = nullptr;
    LineState lines_[MAX_LINES] = {};

    int nextCardId_ = 5001;
    int nextTicketId_ = 1001;
    int nextWaitId_ = 1;
    int nextTrainId_ = 101;
    long long nextWaitSeq_ = 1;
    long long ticketRevenue_ = 0;
    long long cardRevenue_ = 0;
    long long passRevenue_ = 0;
    unsigned long long adminHash_ = 0;
    int failedLogins_ = 0;
    long long lockedUntil_ = 0;
    bool dirty_ = false;
};

unsigned long long hashPassword(const string& password);

inline string rs(long long amount)
{
    return "Rs. " + to_string(amount);
}

inline bool parseInt(const string& text, int& out)
{
    if (text.empty())
        return false;
    for (char c : text)
        if (!isdigit((unsigned char)c))
            return false;
    if (text.size() > 9)
        return false;
    out = atoi(text.c_str());
    return true;
}

inline string trim(const string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

inline Result ok(const string& message) { return {true, message}; }
inline Result fail(const string& message) { return {false, message}; }

class Metro;

enum class LoadStatus { Loaded, NotFound, Corrupt };

class Storage {
public:
    static bool save(const Metro& metro, const string& path, string& error);
    static LoadStatus load(Metro& metro, const string& path, string& error);
};

class Metro;

enum class SeedStatus { Loaded, FolderMissing, Error };

SeedStatus loadStartingData(Metro& metro, const string& folder, string& error, string& summary);

struct SimConfig {
    int passengers = 500;
    int durationMin = 60;
    int capacity = 100;
    int frequencyMin = 5;
    int platformCapacity = 300;
    unsigned int seed = 42;
};

struct SimLineResult {
    string name;
    int arrived = 0;
    int served = 0;
    int trains = 0;
    int fullTrains = 0;
    double avgWait = 0.0;
    int maxWait = 0;
    double avgOccupancy = 0.0;
    int heldAtGate = 0;
    int maxAtGate = 0;
};

struct SimResult {
    int passengers = 0;
    int served = 0;
    int leftBehind = 0;
    int trains = 0;
    int fullTrains = 0;
    double avgWait = 0.0;
    int maxWait = 0;
    double avgOccupancy = 0.0;
    int heldAtGate = 0;
    int maxAtGate = 0;
    int minutesSimulated = 0;
    Array<SimLineResult> lines;
};

SimResult runSimulation(const SimConfig& config, const Array<string>& lineNames);

namespace ui {
using Row = Array<string>;

struct InputClosed : runtime_error {
    InputClosed() : runtime_error("input closed") {}
};



void clearScreen();

void title(const string& text);
void success(const string& text);
void error(const string& text);
void warn(const string& text);
void info(const string& text);

string trim(const string& s);
bool parseInt(const string& text, int& out);
string readLine(const string& prompt);
int readInt(const string& prompt, int minValue, int maxValue);
string readText(const string& prompt, size_t maxLen);
bool confirm(const string& question);

class Table {
public:
    Table(Row headers);
    void addRow(Row row);
    bool empty() const { return rows_.empty(); }
    void print() const;

private:
    Row headers_;
    Array<Row> rows_;
};

}

struct RouteSummary {
    long long meters = 0;
    int stops = 0;
    int changes = 0;
    int minutes = 0;
};

class App {
public:
    App(Metro& metro, string dataPath);
    void run();

private:
    int menu(const string& heading, const Array<string>& options);
    void autosave();
    void finishAction();

    void passengerMenu();
    void showStations();
    void showLine();
    void planJourney();
    void showNextTrains();
    void bookTicket();
    void cancelTicket();
    void boardTrain();
    void checkStatus();
    void leaveWaitlist();

    void cardMenu();
    void newCard();
    void cardDetails();
    void rechargeCard();
    void tapIn();
    void tapOut();
    void buyPass();
    void journeyHistory();
    void reportLostCard();

    bool adminLogin();
    void adminMenu();
    void addStation();
    void addLine();
    void addConnection();
    void toggleStation();
    void registerStudent();
    void deactivateStudent();
    void showStudents();
    void registerWomenCard();
    void deactivateWomenCard();
    void showWomenCards();
    void showTrainStatus();
    void dispatchTrain();
    void setCapacity();
    void showTickets();
    void showTicketsByFare();
    void showTicketsInRange();
    void showReports();
    void showActivity();
    void undo();
    void simulation();
    void advanceClock();
    void changePassword();
    void saveNow();

    int askStation(const string& prompt);
    int askLine(const string& prompt);
    int askCategory(int& studentId);
    int askCardId();
    int readIntDefault(const string& prompt, int minValue, int maxValue, int fallback);
    RouteSummary printRoute(const Route& route);
    void printFares(long long meters);
    void printTicket(const Ticket& t);
    void printTicketTable(const Array<Ticket>& tickets);
    void show(const Result& r);
    void showNotes(const Array<string>& notes);
    string lineLabel(int line) const;
    string linesText(const Array<int>& lines) const;
    string stationLabel(int station) const;

    Metro& m_;
    string dataPath_;
};

#endif
