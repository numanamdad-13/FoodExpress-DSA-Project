#include "FoodExpress.h"

// ==============================================================================
//  Helper Conversions
// ==============================================================================
string priorityToString(Priority p) {
    switch (p) {
        case Priority::VIP: return "VIP";
        case Priority::PREMIUM: return "Premium";
        case Priority::NORMAL: return "Normal";
        default: return "Unknown";
    }
}

string statusToString(OrderStatus s) {
    switch (s) {
        case OrderStatus::QUEUED: return "Queued";
        case OrderStatus::PREPARING: return "Preparing";
        case OrderStatus::READY: return "Ready";
        case OrderStatus::DISPATCHED: return "Dispatched";
        case OrderStatus::DELIVERED: return "Delivered";
        case OrderStatus::CANCELLED: return "Cancelled";
        default: return "Unknown";
    }
}

string actionToString(ActionType a) {
    switch (a) {
        case ActionType::ORDER_ADDED: return "Order Added";
        case ActionType::ORDER_CANCELLED: return "Order Cancelled";
        case ActionType::ORDER_PROCESSED: return "Order Processed";
        case ActionType::STATUS_CHANGED: return "Status Changed";
        default: return "Unknown";
    }
}

// ==============================================================================
//  Array<T> Implementation
// ==============================================================================
template <typename T>
Array<T>::Array() : mData(nullptr), mSize(0), mCapacity(0) {}

template <typename T>
Array<T>::~Array() { delete[] mData; }

template <typename T>
Array<T>::Array(const Array &other) : mData(nullptr), mSize(other.mSize), mCapacity(other.mCapacity) {
    if (mCapacity > 0) {
        mData = new T[mCapacity];
        for (int i = 0; i < mSize; ++i) mData[i] = other.mData[i];
    }
}

template <typename T>
Array<T> &Array<T>::operator=(const Array &other) {
    if (this != &other) {
        delete[] mData;
        mSize = other.mSize;
        mCapacity = other.mCapacity;
        mData = (mCapacity > 0) ? new T[mCapacity] : nullptr;
        for (int i = 0; i < mSize; ++i) mData[i] = other.mData[i];
    }
    return *this;
}

template <typename T>
Array<T>::Array(Array &&other) noexcept : mData(other.mData), mSize(other.mSize), mCapacity(other.mCapacity) {
    other.mData = nullptr;
    other.mSize = other.mCapacity = 0;
}

template <typename T>
Array<T> &Array<T>::operator=(Array &&other) noexcept {
    if (this != &other) {
        delete[] mData;
        mData = other.mData;
        mSize = other.mSize;
        mCapacity = other.mCapacity;
        other.mData = nullptr;
        other.mSize = other.mCapacity = 0;
    }
    return *this;
}

template <typename T>
int Array<T>::size() const { return mSize; }

template <typename T>
bool Array<T>::empty() const { return mSize == 0; }

template <typename T>
T &Array<T>::operator[](int i) { return mData[i]; }

template <typename T>
const T &Array<T>::operator[](int i) const { return mData[i]; }

template <typename T>
void Array<T>::push(const T &val) {
    if (mSize == mCapacity) {
        mCapacity = (mCapacity == 0) ? 4 : mCapacity * 2;
        T *newData = new T[mCapacity];
        for (int i = 0; i < mSize; ++i) newData[i] = mData[i];
        delete[] mData;
        mData = newData;
    }
    mData[mSize++] = val;
}

template <typename T>
void Array<T>::removeLast() { if (mSize > 0) --mSize; }

template <typename T>
void Array<T>::removeAt(int idx) {
    if (idx < 0 || idx >= mSize) return;
    for (int i = idx; i < mSize - 1; ++i) mData[i] = mData[i + 1];
    --mSize;
}

template <typename T>
void Array<T>::clear() { mSize = 0; }

// Explicit instantiation for used types
template class Array<Order>;
template class Array<Rider>;
template class Array<OrderRecord>;
template class Array<OrderTimeline>;
template class Array<UndoAction>;
template class Array<StateTransition>;

// ==============================================================================
//  InputHelper & UI Implementation
// ==============================================================================
int InputHelper::readInt(const string &prompt, int minVal, int maxVal) {
    string line;
    while (true) {
        cout << prompt;
        if (!getline(cin, line)) return minVal;
        size_t start = line.find_first_not_of(" \t");
        if (start == string::npos) { cout << "  [Error] Please enter a valid number.\n"; continue; }
        try {
            size_t pos = 0;
            int value = stoi(line.substr(start), &pos);
            string rest = line.substr(start + pos);
            bool clean = true;
            for (size_t ci = 0; ci < rest.size(); ++ci)
                if (!isspace(static_cast<unsigned char>(rest[ci]))) { clean = false; break; }
            if (!clean) { cout << "  [Error] Please enter a valid number.\n"; continue; }
            if (value < minVal || value > maxVal) {
                cout << "  [Error] Value must be between " << minVal << " and " << maxVal << ".\n";
                continue;
            }
            return value;
        } catch (...) { cout << "  [Error] Please enter a valid number.\n"; }
    }
}

string InputHelper::readLine(const string &prompt) {
    string value;
    while (true) {
        cout << prompt;
        if (!getline(cin, value)) return "";
        if (!value.empty()) return value;
        cout << "  [Error] Input cannot be empty.\n";
    }
}

void InputHelper::pause() {
    cout << "\n  Press Enter to continue...";
    string dummy;
    getline(cin, dummy);
}

void UI::header(const string &title) {
    const int WIDTH = 64;
    cout << "\n  "; for (int i = 0; i < WIDTH; ++i) cout << "=";
    cout << "\n  ||  " << left << setw(WIDTH - 5) << title << "||\n  ";
    for (int i = 0; i < WIDTH; ++i) cout << "=";
    cout << "\n";
}

void UI::subHeader(const string &title) {
    cout << "\n  --- " << title << " ";
    int remaining = 55 - static_cast<int>(title.length());
    for (int i = 0; i < remaining && i < 55; ++i) cout << "-";
    cout << "\n";
}

void UI::success(const string &msg) { cout << "  [+] " << msg << "\n"; }
void UI::error(const string &msg)   { cout << "  [X] " << msg << "\n"; }
void UI::info(const string &msg)    { cout << "  [i] " << msg << "\n"; }
void UI::warning(const string &msg) { cout << "  [!] " << msg << "\n"; }

// ==============================================================================
//  Order Implementation
// ==============================================================================
Order::Order() : mId(0), mCategory("Regular"), mPriority(Priority::NORMAL), mPrepTime(10),
                 mDeadline(0), mStatus(OrderStatus::QUEUED), mAssignedRider(-1),
                 mAssignedKitchen(-1), mDelayed(false) {}

Order::Order(int id, const string &customer, const string &item, const string &category,
             Priority pri, int prepTime, int deadline)
    : mId(id), mCustomerName(customer), mItem(item), mCategory(category),
      mPriority(pri), mPrepTime(prepTime), mDeadline(deadline),
      mStatus(OrderStatus::QUEUED), mAssignedRider(-1), mAssignedKitchen(-1),
      mDelayed(false) {}

int Order::getId() const { return mId; }
const string &Order::getCustomerName() const { return mCustomerName; }
const string &Order::getItem() const { return mItem; }
const string &Order::getCategory() const { return mCategory; }
Priority Order::getPriority() const { return mPriority; }
int Order::getPrepTime() const { return mPrepTime; }
int Order::getDeadline() const { return mDeadline; }
OrderStatus Order::getStatus() const { return mStatus; }
int Order::getAssignedRider() const { return mAssignedRider; }
int Order::getAssignedKitchen() const { return mAssignedKitchen; }
bool Order::isValid() const { return mId != 0; }
bool Order::isDelayed() const { return mDelayed; }
void Order::setStatus(OrderStatus s) { mStatus = s; }
void Order::setPriority(Priority p) { mPriority = p; }
void Order::setAssignedRider(int rid) { mAssignedRider = rid; }
void Order::setAssignedKitchen(int kid) { mAssignedKitchen = kid; }
void Order::setDelayed(bool d) { mDelayed = d; }
void Order::setPrepTime(int t) { mPrepTime = t; }

// ==============================================================================
//  Rider Implementation
// ==============================================================================
Rider::Rider() : mId(0), mCurrentLoad(0), mCapacity(3), mLocationNode(0), mAvailable(true), mTotalDeliveries(0) {}
Rider::Rider(int id, const string &name, int capacity, int location)
    : mId(id), mName(name), mCurrentLoad(0), mCapacity(capacity), mLocationNode(location), mAvailable(true), mTotalDeliveries(0) {}

int Rider::getId() const { return mId; }
const string &Rider::getName() const { return mName; }
int Rider::getCurrentLoad() const { return mCurrentLoad; }
int Rider::getCapacity() const { return mCapacity; }
int Rider::getLocationNode() const { return mLocationNode; }
bool Rider::isAvailable() const { return mAvailable; }
int Rider::getTotalDeliveries() const { return mTotalDeliveries; }
bool Rider::canTakeOrder() const { return mAvailable && mCurrentLoad < mCapacity; }
void Rider::setAvailable(bool a) { mAvailable = a; }
void Rider::setLocationNode(int loc) { mLocationNode = loc; }
void Rider::incrementLoad() { mCurrentLoad++; }
bool Rider::decrementLoad() {
    if (mCurrentLoad <= 0) return false;
    mCurrentLoad--;
    mTotalDeliveries++;
    return true;
}

// ==============================================================================
//  OrderRecord Implementation
// ==============================================================================
OrderRecord::OrderRecord() : mOrderId(0), mPriority(Priority::NORMAL), mFinalStatus(OrderStatus::QUEUED), mRouteDistance(-1), mEstimatedETA(-1) {}
OrderRecord::OrderRecord(const Order &o, const string &kitchen, const string &rider, int dist, int eta)
    : mOrderId(o.getId()), mCustomerName(o.getCustomerName()), mItem(o.getItem()),
      mCategory(o.getCategory()), mPriority(o.getPriority()), mFinalStatus(o.getStatus()),
      mKitchenName(kitchen), mRiderName(rider), mRouteDistance(dist), mEstimatedETA(eta) {}
OrderRecord::OrderRecord(int id, const string &cust, const string &item, const string &cat,
                         Priority pri, OrderStatus stat, const string &kitchen, const string &rider,
                         int dist, int eta)
    : mOrderId(id), mCustomerName(cust), mItem(item), mCategory(cat), mPriority(pri),
      mFinalStatus(stat), mKitchenName(kitchen), mRiderName(rider), mRouteDistance(dist), mEstimatedETA(eta) {}

int OrderRecord::getId() const { return mOrderId; }
const string &OrderRecord::getCustomerName() const { return mCustomerName; }
const string &OrderRecord::getItem() const { return mItem; }
const string &OrderRecord::getCategory() const { return mCategory; }
Priority OrderRecord::getPriority() const { return mPriority; }
OrderStatus OrderRecord::getFinalStatus() const { return mFinalStatus; }
const string &OrderRecord::getKitchenName() const { return mKitchenName; }
const string &OrderRecord::getRiderName() const { return mRiderName; }
int OrderRecord::getRouteDistance() const { return mRouteDistance; }
int OrderRecord::getEstimatedETA() const { return mEstimatedETA; }

void OrderRecord::display() const {
    cout << "  " << left << setw(7) << mOrderId << setw(14) << mCustomerName
         << setw(14) << mItem << setw(10) << mCategory << setw(10) << priorityToString(mPriority)
         << setw(12) << statusToString(mFinalStatus) << setw(16) << (mKitchenName.empty() ? "---" : mKitchenName)
         << setw(12) << (mRiderName.empty() ? "---" : mRiderName) << setw(8) << (mRouteDistance >= 0 ? to_string(mRouteDistance) : "---")
         << setw(8) << (mEstimatedETA >= 0 ? (to_string(mEstimatedETA) + "m") : "---") << "\n";
}

// ==============================================================================
//  OrderHeap (Max-Heap) Implementation
// ==============================================================================
int OrderHeap::parent(int i) const { return (i - 1) / 2; }
int OrderHeap::leftChild(int i) const { return 2 * i + 1; }
int OrderHeap::rightChild(int i) const { return 2 * i + 2; }

bool OrderHeap::higherPriority(const Order &a, const Order &b) const {
    if (static_cast<int>(a.getPriority()) != static_cast<int>(b.getPriority()))
        return static_cast<int>(a.getPriority()) > static_cast<int>(b.getPriority());
    return a.getDeadline() < b.getDeadline();
}

void OrderHeap::swapAt(int i, int j) {
    Order tmp = mHeap[i];
    mHeap[i] = mHeap[j];
    mHeap[j] = tmp;
}

void OrderHeap::siftUp(int i) {
    while (i > 0 && higherPriority(mHeap[i], mHeap[parent(i)])) {
        swapAt(i, parent(i));
        i = parent(i);
    }
}

void OrderHeap::siftDown(int i) {
    int n = mHeap.size();
    while (true) {
        int best = i, l = leftChild(i), r = rightChild(i);
        if (l < n && higherPriority(mHeap[l], mHeap[best])) best = l;
        if (r < n && higherPriority(mHeap[r], mHeap[best])) best = r;
        if (best == i) break;
        swapAt(i, best);
        i = best;
    }
}

void OrderHeap::insert(const Order &o) { mHeap.push(o); siftUp(mHeap.size() - 1); }
Order OrderHeap::extractTop() {
    if (mHeap.empty()) return Order();
    Order top = mHeap[0];
    mHeap[0] = mHeap[mHeap.size() - 1];
    mHeap.removeLast();
    if (!mHeap.empty()) siftDown(0);
    return top;
}

bool OrderHeap::cancelOrder(int orderId) {
    for (int i = 0; i < mHeap.size(); ++i) {
        if (mHeap[i].getId() == orderId) {
            mHeap[i] = mHeap[mHeap.size() - 1];
            mHeap.removeLast();
            if (i < mHeap.size()) { siftUp(i); siftDown(i); }
            return true;
        }
    }
    return false;
}

Order *OrderHeap::findOrder(int orderId) {
    for (int i = 0; i < mHeap.size(); ++i)
        if (mHeap[i].getId() == orderId) return &mHeap[i];
    return nullptr;
}

bool OrderHeap::updatePriority(int orderId, Priority newPri) {
    for (int i = 0; i < mHeap.size(); ++i) {
        if (mHeap[i].getId() == orderId) {
            mHeap[i].setPriority(newPri);
            siftUp(i); siftDown(i);
            return true;
        }
    }
    return false;
}

bool OrderHeap::markDelayed(int orderId) {
    for (int i = 0; i < mHeap.size(); ++i) {
        if (mHeap[i].getId() == orderId) {
            mHeap[i].setDelayed(true);
            return true;
        }
    }
    return false;
}

Array<Order> OrderHeap::getDelayedOrders() const {
    Array<Order> res;
    for (int i = 0; i < mHeap.size(); ++i)
        if (mHeap[i].isDelayed()) res.push(mHeap[i]);
    return res;
}

Order OrderHeap::peekTop() const { return mHeap.empty() ? Order() : mHeap[0]; }
int OrderHeap::getSize() const { return mHeap.size(); }
bool OrderHeap::isEmpty() const { return mHeap.empty(); }

void OrderHeap::display() const {
    if (mHeap.empty()) { cout << "  (order queue is empty)\n"; return; }
    cout << "  " << left << setw(7) << "ID" << setw(16) << "Customer"
         << setw(16) << "Item" << setw(10) << "Category" << setw(10) << "Priority"
         << setw(10) << "Deadline" << setw(10) << "PrepTime" << setw(10) << "Status"
         << setw(8) << "Delayed" << "\n";
    cout << "  " << string(95, '-') << "\n";
    for (int i = 0; i < mHeap.size(); ++i) {
        const Order &o = mHeap[i];
        cout << "  " << left << setw(7) << o.getId() << setw(16) << o.getCustomerName()
             << setw(16) << o.getItem() << setw(10) << o.getCategory()
             << setw(10) << priorityToString(o.getPriority())
             << setw(10) << (to_string(o.getDeadline()) + "m")
             << setw(10) << (to_string(o.getPrepTime()) + "m")
             << setw(10) << statusToString(o.getStatus())
             << setw(8) << (o.isDelayed() ? "Yes" : "No") << "\n";
    }
}

Array<Order> OrderHeap::getAllOrders() const {
    Array<Order> res;
    for (int i = 0; i < mHeap.size(); ++i) res.push(mHeap[i]);
    return res;
}

// ==============================================================================
//  CityGraph Implementation
// ==============================================================================
CityGraph::CityGraph() : mNodeCount(0) {
    for (int i = 0; i < MAX_NODES; ++i)
        for (int j = 0; j < MAX_NODES; ++j)
            mAdj[i][j] = (i == j) ? 0 : INF, mBlocked[i][j] = false;
}
int CityGraph::getNodeCount() const { return mNodeCount; }
bool CityGraph::isValidNode(int n) const { return n >= 0 && n < mNodeCount; }
const string &CityGraph::getLocationName(int idx) const {
    static const string unknown = "???";
    return (idx >= 0 && idx < mNodeCount) ? mNames[idx] : unknown;
}
bool CityGraph::addLocation(const string &name) {
    if (mNodeCount >= MAX_NODES) return false;
    mNames[mNodeCount++] = name;
    return true;
}
bool CityGraph::addRoad(int u, int v, int weight) {
    if (!isValidNode(u) || !isValidNode(v) || weight <= 0) return false;
    mAdj[u][v] = mAdj[v][u] = weight;
    return true;
}
bool CityGraph::blockRoad(int u, int v) {
    if (!isValidNode(u) || !isValidNode(v)) return false;
    mBlocked[u][v] = mBlocked[v][u] = true;
    return true;
}
bool CityGraph::unblockRoad(int u, int v) {
    if (!isValidNode(u) || !isValidNode(v)) return false;
    mBlocked[u][v] = mBlocked[v][u] = false;
    return true;
}
bool CityGraph::isBlocked(int u, int v) const {
    if (!isValidNode(u) || !isValidNode(v)) return false;
    return mBlocked[u][v];
}
int CityGraph::getRoadWeight(int u, int v) const {
    if (!isValidNode(u) || !isValidNode(v)) return -1;
    return (mAdj[u][v] >= INF) ? -1 : mAdj[u][v];
}

int CityGraph::findShortestPath(int src, int dest, Array<int> &path) const {
    if (!isValidNode(src) || !isValidNode(dest)) return -1;
    int dist[MAX_NODES], prev[MAX_NODES];
    bool visited[MAX_NODES];
    for (int i = 0; i < mNodeCount; ++i) dist[i] = INF, prev[i] = -1, visited[i] = false;
    dist[src] = 0;
    for (int iter = 0; iter < mNodeCount; ++iter) {
        int u = -1;
        for (int i = 0; i < mNodeCount; ++i)
            if (!visited[i] && (u == -1 || dist[i] < dist[u])) u = i;
        if (u == -1 || dist[u] >= INF) break;
        visited[u] = true;
        for (int v = 0; v < mNodeCount; ++v) {
            if (mAdj[u][v] < INF && !mBlocked[u][v] && !visited[v]) {
                int nd = dist[u] + mAdj[u][v];
                if (nd < dist[v]) dist[v] = nd, prev[v] = u;
            }
        }
    }
    if (dist[dest] >= INF) return -1;
    Array<int> rev;
    for (int at = dest; at != -1; at = prev[at]) rev.push(at);
    for (int i = rev.size() - 1; i >= 0; --i) path.push(rev[i]);
    return dist[dest];
}

int CityGraph::shortestDistance(int src, int dest) const {
    if (!isValidNode(src) || !isValidNode(dest)) return -1;
    int dist[MAX_NODES];
    bool visited[MAX_NODES];
    for (int i = 0; i < mNodeCount; ++i) dist[i] = INF, visited[i] = false;
    dist[src] = 0;
    for (int iter = 0; iter < mNodeCount; ++iter) {
        int u = -1;
        for (int i = 0; i < mNodeCount; ++i)
            if (!visited[i] && (u == -1 || dist[i] < dist[u])) u = i;
        if (u == -1 || dist[u] >= INF) break;
        visited[u] = true;
        for (int v = 0; v < mNodeCount; ++v) {
            if (mAdj[u][v] < INF && !mBlocked[u][v] && !visited[v]) {
                if (dist[u] + mAdj[u][v] < dist[v]) dist[v] = dist[u] + mAdj[u][v];
            }
        }
    }
    return (dist[dest] >= INF) ? -1 : dist[dest];
}

void CityGraph::compareRoutes(int src, int dest1, int dest2) const {
    Array<int> p1, p2;
    int d1 = findShortestPath(src, dest1, p1);
    int d2 = findShortestPath(src, dest2, p2);
    UI::subHeader("Route Comparison");
    cout << "  From: " << getLocationName(src) << "\n\n";
    cout << "  Route A -> " << getLocationName(dest1) << ": ";
    if (d1 >= 0) {
        cout << d1 << " units  [";
        for (int i = 0; i < p1.size(); ++i) {
            if (i > 0) cout << " -> ";
            cout << getLocationName(p1[i]);
        }
        cout << "]\n";
    } else cout << "UNREACHABLE\n";
    cout << "  Route B -> " << getLocationName(dest2) << ": ";
    if (d2 >= 0) {
        cout << d2 << " units  [";
        for (int i = 0; i < p2.size(); ++i) {
            if (i > 0) cout << " -> ";
            cout << getLocationName(p2[i]);
        }
        cout << "]\n";
    } else cout << "UNREACHABLE\n";
    if (d1 >= 0 && d2 >= 0) {
        if (d1 < d2) cout << "\n  => Route A is shorter by " << (d2 - d1) << " units.\n";
        else if (d2 < d1) cout << "\n  => Route B is shorter by " << (d1 - d2) << " units.\n";
        else cout << "\n  => Both routes are equal distance.\n";
    }
}

int CityGraph::estimateDeliveryCost(int src, int dest, int ratePerUnit) const {
    int dist = shortestDistance(src, dest);
    if (dist < 0) return -1;
    return dist * ratePerUnit;
}

void CityGraph::displayMap() const {
    cout << "\n  Locations:\n";
    for (int i = 0; i < mNodeCount; ++i) cout << "    [" << i << "] " << mNames[i] << "\n";
    cout << "\n  Adjacency Matrix (--- = no road, X = blocked):\n        ";
    for (int i = 0; i < mNodeCount; ++i) cout << left << setw(8) << i;
    cout << "\n";
    for (int i = 0; i < mNodeCount; ++i) {
        cout << "  " << left << setw(5) << i;
        for (int j = 0; j < mNodeCount; ++j) {
            if (mAdj[i][j] >= INF) cout << setw(8) << "---";
            else {
                string val = to_string(mAdj[i][j]);
                if (mBlocked[i][j]) val += "X";
                cout << setw(8) << val;
            }
        }
        cout << "\n";
    }
}

// ==============================================================================
//  HashMap Implementation
// ==============================================================================
template <typename V>
HashMap<V>::Node::Node(int k, const V &v, Node *n) : key(k), value(v), next(n) {}

template <typename V>
HashMap<V>::HashMap() : mCount(0) {
    for (int i = 0; i < BUCKET_COUNT; ++i) mBuckets[i] = nullptr;
}
template <typename V>
HashMap<V>::~HashMap() { clear(); }
template <typename V>
int HashMap<V>::hashFunc(int key) const { return ((key % BUCKET_COUNT) + BUCKET_COUNT) % BUCKET_COUNT; }
template <typename V>
int HashMap<V>::getCount() const { return mCount; }
template <typename V>
void HashMap<V>::insert(int key, const V &value) {
    int idx = hashFunc(key);
    Node *cur = mBuckets[idx];
    while (cur) {
        if (cur->key == key) { cur->value = value; return; }
        cur = cur->next;
    }
    mBuckets[idx] = new Node(key, value, mBuckets[idx]);
    ++mCount;
}
template <typename V>
V *HashMap<V>::find(int key) {
    int idx = hashFunc(key);
    Node *cur = mBuckets[idx];
    while (cur) { if (cur->key == key) return &cur->value; cur = cur->next; }
    return nullptr;
}
template <typename V>
const V *HashMap<V>::find(int key) const {
    int idx = hashFunc(key);
    Node *cur = mBuckets[idx];
    while (cur) { if (cur->key == key) return &cur->value; cur = cur->next; }
    return nullptr;
}
template <typename V>
bool HashMap<V>::remove(int key) {
    int idx = hashFunc(key);
    Node *prev = nullptr, *cur = mBuckets[idx];
    while (cur) {
        if (cur->key == key) {
            if (prev) prev->next = cur->next;
            else mBuckets[idx] = cur->next;
            delete cur;
            --mCount;
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}
template <typename V>
Array<V> HashMap<V>::getAllValues() const {
    Array<V> res;
    for (int i = 0; i < BUCKET_COUNT; ++i) {
        Node *cur = mBuckets[i];
        while (cur) { res.push(cur->value); cur = cur->next; }
    }
    return res;
}
template <typename V>
void HashMap<V>::clear() {
    for (int i = 0; i < BUCKET_COUNT; ++i) {
        Node *cur = mBuckets[i];
        while (cur) { Node *next = cur->next; delete cur; cur = next; }
        mBuckets[i] = nullptr;
    }
    mCount = 0;
}
// Explicit instantiation
template class HashMap<OrderRecord>;
template class HashMap<OrderTimeline>;

// ==============================================================================
//  RiderHeap (Min-Heap) Implementation
// ==============================================================================
int RiderHeap::parent(int i) const { return (i - 1) / 2; }
int RiderHeap::leftChild(int i) const { return 2 * i + 1; }
int RiderHeap::rightChild(int i) const { return 2 * i + 2; }
void RiderHeap::swapAt(int i, int j) { Rider tmp = mHeap[i]; mHeap[i] = mHeap[j]; mHeap[j] = tmp; }
void RiderHeap::siftUp(int i) {
    while (i > 0 && mHeap[i].mCurrentLoad < mHeap[parent(i)].mCurrentLoad) {
        swapAt(i, parent(i));
        i = parent(i);
    }
}
void RiderHeap::siftDown(int i) {
    int n = mHeap.size();
    while (true) {
        int best = i, l = leftChild(i), r = rightChild(i);
        if (l < n && mHeap[l].mCurrentLoad < mHeap[best].mCurrentLoad) best = l;
        if (r < n && mHeap[r].mCurrentLoad < mHeap[best].mCurrentLoad) best = r;
        if (best == i) break;
        swapAt(i, best);
        i = best;
    }
}
void RiderHeap::addRider(const Rider &r) { mHeap.push(r); siftUp(mHeap.size() - 1); }
Rider RiderHeap::assignBestRider() {
    if (mHeap.empty()) return Rider();
    int bestIdx = -1;
    for (int i = 0; i < mHeap.size(); ++i)
        if (mHeap[i].canTakeOrder()) { bestIdx = i; break; }
    if (bestIdx == -1) return Rider();
    mHeap[bestIdx].incrementLoad();
    Rider assigned = mHeap[bestIdx];
    siftDown(bestIdx);
    return assigned;
}
Rider RiderHeap::assignBestRiderWithRoute(const CityGraph &city, int destNode) {
    if (mHeap.empty()) return Rider();
    int bestIdx = -1, bestScore = INT_MAX;
    for (int i = 0; i < mHeap.size(); ++i) {
        if (!mHeap[i].canTakeOrder()) continue;
        int dist = city.shortestDistance(mHeap[i].getLocationNode(), destNode);
        if (dist < 0) dist = 9999;
        int score = mHeap[i].getCurrentLoad() * 100 + dist;
        if (bestIdx == -1 || score < bestScore) bestIdx = i, bestScore = score;
    }
    if (bestIdx == -1) return Rider();
    mHeap[bestIdx].incrementLoad();
    Rider assigned = mHeap[bestIdx];
    siftDown(bestIdx);
    return assigned;
}
bool RiderHeap::completeDelivery(int riderId) {
    for (int i = 0; i < mHeap.size(); ++i) {
        if (mHeap[i].getId() == riderId) {
            if (!mHeap[i].decrementLoad()) return false;
            siftUp(i);
            return true;
        }
    }
    return false;
}
bool RiderHeap::toggleAvailability(int riderId) {
    for (int i = 0; i < mHeap.size(); ++i) {
        if (mHeap[i].getId() == riderId) {
            mHeap[i].setAvailable(!mHeap[i].isAvailable());
            return true;
        }
    }
    return false;
}
int RiderHeap::getSize() const { return mHeap.size(); }
void RiderHeap::display() const {
    if (mHeap.empty()) { cout << "  (no riders registered)\n"; return; }
    cout << "  " << left << setw(6) << "ID" << setw(16) << "Name" << setw(10) << "Load"
         << setw(10) << "Capacity" << setw(12) << "Available" << setw(12) << "Location"
         << setw(12) << "Delivered" << "\n";
    cout << "  " << string(78, '-') << "\n";
    for (int i = 0; i < mHeap.size(); ++i) {
        const Rider &r = mHeap[i];
        cout << "  " << left << setw(6) << r.getId() << setw(16) << r.getName()
             << setw(10) << (to_string(r.getCurrentLoad()) + "/" + to_string(r.getCapacity()))
             << setw(10) << r.getCapacity() << setw(12) << (r.isAvailable() ? "Yes" : "No")
             << setw(12) << ("Node " + to_string(r.getLocationNode()))
             << setw(12) << r.getTotalDeliveries() << "\n";
    }
}
Array<Rider> RiderHeap::getAllRiders() const {
    Array<Rider> res;
    for (int i = 0; i < mHeap.size(); ++i) res.push(mHeap[i]);
    return res;
}

// ==============================================================================
//  KitchenQueue & KitchenSystem Implementation
// ==============================================================================
KitchenQueue::Node::Node(int id, const string &it, int pt) : orderId(id), item(it), prepTime(pt), next(nullptr) {}
KitchenQueue::KitchenQueue() : mFront(nullptr), mRear(nullptr), mSize(0), mMaxCapacity(5) {}
KitchenQueue::~KitchenQueue() { clear(); }
KitchenQueue::KitchenQueue(const KitchenQueue &other) : mFront(nullptr), mRear(nullptr), mSize(0), mMaxCapacity(5) { copyFrom(other); }
KitchenQueue &KitchenQueue::operator=(const KitchenQueue &other) {
    if (this != &other) { clear(); copyFrom(other); }
    return *this;
}
void KitchenQueue::clear() { while (mFront) { Node *t = mFront; mFront = mFront->next; delete t; } mFront = mRear = nullptr; mSize = 0; }
void KitchenQueue::copyFrom(const KitchenQueue &other) {
    mMaxCapacity = other.mMaxCapacity;
    Node *cur = other.mFront;
    while (cur) { enqueue(cur->orderId, cur->item, cur->prepTime); cur = cur->next; }
}
int KitchenQueue::getSize() const { return mSize; }
int KitchenQueue::getMaxCapacity() const { return mMaxCapacity; }
bool KitchenQueue::isFull() const { return mSize >= mMaxCapacity; }
bool KitchenQueue::isEmpty() const { return mSize == 0; }
void KitchenQueue::setMaxCapacity(int cap) { mMaxCapacity = cap; }
int KitchenQueue::totalWaitTime() const { int t = 0; Node *c = mFront; while (c) { t += c->prepTime; c = c->next; } return t; }
void KitchenQueue::enqueue(int orderId, const string &item, int prepTime) {
    Node *n = new Node(orderId, item, prepTime);
    if (!mRear) mFront = mRear = n;
    else mRear->next = n, mRear = n;
    ++mSize;
}
int KitchenQueue::dequeue() {
    if (!mFront) return -1;
    int id = mFront->orderId;
    Node *t = mFront;
    mFront = mFront->next;
    if (!mFront) mRear = nullptr;
    delete t;
    --mSize;
    return id;
}
int KitchenQueue::peekFront() const { return mFront ? mFront->orderId : -1; }
void KitchenQueue::display(const string &kitchenName) const {
    string bar = "[";
    for (int i = 0; i < mMaxCapacity; ++i) bar += (i < mSize) ? "#" : ".";
    bar += "]";
    cout << "  " << left << setw(18) << kitchenName << " " << bar << "  " << mSize << "/" << mMaxCapacity
         << "  Wait: " << setw(4) << (to_string(totalWaitTime()) + "m") << (isFull() ? "  ** OVERLOADED **" : "") << "\n";
    Node *c = mFront;
    int p = 1;
    while (c) {
        cout << "      " << p++ << ". Order #" << c->orderId << "  " << left << setw(16) << c->item
             << " (" << c->prepTime << " min)\n";
        c = c->next;
    }
}

KitchenSystem::KitchenSystem() : mCount(0) {}
int KitchenSystem::getNumKitchens() const { return mCount; }
const string &KitchenSystem::getKitchenName(int idx) const { static const string unknown = "???"; return (idx >= 0 && idx < mCount) ? mNames[idx] : unknown; }
bool KitchenSystem::isValidKitchen(int idx) const { return idx >= 0 && idx < mCount; }
bool KitchenSystem::addKitchen(const string &name, int cap) {
    if (mCount >= MAX_KITCHENS) return false;
    mNames[mCount] = name;
    mKitchens[mCount].setMaxCapacity(cap);
    ++mCount;
    return true;
}
int KitchenSystem::distributeOrder(int orderId, const string &item, int prepTime) {
    int best = -1;
    for (int i = 0; i < mCount; ++i)
        if (!mKitchens[i].isFull() && (best == -1 || mKitchens[i].getSize() < mKitchens[best].getSize()))
            best = i;
    if (best == -1) return -1;
    mKitchens[best].enqueue(orderId, item, prepTime);
    return best;
}
int KitchenSystem::completeOrder(int kitchenIdx) {
    if (!isValidKitchen(kitchenIdx) || mKitchens[kitchenIdx].isEmpty()) return -1;
    return mKitchens[kitchenIdx].dequeue();
}
int KitchenSystem::getKitchenWaitTime(int idx) const { return isValidKitchen(idx) ? mKitchens[idx].totalWaitTime() : 0; }
int KitchenSystem::getKitchenLoad(int idx) const { return isValidKitchen(idx) ? mKitchens[idx].getSize() : 0; }
void KitchenSystem::detectOverloaded() const {
    bool found = false;
    for (int i = 0; i < mCount; ++i) {
        if (mKitchens[i].isFull()) {
            if (!found) UI::subHeader("Overloaded Kitchens Detected"), found = true;
            cout << "  [!!] " << mNames[i] << " is at full capacity ("
                 << mKitchens[i].getSize() << "/" << mKitchens[i].getMaxCapacity()
                 << "), wait time: " << mKitchens[i].totalWaitTime() << " min\n";
        }
    }
    if (!found) UI::success("No kitchens are overloaded.");
}
bool KitchenSystem::rebalance() {
    if (mCount < 2) return false;
    int maxIdx = 0, minIdx = 0;
    for (int i = 1; i < mCount; ++i) {
        if (mKitchens[i].getSize() > mKitchens[maxIdx].getSize()) maxIdx = i;
        if (mKitchens[i].getSize() < mKitchens[minIdx].getSize()) minIdx = i;
    }
    if (maxIdx == minIdx || mKitchens[maxIdx].getSize() - mKitchens[minIdx].getSize() <= 1) return false;
    if (mKitchens[minIdx].isFull()) return false;
    int orderId = mKitchens[maxIdx].dequeue();
    if (orderId < 0) return false;
    mKitchens[minIdx].enqueue(orderId, "Rebalanced", 10);
    return true;
}
int KitchenSystem::estimateWaitTime() const {
    int minWait = INT_MAX;
    for (int i = 0; i < mCount; ++i)
        if (!mKitchens[i].isFull() && mKitchens[i].totalWaitTime() < minWait)
            minWait = mKitchens[i].totalWaitTime();
    return (minWait == INT_MAX) ? -1 : minWait;
}
void KitchenSystem::displayAll() const {
    if (mCount == 0) { cout << "  (no kitchens configured)\n"; return; }
    for (int i = 0; i < mCount; ++i) { cout << "  [" << i << "] "; mKitchens[i].display(mNames[i]); }
}

// ==============================================================================
//  SearchEngine Implementation
// ==============================================================================
bool SearchEngine::containsIgnoreCase(const string &haystack, const string &needle) {
    if (needle.empty()) return true;
    if (needle.size() > haystack.size()) return false;
    for (size_t i = 0; i <= haystack.size() - needle.size(); ++i) {
        bool match = true;
        for (size_t j = 0; j < needle.size(); ++j)
            if (tolower(static_cast<unsigned char>(haystack[i + j])) != tolower(static_cast<unsigned char>(needle[j]))) {
                match = false; break;
            }
        if (match) return true;
    }
    return false;
}
void SearchEngine::indexOrder(const OrderRecord &record) { mIndex.insert(record.getId(), record); mAllRecords.push(record); }
int SearchEngine::getCount() const { return mIndex.getCount(); }
OrderRecord *SearchEngine::findById(int orderId) { return mIndex.find(orderId); }
Array<OrderRecord> SearchEngine::searchByCustomer(const string &name) const {
    Array<OrderRecord> res;
    for (int i = 0; i < mAllRecords.size(); ++i)
        if (containsIgnoreCase(mAllRecords[i].getCustomerName(), name)) res.push(mAllRecords[i]);
    return res;
}
Array<OrderRecord> SearchEngine::searchByItem(const string &item) const {
    Array<OrderRecord> res;
    for (int i = 0; i < mAllRecords.size(); ++i)
        if (containsIgnoreCase(mAllRecords[i].getItem(), item)) res.push(mAllRecords[i]);
    return res;
}
Array<OrderRecord> SearchEngine::searchByStatus(OrderStatus status) const {
    Array<OrderRecord> res;
    for (int i = 0; i < mAllRecords.size(); ++i)
        if (mAllRecords[i].getFinalStatus() == status) res.push(mAllRecords[i]);
    return res;
}
Array<OrderRecord> SearchEngine::searchByPriority(Priority priority) const {
    Array<OrderRecord> res;
    for (int i = 0; i < mAllRecords.size(); ++i)
        if (mAllRecords[i].getPriority() == priority) res.push(mAllRecords[i]);
    return res;
}
Array<OrderRecord> SearchEngine::searchByCategory(const string &cat) const {
    Array<OrderRecord> res;
    for (int i = 0; i < mAllRecords.size(); ++i)
        if (containsIgnoreCase(mAllRecords[i].getCategory(), cat)) res.push(mAllRecords[i]);
    return res;
}
Array<OrderRecord> SearchEngine::searchByKitchen(const string &kitchen) const {
    Array<OrderRecord> res;
    for (int i = 0; i < mAllRecords.size(); ++i)
        if (containsIgnoreCase(mAllRecords[i].getKitchenName(), kitchen)) res.push(mAllRecords[i]);
    return res;
}
Array<OrderRecord> SearchEngine::searchByRider(const string &rider) const {
    Array<OrderRecord> res;
    for (int i = 0; i < mAllRecords.size(); ++i)
        if (containsIgnoreCase(mAllRecords[i].getRiderName(), rider)) res.push(mAllRecords[i]);
    return res;
}
const Array<OrderRecord> &SearchEngine::getAllRecords() const { return mAllRecords; }
void SearchEngine::displayResultsHeader() {
    cout << "  " << left << setw(7) << "ID" << setw(14) << "Customer" << setw(14) << "Item"
         << setw(10) << "Category" << setw(10) << "Priority" << setw(12) << "Status"
         << setw(16) << "Kitchen" << setw(12) << "Rider" << setw(8) << "Dist" << setw(8) << "ETA" << "\n";
    cout << "  " << string(109, '-') << "\n";
}
void SearchEngine::displayResults(const Array<OrderRecord> &results) {
    if (results.empty()) { cout << "  No matching records found.\n"; return; }
    displayResultsHeader();
    for (int i = 0; i < results.size(); ++i) results[i].display();
    cout << "\n  Found " << results.size() << " record(s).\n";
}
void SearchEngine::displayAll() const { displayResults(mAllRecords); }

// ==============================================================================
//  OrderTracker Implementation
// ==============================================================================
StateTransition::StateTransition() : fromState(OrderStatus::QUEUED), toState(OrderStatus::QUEUED), timestamp(0) {}
StateTransition::StateTransition(OrderStatus from, OrderStatus to, int ts, const string &desc)
    : fromState(from), toState(to), timestamp(ts), description(desc) {}
OrderTimeline::OrderTimeline() : orderId(0) {}
OrderTimeline::OrderTimeline(int id, const string &cust, const string &it) : orderId(id), customerName(cust), item(it) {}
void OrderTimeline::addTransition(const StateTransition &t) { transitions.push(t); }
OrderStatus OrderTimeline::currentState() const { return transitions.empty() ? OrderStatus::QUEUED : transitions[transitions.size() - 1].toState; }
void OrderTimeline::replay() const {
    cout << "\n  Order #" << orderId << " | " << customerName << " | " << item
         << "\n  " << string(65, '-') << "\n";
    if (transitions.empty()) { cout << "  (no state transitions recorded)\n"; return; }
    for (int i = 0; i < transitions.size(); ++i) {
        const StateTransition &t = transitions[i];
        cout << "  [T=" << left << setw(4) << t.timestamp << "]  "
             << setw(12) << statusToString(t.fromState) << " --> "
             << setw(12) << statusToString(t.toState) << " : " << t.description << "\n";
    }
    cout << "  " << string(65, '-') << "\n";
    cout << "  Current State: " << statusToString(currentState()) << "\n";
}
UndoAction::UndoAction() : type(ActionType::ORDER_ADDED), timestamp(0) {}
UndoAction::UndoAction(ActionType t, const Order &snap, const string &desc, int ts)
    : type(t), snapshot(snap), description(desc), timestamp(ts) {}
OrderTracker::OrderTracker() : mClock(0) {}
int OrderTracker::getClock() const { return mClock; }
void OrderTracker::recordOrderCreated(const Order &order) {
    OrderTimeline tl(order.getId(), order.getCustomerName(), order.getItem());
    tl.addTransition(StateTransition(OrderStatus::QUEUED, OrderStatus::QUEUED, mClock++, "Order created and queued"));
    mTimelines.insert(order.getId(), tl);
    mUndoStack.push(UndoAction(ActionType::ORDER_ADDED, order, "Added Order #" + to_string(order.getId()), mClock));
}
void OrderTracker::recordStateChange(int orderId, OrderStatus from, OrderStatus to, const string &desc) {
    OrderTimeline *tl = mTimelines.find(orderId);
    if (tl) tl->addTransition(StateTransition(from, to, mClock++, desc));
}
void OrderTracker::recordOrderCancelled(const Order &order) {
    recordStateChange(order.getId(), order.getStatus(), OrderStatus::CANCELLED, "Order cancelled");
    mUndoStack.push(UndoAction(ActionType::ORDER_CANCELLED, order, "Cancelled Order #" + to_string(order.getId()), mClock));
}
void OrderTracker::recordOrderProcessed(const Order &order) {
    mUndoStack.push(UndoAction(ActionType::ORDER_PROCESSED, order, "Processed Order #" + to_string(order.getId()), mClock));
}
bool OrderTracker::undoLastAction(OrderHeap &orderQueue) {
    if (mUndoStack.empty()) return false;
    UndoAction action = mUndoStack[mUndoStack.size() - 1];
    mUndoStack.removeLast();
    if (action.type == ActionType::ORDER_ADDED) {
        orderQueue.cancelOrder(action.snapshot.getId());
        mTimelines.remove(action.snapshot.getId());
        UI::success("Undone: " + action.description + " (removed from queue)");
    } else if (action.type == ActionType::ORDER_CANCELLED || action.type == ActionType::ORDER_PROCESSED) {
        orderQueue.insert(action.snapshot);
        OrderTimeline *tl = mTimelines.find(action.snapshot.getId());
        if (tl) {
            OrderStatus prevState = (action.type == ActionType::ORDER_CANCELLED) ? OrderStatus::CANCELLED : OrderStatus::DISPATCHED;
            tl->addTransition(StateTransition(prevState, action.snapshot.getStatus(), mClock++, "Undo: restored order"));
        }
        UI::success("Undone: " + action.description + " (restored to queue)");
    }
    return true;
}
bool OrderTracker::replayTimeline(int orderId) const {
    const OrderTimeline *tl = mTimelines.find(orderId);
    if (!tl) return false;
    tl->replay();
    return true;
}
void OrderTracker::displayAllTimelines() const {
    Array<OrderTimeline> all = mTimelines.getAllValues();
    if (all.empty()) { cout << "  (no orders tracked)\n"; return; }
    cout << "  " << left << setw(7) << "ID" << setw(16) << "Customer" << setw(16) << "Item"
         << setw(14) << "Current State" << setw(14) << "Transitions" << "\n";
    cout << "  " << string(67, '-') << "\n";
    for (int i = 0; i < all.size(); ++i) {
        cout << "  " << left << setw(7) << all[i].orderId << setw(16) << all[i].customerName
             << setw(16) << all[i].item << setw(14) << statusToString(all[i].currentState())
             << setw(14) << all[i].transitions.size() << "\n";
    }
}
void OrderTracker::displayUndoStack() const {
    if (mUndoStack.empty()) { cout << "  (undo stack is empty)\n"; return; }
    cout << "  " << left << setw(5) << "#" << setw(18) << "Action" << setw(8) << "Ord.ID"
         << setw(30) << "Description" << setw(6) << "T" << "\n";
    cout << "  " << string(67, '-') << "\n";
    for (int i = mUndoStack.size() - 1; i >= 0; --i) {
        const UndoAction &a = mUndoStack[i];
        cout << "  " << left << setw(5) << (mUndoStack.size() - i) << setw(18) << actionToString(a.type)
             << setw(8) << a.snapshot.getId() << setw(30) << a.description << setw(6) << a.timestamp << "\n";
    }
}
int OrderTracker::getUndoCount() const { return mUndoStack.size(); }
int OrderTracker::getTrackedCount() const { return mTimelines.getCount(); }

// ==============================================================================
//  DeliveryPipeline Implementation
// ==============================================================================
DeliveryPipeline::DeliveryPipeline(OrderHeap &oq, RiderHeap &rp, CityGraph &cg,
                                   KitchenSystem &ks, SearchEngine &se, OrderTracker &tr)
    : mOrderQueue(oq), mRiderPool(rp), mCity(cg), mKitchens(ks), mSearchEngine(se), mTracker(tr) {}

bool DeliveryPipeline::processNextOrder(int destNode) {
    if (mOrderQueue.isEmpty()) { UI::warning("No orders in the scheduling queue."); return false; }
    Order order = mOrderQueue.extractTop();
    cout << "\n"; UI::header("Processing Order #" + to_string(order.getId()));
    cout << "  Customer : " << order.getCustomerName() << "\n";
    cout << "  Item     : " << order.getItem() << "\n";
    cout << "  Category : " << order.getCategory() << "\n";
    cout << "  Priority : " << priorityToString(order.getPriority()) << "\n";
    cout << "  PrepTime : " << order.getPrepTime() << " min\n";
    cout << "  Deadline : " << order.getDeadline() << " min\n";

    string kitchenName = "", riderName = "";
    int routeDist = -1, eta = -1;

    UI::subHeader("Step 1: Kitchen Assignment");
    int kitchenIdx = mKitchens.distributeOrder(order.getId(), order.getItem(), order.getPrepTime());
    if (kitchenIdx >= 0) {
        order.setAssignedKitchen(kitchenIdx);
        order.setStatus(OrderStatus::PREPARING);
        kitchenName = mKitchens.getKitchenName(kitchenIdx);
        mTracker.recordStateChange(order.getId(), OrderStatus::QUEUED, OrderStatus::PREPARING, "Assigned to " + kitchenName);
        UI::success("Assigned to " + kitchenName + " (wait: " + to_string(mKitchens.getKitchenWaitTime(kitchenIdx)) + " min)");
    } else UI::error("All kitchens are at full capacity!");

    UI::subHeader("Step 2: Rider Dispatch");
    Rider rider = mRiderPool.assignBestRiderWithRoute(mCity, destNode);
    if (rider.getId() != 0) {
        order.setAssignedRider(rider.getId());
        order.setStatus(OrderStatus::DISPATCHED);
        riderName = rider.getName();
        mTracker.recordStateChange(order.getId(), OrderStatus::PREPARING, OrderStatus::DISPATCHED, "Dispatched to rider " + riderName);
        UI::success("Dispatched to " + riderName + " (Load: " + to_string(rider.getCurrentLoad()) +
                    "/" + to_string(rider.getCapacity()) + ", Location: Node " + to_string(rider.getLocationNode()) + ")");
    } else UI::error("No riders available for dispatch!");

    UI::subHeader("Step 3: Route Optimization");
    if (rider.getId() != 0 && mCity.isValidNode(destNode)) {
        Array<int> path;
        routeDist = mCity.findShortestPath(rider.getLocationNode(), destNode, path);
        if (routeDist >= 0) {
            int kitchenWait = (kitchenIdx >= 0) ? mKitchens.getKitchenWaitTime(kitchenIdx) : 0;
            eta = routeDist + kitchenWait;
            cout << "  Route: ";
            for (int i = 0; i < path.size(); ++i) {
                if (i > 0) cout << " -> ";
                cout << mCity.getLocationName(path[i]);
            }
            cout << "\n  Distance: " << routeDist << " units\n";
            cout << "  Kitchen Wait: " << kitchenWait << " min\n";
            cout << "  Estimated ETA: " << eta << " min\n";
            int cost = mCity.estimateDeliveryCost(rider.getLocationNode(), destNode);
            if (cost >= 0) cout << "  Delivery Cost: Rs. " << cost << "\n";
            if (eta > order.getDeadline()) {
                UI::warning("ETA (" + to_string(eta) + " min) exceeds deadline (" + to_string(order.getDeadline()) + " min)!");
                order.setDelayed(true);
            }
        } else UI::error("No route available to destination!");
    }
    order.setStatus(OrderStatus::DISPATCHED);
    OrderRecord rec(order, kitchenName, riderName, routeDist, eta);
    mSearchEngine.indexOrder(rec);
    mTracker.recordOrderProcessed(order);
    UI::subHeader("Order #" + to_string(order.getId()) + " Processing Complete");
    return true;
}

// ==============================================================================
//  PerformanceAnalyzer Implementation
// ==============================================================================
double PerformanceAnalyzer::getTimeMs() { return (double)clock() / CLOCKS_PER_SEC * 1000.0; }
void PerformanceAnalyzer::benchmarkInsertion(int count) {
    UI::subHeader("Benchmark: Order Insertion (" + to_string(count) + " orders)");
    OrderHeap testHeap;
    double start = getTimeMs();
    for (int i = 1; i <= count; ++i) {
        Priority p = static_cast<Priority>((i % 3) + 1);
        testHeap.insert(Order(i, "Cust" + to_string(i), "Item" + to_string(i), "Regular", p, 10 + (i % 20), 30 + (i % 60)));
    }
    double elapsed = getTimeMs() - start;
    cout << "  Inserted " << count << " orders in " << fixed << setprecision(3) << elapsed << " ms\n";
    cout << "  Avg per insertion: " << fixed << setprecision(4) << (elapsed / count) << " ms\n";
}
void PerformanceAnalyzer::benchmarkDeletion(int count) {
    UI::subHeader("Benchmark: Order Extraction (" + to_string(count) + " orders)");
    OrderHeap testHeap;
    for (int i = 1; i <= count; ++i) testHeap.insert(Order(i, "C" + to_string(i), "I" + to_string(i), "Regular", static_cast<Priority>((i % 3) + 1), 10, 30));
    double start = getTimeMs();
    for (int i = 0; i < count; ++i) testHeap.extractTop();
    double elapsed = getTimeMs() - start;
    cout << "  Extracted " << count << " orders in " << fixed << setprecision(3) << elapsed << " ms\n";
    cout << "  Avg per extraction: " << fixed << setprecision(4) << (elapsed / count) << " ms\n";
}
void PerformanceAnalyzer::benchmarkSearching(int count) {
    UI::subHeader("Benchmark: Hash Map Search (" + to_string(count) + " lookups)");
    HashMap<OrderRecord> testMap;
    for (int i = 1; i <= count; ++i) testMap.insert(i, OrderRecord(Order(i, "C" + to_string(i), "I" + to_string(i), "Regular", static_cast<Priority>((i % 3) + 1), 10, 30)));
    double start = getTimeMs();
    int found = 0;
    for (int i = 1; i <= count; ++i) if (testMap.find(i)) ++found;
    double elapsed = getTimeMs() - start;
    cout << "  Searched " << count << " records in " << fixed << setprecision(3) << elapsed << " ms\n";
    cout << "  Found: " << found << "/" << count << "\n";
    cout << "  Avg per lookup: " << fixed << setprecision(4) << (elapsed / count) << " ms\n";
}
void PerformanceAnalyzer::benchmarkScheduling(int count) {
    UI::subHeader("Benchmark: Scheduling Cycle (" + to_string(count) + " insert+extract)");
    OrderHeap testHeap;
    double start = getTimeMs();
    for (int i = 1; i <= count; ++i) {
        testHeap.insert(Order(i, "C" + to_string(i), "I" + to_string(i), "Regular", static_cast<Priority>((i % 3) + 1), 10, 30));
        if (i % 2 == 0) testHeap.extractTop();
    }
    while (!testHeap.isEmpty()) testHeap.extractTop();
    double elapsed = getTimeMs() - start;
    cout << "  Completed " << count << " scheduling cycles in " << fixed << setprecision(3) << elapsed << " ms\n";
}
void PerformanceAnalyzer::benchmarkRouting() {
    UI::subHeader("Benchmark: Dijkstra Routing (20-node graph, 100 queries)");
    CityGraph testGraph;
    for (int i = 0; i < 20; ++i) testGraph.addLocation("N" + to_string(i));
    for (int i = 0; i < 19; ++i) testGraph.addRoad(i, i + 1, 5 + (i % 10));
    for (int i = 0; i < 15; ++i) testGraph.addRoad(i, (i + 3) % 20, 8 + (i % 7));
    int queries = 100;
    double start = getTimeMs();
    for (int q = 0; q < queries; ++q) { Array<int> path; testGraph.findShortestPath(q % 20, (q * 7 + 3) % 20, path); }
    double elapsed = getTimeMs() - start;
    cout << "  Ran " << queries << " shortest-path queries in " << fixed << setprecision(3) << elapsed << " ms\n";
    cout << "  Avg per query: " << fixed << setprecision(4) << (elapsed / queries) << " ms\n";
}
void PerformanceAnalyzer::benchmarkScalability() {
    UI::subHeader("Benchmark: Scalability Analysis");
    cout << "  " << left << setw(12) << "Dataset" << setw(15) << "Insert(ms)" << setw(15) << "Extract(ms)" << setw(15) << "Search(ms)" << "\n";
    cout << "  " << string(57, '-') << "\n";
    int sizes[] = {100, 500, 1000, 2000, 5000};
    for (int s = 0; s < 5; ++s) {
        int n = sizes[s];
        OrderHeap heap;
        double t1 = getTimeMs();
        for (int i = 1; i <= n; ++i) heap.insert(Order(i, "C" + to_string(i), "I" + to_string(i), "Regular", static_cast<Priority>((i % 3) + 1), 10, 30));
        double insTime = getTimeMs() - t1;
        double t2 = getTimeMs();
        for (int i = 0; i < n; ++i) heap.extractTop();
        double extTime = getTimeMs() - t2;
        HashMap<OrderRecord> map;
        for (int i = 1; i <= n; ++i) map.insert(i, OrderRecord(Order(i, "C" + to_string(i), "I" + to_string(i), "Regular", static_cast<Priority>((i % 3) + 1), 10, 30)));
        double t3 = getTimeMs();
        for (int i = 1; i <= n; ++i) map.find(i);
        double schTime = getTimeMs() - t3;
        cout << "  " << left << setw(12) << n << setw(15) << fixed << setprecision(3) << insTime
             << setw(15) << fixed << setprecision(3) << extTime << setw(15) << fixed << setprecision(3) << schTime << "\n";
    }
}
void PerformanceAnalyzer::estimateMemoryUsage(int orderCount, int riderCount, int kitchenCount, int graphNodes) {
    UI::subHeader("Memory Usage Estimation");
    long long orderMem = (long long)orderCount * sizeof(Order);
    long long riderMem = (long long)riderCount * sizeof(Rider);
    long long graphMem = (long long)graphNodes * graphNodes * (sizeof(int) + sizeof(bool));
    long long hashMem = (long long)127 * sizeof(void *) + (long long)orderCount * (sizeof(OrderRecord) + sizeof(void *) + sizeof(int));
    long long totalMem = orderMem + riderMem + graphMem + hashMem;
    cout << "  Orders (" << orderCount << "): ~" << orderMem << " bytes\n";
    cout << "  Riders (" << riderCount << "): ~" << riderMem << " bytes\n";
    cout << "  Graph (" << graphNodes << " nodes): ~" << graphMem << " bytes\n";
    cout << "  Hash Index: ~" << hashMem << " bytes\n";
    cout << "  " << string(40, '-') << "\n";
    cout << "  Total Estimated: ~" << totalMem << " bytes (" << fixed << setprecision(2) << (totalMem / 1024.0) << " KB)\n";
}
void PerformanceAnalyzer::runFullAnalysis() {
    UI::header("Performance Analysis Module");
    benchmarkInsertion(1000);
    benchmarkDeletion(1000);
    benchmarkSearching(1000);
    benchmarkScheduling(1000);
    benchmarkRouting();
    benchmarkScalability();
    estimateMemoryUsage(1000, 50, 10, 20);
    cout << "\n"; UI::success("Performance analysis complete.");
}

// ==============================================================================
//  FileHandler Implementation
// ==============================================================================
string FileHandler::trimLine(const string &s) {
    int end = (int)s.size() - 1;
    while (end >= 0 && (s[end] == '\r' || s[end] == '\n' || s[end] == ' ')) --end;
    return s.substr(0, end + 1);
}
int FileHandler::safeStoi(const string &s, int fallback) {
    try { string trimmed = trimLine(s); return trimmed.empty() ? fallback : stoi(trimmed); } catch (...) { return fallback; }
}

void FileHandler::saveOrders(const OrderHeap &heap, const string &filename) {
    ofstream file(filename);
    if (!file.is_open()) { UI::error("Cannot open " + filename + " for writing."); return; }
    file << "ID,CustomerName,Item,Category,Priority,PrepTime,Deadline,Status,Delayed\n";
    Array<Order> orders = heap.getAllOrders();
    for (int i = 0; i < orders.size(); ++i) {
        const Order &o = orders[i];
        file << o.getId() << "," << o.getCustomerName() << "," << o.getItem() << "," << o.getCategory() << ","
             << static_cast<int>(o.getPriority()) << "," << o.getPrepTime() << "," << o.getDeadline() << ","
             << static_cast<int>(o.getStatus()) << "," << (o.isDelayed() ? 1 : 0) << "\n";
    }
    file.close();
    UI::success("Orders saved to " + filename + " (" + to_string(orders.size()) + " records)");
}
int FileHandler::loadOrders(OrderHeap &heap, OrderTracker &tracker, const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) return 0;
    string line; getline(file, line); // header
    int count = 0;
    while (getline(file, line)) {
        line = trimLine(line);
        if (line.empty()) continue;
        try {
            stringstream ss(line);
            string tok, customer, item, category;
            getline(ss, tok, ','); int id = safeStoi(tok);
            getline(ss, customer, ',');
            getline(ss, item, ',');
            getline(ss, category, ',');
            getline(ss, tok, ','); int pri = safeStoi(tok, 1);
            getline(ss, tok, ','); int prep = safeStoi(tok, 10);
            getline(ss, tok, ','); int deadline = safeStoi(tok, 30);
            getline(ss, tok, ','); int status = safeStoi(tok, 0);
            getline(ss, tok); int delayed = safeStoi(tok, 0);
            Order o(id, customer, item, category, static_cast<Priority>(pri), prep, deadline);
            o.setStatus(static_cast<OrderStatus>(status));
            o.setDelayed(delayed == 1);
            heap.insert(o);
            tracker.recordOrderCreated(o);
            ++count;
        } catch (...) {}
    }
    file.close();
    return count;
}
void FileHandler::saveRiders(const RiderHeap &riders, const string &filename) {
    ofstream file(filename);
    if (!file.is_open()) { UI::error("Cannot open " + filename + " for writing."); return; }
    file << "ID,Name,Capacity,Location,Available,CurrentLoad,TotalDeliveries\n";
    Array<Rider> all = riders.getAllRiders();
    for (int i = 0; i < all.size(); ++i) {
        const Rider &r = all[i];
        file << r.getId() << "," << r.getName() << "," << r.getCapacity() << "," << r.getLocationNode() << ","
             << (r.isAvailable() ? 1 : 0) << "," << r.getCurrentLoad() << "," << r.getTotalDeliveries() << "\n";
    }
    file.close();
    UI::success("Riders saved to " + filename + " (" + to_string(all.size()) + " records)");
}
int FileHandler::loadRiders(RiderHeap &riders, const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) return 0;
    string line; getline(file, line);
    int count = 0;
    while (getline(file, line)) {
        line = trimLine(line);
        if (line.empty()) continue;
        try {
            stringstream ss(line);
            string tok, name;
            getline(ss, tok, ','); int id = safeStoi(tok);
            getline(ss, name, ',');
            getline(ss, tok, ','); int cap = safeStoi(tok, 3);
            getline(ss, tok, ','); int loc = safeStoi(tok, 0);
            getline(ss, tok, ','); int avail = safeStoi(tok, 1);
            getline(ss, tok, ','); int load = safeStoi(tok, 0);
            getline(ss, tok); int delivered = safeStoi(tok, 0);
            Rider r(id, name, cap, loc);
            if (!avail) r.setAvailable(false);
            for (int i = 0; i < load; ++i) r.incrementLoad();
            riders.addRider(r);
            ++count;
        } catch (...) {}
    }
    file.close();
    return count;
}
void FileHandler::saveRecords(const SearchEngine &engine, const string &filename) {
    ofstream file(filename);
    if (!file.is_open()) { UI::error("Cannot open " + filename + " for writing."); return; }
    file << "ID,Customer,Item,Category,Priority,Status,Kitchen,Rider,Distance,ETA\n";
    const Array<OrderRecord> &records = engine.getAllRecords();
    for (int i = 0; i < records.size(); ++i) {
        const OrderRecord &r = records[i];
        file << r.getId() << "," << r.getCustomerName() << "," << r.getItem() << "," << r.getCategory() << ","
             << static_cast<int>(r.getPriority()) << "," << static_cast<int>(r.getFinalStatus()) << ","
             << r.getKitchenName() << "," << r.getRiderName() << "," << r.getRouteDistance() << ","
             << r.getEstimatedETA() << "\n";
    }
    file.close();
    UI::success("Delivery records saved to " + filename + " (" + to_string(records.size()) + " records)");
}
int FileHandler::loadRecords(SearchEngine &engine, const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) return 0;
    string line; getline(file, line);
    int count = 0;
    while (getline(file, line)) {
        line = trimLine(line);
        if (line.empty()) continue;
        try {
            stringstream ss(line);
            string tok, customer, item, category, kitchen, rider;
            getline(ss, tok, ','); int id = safeStoi(tok);
            getline(ss, customer, ',');
            getline(ss, item, ',');
            getline(ss, category, ',');
            getline(ss, tok, ','); int pri = safeStoi(tok, 1);
            getline(ss, tok, ','); int stat = safeStoi(tok, 0);
            getline(ss, kitchen, ',');
            getline(ss, rider, ',');
            getline(ss, tok, ','); int dist = safeStoi(tok, -1);
            getline(ss, tok); int eta = safeStoi(tok, -1);
            OrderRecord rec(id, customer, item, category, static_cast<Priority>(pri),
                            static_cast<OrderStatus>(stat), kitchen, rider, dist, eta);
            engine.indexOrder(rec);
            ++count;
        } catch (...) {}
    }
    file.close();
    return count;
}
void FileHandler::saveGraph(const CityGraph &graph, const string &filename) {
    ofstream file(filename);
    if (!file.is_open()) { UI::error("Cannot open " + filename + " for writing."); return; }
    int n = graph.getNodeCount();
    file << "LOCATIONS," << n << "\n";
    for (int i = 0; i < n; ++i) file << i << "," << graph.getLocationName(i) << "\n";
    file << "ROADS\n";
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            int w = graph.getRoadWeight(i, j);
            if (w > 0) file << i << "," << j << "," << w << "," << (graph.isBlocked(i, j) ? 1 : 0) << "\n";
        }
    file.close();
    UI::success("City graph saved to " + filename);
}
bool FileHandler::loadGraph(CityGraph &graph, const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) return false;
    try {
        string line;
        getline(file, line); line = trimLine(line);
        stringstream hss(line);
        string tag; getline(hss, tag, ','); getline(hss, tag);
        int nodeCount = safeStoi(tag, 0);
        for (int i = 0; i < nodeCount; ++i) {
            getline(file, line); line = trimLine(line);
            stringstream ss(line); string idx, name;
            getline(ss, idx, ','); getline(ss, name);
            graph.addLocation(trimLine(name));
        }
        getline(file, line); // "ROADS"
        while (getline(file, line)) {
            line = trimLine(line);
            if (line.empty()) continue;
            stringstream ss(line);
            string tok;
            getline(ss, tok, ','); int u = safeStoi(tok);
            getline(ss, tok, ','); int v = safeStoi(tok);
            getline(ss, tok, ','); int w = safeStoi(tok);
            getline(ss, tok); int blocked = safeStoi(tok);
            graph.addRoad(u, v, w);
            if (blocked) graph.blockRoad(u, v);
        }
    } catch (...) {}
    file.close();
    return true;
}
void FileHandler::saveKitchens(const KitchenSystem &kitchens, const string &filename) {
    ofstream file(filename);
    if (!file.is_open()) { UI::error("Cannot open " + filename + " for writing."); return; }
    file << "Name,MaxCapacity\n";
    for (int i = 0; i < kitchens.getNumKitchens(); ++i)
        file << kitchens.getKitchenName(i) << "," << 5 << "\n";
    file.close();
    UI::success("Kitchen config saved to " + filename);
}
int FileHandler::loadKitchens(KitchenSystem &kitchens, const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) return 0;
    string line; getline(file, line);
    int count = 0;
    while (getline(file, line)) {
        line = trimLine(line);
        if (line.empty()) continue;
        try {
            stringstream ss(line);
            string name, tok;
            getline(ss, name, ',');
            getline(ss, tok);
            int cap = safeStoi(tok, 5);
            kitchens.addKitchen(name, cap);
            ++count;
        } catch (...) {}
    }
    file.close();
    return count;
}
void FileHandler::saveAll(const OrderHeap &heap, const RiderHeap &riders,
                          const CityGraph &graph, const KitchenSystem &kitchens,
                          const SearchEngine &engine) {
    UI::header("Saving All Data to Files");
    saveOrders(heap);
    saveRiders(riders);
    saveGraph(graph);
    saveKitchens(kitchens);
    saveRecords(engine);
    cout << "\n"; UI::success("All data saved successfully!");
}
void FileHandler::loadAll(OrderHeap &heap, RiderHeap &riders, CityGraph &graph,
                          KitchenSystem &kitchens, SearchEngine &engine, OrderTracker &tracker) {
    UI::header("Loading Data from Files");
    int orders = loadOrders(heap, tracker);
    int riderCount = loadRiders(riders);
    bool graphLoaded = loadGraph(graph);
    int kitchenCount = loadKitchens(kitchens);
    int records = loadRecords(engine);
    cout << "  Orders loaded    : " << orders << "\n";
    cout << "  Riders loaded    : " << riderCount << "\n";
    cout << "  Graph loaded     : " << (graphLoaded ? "Yes" : "No") << "\n";
    cout << "  Kitchens loaded  : " << kitchenCount << "\n";
    cout << "  Records loaded   : " << records << "\n";
    if (orders || riderCount || graphLoaded || kitchenCount || records) UI::success("Data loaded from previous session!");
    else UI::info("No previous data found. Starting fresh.");
}

// ==============================================================================
//  Demo Data & AutoSave
// ==============================================================================
int gNextOrderId = 200;

bool filesExist() {
    ifstream f1("orders.csv"), f2("riders.csv"), f3("city_graph.csv");
    bool exists = f1.is_open() || f2.is_open() || f3.is_open();
    f1.close(); f2.close(); f3.close();
    return exists;
}

void seedDemoData(OrderHeap &oq, RiderHeap &rp, CityGraph &cg,
                  KitchenSystem &ks, SearchEngine &se, OrderTracker &tr) {
    cg.addLocation("Central Hub");   cg.addLocation("North Market");
    cg.addLocation("East Plaza");    cg.addLocation("South Block");
    cg.addLocation("West Avenue");   cg.addLocation("University");
    cg.addLocation("Airport Road");  cg.addLocation("Tech Park");
    cg.addRoad(0, 1, 5); cg.addRoad(0, 2, 8); cg.addRoad(0, 3, 6); cg.addRoad(0, 4, 4);
    cg.addRoad(1, 2, 3); cg.addRoad(1, 5, 7); cg.addRoad(2, 3, 4); cg.addRoad(2, 6, 9);
    cg.addRoad(3, 4, 5); cg.addRoad(3, 7, 6); cg.addRoad(4, 5, 8); cg.addRoad(5, 6, 4);
    cg.addRoad(6, 7, 3); cg.addRoad(5, 7, 5);
    ks.addKitchen("Alpha Grill", 5); ks.addKitchen("Beta Pizza", 4);
    ks.addKitchen("Gamma Wok", 3);   ks.addKitchen("Delta Diner", 4);
    rp.addRider(Rider(1, "Kamran", 3, 0)); rp.addRider(Rider(2, "Zubair", 2, 1));
    rp.addRider(Rider(3, "Ahmed", 4, 3));  rp.addRider(Rider(4, "Fatima", 3, 5));
    Order o1(101, "Ali Khan", "Biryani", "VIP", Priority::VIP, 15, 25);
    Order o2(102, "Sara Ahmed", "Pizza", "Regular", Priority::NORMAL, 12, 40);
    Order o3(103, "Hassan Raza", "Burger", "Premium", Priority::PREMIUM, 8, 20);
    Order o4(104, "Ayesha Noor", "Pasta", "Regular", Priority::NORMAL, 18, 45);
    Order o5(105, "Usman Ali", "Shawarma", "VIP", Priority::VIP, 10, 15);
    oq.insert(o1); tr.recordOrderCreated(o1);
    oq.insert(o2); tr.recordOrderCreated(o2);
    oq.insert(o3); tr.recordOrderCreated(o3);
    oq.insert(o4); tr.recordOrderCreated(o4);
    oq.insert(o5); tr.recordOrderCreated(o5);
    se.indexOrder(OrderRecord(o1)); se.indexOrder(OrderRecord(o2));
    se.indexOrder(OrderRecord(o3)); se.indexOrder(OrderRecord(o4)); se.indexOrder(OrderRecord(o5));
    UI::success("Demo data seeded (8 locations, 4 kitchens, 4 riders, 5 orders).");
}

void autoSave(const OrderHeap &oq, const RiderHeap &rp, const CityGraph &cg,
              const KitchenSystem &ks, const SearchEngine &se) {
    ofstream file("orders.csv");
    if (file.is_open()) {
        file << "ID,CustomerName,Item,Category,Priority,PrepTime,Deadline,Status,Delayed\n";
        Array<Order> orders = oq.getAllOrders();
        for (int i = 0; i < orders.size(); ++i) {
            const Order &o = orders[i];
            file << o.getId() << "," << o.getCustomerName() << "," << o.getItem() << ","
                 << o.getCategory() << "," << static_cast<int>(o.getPriority()) << ","
                 << o.getPrepTime() << "," << o.getDeadline() << "," << static_cast<int>(o.getStatus())
                 << "," << (o.isDelayed() ? 1 : 0) << "\n";
        }
        file.close();
    }
    ofstream rfile("riders.csv");
    if (rfile.is_open()) {
        rfile << "ID,Name,Capacity,Location,Available,CurrentLoad,TotalDeliveries\n";
        Array<Rider> all = rp.getAllRiders();
        for (int i = 0; i < all.size(); ++i) {
            const Rider &r = all[i];
            rfile << r.getId() << "," << r.getName() << "," << r.getCapacity() << ","
                  << r.getLocationNode() << "," << (r.isAvailable() ? 1 : 0) << ","
                  << r.getCurrentLoad() << "," << r.getTotalDeliveries() << "\n";
        }
        rfile.close();
    }
    ofstream gfile("city_graph.csv");
    if (gfile.is_open()) {
        int n = cg.getNodeCount();
        gfile << "LOCATIONS," << n << "\n";
        for (int i = 0; i < n; ++i) gfile << i << "," << cg.getLocationName(i) << "\n";
        gfile << "ROADS\n";
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j) {
                int w = cg.getRoadWeight(i, j);
                if (w > 0) gfile << i << "," << j << "," << w << "," << (cg.isBlocked(i, j) ? 1 : 0) << "\n";
            }
        gfile.close();
    }
    ofstream kfile("kitchens.csv");
    if (kfile.is_open()) {
        kfile << "Name,MaxCapacity\n";
        for (int i = 0; i < ks.getNumKitchens(); ++i) kfile << ks.getKitchenName(i) << "," << 5 << "\n";
        kfile.close();
    }
    ofstream rfile2("delivery_records.csv");
    if (rfile2.is_open()) {
        rfile2 << "ID,Customer,Item,Category,Priority,Status,Kitchen,Rider,Distance,ETA\n";
        const Array<OrderRecord> &records = se.getAllRecords();
        for (int i = 0; i < records.size(); ++i) {
            const OrderRecord &r = records[i];
            rfile2 << r.getId() << "," << r.getCustomerName() << "," << r.getItem() << ","
                   << r.getCategory() << "," << static_cast<int>(r.getPriority()) << ","
                   << static_cast<int>(r.getFinalStatus()) << "," << r.getKitchenName() << ","
                   << r.getRiderName() << "," << r.getRouteDistance() << "," << r.getEstimatedETA() << "\n";
        }
        rfile2.close();
    }
}