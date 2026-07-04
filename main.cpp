#include "FoodExpress.h"

extern int gNextOrderId;

// Forward declarations
void adminMenu(OrderHeap &orderQueue, RiderHeap &riderPool, CityGraph &cityGraph,
               KitchenSystem &kitchens, SearchEngine &searchEngine, OrderTracker &tracker);
void riderMenu(RiderHeap &riderPool, SearchEngine &searchEngine);
void customerMenu(OrderHeap &orderQueue, SearchEngine &searchEngine, OrderTracker &tracker,
                  const CityGraph &cityGraph, KitchenSystem &kitchens, RiderHeap &riderPool);
void autoSaveAll(const OrderHeap &orderQueue, const RiderHeap &riderPool,
                 const CityGraph &cityGraph, const KitchenSystem &kitchens,
                 const SearchEngine &searchEngine);

// Simple authentication
bool authenticateAdmin() {
    string pwd = InputHelper::readLine("  Admin Password: ");
    return pwd == "admin123";
}

bool authenticateRider(int riderId, RiderHeap &riderPool) {
    Array<Rider> riders = riderPool.getAllRiders();
    bool found = false;
    for (int i = 0; i < riders.size(); ++i) {
        if (riders[i].getId() == riderId) {
            found = true;
            break;
        }
    }
    if (!found) return false;
    string pwd = InputHelper::readLine("  Rider Password: ");
    return pwd == "rider123";
}

int main() {
    OrderHeap orderQueue;
    RiderHeap riderPool;
    CityGraph cityGraph;
    KitchenSystem kitchens;
    SearchEngine searchEngine;
    OrderTracker tracker;

    if (filesExist()) {
        FileHandler::loadAll(orderQueue, riderPool, cityGraph, kitchens, searchEngine, tracker);
        Array<Order> loaded = orderQueue.getAllOrders();
        for (int i = 0; i < loaded.size(); ++i)
            if (loaded[i].getId() >= gNextOrderId) gNextOrderId = loaded[i].getId() + 1;
        const Array<OrderRecord> &recs = searchEngine.getAllRecords();
        for (int i = 0; i < recs.size(); ++i)
            if (recs[i].getId() >= gNextOrderId) gNextOrderId = recs[i].getId() + 1;
    } else {
        seedDemoData(orderQueue, riderPool, cityGraph, kitchens, searchEngine, tracker);
        autoSaveAll(orderQueue, riderPool, cityGraph, kitchens, searchEngine);
    }

    bool running = true;
    while (running) {
        cout << "\n\n";
        UI::header("FOODEXPRESS DISPATCH SYSTEM");
        cout << "  Welcome! Please select your role:\n\n";
        cout << "    1. Administrator\n";
        cout << "    2. Rider\n";
        cout << "    3. Customer\n";
        cout << "    0. Exit\n\n";
        int role = InputHelper::readInt("  Your choice: ", 0, 3);

        switch (role) {
            case 1:
                if (authenticateAdmin())
                    adminMenu(orderQueue, riderPool, cityGraph, kitchens, searchEngine, tracker);
                else
                    UI::error("Authentication failed.");
                break;
            case 2: {
                int rid = InputHelper::readInt("  Enter your Rider ID: ", 1, 999);
                if (authenticateRider(rid, riderPool))
                    riderMenu(riderPool, searchEngine);
                else
                    UI::error("Authentication failed.");
                break;
            }
            case 3:
                customerMenu(orderQueue, searchEngine, tracker, cityGraph, kitchens, riderPool);
                break;
            case 0:
                running = false;
                FileHandler::saveAll(orderQueue, riderPool, cityGraph, kitchens, searchEngine);
                cout << "\n  Thank you for using FoodExpress! Goodbye.\n\n";
                break;
            default:
                UI::error("Invalid choice.");
        }
    }
    return 0;
}

// ============================================================================
// ADMIN MENU – Cleaned up, only essential functions
// ============================================================================
void adminMenu(OrderHeap &orderQueue, RiderHeap &riderPool, CityGraph &cityGraph,
               KitchenSystem &kitchens, SearchEngine &searchEngine, OrderTracker &tracker) {
    int choice;
    do {
        cout << "\n";
        UI::header("ADMINISTRATOR DASHBOARD");
        cout << "  [1]  View Order Queue\n";
        cout << "  [2]  Cancel Order\n";
        cout << "  [3]  View Kitchens\n";
        cout << "  [4]  Register New Rider\n";
        cout << "  [5]  View All Riders\n";
        cout << "  [6]  Toggle Rider Availability\n";
        cout << "  [7]  View City Map\n";
        cout << "  [8]  Find Shortest Route\n";
        cout << "  [9]  Search Orders\n";
        cout << "  [10] View All Delivery Records\n";
        cout << "  [0]  Logout\n";
        choice = InputHelper::readInt("  Enter choice: ", 0, 10);

        switch (choice) {
        case 1:
            UI::header("Order Queue");
            orderQueue.display();
            break;
        case 2: {
            UI::header("Cancel Order");
            int id = InputHelper::readInt("  Order ID: ", 1, 99999);
            Order *found = orderQueue.findOrder(id);
            if (found) {
                tracker.recordOrderCancelled(*found);
                orderQueue.cancelOrder(id);
                UI::success("Cancelled.");
                autoSaveAll(orderQueue, riderPool, cityGraph, kitchens, searchEngine);
            } else UI::error("Not found.");
            break;
        }
        case 3:
            UI::header("Kitchens");
            kitchens.displayAll();
            break;
        case 4: {
            UI::header("Register Rider");
            int id = InputHelper::readInt("  ID: ", 1, 999);
            string name = InputHelper::readLine("  Name: ");
            int cap = InputHelper::readInt("  Capacity: ", 1, 10);
            cityGraph.displayMap();
            int loc = InputHelper::readInt("  Start node: ", 0, cityGraph.getNodeCount() - 1);
            riderPool.addRider(Rider(id, name, cap, loc));
            UI::success("Rider added.");
            autoSaveAll(orderQueue, riderPool, cityGraph, kitchens, searchEngine);
            break;
        }
        case 5:
            riderPool.display();
            break;
        case 6: {
            riderPool.display();
            int rid = InputHelper::readInt("  Rider ID: ", 1, 999);
            if (riderPool.toggleAvailability(rid)) {
                UI::success("Toggled.");
                autoSaveAll(orderQueue, riderPool, cityGraph, kitchens, searchEngine);
            } else UI::error("Not found.");
            break;
        }
        case 7:
            cityGraph.displayMap();
            break;
        case 8: {
            cityGraph.displayMap();
            int s = InputHelper::readInt("  Source: ", 0, cityGraph.getNodeCount() - 1);
            int d = InputHelper::readInt("  Dest: ", 0, cityGraph.getNodeCount() - 1);
            Array<int> path;
            int dist = cityGraph.findShortestPath(s, d, path);
            if (dist >= 0) {
                cout << "  Distance: " << dist << "\n  Path: ";
                for (int i = 0; i < path.size(); ++i) {
                    if (i) cout << " -> ";
                    cout << cityGraph.getLocationName(path[i]);
                }
                cout << "\n";
            } else UI::error("No route.");
            break;
        }
        case 9: {
            UI::header("Search Orders");
            cout << "  1. By ID\n  2. By Customer Name\n  3. By Item\n  4. By Status\n";
            int f = InputHelper::readInt("  Choice: ", 1, 4);
            switch (f) {
                case 1: {
                    int id = InputHelper::readInt("  ID: ", 1, 99999);
                    OrderRecord *rec = searchEngine.findById(id);
                    if (rec) { SearchEngine::displayResultsHeader(); rec->display(); }
                    else UI::error("Not found.");
                    break;
                }
                case 2: {
                    string n = InputHelper::readLine("  Name: ");
                    SearchEngine::displayResults(searchEngine.searchByCustomer(n));
                    break;
                }
                case 3: {
                    string i = InputHelper::readLine("  Item: ");
                    SearchEngine::displayResults(searchEngine.searchByItem(i));
                    break;
                }
                case 4: {
                    int s = InputHelper::readInt("  0.Queued 1.Preparing 2.Ready 3.Dispatched 4.Delivered 5.Cancelled: ", 0, 5);
                    SearchEngine::displayResults(searchEngine.searchByStatus(static_cast<OrderStatus>(s)));
                    break;
                }
            }
            break;
        }
        case 10:
            UI::header("All Delivery Records");
            searchEngine.displayAll();
            break;
        case 0:
            break;
        default:
            UI::error("Invalid choice.");
        }
        if (choice != 0) InputHelper::pause();
    } while (choice != 0);
}

// ============================================================================
// RIDER MENU – Unchanged
// ============================================================================
void riderMenu(RiderHeap &riderPool, SearchEngine &searchEngine) {
    int rid = InputHelper::readInt("  Enter your Rider ID again: ", 1, 999);
    Array<Rider> riders = riderPool.getAllRiders();
    string riderName;
    bool found = false;
    for (int i = 0; i < riders.size(); ++i) {
        if (riders[i].getId() == rid) {
            found = true;
            riderName = riders[i].getName();
            break;
        }
    }
    if (!found) {
        UI::error("Rider ID not found.");
        return;
    }

    int choice;
    do {
        cout << "\n";
        UI::header("RIDER MENU (ID: " + to_string(rid) + ")");
        cout << "  1. View my deliveries\n";
        cout << "  2. Complete a delivery\n";
        cout << "  3. Toggle my availability\n";
        cout << "  4. View my statistics\n";
        cout << "  0. Logout\n";
        choice = InputHelper::readInt("  Choice: ", 0, 4);

        switch (choice) {
        case 1: {
            UI::subHeader("Your Deliveries");
            Array<OrderRecord> records = searchEngine.searchByRider(riderName);
            if (records.empty()) UI::info("No deliveries found.");
            else SearchEngine::displayResults(records);
            break;
        }
        case 2: {
            if (riderPool.completeDelivery(rid))
                UI::success("Delivery completed. Load decreased.");
            else UI::error("Could not complete delivery.");
            break;
        }
        case 3: {
            if (riderPool.toggleAvailability(rid))
                UI::success("Availability toggled.");
            else UI::error("Failed.");
            break;
        }
        case 4: {
            UI::subHeader("Your Statistics");
            for (int i = 0; i < riders.size(); ++i) {
                if (riders[i].getId() == rid) {
                    cout << "  Name: " << riders[i].getName() << "\n";
                    cout << "  Load: " << riders[i].getCurrentLoad() << "/" << riders[i].getCapacity() << "\n";
                    cout << "  Total Deliveries: " << riders[i].getTotalDeliveries() << "\n";
                    cout << "  Available: " << (riders[i].isAvailable() ? "Yes" : "No") << "\n";
                    cout << "  Location: Node " << riders[i].getLocationNode() << "\n";
                    break;
                }
            }
            break;
        }
        }
        if (choice != 0) InputHelper::pause();
    } while (choice != 0);
}

// ============================================================================
// CUSTOMER MENU – Auto-assign kitchen and rider immediately after order
// ============================================================================
void customerMenu(OrderHeap &orderQueue, SearchEngine &searchEngine, OrderTracker &tracker,
                  const CityGraph &cityGraph, KitchenSystem &kitchens, RiderHeap &riderPool) {
    int choice;
    do {
        cout << "\n";
        UI::header("CUSTOMER PORTAL");
        cout << "  1. Place a new order (auto-assigned to kitchen & rider)\n";
        cout << "  2. Check order status\n";
        cout << "  3. Cancel my order (only if still queued)\n";
        cout << "  0. Logout\n";
        choice = InputHelper::readInt("  Choice: ", 0, 3);

        switch (choice) {
        case 1: {
            UI::subHeader("Place Your Order");
            int id = gNextOrderId++;
            string cust = InputHelper::readLine("  Your Name: ");
            string item = InputHelper::readLine("  Food Item: ");
            cout << "  Customer Category:\n    1. Regular\n    2. Premium\n    3. VIP\n";
            int catChoice = InputHelper::readInt("  Select (1-3): ", 1, 3);
            string category = (catChoice == 3) ? "VIP" : (catChoice == 2) ? "Premium" : "Regular";
            cout << "  Priority:\n    1. Normal\n    2. Premium\n    3. VIP\n";
            int priChoice = InputHelper::readInt("  Select (1-3): ", 1, 3);
            Priority pri = static_cast<Priority>(priChoice);
            int deadline = InputHelper::readInt("  Expected delivery deadline (minutes): ", 1, 180);
            int prepTime = 15; // fixed

            // Create order
            Order o(id, cust, item, category, pri, prepTime, deadline);
            orderQueue.insert(o);
            tracker.recordOrderCreated(o);

            // AUTO-ASSIGN: kitchen first
            int kitchenIdx = kitchens.distributeOrder(o.getId(), o.getItem(), o.getPrepTime());
            string kitchenName = (kitchenIdx >= 0) ? kitchens.getKitchenName(kitchenIdx) : "None";
            if (kitchenIdx >= 0) {
                o.setAssignedKitchen(kitchenIdx);
                o.setStatus(OrderStatus::PREPARING);
                tracker.recordStateChange(o.getId(), OrderStatus::QUEUED, OrderStatus::PREPARING,
                                          "Assigned to " + kitchenName);
            }

            // AUTO-ASSIGN: rider (ask user for destination node)
            cityGraph.displayMap();
            int destNode = InputHelper::readInt("  Delivery destination node: ", 0, cityGraph.getNodeCount() - 1);
            Rider rider = riderPool.assignBestRiderWithRoute(cityGraph, destNode);
            string riderName = "None";
            int routeDist = -1, eta = -1;
            if (rider.getId() != 0) {
                riderName = rider.getName();
                o.setAssignedRider(rider.getId());
                o.setStatus(OrderStatus::DISPATCHED);
                tracker.recordStateChange(o.getId(), OrderStatus::PREPARING, OrderStatus::DISPATCHED,
                                          "Dispatched to " + riderName);
                // Calculate route
                Array<int> path;
                routeDist = cityGraph.findShortestPath(rider.getLocationNode(), destNode, path);
                int kitchenWait = (kitchenIdx >= 0) ? kitchens.getKitchenWaitTime(kitchenIdx) : 0;
                eta = (routeDist >= 0) ? routeDist + kitchenWait : -1;
            }

            // Record final order
            OrderRecord rec(o, kitchenName, riderName, routeDist, eta);
            searchEngine.indexOrder(rec);
            tracker.recordOrderProcessed(o);

            UI::success("\n  Order #" + to_string(id) + " placed successfully!");
            cout << "  Kitchen: " << kitchenName << "\n";
            cout << "  Rider: " << riderName << "\n";
            if (routeDist >= 0) cout << "  Route distance: " << routeDist << " units\n";
            if (eta >= 0) cout << "  Estimated ETA: " << eta << " minutes\n";
            cout << "  Your order will be delivered shortly.\n";

            autoSaveAll(orderQueue, riderPool, cityGraph, kitchens, searchEngine);
            break;
        }
        case 2: {
            int oid = InputHelper::readInt("  Enter Order ID: ", 1, 99999);
            OrderRecord *rec = searchEngine.findById(oid);
            if (rec) {
                SearchEngine::displayResultsHeader();
                rec->display();
            } else {
                Order *o = orderQueue.findOrder(oid);
                if (o) {
                    cout << "  Order #" << o->getId() << " is still in the queue.\n";
                    cout << "  Current status: " << statusToString(o->getStatus()) << "\n";
                } else {
                    UI::error("Order not found.");
                }
            }
            break;
        }
        case 3: {
            int oid = InputHelper::readInt("  Enter Order ID to cancel: ", 1, 99999);
            Order *o = orderQueue.findOrder(oid);
            if (o && o->getStatus() == OrderStatus::QUEUED) {
                tracker.recordOrderCancelled(*o);
                orderQueue.cancelOrder(oid);
                UI::success("Order #" + to_string(oid) + " cancelled.");
                autoSaveAll(orderQueue, riderPool, cityGraph, kitchens, searchEngine);
            } else {
                UI::error("Cannot cancel. Order not found or already being processed.");
            }
            break;
        }
        }
        if (choice != 0) InputHelper::pause();
    } while (choice != 0);
}

// ============================================================================
// Auto-save helper (same as before)
// ============================================================================
void autoSaveAll(const OrderHeap &orderQueue, const RiderHeap &riderPool,
                 const CityGraph &cityGraph, const KitchenSystem &kitchens,
                 const SearchEngine &searchEngine) {
    // Orders
    ofstream file("orders.csv");
    if (file.is_open()) {
        file << "ID,CustomerName,Item,Category,Priority,PrepTime,Deadline,Status,Delayed\n";
        Array<Order> orders = orderQueue.getAllOrders();
        for (int i = 0; i < orders.size(); ++i) {
            const Order &o = orders[i];
            file << o.getId() << "," << o.getCustomerName() << "," << o.getItem() << ","
                 << o.getCategory() << "," << static_cast<int>(o.getPriority()) << ","
                 << o.getPrepTime() << "," << o.getDeadline() << ","
                 << static_cast<int>(o.getStatus()) << "," << (o.isDelayed() ? 1 : 0) << "\n";
        }
        file.close();
    }
    // Riders
    ofstream rfile("riders.csv");
    if (rfile.is_open()) {
        rfile << "ID,Name,Capacity,Location,Available,CurrentLoad,TotalDeliveries\n";
        Array<Rider> riders = riderPool.getAllRiders();
        for (int i = 0; i < riders.size(); ++i) {
            const Rider &r = riders[i];
            rfile << r.getId() << "," << r.getName() << "," << r.getCapacity() << ","
                  << r.getLocationNode() << "," << (r.isAvailable() ? 1 : 0) << ","
                  << r.getCurrentLoad() << "," << r.getTotalDeliveries() << "\n";
        }
        rfile.close();
    }
    // Graph
    ofstream gfile("city_graph.csv");
    if (gfile.is_open()) {
        int n = cityGraph.getNodeCount();
        gfile << "LOCATIONS," << n << "\n";
        for (int i = 0; i < n; ++i)
            gfile << i << "," << cityGraph.getLocationName(i) << "\n";
        gfile << "ROADS\n";
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j) {
                int w = cityGraph.getRoadWeight(i, j);
                if (w > 0)
                    gfile << i << "," << j << "," << w << ","
                          << (cityGraph.isBlocked(i, j) ? 1 : 0) << "\n";
            }
        gfile.close();
    }
    // Kitchens
    ofstream kfile("kitchens.csv");
    if (kfile.is_open()) {
        kfile << "Name,MaxCapacity\n";
        for (int i = 0; i < kitchens.getNumKitchens(); ++i)
            kfile << kitchens.getKitchenName(i) << "," << 5 << "\n";
        kfile.close();
    }
    // Delivery records
    ofstream dfile("delivery_records.csv");
    if (dfile.is_open()) {
        dfile << "ID,Customer,Item,Category,Priority,Status,Kitchen,Rider,Distance,ETA\n";
        const Array<OrderRecord> &records = searchEngine.getAllRecords();
        for (int i = 0; i < records.size(); ++i) {
            const OrderRecord &r = records[i];
            dfile << r.getId() << "," << r.getCustomerName() << "," << r.getItem() << ","
                  << r.getCategory() << "," << static_cast<int>(r.getPriority()) << ","
                  << static_cast<int>(r.getFinalStatus()) << "," << r.getKitchenName() << ","
                  << r.getRiderName() << "," << r.getRouteDistance() << ","
                  << r.getEstimatedETA() << "\n";
        }
        dfile.close();
    }
}