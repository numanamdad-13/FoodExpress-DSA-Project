#ifndef FOODEXPRESS_H
#define FOODEXPRESS_H

#include <iostream>
#include <string>
#include <iomanip>
#include <climits>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>

using namespace std;

// ==============================================================================
//  Enumerations
// ==============================================================================
enum class Priority { NORMAL = 1, PREMIUM = 2, VIP = 3 };
enum class OrderStatus { QUEUED, PREPARING, READY, DISPATCHED, DELIVERED, CANCELLED };
enum class ActionType { ORDER_ADDED, ORDER_CANCELLED, ORDER_PROCESSED, STATUS_CHANGED };

string priorityToString(Priority p);
string statusToString(OrderStatus s);
string actionToString(ActionType a);

// ==============================================================================
//  Memory-Safe Dynamic Array (Rule-of-Five)
// ==============================================================================
template <typename T>
class Array {
private:
    T *mData;
    int mSize;
    int mCapacity;

public:
    Array();
    ~Array();
    Array(const Array &other);
    Array &operator=(const Array &other);
    Array(Array &&other) noexcept;
    Array &operator=(Array &&other) noexcept;

    int size() const;
    bool empty() const;
    T &operator[](int i);
    const T &operator[](int i) const;
    void push(const T &val);
    void removeLast();
    void removeAt(int idx);
    void clear();
};

// ==============================================================================
//  Input & UI Helpers
// ==============================================================================
class InputHelper {
public:
    static int readInt(const string &prompt, int minVal = INT_MIN, int maxVal = INT_MAX);
    static string readLine(const string &prompt);
    static void pause();
};

class UI {
public:
    static void header(const string &title);
    static void subHeader(const string &title);
    static void success(const string &msg);
    static void error(const string &msg);
    static void info(const string &msg);
    static void warning(const string &msg);
};

// ==============================================================================
//  Core Entities
// ==============================================================================
class Order {
private:
    int mId;
    string mCustomerName;
    string mItem;
    string mCategory;
    Priority mPriority;
    int mPrepTime;
    int mDeadline;
    OrderStatus mStatus;
    int mAssignedRider;
    int mAssignedKitchen;
    bool mDelayed;

public:
    Order();
    Order(int id, const string &customer, const string &item, const string &category,
          Priority pri, int prepTime, int deadline);

    int getId() const;
    const string &getCustomerName() const;
    const string &getItem() const;
    const string &getCategory() const;
    Priority getPriority() const;
    int getPrepTime() const;
    int getDeadline() const;
    OrderStatus getStatus() const;
    int getAssignedRider() const;
    int getAssignedKitchen() const;
    bool isValid() const;
    bool isDelayed() const;

    void setStatus(OrderStatus s);
    void setPriority(Priority p);
    void setAssignedRider(int rid);
    void setAssignedKitchen(int kid);
    void setDelayed(bool d);
    void setPrepTime(int t);
};

class Rider {
private:
    int mId;
    string mName;
    int mCurrentLoad;
    int mCapacity;
    int mLocationNode;
    bool mAvailable;
    int mTotalDeliveries;

public:
    Rider();
    Rider(int id, const string &name, int capacity, int location);

    int getId() const;
    const string &getName() const;
    int getCurrentLoad() const;
    int getCapacity() const;
    int getLocationNode() const;
    bool isAvailable() const;
    int getTotalDeliveries() const;
    bool canTakeOrder() const;

    void setAvailable(bool a);
    void setLocationNode(int loc);
    void incrementLoad();
    bool decrementLoad();

    friend class RiderHeap;
};

class OrderRecord {
private:
    int mOrderId;
    string mCustomerName;
    string mItem;
    string mCategory;
    Priority mPriority;
    OrderStatus mFinalStatus;
    string mKitchenName;
    string mRiderName;
    int mRouteDistance;
    int mEstimatedETA;

public:
    OrderRecord();
    OrderRecord(const Order &o, const string &kitchen = "", const string &rider = "",
                int dist = -1, int eta = -1);
    OrderRecord(int id, const string &cust, const string &item, const string &cat,
                Priority pri, OrderStatus stat, const string &kitchen, const string &rider,
                int dist, int eta);

    int getId() const;
    const string &getCustomerName() const;
    const string &getItem() const;
    const string &getCategory() const;
    Priority getPriority() const;
    OrderStatus getFinalStatus() const;
    const string &getKitchenName() const;
    const string &getRiderName() const;
    int getRouteDistance() const;
    int getEstimatedETA() const;

    void display() const;
};

// ==============================================================================
//  MODULE 1: Max-Heap Order Scheduling
// ==============================================================================
class OrderHeap {
private:
    Array<Order> mHeap;
    int parent(int i) const;
    int leftChild(int i) const;
    int rightChild(int i) const;
    bool higherPriority(const Order &a, const Order &b) const;
    void swapAt(int i, int j);
    void siftUp(int i);
    void siftDown(int i);

public:
    void insert(const Order &o);
    Order extractTop();
    bool cancelOrder(int orderId);
    Order *findOrder(int orderId);
    bool updatePriority(int orderId, Priority newPri);
    bool markDelayed(int orderId);
    Array<Order> getDelayedOrders() const;
    Order peekTop() const;
    int getSize() const;
    bool isEmpty() const;
    void display() const;
    Array<Order> getAllOrders() const;
};

// ==============================================================================
//  MODULE 4: City Graph (Adjacency Matrix + Dijkstra)
// ==============================================================================
class CityGraph {
public:
    static const int MAX_NODES = 20;

private:
    static const int INF = INT_MAX / 2;
    int mNodeCount;
    string mNames[MAX_NODES];
    int mAdj[MAX_NODES][MAX_NODES];
    bool mBlocked[MAX_NODES][MAX_NODES];

public:
    CityGraph();
    int getNodeCount() const;
    bool isValidNode(int n) const;
    const string &getLocationName(int idx) const;
    bool addLocation(const string &name);
    bool addRoad(int u, int v, int weight);
    bool blockRoad(int u, int v);
    bool unblockRoad(int u, int v);
    bool isBlocked(int u, int v) const;
    int getRoadWeight(int u, int v) const;
    int findShortestPath(int src, int dest, Array<int> &path) const;
    int shortestDistance(int src, int dest) const;
    void compareRoutes(int src, int dest1, int dest2) const;
    int estimateDeliveryCost(int src, int dest, int ratePerUnit = 5) const;
    void displayMap() const;
};

// ==============================================================================
//  Hash Map (Chaining)
// ==============================================================================
template <typename V>
class HashMap {
private:
    struct Node {
        int key;
        V value;
        Node *next;
        Node(int k, const V &v, Node *n = nullptr);
    };
    static const int BUCKET_COUNT = 127;
    Node *mBuckets[BUCKET_COUNT];
    int mCount;
    int hashFunc(int key) const;

public:
    HashMap();
    ~HashMap();
    HashMap(const HashMap &) = delete;
    HashMap &operator=(const HashMap &) = delete;

    int getCount() const;
    void insert(int key, const V &value);
    V *find(int key);
    const V *find(int key) const;
    bool remove(int key);
    Array<V> getAllValues() const;
    void clear();
};

// ==============================================================================
//  MODULE 3: Rider Dispatch Min-Heap
// ==============================================================================
class RiderHeap {
private:
    Array<Rider> mHeap;
    int parent(int i) const;
    int leftChild(int i) const;
    int rightChild(int i) const;
    void swapAt(int i, int j);
    void siftUp(int i);
    void siftDown(int i);

public:
    void addRider(const Rider &r);
    Rider assignBestRider();
    Rider assignBestRiderWithRoute(const CityGraph &city, int destNode);
    bool completeDelivery(int riderId);
    bool toggleAvailability(int riderId);
    int getSize() const;
    void display() const;
    Array<Rider> getAllRiders() const;
};

// ==============================================================================
//  MODULE 2: Kitchen Queue (Linked-List)
// ==============================================================================
class KitchenQueue {
private:
    struct Node {
        int orderId;
        string item;
        int prepTime;
        Node *next;
        Node(int id, const string &it, int pt);
    };
    Node *mFront;
    Node *mRear;
    int mSize;
    int mMaxCapacity;
    void clear();
    void copyFrom(const KitchenQueue &other);

public:
    KitchenQueue();
    ~KitchenQueue();
    KitchenQueue(const KitchenQueue &other);
    KitchenQueue &operator=(const KitchenQueue &other);

    int getSize() const;
    int getMaxCapacity() const;
    bool isFull() const;
    bool isEmpty() const;
    void setMaxCapacity(int cap);
    int totalWaitTime() const;
    void enqueue(int orderId, const string &item, int prepTime);
    int dequeue();
    int peekFront() const;
    void display(const string &kitchenName) const;
};

class KitchenSystem {
private:
    static const int MAX_KITCHENS = 10;
    KitchenQueue mKitchens[MAX_KITCHENS];
    string mNames[MAX_KITCHENS];
    int mCount;

public:
    KitchenSystem();
    int getNumKitchens() const;
    const string &getKitchenName(int idx) const;
    bool isValidKitchen(int idx) const;
    bool addKitchen(const string &name, int cap);
    int distributeOrder(int orderId, const string &item, int prepTime);
    int completeOrder(int kitchenIdx);
    int getKitchenWaitTime(int idx) const;
    int getKitchenLoad(int idx) const;
    void detectOverloaded() const;
    bool rebalance();
    int estimateWaitTime() const;
    void displayAll() const;
};

// ==============================================================================
//  MODULE 5: Search & Retrieval Engine
// ==============================================================================
class SearchEngine {
private:
    HashMap<OrderRecord> mIndex;
    Array<OrderRecord> mAllRecords;
    static bool containsIgnoreCase(const string &haystack, const string &needle);

public:
    void indexOrder(const OrderRecord &record);
    int getCount() const;
    OrderRecord *findById(int orderId);
    Array<OrderRecord> searchByCustomer(const string &name) const;
    Array<OrderRecord> searchByItem(const string &item) const;
    Array<OrderRecord> searchByStatus(OrderStatus status) const;
    Array<OrderRecord> searchByPriority(Priority priority) const;
    Array<OrderRecord> searchByCategory(const string &cat) const;
    Array<OrderRecord> searchByKitchen(const string &kitchen) const;
    Array<OrderRecord> searchByRider(const string &rider) const;
    const Array<OrderRecord> &getAllRecords() const;
    static void displayResultsHeader();
    static void displayResults(const Array<OrderRecord> &results);
    void displayAll() const;
};

// ==============================================================================
//  MODULE 6: Order Tracking & Undo
// ==============================================================================
struct StateTransition {
    OrderStatus fromState;
    OrderStatus toState;
    int timestamp;
    string description;
    StateTransition();
    StateTransition(OrderStatus from, OrderStatus to, int ts, const string &desc);
};

struct OrderTimeline {
    int orderId;
    string customerName;
    string item;
    Array<StateTransition> transitions;
    OrderTimeline();
    OrderTimeline(int id, const string &cust, const string &it);
    void addTransition(const StateTransition &t);
    OrderStatus currentState() const;
    void replay() const;
};

struct UndoAction {
    ActionType type;
    Order snapshot;
    string description;
    int timestamp;
    UndoAction();
    UndoAction(ActionType t, const Order &snap, const string &desc, int ts);
};

class OrderTracker {
private:
    HashMap<OrderTimeline> mTimelines;
    Array<UndoAction> mUndoStack;
    int mClock;

public:
    OrderTracker();
    int getClock() const;
    void recordOrderCreated(const Order &order);
    void recordStateChange(int orderId, OrderStatus from, OrderStatus to, const string &desc);
    void recordOrderCancelled(const Order &order);
    void recordOrderProcessed(const Order &order);
    bool undoLastAction(OrderHeap &orderQueue);
    bool replayTimeline(int orderId) const;
    void displayAllTimelines() const;
    void displayUndoStack() const;
    int getUndoCount() const;
    int getTrackedCount() const;
};

// ==============================================================================
//  MODULE 5 Integrated: Delivery Pipeline
// ==============================================================================
class DeliveryPipeline {
private:
    OrderHeap &mOrderQueue;
    RiderHeap &mRiderPool;
    CityGraph &mCity;
    KitchenSystem &mKitchens;
    SearchEngine &mSearchEngine;
    OrderTracker &mTracker;

public:
    DeliveryPipeline(OrderHeap &oq, RiderHeap &rp, CityGraph &cg,
                     KitchenSystem &ks, SearchEngine &se, OrderTracker &tr);
    bool processNextOrder(int destNode);
};

// ==============================================================================
//  MODULE 7: Performance Analyzer
// ==============================================================================
class PerformanceAnalyzer {
private:
    static double getTimeMs();
public:
    static void benchmarkInsertion(int count);
    static void benchmarkDeletion(int count);
    static void benchmarkSearching(int count);
    static void benchmarkScheduling(int count);
    static void benchmarkRouting();
    static void benchmarkScalability();
    static void estimateMemoryUsage(int orderCount, int riderCount, int kitchenCount, int graphNodes);
    static void runFullAnalysis();
};

// ==============================================================================
//  File Handler (CSV)
// ==============================================================================
class FileHandler {
private:
    static string trimLine(const string &s);
    static int safeStoi(const string &s, int fallback = 0);
public:
    static void saveOrders(const OrderHeap &heap, const string &filename = "orders.csv");
    static int loadOrders(OrderHeap &heap, OrderTracker &tracker, const string &filename = "orders.csv");
    static void saveRiders(const RiderHeap &riders, const string &filename = "riders.csv");
    static int loadRiders(RiderHeap &riders, const string &filename = "riders.csv");
    static void saveRecords(const SearchEngine &engine, const string &filename = "delivery_records.csv");
    static int loadRecords(SearchEngine &engine, const string &filename = "delivery_records.csv");
    static void saveGraph(const CityGraph &graph, const string &filename = "city_graph.csv");
    static bool loadGraph(CityGraph &graph, const string &filename = "city_graph.csv");
    static void saveKitchens(const KitchenSystem &kitchens, const string &filename = "kitchens.csv");
    static int loadKitchens(KitchenSystem &kitchens, const string &filename = "kitchens.csv");
    static void saveAll(const OrderHeap &heap, const RiderHeap &riders,
                        const CityGraph &graph, const KitchenSystem &kitchens,
                        const SearchEngine &engine);
    static void loadAll(OrderHeap &heap, RiderHeap &riders, CityGraph &graph,
                        KitchenSystem &kitchens, SearchEngine &engine, OrderTracker &tracker);
};

// ==============================================================================
//  Demo Data & Global Helpers
// ==============================================================================
extern int gNextOrderId;
bool filesExist();
void seedDemoData(OrderHeap &oq, RiderHeap &rp, CityGraph &cg,
                  KitchenSystem &ks, SearchEngine &se, OrderTracker &tr);
void autoSave(const OrderHeap &oq, const RiderHeap &rp, const CityGraph &cg,
              const KitchenSystem &ks, const SearchEngine &se);

#endif // FOODEXPRESS_H